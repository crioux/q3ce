/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//

/*****************************************************************************
 * name:		ai_main.c
 *
 * desc:		Quake3 bot AI
 *
 * $Archive: /MissionPack/code/game/ai_main.c $
 *
 *****************************************************************************/
#include"game_pch.h"



//bot states
bot_state_t	*botstates[MAX_CLIENTS];
//number of bots
int numbots;
//floating point time
gfixed floattime;
//time to do a regular update
gfixed regularupdate_time;
//
int bot_interbreed;
int bot_interbreedmatchcount;
//
vmCvar_t bot_thinktime;
vmCvar_t bot_memorydump;
vmCvar_t bot_saveroutingcache;
vmCvar_t bot_pause;
vmCvar_t bot_report;
vmCvar_t bot_testsolid;
vmCvar_t bot_testclusters;
vmCvar_t bot_developer;
vmCvar_t bot_interbreedchar;
vmCvar_t bot_interbreedbots;
vmCvar_t bot_interbreedcycle;
vmCvar_t bot_interbreedwrite;


void ExitLevel( void );


/*
==================
BotAI_Print
==================
*/
void QDECL BotAI_Print(int type, const char *fmt, ...) {
	char str[2048];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	va_end(ap);

	switch(type) {
		case PRT_MESSAGE: {
			G_Printf("%s", str);
			break;
		}
		case PRT_WARNING: {
			G_Printf( S_COLOR_YELLOW "Warning: %s", str );
			break;
		}
		case PRT_ERROR: {
			G_Printf( S_COLOR_RED "Error: %s", str );
			break;
		}
		case PRT_FATAL: {
			G_Printf( S_COLOR_RED "Fatal: %s", str );
			break;
		}
		case PRT_EXIT: {
			G_Error( S_COLOR_RED "Exit: %s", str );
			break;
		}
		default: {
			G_Printf( "unknown print type\n" );
			break;
		}
	}
}


/*
==================
BotAI_Trace
==================
*/
void BotAI_Trace(bsp_trace_t *bsptrace, bvec3_t start, bvec3_t mins, bvec3_t maxs, bvec3_t end, int passent, int contentmask) {
	trace_t trace;

	_G_trap_Trace(&trace, start, mins, maxs, end, passent, contentmask);
	//copy the trace information
	bsptrace->allsolid = trace.allsolid;
	bsptrace->startsolid = trace.startsolid;
	bsptrace->fraction = trace.fraction;
	VectorCopy(trace.endpos, bsptrace->endpos);
	bsptrace->plane.dist = trace.plane.dist;
	VectorCopy(trace.plane.normal, bsptrace->plane.normal);
	bsptrace->plane.signbits = trace.plane.signbits;
	bsptrace->plane.type = trace.plane.type;
	bsptrace->surface.value = trace.surfaceFlags;
	bsptrace->ent = trace.entityNum;
	bsptrace->exp_dist = BFIXED_0;
	bsptrace->sidenum = 0;
	bsptrace->contents = 0;
}

/*
==================
BotAI_GetClientState
==================
*/
int BotAI_GetClientState( int clientNum, playerState_t *state ) {
	gentity_t	*ent;

	ent = &g_entities[clientNum];
	if ( !ent->inuse ) {
		return qfalse;
	}
	if ( !ent->client ) {
		return qfalse;
	}

	memcpy( state, &ent->client->ps, sizeof(playerState_t) );
	return qtrue;
}

/*
==================
BotAI_GetEntityState
==================
*/
int BotAI_GetEntityState( int entityNum, entityState_t *state ) {
	gentity_t	*ent;

	ent = &g_entities[entityNum];
	memset( state, 0, sizeof(entityState_t) );
	if (!ent->inuse) return qfalse;
	if (!ent->r.linked) return qfalse;
	if (ent->r.svFlags & SVF_NOCLIENT) return qfalse;
	memcpy( state, &ent->s, sizeof(entityState_t) );
	return qtrue;
}

/*
==================
BotAI_GetSnapshotEntity
==================
*/
int BotAI_GetSnapshotEntity( int clientNum, int sequence, entityState_t *state ) {
	int		entNum;

	entNum = _G_trap_BotGetSnapshotEntity( clientNum, sequence );
	if ( entNum == -1 ) {
		memset(state, 0, sizeof(entityState_t));
		return -1;
	}

	BotAI_GetEntityState( entNum, state );

	return sequence + 1;
}

/*
==================
BotAI_BotInitialChat
==================
*/
void QDECL BotAI_BotInitialChat( bot_state_t *bs, const char *type, ... ) {
	int		i, mcontext;
	va_list	ap;
	char	*p;
	const char	*vars[MAX_MATCHVARIABLES];

	memset(vars, 0, sizeof(vars));
	va_start(ap, type);
	p = va_arg(ap, char *);
	for (i = 0; i < MAX_MATCHVARIABLES; i++) {
		if( !p ) {
			break;
		}
		vars[i] = p;
		p = va_arg(ap, char *);
	}
	va_end(ap);

	mcontext = BotSynonymContext(bs);

	_G_trap_BotInitialChat( bs->cs, type, mcontext, vars[0], vars[1], vars[2], vars[3], vars[4], vars[5], vars[6], vars[7] );
}


/*
==================
BotTestAAS
==================
*/
void BotTestAAS(bvec3_t origin) {
	int areanum;
	aas_areainfo_t info;

	_G_trap_Cvar_Update(&bot_testsolid);
	_G_trap_Cvar_Update(&bot_testclusters);
	if (bot_testsolid.integer) {
		if (!_G_trap_AAS_Initialized()) return;
		areanum = BotPointAreaNum(origin);
		if (areanum) BotAI_Print(PRT_MESSAGE, "\remtpy area");
		else BotAI_Print(PRT_MESSAGE, "\r^1SOLID area");
	}
	else if (bot_testclusters.integer) {
		if (!_G_trap_AAS_Initialized()) return;
		areanum = BotPointAreaNum(origin);
		if (!areanum)
			BotAI_Print(PRT_MESSAGE, "\r^1Solid!                              ");
		else {
			_G_trap_AAS_AreaInfo(areanum, &info);
			BotAI_Print(PRT_MESSAGE, "\rarea %d, cluster %d       ", areanum, info.cluster);
		}
	}
}

/*
==================
BotReportStatus
==================
*/
void BotReportStatus(bot_state_t *bs) {
	char goalname[MAX_MESSAGE_SIZE];
	char netname[MAX_MESSAGE_SIZE];
	const char *leader;
	char flagstatus[32];
	//
	ClientName(bs->client, netname, sizeof(netname));
	if (Q_stricmp(netname, bs->teamleader) == 0) leader = "L";
	else leader = " ";

	strcpy(flagstatus, "  ");
	if (gametype == GT_CTF) {
		if (BotCTFCarryingFlag(bs)) {
			if (BotTeam(bs) == TEAM_RED) strcpy(flagstatus, S_COLOR_RED"F ");
			else strcpy(flagstatus, S_COLOR_BLUE"F ");
		}
	}
#ifdef MISSIONPACK
	else if (gametype == GT_1FCTF) {
		if (Bot1FCTFCarryingFlag(bs)) {
			if (BotTeam(bs) == TEAM_RED) strcpy(flagstatus, S_COLOR_RED"F ");
			else strcpy(flagstatus, S_COLOR_BLUE"F ");
		}
	}
	else if (gametype == GT_HARVESTER) {
		if (BotHarvesterCarryingCubes(bs)) {
			if (BotTeam(bs) == TEAM_RED) Com_sprintf(flagstatus, sizeof(flagstatus), S_COLOR_RED"%2d", bs->inventory[INVENTORY_REDCUBE]);
			else Com_sprintf(flagstatus, sizeof(flagstatus), S_COLOR_BLUE"%2d", bs->inventory[INVENTORY_BLUECUBE]);
		}
	}
#endif

	switch(bs->ltgtype) {
		case LTG_TEAMHELP:
		{
			EasyClientName(bs->teammate, goalname, sizeof(goalname));
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: helping %s\n", netname, leader, flagstatus, goalname);
			break;
		}
		case LTG_TEAMACCOMPANY:
		{
			EasyClientName(bs->teammate, goalname, sizeof(goalname));
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: accompanying %s\n", netname, leader, flagstatus, goalname);
			break;
		}
		case LTG_DEFENDKEYAREA:
		{
			_G_trap_BotGoalName(bs->teamgoal.number, goalname, sizeof(goalname));
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: defending %s\n", netname, leader, flagstatus, goalname);
			break;
		}
		case LTG_GETITEM:
		{
			_G_trap_BotGoalName(bs->teamgoal.number, goalname, sizeof(goalname));
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: getting item %s\n", netname, leader, flagstatus, goalname);
			break;
		}
		case LTG_KILL:
		{
			ClientName(bs->teamgoal.entitynum, goalname, sizeof(goalname));
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: killing %s\n", netname, leader, flagstatus, goalname);
			break;
		}
		case LTG_CAMP:
		case LTG_CAMPORDER:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: camping\n", netname, leader, flagstatus);
			break;
		}
		case LTG_PATROL:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: patrolling\n", netname, leader, flagstatus);
			break;
		}
		case LTG_GETFLAG:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: capturing flag\n", netname, leader, flagstatus);
			break;
		}
		case LTG_RUSHBASE:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: rushing base\n", netname, leader, flagstatus);
			break;
		}
		case LTG_RETURNFLAG:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: returning flag\n", netname, leader, flagstatus);
			break;
		}
		case LTG_ATTACKENEMYBASE:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: attacking the enemy base\n", netname, leader, flagstatus);
			break;
		}
		case LTG_HARVEST:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: harvesting\n", netname, leader, flagstatus);
			break;
		}
		default:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: roaming\n", netname, leader, flagstatus);
			break;
		}
	}
}

/*
==================
BotTeamplayReport
==================
*/
void BotTeamplayReport(void) {
	int i;
	char buf[MAX_INFO_STRING];

	BotAI_Print(PRT_MESSAGE, S_COLOR_RED"RED\n");
	for (i = 0; i < maxclients && i < MAX_CLIENTS; i++) {
		//
		if ( !botstates[i] || !botstates[i]->inuse ) continue;
		//
		_G_trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) continue;
		//skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_RED) {
			BotReportStatus(botstates[i]);
		}
	}
	BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE"BLUE\n");
	for (i = 0; i < maxclients && i < MAX_CLIENTS; i++) {
		//
		if ( !botstates[i] || !botstates[i]->inuse ) continue;
		//
		_G_trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) continue;
		//skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_BLUE) {
			BotReportStatus(botstates[i]);
		}
	}
}

/*
==================
BotSetInfoConfigString
==================
*/
void BotSetInfoConfigString(bot_state_t *bs) {
	char goalname[MAX_MESSAGE_SIZE];
	char netname[MAX_MESSAGE_SIZE];
	char action[MAX_MESSAGE_SIZE];
	const char *leader; 
	char carrying[32];
	const char *cs;
	bot_goal_t goal;
	//
	ClientName(bs->client, netname, sizeof(netname));
	if (Q_stricmp(netname, bs->teamleader) == 0) leader = "L";
	else leader = " ";

	strcpy(carrying, "  ");
	if (gametype == GT_CTF) {
		if (BotCTFCarryingFlag(bs)) {
			strcpy(carrying, "F ");
		}
	}
#ifdef MISSIONPACK
	else if (gametype == GT_1FCTF) {
		if (Bot1FCTFCarryingFlag(bs)) {
			strcpy(carrying, "F ");
		}
	}
	else if (gametype == GT_HARVESTER) {
		if (BotHarvesterCarryingCubes(bs)) {
			if (BotTeam(bs) == TEAM_RED) Com_sprintf(carrying, sizeof(carrying), "%2d", bs->inventory[INVENTORY_REDCUBE]);
			else Com_sprintf(carrying, sizeof(carrying), "%2d", bs->inventory[INVENTORY_BLUECUBE]);
		}
	}
#endif

	switch(bs->ltgtype) {
		case LTG_TEAMHELP:
		{
			EasyClientName(bs->teammate, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "helping %s", goalname);
			break;
		}
		case LTG_TEAMACCOMPANY:
		{
			EasyClientName(bs->teammate, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "accompanying %s", goalname);
			break;
		}
		case LTG_DEFENDKEYAREA:
		{
			_G_trap_BotGoalName(bs->teamgoal.number, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "defending %s", goalname);
			break;
		}
		case LTG_GETITEM:
		{
			_G_trap_BotGoalName(bs->teamgoal.number, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "getting item %s", goalname);
			break;
		}
		case LTG_KILL:
		{
			ClientName(bs->teamgoal.entitynum, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "killing %s", goalname);
			break;
		}
		case LTG_CAMP:
		case LTG_CAMPORDER:
		{
			Com_sprintf(action, sizeof(action), "camping");
			break;
		}
		case LTG_PATROL:
		{
			Com_sprintf(action, sizeof(action), "patrolling");
			break;
		}
		case LTG_GETFLAG:
		{
			Com_sprintf(action, sizeof(action), "capturing flag");
			break;
		}
		case LTG_RUSHBASE:
		{
			Com_sprintf(action, sizeof(action), "rushing base");
			break;
		}
		case LTG_RETURNFLAG:
		{
			Com_sprintf(action, sizeof(action), "returning flag");
			break;
		}
		case LTG_ATTACKENEMYBASE:
		{
			Com_sprintf(action, sizeof(action), "attacking the enemy base");
			break;
		}
		case LTG_HARVEST:
		{
			Com_sprintf(action, sizeof(action), "harvesting");
			break;
		}
		default:
		{
			_G_trap_BotGetTopGoal(bs->gs, &goal);
			_G_trap_BotGoalName(goal.number, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "roaming %s", goalname);
			break;
		}
	}
  	cs = va("l\\%s\\c\\%s\\a\\%s",
				leader,
				carrying,
				action);
  	_G_trap_SetConfigstring (CS_BOTINFO + bs->client, cs);
}

/*
==============
BotUpdateInfoConfigStrings
==============
*/
void BotUpdateInfoConfigStrings(void) {
	int i;
	char buf[MAX_INFO_STRING];

	for (i = 0; i < maxclients && i < MAX_CLIENTS; i++) {
		//
		if ( !botstates[i] || !botstates[i]->inuse )
			continue;
		//
		_G_trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n")))
			continue;
		BotSetInfoConfigString(botstates[i]);
	}
}

/*
==============
BotInterbreedBots
==============
*/
void BotInterbreedBots(void) {
	gfixed ranks[MAX_CLIENTS];
	int parent1, parent2, child;
	int i;

	// get rankings for all the bots
	for (i = 0; i < MAX_CLIENTS; i++) {
		if ( botstates[i] && botstates[i]->inuse ) {
			ranks[i] = MAKE_GFIXED(botstates[i]->num_kills * 2 - botstates[i]->num_deaths);
		}
		else {
			ranks[i] = -GFIXED_1;
		}
	}

	if (_G_trap_GeneticParentsAndChildSelection(MAX_CLIENTS, ranks, &parent1, &parent2, &child)) {
		_G_trap_BotInterbreedGoalFuzzyLogic(botstates[parent1]->gs, botstates[parent2]->gs, botstates[child]->gs);
		_G_trap_BotMutateGoalFuzzyLogic(botstates[child]->gs, GFIXED_1);
	}
	// reset the kills and deaths
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (botstates[i] && botstates[i]->inuse) {
			botstates[i]->num_kills = 0;
			botstates[i]->num_deaths = 0;
		}
	}
}

/*
==============
BotWriteInterbreeded
==============
*/
void BotWriteInterbreeded(char *filename) {
	gfixed rank, bestrank;
	int i, bestbot;

	bestrank = GFIXED_0;
	bestbot = -1;
	// get the best bot
	for (i = 0; i < MAX_CLIENTS; i++) {
		if ( botstates[i] && botstates[i]->inuse ) {
			rank = MAKE_GFIXED(botstates[i]->num_kills * 2 - botstates[i]->num_deaths);
		}
		else {
			rank = -GFIXED_1;
		}
		if (rank > bestrank) {
			bestrank = rank;
			bestbot = i;
		}
	}
	if (bestbot >= 0) {
		//write out the new goal fuzzy logic
		_G_trap_BotSaveGoalFuzzyLogic(botstates[bestbot]->gs, filename);
	}
}

/*
==============
BotInterbreedEndMatch

add link back into ExitLevel?
==============
*/
void BotInterbreedEndMatch(void) {

	if (!bot_interbreed) return;
	bot_interbreedmatchcount++;
	if (bot_interbreedmatchcount >= bot_interbreedcycle.integer) {
		bot_interbreedmatchcount = 0;
		//
		_G_trap_Cvar_Update(&bot_interbreedwrite);
		if (strlen(bot_interbreedwrite.string)) {
			BotWriteInterbreeded(bot_interbreedwrite.string);
			_G_trap_Cvar_Set("bot_interbreedwrite", "");
		}
		BotInterbreedBots();
	}
}

/*
==============
BotInterbreeding
==============
*/
void BotInterbreeding(void) {
	int i;

	_G_trap_Cvar_Update(&bot_interbreedchar);
	if (!strlen(bot_interbreedchar.string)) return;
	//make sure we are in tournament mode
	if (gametype != GT_TOURNAMENT) {
		_G_trap_Cvar_Set("g_gametype", va("%d", GT_TOURNAMENT));
		ExitLevel();
		return;
	}
	//shutdown all the bots
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (botstates[i] && botstates[i]->inuse) {
			BotAIShutdownClient(botstates[i]->client, qfalse);
		}
	}
	//make sure all item weight configs are reloaded and Not shared
	_G_trap_BotLibVarSet("bot_reloadcharacters", "1");
	//add a number of bots using the desired bot character
	for (i = 0; i < bot_interbreedbots.integer; i++) {
		_G_trap_SendConsoleCommand( EXEC_INSERT, va("addbot %s 4 free %i %s%d\n",
						bot_interbreedchar.string, i * 50, bot_interbreedchar.string, i) );
	}
	//
	_G_trap_Cvar_Set("bot_interbreedchar", "");
	bot_interbreed = qtrue;
}

/*
==============
BotEntityInfo
==============
*/
void BotEntityInfo(int entnum, aas_entityinfo_t *info) {
	_G_trap_AAS_EntityInfo(entnum, info);
}

/*
==============
NumBots
==============
*/
int NumBots(void) {
	return numbots;
}

/*
==============
BotTeamLeader
==============
*/
int BotTeamLeader(bot_state_t *bs) {
	int leader;

	leader = ClientFromName(bs->teamleader);
	if (leader < 0) return qfalse;
	if (!botstates[leader] || !botstates[leader]->inuse) return qfalse;
	return qtrue;
}

/*
==============
AngleDifference
==============
*/
afixed AngleDifference(afixed ang1, afixed ang2) {
	afixed diff;

	diff = ang1 - ang2;
	if (ang1 > ang2) {
		if (diff > AFIXED(180,0)) diff -= AFIXED(360,0);
	}
	else {
		if (diff < -AFIXED(180,0)) diff += AFIXED(360,0);
	}
	return diff;
}



/*
==============
BotChangeViewAngle
==============
*/
afixed BotChangeViewAngle(afixed angle, afixed ideal_angle, afixed speed) {
	afixed move;

	angle = AngleMod(angle);
	ideal_angle = AngleMod(ideal_angle);
	if (angle == ideal_angle) return angle;
	move = ideal_angle - angle;
	if (ideal_angle > angle) {
		if (move > AFIXED(180,0)) move -= AFIXED(360,0);
	}
	else {
		if (move < -AFIXED(180,0)) move += AFIXED(360,0);
	}
	if (move > AFIXED_0) {
		if (move > speed) move = speed;
	}
	else {
		if (move < -speed) move = -speed;
	}
	return AngleMod(angle + move);
}

/*
==============
BotChangeViewAngles
==============
*/
void BotChangeViewAngles(bot_state_t *bs, gfixed thinktime) {
	afixed diff, factor, maxchange, anglespeed, disired_speed;
	int i;

	if (bs->ideal_viewangles[PITCH] > AFIXED(180,0)) bs->ideal_viewangles[PITCH] -= AFIXED(360,0);
	//
	if (bs->enemy >= 0) {
		factor = MAKE_AFIXED(_G_trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_VIEW_FACTOR, GFIXED(0,01), GFIXED_1));
		maxchange = MAKE_AFIXED(_G_trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_VIEW_MAXCHANGE, GFIXED_1, GFIXED(1800,0)));
	}
	else {
		factor = AFIXED(0,05);
		maxchange = AFIXED(360,0);
	}
	if (maxchange < AFIXED(240,0)) maxchange = AFIXED(240,0);
	maxchange *= MAKE_AFIXED(thinktime);
	for (i = 0; i < 2; i++) {
		//
		if (bot_challenge.integer) {
			//smooth slowdown view model
			diff = MAKE_AFIXED(abs(FIXED_TO_INT(AngleDifference(bs->viewangles[i], bs->ideal_viewangles[i]))));
			anglespeed = diff * factor;
			if (anglespeed > maxchange) anglespeed = maxchange;
			bs->viewangles[i] = BotChangeViewAngle(bs->viewangles[i],
											bs->ideal_viewangles[i], anglespeed);
		}
		else {
			//over reaction view model
			bs->viewangles[i] = AngleMod(bs->viewangles[i]);
			bs->ideal_viewangles[i] = AngleMod(bs->ideal_viewangles[i]);
			diff = AngleDifference(bs->viewangles[i], bs->ideal_viewangles[i]);
			disired_speed = diff * factor;
			bs->viewanglespeed[i] += (bs->viewanglespeed[i] - disired_speed);
			if (bs->viewanglespeed[i] > AFIXED(180,0)) bs->viewanglespeed[i] = maxchange;
			if (bs->viewanglespeed[i] < -AFIXED(180,0)) bs->viewanglespeed[i] = -maxchange;
			anglespeed = bs->viewanglespeed[i];
			if (anglespeed > maxchange) anglespeed = maxchange;
			if (anglespeed < -maxchange) anglespeed = -maxchange;
			bs->viewangles[i] += anglespeed;
			bs->viewangles[i] = AngleMod(bs->viewangles[i]);
			//demping
			bs->viewanglespeed[i] *= AFIXED(0,45) * (AFIXED_1 - factor);
		}
		//BotAI_Print(PRT_MESSAGE, "ideal_angles %f %f\n", FIXED_TO_DOUBLE(bs->ideal_viewangles[0]), FIXED_TO_DOUBLE(bs->ideal_viewangles[1]), FIXED_TO_DOUBLE(bs->ideal_viewangles[2]));
		//bs->viewangles[i] = bs->ideal_viewangles[i];
	}
	//bs->viewangles[PITCH] = 0;
	if (bs->viewangles[PITCH] > AFIXED(180,0)) bs->viewangles[PITCH] -= AFIXED(360,0);
	//elementary action: view
	_G_trap_EA_View(bs->client, bs->viewangles);
}

/*
==============
BotInputToUserCommand
==============
*/
void BotInputToUserCommand(bot_input_t *bi, usercmd_t *ucmd, int delta_angles[3], int time) {
	avec3_t angles;
	avec3_t forward, right;
	short temp;
	int j;

	//clear the whole structure
	memset(ucmd, 0, sizeof(usercmd_t));
	//
	//Com_Printf("dir = %f %f %f speed = %f\n", FIXED_TO_DOUBLE(bi->dir[0]), FIXED_TO_DOUBLE(bi->dir[1]), FIXED_TO_DOUBLE(bi->dir[2]), FIXED_TO_DOUBLE(bi->speed));
	//the duration for the user command in milli seconds
	ucmd->serverTime = time;
	//
	if (bi->actionflags & ACTION_DELAYEDJUMP) {
		bi->actionflags |= ACTION_JUMP;
		bi->actionflags &= ~ACTION_DELAYEDJUMP;
	}
	//set the buttons
	if (bi->actionflags & ACTION_RESPAWN) ucmd->buttons = BUTTON_ATTACK;
	if (bi->actionflags & ACTION_ATTACK) ucmd->buttons |= BUTTON_ATTACK;
	if (bi->actionflags & ACTION_TALK) ucmd->buttons |= BUTTON_TALK;
	if (bi->actionflags & ACTION_GESTURE) ucmd->buttons |= BUTTON_GESTURE;
	if (bi->actionflags & ACTION_USE) ucmd->buttons |= BUTTON_USE_HOLDABLE;
	if (bi->actionflags & ACTION_WALK) ucmd->buttons |= BUTTON_WALKING;
	if (bi->actionflags & ACTION_AFFIRMATIVE) ucmd->buttons |= BUTTON_AFFIRMATIVE;
	if (bi->actionflags & ACTION_NEGATIVE) ucmd->buttons |= BUTTON_NEGATIVE;
	if (bi->actionflags & ACTION_GETFLAG) ucmd->buttons |= BUTTON_GETFLAG;
	if (bi->actionflags & ACTION_GUARDBASE) ucmd->buttons |= BUTTON_GUARDBASE;
	if (bi->actionflags & ACTION_PATROL) ucmd->buttons |= BUTTON_PATROL;
	if (bi->actionflags & ACTION_FOLLOWME) ucmd->buttons |= BUTTON_FOLLOWME;
	//
	ucmd->weapon = bi->weapon;
	//set the view angles
	//NOTE: the ucmd->angles are the angles WITHOUT the delta angles
	ucmd->angles[PITCH] = ANGLE2SHORT(bi->viewangles[PITCH]);
	ucmd->angles[YAW] = ANGLE2SHORT(bi->viewangles[YAW]);
	ucmd->angles[ROLL] = ANGLE2SHORT(bi->viewangles[ROLL]);
	//subtract the delta angles
	for (j = 0; j < 3; j++) {
		temp = ucmd->angles[j] - delta_angles[j];
		/*NOTE: disabled because temp should be mod first
		if ( j == PITCH ) {
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) temp = 16000;
			else if ( temp < -16000 ) temp = -16000;
		}
		*/
		ucmd->angles[j] = temp;
	}
	//NOTE: movement is relative to the REAL view angles
	//get the horizontal forward and right vector
	//get the pitch in the range [-180, 180]
	if (FIXED_NOT_ZERO(bi->dir[2])) angles[PITCH] = bi->viewangles[PITCH];
	else angles[PITCH] = AFIXED_0;
	angles[YAW] = bi->viewangles[YAW];
	angles[ROLL] = AFIXED_0;
	AngleVectors(angles, forward, right, NULL);
	//bot input speed is in the range [0, 400]
	bi->speed = bi->speed * BFIXED(127 ,0) / BFIXED(400,0);
	//set the view independent movement
	ucmd->forwardmove = FIXED_TO_INT(FIXED_VEC3DOT(forward, bi->dir) * bi->speed);
	ucmd->rightmove = FIXED_TO_INT(FIXED_VEC3DOT(right, bi->dir) * bi->speed);
	ucmd->upmove = FIXED_TO_INT(MAKE_GFIXED(abs(FIXED_TO_INT(forward[2]))) * bi->dir[2] * bi->speed);
	//normal keyboard movement
	if (bi->actionflags & ACTION_MOVEFORWARD) ucmd->forwardmove += 127;
	if (bi->actionflags & ACTION_MOVEBACK) ucmd->forwardmove -= 127;
	if (bi->actionflags & ACTION_MOVELEFT) ucmd->rightmove -= 127;
	if (bi->actionflags & ACTION_MOVERIGHT) ucmd->rightmove += 127;
	//jump/moveup
	if (bi->actionflags & ACTION_JUMP) ucmd->upmove += 127;
	//crouch/movedown
	if (bi->actionflags & ACTION_CROUCH) ucmd->upmove -= 127;
	//
	//Com_Printf("forward = %d right = %d up = %d\n", ucmd.forwardmove, ucmd.rightmove, ucmd.upmove);
	//Com_Printf("ucmd->serverTime = %d\n", ucmd->serverTime);
}

/*
==============
BotUpdateInput
==============
*/
void BotUpdateInput(bot_state_t *bs, int time, int elapsed_time) {
	bot_input_t bi;
	int j;

	//add the delta angles to the bot's current view angles
	for (j = 0; j < 3; j++) {
		bs->viewangles[j] = AngleMod(bs->viewangles[j] + SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	}
	//change the bot view angles
	BotChangeViewAngles(bs, MSECTIME_G(elapsed_time));
	//retrieve the bot input
	_G_trap_EA_GetInput(bs->client, MSECTIME_G(time), &bi);
	//respawn hack
	if (bi.actionflags & ACTION_RESPAWN) {
		if (bs->lastucmd.buttons & BUTTON_ATTACK) bi.actionflags &= ~(ACTION_RESPAWN|ACTION_ATTACK);
	}
	//convert the bot input to a usercmd
	BotInputToUserCommand(&bi, &bs->lastucmd, bs->cur_ps.delta_angles, time);
	//subtract the delta angles
	for (j = 0; j < 3; j++) {
		bs->viewangles[j] = AngleMod(bs->viewangles[j] - SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	}
}

/*
==============
BotAIRegularUpdate
==============
*/
void BotAIRegularUpdate(void) {
	if (regularupdate_time < FloatTime()) {
		_G_trap_BotUpdateEntityItems();
		regularupdate_time = FloatTime() + GFIXED(0,3);
	}
}

/*
==============
RemoveColorEscapeSequences
==============
*/
void RemoveColorEscapeSequences( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (Q_IsColorString(&text[i])) {
			i++;
			continue;
		}
		if (text[i] > 0x7E)
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

/*
==============
BotAI
==============
*/
int BotAI(int client, gfixed thinktime) {
	bot_state_t *bs;
	char buf[1024], *args;
	int j;

	_G_trap_EA_ResetInput(client);
	//
	bs = botstates[client];
	if (!bs || !bs->inuse) {
		BotAI_Print(PRT_FATAL, "BotAI: client %d is not setup\n", client);
		return qfalse;
	}

	//retrieve the current client state
	BotAI_GetClientState( client, &bs->cur_ps );

	//retrieve any waiting server commands
	while( _G_trap_BotGetServerCommand(client, buf, sizeof(buf)) ) {
		//have buf point to the command and args to the command arguments
		args = strchr( buf, ' ');
		if (!args) continue;
		*args++ = '\0';

		//remove color espace sequences from the arguments
		RemoveColorEscapeSequences( args );

		if (!Q_stricmp(buf, "cp "))
			{ /*CenterPrintf*/ }
		else if (!Q_stricmp(buf, "cs"))
			{ /*ConfigStringModified*/ }
		else if (!Q_stricmp(buf, "print")) {
			//remove first and last quote from the chat message
			memmove(args, args+1, strlen(args));
			args[strlen(args)-1] = '\0';
			_G_trap_BotQueueConsoleMessage(bs->cs, CMS_NORMAL, args);
		}
		else if (!Q_stricmp(buf, "chat")) {
			//remove first and last quote from the chat message
			memmove(args, args+1, strlen(args));
			args[strlen(args)-1] = '\0';
			_G_trap_BotQueueConsoleMessage(bs->cs, CMS_CHAT, args);
		}
		else if (!Q_stricmp(buf, "tchat")) {
			//remove first and last quote from the chat message
			memmove(args, args+1, strlen(args));
			args[strlen(args)-1] = '\0';
			_G_trap_BotQueueConsoleMessage(bs->cs, CMS_CHAT, args);
		}
#ifdef MISSIONPACK
		else if (!Q_stricmp(buf, "vchat")) {
			BotVoiceChatCommand(bs, SAY_ALL, args);
		}
		else if (!Q_stricmp(buf, "vtchat")) {
			BotVoiceChatCommand(bs, SAY_TEAM, args);
		}
		else if (!Q_stricmp(buf, "vtell")) {
			BotVoiceChatCommand(bs, SAY_TELL, args);
		}
#endif
		else if (!Q_stricmp(buf, "scores"))
			{ /*FIXME: parse scores?*/ }
		else if (!Q_stricmp(buf, "clientLevelShot"))
			{ /*ignore*/ }
	}
	//add the delta angles to the bot's current view angles
	for (j = 0; j < 3; j++) {
		bs->viewangles[j] = AngleMod(bs->viewangles[j] + SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	}
	//increase the local time of the bot
	bs->ltime += thinktime;
	//
	bs->thinktime = thinktime;
	//origin of the bot
	VectorCopy(bs->cur_ps.origin, bs->origin);
	//eye coordinates of the bot
	VectorCopy(bs->cur_ps.origin, bs->eye);
	bs->eye[2] += MAKE_BFIXED(bs->cur_ps.viewheight);
	//get the area the bot is in
	bs->areanum = BotPointAreaNum(bs->origin);
	//the real AI
	BotDeathmatchAI(bs, thinktime);
	//set the weapon selection every AI frame
	_G_trap_EA_SelectWeapon(bs->client, bs->weaponnum);
	//subtract the delta angles
	for (j = 0; j < 3; j++) {
		bs->viewangles[j] = AngleMod(bs->viewangles[j] - SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	}
	//everything was ok
	return qtrue;
}

/*
==================
BotScheduleBotThink
==================
*/
void BotScheduleBotThink(void) {
	int i, botnum;

	botnum = 0;

	for( i = 0; i < MAX_CLIENTS; i++ ) {
		if( !botstates[i] || !botstates[i]->inuse ) {
			continue;
		}
		//initialize the bot think residual time
		botstates[i]->botthink_residual = bot_thinktime.integer * botnum / numbots;
		botnum++;
	}
}

/*
==============
BotWriteSessionData
==============
*/
void BotWriteSessionData(bot_state_t *bs) {
	const char	*s;
	const char	*var;

	s = va(
			"%i %i %i %i %i %i %i %i"
			" %f %f %f"
			" %f %f %f"
			" %f %f %f",
		bs->lastgoal_decisionmaker,
		bs->lastgoal_ltgtype,
		bs->lastgoal_teammate,
		bs->lastgoal_teamgoal.areanum,
		bs->lastgoal_teamgoal.entitynum,
		bs->lastgoal_teamgoal.flags,
		bs->lastgoal_teamgoal.iteminfo,
		bs->lastgoal_teamgoal.number,
		FIXED_TO_DOUBLE(bs->lastgoal_teamgoal.origin[0]),
		FIXED_TO_DOUBLE(bs->lastgoal_teamgoal.origin[1]),
		FIXED_TO_DOUBLE(bs->lastgoal_teamgoal.origin[2]),
		FIXED_TO_DOUBLE(bs->lastgoal_teamgoal.mins[0]),
		FIXED_TO_DOUBLE(bs->lastgoal_teamgoal.mins[1]),
		FIXED_TO_DOUBLE(bs->lastgoal_teamgoal.mins[2]),
		FIXED_TO_DOUBLE(bs->lastgoal_teamgoal.maxs[0]),
		FIXED_TO_DOUBLE(bs->lastgoal_teamgoal.maxs[1]),
		FIXED_TO_DOUBLE(bs->lastgoal_teamgoal.maxs[2])
		);

	var = va( "botsession%i", bs->client );

	_G_trap_Cvar_Set( var, s );
}

/*
==============
BotReadSessionData
==============
*/
void BotReadSessionData(bot_state_t *bs) {
	char	s[MAX_STRING_CHARS];
	const char	*var;
	float f0,f1,f2,f3,f4,f5,f6,f7,f8;

	var = va( "botsession%i", bs->client );
	_G_trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );



	sscanf(s,
			"%i %i %i %i %i %i %i %i"
			" %f %f %f"
			" %f %f %f"
			" %f %f %f",
		&bs->lastgoal_decisionmaker,
		&bs->lastgoal_ltgtype,
		&bs->lastgoal_teammate,
		&bs->lastgoal_teamgoal.areanum,
		&bs->lastgoal_teamgoal.entitynum,
		&bs->lastgoal_teamgoal.flags,
		&bs->lastgoal_teamgoal.iteminfo,
		&bs->lastgoal_teamgoal.number,
		&f0,
		&f1,
		&f2,
		&f3,
		&f4,
		&f5,
		&f6,
		&f7,
		&f8
		);

	bs->lastgoal_teamgoal.origin[0]=MAKE_BFIXED(f0);
	bs->lastgoal_teamgoal.origin[1]=MAKE_BFIXED(f1);
	bs->lastgoal_teamgoal.origin[2]=MAKE_BFIXED(f2);
	bs->lastgoal_teamgoal.mins[0]=MAKE_BFIXED(f3);
	bs->lastgoal_teamgoal.mins[1]=MAKE_BFIXED(f4);
	bs->lastgoal_teamgoal.mins[2]=MAKE_BFIXED(f5);
	bs->lastgoal_teamgoal.maxs[0]=MAKE_BFIXED(f6);
	bs->lastgoal_teamgoal.maxs[1]=MAKE_BFIXED(f7);
	bs->lastgoal_teamgoal.maxs[2]=MAKE_BFIXED(f8);
}

/*
==============
BotAISetupClient
==============
*/
int BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart) {
	char filename[MAX_PATH], name[MAX_PATH], gender[MAX_PATH];
	bot_state_t *bs;
	int errnum;

	if (!botstates[client]) botstates[client] = (bot_state_t *)G_Alloc(sizeof(bot_state_t));
	bs = botstates[client];

	if (bs && bs->inuse) {
		BotAI_Print(PRT_FATAL, "BotAISetupClient: client %d already setup\n", client);
		return qfalse;
	}

	if (!_G_trap_AAS_Initialized()) {
		BotAI_Print(PRT_FATAL, "AAS not initialized\n");
		return qfalse;
	}

	//load the bot character
	bs->character = _G_trap_BotLoadCharacter(settings->characterfile, settings->skill);
	if (!bs->character) {
		BotAI_Print(PRT_FATAL, "couldn't load skill %f from %s\n", FIXED_TO_DOUBLE(settings->skill), settings->characterfile);
		return qfalse;
	}
	//copy the settings
	memcpy(&bs->settings, settings, sizeof(bot_settings_t));
	//allocate a goal state
	bs->gs = _G_trap_BotAllocGoalState(client);
	//load the item weights
	_G_trap_Characteristic_String(bs->character, CHARACTERISTIC_ITEMWEIGHTS, filename, MAX_PATH);
	errnum = _G_trap_BotLoadItemWeights(bs->gs, filename);
	if (errnum != BLERR_NOERROR) {
		_G_trap_BotFreeGoalState(bs->gs);
		return qfalse;
	}
	//allocate a weapon state
	bs->ws = _G_trap_BotAllocWeaponState();
	//load the weapon weights
	_G_trap_Characteristic_String(bs->character, CHARACTERISTIC_WEAPONWEIGHTS, filename, MAX_PATH);
	errnum = _G_trap_BotLoadWeaponWeights(bs->ws, filename);
	if (errnum != BLERR_NOERROR) {
		_G_trap_BotFreeGoalState(bs->gs);
		_G_trap_BotFreeWeaponState(bs->ws);
		return qfalse;
	}
	//allocate a chat state
	bs->cs = _G_trap_BotAllocChatState();
	//load the chat file
	_G_trap_Characteristic_String(bs->character, CHARACTERISTIC_CHAT_FILE, filename, MAX_PATH);
	_G_trap_Characteristic_String(bs->character, CHARACTERISTIC_CHAT_NAME, name, MAX_PATH);
	errnum = _G_trap_BotLoadChatFile(bs->cs, filename, name);
	if (errnum != BLERR_NOERROR) {
		_G_trap_BotFreeChatState(bs->cs);
		_G_trap_BotFreeGoalState(bs->gs);
		_G_trap_BotFreeWeaponState(bs->ws);
		return qfalse;
	}
	//get the gender characteristic
	_G_trap_Characteristic_String(bs->character, CHARACTERISTIC_GENDER, gender, MAX_PATH);
	//set the chat gender
	if (*gender == 'f' || *gender == 'F') _G_trap_BotSetChatGender(bs->cs, CHAT_GENDERFEMALE);
	else if (*gender == 'm' || *gender == 'M') _G_trap_BotSetChatGender(bs->cs, CHAT_GENDERMALE);
	else _G_trap_BotSetChatGender(bs->cs, CHAT_GENDERLESS);

	bs->inuse = qtrue;
	bs->client = client;
	bs->entitynum = client;
	bs->setupcount = 4;
	bs->entergame_time = FloatTime();
	bs->ms = _G_trap_BotAllocMoveState();
	bs->walker = _G_trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_WALKER, GFIXED_0, GFIXED_1);
	numbots++;

	if (_G_trap_Cvar_VariableIntegerValue("bot_testichat")) {
		_G_trap_BotLibVarSet("bot_testichat", "1");
		BotChatTest(bs);
	}
	//NOTE: reschedule the bot thinking
	BotScheduleBotThink();
	//if interbreeding start with a mutation
	if (bot_interbreed) {
		_G_trap_BotMutateGoalFuzzyLogic(bs->gs, GFIXED_1);
	}
	// if we kept the bot client
	if (restart) {
		BotReadSessionData(bs);
	}
	//bot has been setup succesfully
	return qtrue;
}

/*
==============
BotAIShutdownClient
==============
*/
int BotAIShutdownClient(int client, qboolean restart) {
	bot_state_t *bs;

	bs = botstates[client];
	if (!bs || !bs->inuse) {
		//BotAI_Print(PRT_ERROR, "BotAIShutdownClient: client %d already shutdown\n", client);
		return qfalse;
	}

	if (restart) {
		BotWriteSessionData(bs);
	}

	if (BotChat_ExitGame(bs)) {
		_G_trap_BotEnterChat(bs->cs, bs->client, CHAT_ALL);
	}

	_G_trap_BotFreeMoveState(bs->ms);
	//free the goal state`			
	_G_trap_BotFreeGoalState(bs->gs);
	//free the chat file
	_G_trap_BotFreeChatState(bs->cs);
	//free the weapon weights
	_G_trap_BotFreeWeaponState(bs->ws);
	//free the bot character
	_G_trap_BotFreeCharacter(bs->character);
	//
	BotFreeWaypoints(bs->checkpoints);
	BotFreeWaypoints(bs->patrolpoints);
	//clear activate goal stack
	BotClearActivateGoalStack(bs);
	//clear the bot state
	memset(bs, 0, sizeof(bot_state_t));
	//set the inuse flag to qfalse
	bs->inuse = qfalse;
	//there's one bot less
	numbots--;
	//everything went ok
	return qtrue;
}

/*
==============
BotResetState

called when a bot enters the intermission or observer mode and
when the level is changed
==============
*/
void BotResetState(bot_state_t *bs) {
	int client, entitynum, inuse;
	int movestate, goalstate, chatstate, weaponstate;
	bot_settings_t settings;
	int character;
	playerState_t ps;							//current player state
	gfixed entergame_time;

	//save some things that should not be reset here
	memcpy(&settings, &bs->settings, sizeof(bot_settings_t));
	memcpy(&ps, &bs->cur_ps, sizeof(playerState_t));
	inuse = bs->inuse;
	client = bs->client;
	entitynum = bs->entitynum;
	character = bs->character;
	movestate = bs->ms;
	goalstate = bs->gs;
	chatstate = bs->cs;
	weaponstate = bs->ws;
	entergame_time = bs->entergame_time;
	//free checkpoints and patrol points
	BotFreeWaypoints(bs->checkpoints);
	BotFreeWaypoints(bs->patrolpoints);
	//reset the whole state
	memset(bs, 0, sizeof(bot_state_t));
	//copy back some state stuff that should not be reset
	bs->ms = movestate;
	bs->gs = goalstate;
	bs->cs = chatstate;
	bs->ws = weaponstate;
	memcpy(&bs->cur_ps, &ps, sizeof(playerState_t));
	memcpy(&bs->settings, &settings, sizeof(bot_settings_t));
	bs->inuse = inuse;
	bs->client = client;
	bs->entitynum = entitynum;
	bs->character = character;
	bs->entergame_time = entergame_time;
	//reset several states
	if (bs->ms) _G_trap_BotResetMoveState(bs->ms);
	if (bs->gs) _G_trap_BotResetGoalState(bs->gs);
	if (bs->ws) _G_trap_BotResetWeaponState(bs->ws);
	if (bs->gs) _G_trap_BotResetAvoidGoals(bs->gs);
	if (bs->ms) _G_trap_BotResetAvoidReach(bs->ms);
}

/*
==============
BotAILoadMap
==============
*/
int BotAILoadMap( int restart ) {
	int			i;
	vmCvar_t	mapname;

	if (!restart) {
		_G_trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
		_G_trap_BotLibLoadMap( mapname.string );
	}

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (botstates[i] && botstates[i]->inuse) {
			BotResetState( botstates[i] );
			botstates[i]->setupcount = 4;
		}
	}

	BotSetupDeathmatchAI();

	return qtrue;
}

#ifdef MISSIONPACK
void ProximityMine_Trigger( gentity_t *trigger, gentity_t *other, trace_t *trace );
#endif

/*
==================
BotAIStartFrame
==================
*/
int BotAIStartFrame(int time) {
	int i;
	gentity_t	*ent;
	bot_entitystate_t state;
	int elapsed_time, thinktime;
	static int local_time;
	static int botlib_residual;
	static int lastbotthink_time;

	G_CheckBotSpawn();

	_G_trap_Cvar_Update(&bot_rocketjump);
	_G_trap_Cvar_Update(&bot_grapple);
	_G_trap_Cvar_Update(&bot_fastchat);
	_G_trap_Cvar_Update(&bot_nochat);
	_G_trap_Cvar_Update(&bot_testrchat);
	_G_trap_Cvar_Update(&bot_thinktime);
	_G_trap_Cvar_Update(&bot_memorydump);
	_G_trap_Cvar_Update(&bot_saveroutingcache);
	_G_trap_Cvar_Update(&bot_pause);
	_G_trap_Cvar_Update(&bot_report);

	if (bot_report.integer) {
//		BotTeamplayReport();
//		_G_trap_Cvar_Set("bot_report", "0");
		BotUpdateInfoConfigStrings();
	}

	if (bot_pause.integer) {
		// execute bot user commands every frame
		for( i = 0; i < MAX_CLIENTS; i++ ) {
			if( !botstates[i] || !botstates[i]->inuse ) {
				continue;
			}
			if( g_entities[i].client->pers.connected != CON_CONNECTED ) {
				continue;
			}
			botstates[i]->lastucmd.forwardmove = 0;
			botstates[i]->lastucmd.rightmove = 0;
			botstates[i]->lastucmd.upmove = 0;
			botstates[i]->lastucmd.buttons = 0;
			botstates[i]->lastucmd.serverTime = time;
			_G_trap_BotUserCommand(botstates[i]->client, &botstates[i]->lastucmd);
		}
		return qtrue;
	}

	if (bot_memorydump.integer) {
		_G_trap_BotLibVarSet("memorydump", "1");
		_G_trap_Cvar_Set("bot_memorydump", "0");
	}
	if (bot_saveroutingcache.integer) {
		_G_trap_BotLibVarSet("saveroutingcache", "1");
		_G_trap_Cvar_Set("bot_saveroutingcache", "0");
	}
	//check if bot interbreeding is activated
	BotInterbreeding();
	//cap the bot think time
	if (bot_thinktime.integer > 200) {
		_G_trap_Cvar_Set("bot_thinktime", "200");
	}
	//if the bot think time changed we should reschedule the bots
	if (bot_thinktime.integer != lastbotthink_time) {
		lastbotthink_time = bot_thinktime.integer;
		BotScheduleBotThink();
	}

	elapsed_time = time - local_time;
	local_time = time;

	botlib_residual += elapsed_time;

	if (elapsed_time > bot_thinktime.integer) thinktime = elapsed_time;
	else thinktime = bot_thinktime.integer;

	// update the bot library
	if ( botlib_residual >= thinktime ) {
		botlib_residual -= thinktime;

		_G_trap_BotLibStartFrame(MSECTIME_G(time));

		if (!_G_trap_AAS_Initialized()) return qfalse;

		//update entities in the botlib
		for (i = 0; i < MAX_GENTITIES; i++) {
			ent = &g_entities[i];
			if (!ent->inuse) {
				_G_trap_BotLibUpdateEntity(i, NULL);
				continue;
			}
			if (!ent->r.linked) {
				_G_trap_BotLibUpdateEntity(i, NULL);
				continue;
			}
			if (ent->r.svFlags & SVF_NOCLIENT) {
				_G_trap_BotLibUpdateEntity(i, NULL);
				continue;
			}
			// do not update missiles
			if (ent->s.eType == ET_MISSILE && ent->s.weapon != WP_GRAPPLING_HOOK) {
				_G_trap_BotLibUpdateEntity(i, NULL);
				continue;
			}
			// do not update event only entities
			if (ent->s.eType > ET_EVENTS) {
				_G_trap_BotLibUpdateEntity(i, NULL);
				continue;
			}
#ifdef MISSIONPACK
			// never link prox mine triggers
			if (ent->r.contents == CONTENTS_TRIGGER) {
				if (ent->touch == ProximityMine_Trigger) {
					_G_trap_BotLibUpdateEntity(i, NULL);
					continue;
				}
			}
#endif
			//
			memset(&state, 0, sizeof(bot_entitystate_t));
			//
			VectorCopy(ent->r.currentOrigin, state.origin);
			if (i < MAX_CLIENTS) {
				VectorCopy(ent->s.apos.trBase, state.angles);
			} else {
				VectorCopy(ent->r.currentAngles, state.angles);
			}
			VectorCopy(ent->s.origin2, state.old_origin);
			VectorCopy(ent->r.mins, state.mins);
			VectorCopy(ent->r.maxs, state.maxs);
			state.type = ent->s.eType;
			state.flags = ent->s.eFlags;
			if (ent->r.bmodel) state.solid = SOLID_BSP;
			else state.solid = SOLID_BBOX;
			state.groundent = ent->s.groundEntityNum;
			state.modelindex = ent->s.modelindex;
			state.modelindex2 = ent->s.modelindex2;
			state.frame = ent->s.frame;
			state.event = ent->s.event;
			state.eventParm = ent->s.eventParm;
			state.powerups = ent->s.powerups;
			state.legsAnim = ent->s.legsAnim;
			state.torsoAnim = ent->s.torsoAnim;
			state.weapon = ent->s.weapon;
			//
			_G_trap_BotLibUpdateEntity(i, &state);
		}

		BotAIRegularUpdate();
	}

	floattime = _G_trap_AAS_Time();

	// execute scheduled bot AI
	for( i = 0; i < MAX_CLIENTS; i++ ) {
		if( !botstates[i] || !botstates[i]->inuse ) {
			continue;
		}
		//
		botstates[i]->botthink_residual += elapsed_time;
		//
		if ( botstates[i]->botthink_residual >= thinktime ) {
			botstates[i]->botthink_residual -= thinktime;

			if (!_G_trap_AAS_Initialized()) return qfalse;

			if (g_entities[i].client->pers.connected == CON_CONNECTED) {
				BotAI(i, MSECTIME_G(thinktime));
			}
		}
	}


	// execute bot user commands every frame
	for( i = 0; i < MAX_CLIENTS; i++ ) {
		if( !botstates[i] || !botstates[i]->inuse ) {
			continue;
		}
		if( g_entities[i].client->pers.connected != CON_CONNECTED ) {
			continue;
		}

		BotUpdateInput(botstates[i], time, elapsed_time);
		_G_trap_BotUserCommand(botstates[i]->client, &botstates[i]->lastucmd);
	}

	return qtrue;
}

/*
==============
BotInitLibrary
==============
*/
int BotInitLibrary(void) {
	char buf[144];

	//set the maxclients and maxentities library variables before calling BotSetupLibrary
	_G_trap_Cvar_VariableStringBuffer("sv_maxclients", buf, sizeof(buf));
	if (!strlen(buf)) strcpy(buf, "8");
	_G_trap_BotLibVarSet("maxclients", buf);
	Com_sprintf(buf, sizeof(buf), "%d", MAX_GENTITIES);
	_G_trap_BotLibVarSet("maxentities", buf);
	//bsp checksum
	_G_trap_Cvar_VariableStringBuffer("sv_mapChecksum", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("sv_mapChecksum", buf);
	//maximum number of aas links
	_G_trap_Cvar_VariableStringBuffer("max_aaslinks", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("max_aaslinks", buf);
	//maximum number of items in a level
	_G_trap_Cvar_VariableStringBuffer("max_levelitems", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("max_levelitems", buf);
	//game type
	_G_trap_Cvar_VariableStringBuffer("g_gametype", buf, sizeof(buf));
	if (!strlen(buf)) strcpy(buf, "0");
	_G_trap_BotLibVarSet("g_gametype", buf);
	//bot developer mode and log file
	_G_trap_BotLibVarSet("bot_developer", bot_developer.string);
	_G_trap_BotLibVarSet("log", buf);
	//no chatting
	_G_trap_Cvar_VariableStringBuffer("bot_nochat", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("nochat", "0");
	//visualize jump pads
	_G_trap_Cvar_VariableStringBuffer("bot_visualizejumppads", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("bot_visualizejumppads", buf);
	//forced clustering calculations
	_G_trap_Cvar_VariableStringBuffer("bot_forceclustering", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("forceclustering", buf);
	//forced reachability calculations
	_G_trap_Cvar_VariableStringBuffer("bot_forcereachability", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("forcereachability", buf);
	//force writing of AAS to file
	_G_trap_Cvar_VariableStringBuffer("bot_forcewrite", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("forcewrite", buf);
	//no AAS optimization
	_G_trap_Cvar_VariableStringBuffer("bot_aasoptimize", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("aasoptimize", buf);
	//
	_G_trap_Cvar_VariableStringBuffer("bot_saveroutingcache", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("saveroutingcache", buf);
	//reload instead of cache bot character files
	_G_trap_Cvar_VariableStringBuffer("bot_reloadcharacters", buf, sizeof(buf));
	if (!strlen(buf)) strcpy(buf, "0");
	_G_trap_BotLibVarSet("bot_reloadcharacters", buf);
	//base directory
	_G_trap_Cvar_VariableStringBuffer("fs_basepath", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("basedir", buf);
	//game directory
	_G_trap_Cvar_VariableStringBuffer("fs_game", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("gamedir", buf);
	//cd directory
	_G_trap_Cvar_VariableStringBuffer("fs_cdpath", buf, sizeof(buf));
	if (strlen(buf)) _G_trap_BotLibVarSet("cddir", buf);
	//
#ifdef MISSIONPACK
	_G_trap_BotLibDefine("MISSIONPACK");
#endif
	//setup the bot library
	return _G_trap_BotLibSetup();
}

/*
==============
BotAISetup
==============
*/
int BotAISetup( int restart ) {
	int			errnum;

	_G_trap_Cvar_Register(&bot_thinktime, "bot_thinktime", "100", CVAR_CHEAT);
	_G_trap_Cvar_Register(&bot_memorydump, "bot_memorydump", "0", CVAR_CHEAT);
	_G_trap_Cvar_Register(&bot_saveroutingcache, "bot_saveroutingcache", "0", CVAR_CHEAT);
	_G_trap_Cvar_Register(&bot_pause, "bot_pause", "0", CVAR_CHEAT);
	_G_trap_Cvar_Register(&bot_report, "bot_report", "0", CVAR_CHEAT);
	_G_trap_Cvar_Register(&bot_testsolid, "bot_testsolid", "0", CVAR_CHEAT);
	_G_trap_Cvar_Register(&bot_testclusters, "bot_testclusters", "0", CVAR_CHEAT);
	_G_trap_Cvar_Register(&bot_developer, "bot_developer", "0", CVAR_CHEAT);
	_G_trap_Cvar_Register(&bot_interbreedchar, "bot_interbreedchar", "", 0);
	_G_trap_Cvar_Register(&bot_interbreedbots, "bot_interbreedbots", "10", 0);
	_G_trap_Cvar_Register(&bot_interbreedcycle, "bot_interbreedcycle", "20", 0);
	_G_trap_Cvar_Register(&bot_interbreedwrite, "bot_interbreedwrite", "", 0);

	//if the game is restarted for a tournament
	if (restart) {
		return qtrue;
	}

	//initialize the bot states
	memset( botstates, 0, sizeof(botstates) );

	errnum = BotInitLibrary();
	if (errnum != BLERR_NOERROR) return qfalse;
	return qtrue;
}

/*
==============
BotAIShutdown
==============
*/
int BotAIShutdown( int restart ) {

	int i;

	//if the game is restarted for a tournament
	if ( restart ) {
		//shutdown all the bots in the botlib
		for (i = 0; i < MAX_CLIENTS; i++) {
			if (botstates[i] && botstates[i]->inuse) {
				BotAIShutdownClient(botstates[i]->client, restart);
			}
		}
		//don't shutdown the bot library
	}
	else {
		_G_trap_BotLibShutdown();
	}
	return qtrue;
}

