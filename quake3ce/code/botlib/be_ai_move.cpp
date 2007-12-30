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

/*****************************************************************************
 * name:		be_ai_move.c
 *
 * desc:		bot movement AI
 *
 * $Archive: /MissionPack/code/botlib/be_ai_move.c $
 *
 *****************************************************************************/

#include"botlib_pch.h"



//#define DEBUG_AI_MOVE
//#define DEBUG_ELEVATOR
//#define DEBUG_GRAPPLE

// bk001204 - redundant bot_avoidspot_t, see ../game/be_ai_move.h

//movement state
//NOTE: the moveflags MFL_ONGROUND, MFL_TELEPORTED, MFL_WATERJUMP and
//		MFL_GRAPPLEPULL must be set outside the movement code
typedef struct bot_movestate_s
{
	//input vars (all set outside the movement code)
	bvec3_t origin;								//origin of the bot
	bvec3_t velocity;							//velocity of the bot
	bvec3_t viewoffset;							//view offset
	int entitynum;								//entity number of the bot
	int client;									//client number of the bot
	gfixed thinktime;							//time the bot thinks
	int presencetype;							//presencetype of the bot
	avec3_t viewangles;							//view angles of the bot
	//state vars
	int areanum;								//area the bot is in
	int lastareanum;							//last area the bot was in
	int lastgoalareanum;						//last goal area number
	int lastreachnum;							//last reachability number
	bvec3_t lastorigin;							//origin previous cycle
	int reachareanum;							//area number of the reachabilty
	int moveflags;								//movement flags
	int jumpreach;								//set when jumped
	gfixed grapplevisible_time;					//last time the grapple was visible
	bfixed lastgrappledist;						//last distance to the grapple end
	gfixed reachability_time;					//time to use current reachability
	int avoidreach[MAX_AVOIDREACH];				//reachabilities to avoid
	gfixed avoidreachtimes[MAX_AVOIDREACH];		//times to avoid the reachabilities
	int avoidreachtries[MAX_AVOIDREACH];		//number of tries before avoiding
	//
	bot_avoidspot_t avoidspots[MAX_AVOIDSPOTS];	//spots to avoid
	int numavoidspots;
} bot_movestate_t;

//used to avoid reachability links for some time after being used
#define AVOIDREACH
#define AVOIDREACH_TIME			6		//avoid links for 6 seconds after use
#define AVOIDREACH_TRIES		4
//prediction times
#define PREDICTIONTIME_JUMP	3		//in seconds
#define PREDICTIONTIME_MOVE	2		//in seconds
//weapon indexes for weapon jumping
#define WEAPONINDEX_ROCKET_LAUNCHER		5
#define WEAPONINDEX_BFG					9

#define MODELTYPE_FUNC_PLAT		1
#define MODELTYPE_FUNC_BOB		2
#define MODELTYPE_FUNC_DOOR		3
#define MODELTYPE_FUNC_STATIC	4

libvar_t *sv_maxstep;
libvar_t *sv_maxbarrier;
libvar_t *sv_gravity;
libvar_t *weapindex_rocketlauncher;
libvar_t *weapindex_bfg10k;
libvar_t *weapindex_grapple;
libvar_t *entitytypemissile;
libvar_t *offhandgrapple;
libvar_t *cmd_grappleoff;
libvar_t *cmd_grappleon;
//type of model, func_plat or func_bobbing
int modeltypes[MAX_MODELS];

bot_movestate_t *botmovestates[MAX_CLIENTS+1];

//========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//========================================================================
int BotAllocMoveState(void)
{
	int i;

	for (i = 1; i <= MAX_CLIENTS; i++)
	{
		if (!botmovestates[i])
		{
			botmovestates[i] = (bot_movestate_t *)GetClearedMemory(sizeof(bot_movestate_t));
			return i;
		} //end if
	} //end for
	return 0;
} //end of the function BotAllocMoveState
//========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//========================================================================
void BotFreeMoveState(int handle)
{
	if (handle <= 0 || handle > MAX_CLIENTS)
	{
		botimport.Print(PRT_FATAL, "move state handle %d out of range\n", handle);
		return;
	} //end if
	if (!botmovestates[handle])
	{
		botimport.Print(PRT_FATAL, "invalid move state %d\n", handle);
		return;
	} //end if
	FreeMemory(botmovestates[handle]);
	botmovestates[handle] = NULL;
} //end of the function BotFreeMoveState
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
bot_movestate_t *BotMoveStateFromHandle(int handle)
{
	if (handle <= 0 || handle > MAX_CLIENTS)
	{
		botimport.Print(PRT_FATAL, "move state handle %d out of range\n", handle);
		return NULL;
	} //end if
	if (!botmovestates[handle])
	{
		botimport.Print(PRT_FATAL, "invalid move state %d\n", handle);
		return NULL;
	} //end if
	return botmovestates[handle];
} //end of the function BotMoveStateFromHandle
//========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//========================================================================
void BotInitMoveState(int handle, bot_initmove_t *initmove)
{
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle(handle);
	if (!ms) return;
	VectorCopy(initmove->origin, ms->origin);
	VectorCopy(initmove->velocity, ms->velocity);
	VectorCopy(initmove->viewoffset, ms->viewoffset);
	ms->entitynum = initmove->entitynum;
	ms->client = initmove->client;
	ms->thinktime = initmove->thinktime;
	ms->presencetype = initmove->presencetype;
	VectorCopy(initmove->viewangles, ms->viewangles);
	//
	ms->moveflags &= ~MFL_ONGROUND;
	if (initmove->or_moveflags & MFL_ONGROUND) ms->moveflags |= MFL_ONGROUND;
	ms->moveflags &= ~MFL_TELEPORTED;	
	if (initmove->or_moveflags & MFL_TELEPORTED) ms->moveflags |= MFL_TELEPORTED;
	ms->moveflags &= ~MFL_WATERJUMP;
	if (initmove->or_moveflags & MFL_WATERJUMP) ms->moveflags |= MFL_WATERJUMP;
	ms->moveflags &= ~MFL_WALK;
	if (initmove->or_moveflags & MFL_WALK) ms->moveflags |= MFL_WALK;
	ms->moveflags &= ~MFL_GRAPPLEPULL;
	if (initmove->or_moveflags & MFL_GRAPPLEPULL) ms->moveflags |= MFL_GRAPPLEPULL;
} //end of the function BotInitMoveState
//========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//========================================================================
afixed AngleDiff(afixed ang1, afixed ang2)
{
	afixed diff;

	diff = ang1 - ang2;
	if (ang1 > ang2)
	{
		if (diff > AFIXED(180,0)) diff -= AFIXED(360,0);
	} //end if
	else
	{
		if (diff < -AFIXED(180,0)) diff += AFIXED(360,0);
	} //end else
	return diff;
} //end of the function AngleDiff
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotFuzzyPointReachabilityArea(bvec3_t origin)
{
	int firstareanum, j, x, y, z;
	int areas[10], numareas, areanum, bestareanum;
	bfixed dist, bestdist;
	bvec3_t points[10], v, end;

	firstareanum = 0;
	areanum = AAS_PointAreaNum(origin);
	if (areanum)
	{
		firstareanum = areanum;
		if (AAS_AreaReachability(areanum)) return areanum;
	} //end if
	VectorCopy(origin, end);
	end[2] += BFIXED(4,0);
	numareas = AAS_TraceAreas(origin, end, areas, points, 10);
	for (j = 0; j < numareas; j++)
	{
		if (AAS_AreaReachability(areas[j])) return areas[j];
	} //end for
	bestdist = BFIXED(999999,0);
	bestareanum = 0;
	for (z = 1; z >= -1; z -= 1)
	{
		for (x = 1; x >= -1; x -= 1)
		{
			for (y = 1; y >= -1; y -= 1)
			{
				VectorCopy(origin, end);
				end[0] += MAKE_BFIXED(x * 8);
				end[1] += MAKE_BFIXED(y * 8);
				end[2] += MAKE_BFIXED(z * 12);
				numareas = AAS_TraceAreas(origin, end, areas, points, 10);
				for (j = 0; j < numareas; j++)
				{
					if (AAS_AreaReachability(areas[j]))
					{
						VectorSubtract(points[j], origin, v);
						dist = FIXED_VEC3LEN(v);
						if (dist < bestdist)
						{
							bestareanum = areas[j];
							bestdist = dist;
						} //end if
					} //end if
					if (!firstareanum) firstareanum = areas[j];
				} //end for
			} //end for
		} //end for
		if (bestareanum) return bestareanum;
	} //end for
	return firstareanum;
} //end of the function BotFuzzyPointReachabilityArea
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotReachabilityArea(bvec3_t origin, int client)
{
	int modelnum, modeltype, reachnum, areanum;
	aas_reachability_t reach;
	bvec3_t org, end, mins, maxs;
	avec3_t up = {AFIXED_0, AFIXED_0, AFIXED_1};
	bsp_trace_t bsptrace;
	aas_trace_t trace;

	//check if the bot is standing on something
	AAS_PresenceTypeBoundingBox(PRESENCE_CROUCH, mins, maxs);
	FIXED_VEC3MA_R(origin, -BFIXED(3,0), up, end);
	bsptrace = AAS_Trace(origin, mins, maxs, end, client, CONTENTS_SOLID|CONTENTS_PLAYERCLIP);
	if (!bsptrace.startsolid && bsptrace.fraction < GFIXED_1 && bsptrace.ent != ENTITYNUM_NONE)
	{
		//if standing on the world the bot should be in a valid area
		if (bsptrace.ent == ENTITYNUM_WORLD)
		{
			return BotFuzzyPointReachabilityArea(origin);
		} //end if

		modelnum = AAS_EntityModelindex(bsptrace.ent);
		modeltype = modeltypes[modelnum];

		//if standing on a func_plat or func_bobbing then the bot is assumed to be
		//in the area the reachability points to
		if (modeltype == MODELTYPE_FUNC_PLAT || modeltype == MODELTYPE_FUNC_BOB)
		{
			reachnum = AAS_NextModelReachability(0, modelnum);
			if (reachnum)
			{
				AAS_ReachabilityFromNum(reachnum, &reach);
				return reach.areanum;
			} //end if
		} //end else if

		//if the bot is swimming the bot should be in a valid area
		if (AAS_Swimming(origin))
		{
			return BotFuzzyPointReachabilityArea(origin);
		} //end if
		//
		areanum = BotFuzzyPointReachabilityArea(origin);
		//if the bot is in an area with reachabilities
		if (areanum && AAS_AreaReachability(areanum)) return areanum;
		//trace down till the ground is hit because the bot is standing on some other entity
		VectorCopy(origin, org);
		VectorCopy(org, end);
		end[2] -= BFIXED(800,0);
		trace = AAS_TraceClientBBox(org, end, PRESENCE_CROUCH, -1);
		if (!trace.startsolid)
		{
			VectorCopy(trace.endpos, org);
		} //end if
		//
		return BotFuzzyPointReachabilityArea(org);
	} //end if
	//
	return BotFuzzyPointReachabilityArea(origin);
} //end of the function BotReachabilityArea
//===========================================================================
// returns the reachability area the bot is in
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
/*
int BotReachabilityArea(bvec3_t origin, int testground)
{
	int firstareanum, i, j, x, y, z;
	int areas[10], numareas, areanum, bestareanum;
	bfixed dist, bestdist;
	bvec3_t org, end, points[10], v;
	aas_trace_t trace;

	firstareanum = 0;
	for (i = 0; i < 2; i++)
	{
		VectorCopy(origin, org);
		//if test at the ground (used when bot is standing on an entity)
		if (i > 0)
		{
			VectorCopy(origin, end);
			end[2] -= 800;
			trace = AAS_TraceClientBBox(origin, end, PRESENCE_CROUCH, -1);
			if (!trace.startsolid)
			{
				VectorCopy(trace.endpos, org);
			} //end if
		} //end if

		firstareanum = 0;
		areanum = AAS_PointAreaNum(org);
		if (areanum)
		{
			firstareanum = areanum;
			if (AAS_AreaReachability(areanum)) return areanum;
		} //end if
		bestdist = 999999;
		bestareanum = 0;
		for (z = 1; z >= -1; z -= 1)
		{
			for (x = 1; x >= -1; x -= 1)
			{
				for (y = 1; y >= -1; y -= 1)
				{
					VectorCopy(org, end);
					end[0] += x * 8;
					end[1] += y * 8;
					end[2] += z * 12;
					numareas = AAS_TraceAreas(org, end, areas, points, 10);
					for (j = 0; j < numareas; j++)
					{
						if (AAS_AreaReachability(areas[j]))
						{
							VectorSubtract(points[j], org, v);
							dist = FIXED_VEC3LEN(v);
							if (dist < bestdist)
							{
								bestareanum = areas[j];
								bestdist = dist;
							} //end if
						} //end if
					} //end for
				} //end for
			} //end for
			if (bestareanum) return bestareanum;
		} //end for
		if (!testground) break;
	} //end for
//#ifdef DEBUG
	//botimport.Print(PRT_MESSAGE, "no reachability area\n");
//#endif //DEBUG
	return firstareanum;
} //end of the function BotReachabilityArea*/
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotOnMover(bvec3_t origin, int entnum, aas_reachability_t *reach)
{
	int i, modelnum;
	bvec3_t mins, maxs, modelorigin, org, end;
	avec3_t angles = {AFIXED_0, AFIXED_0, AFIXED_0};
	bvec3_t boxmins = {-BFIXED(16,0), -BFIXED(16,0), -BFIXED(8,0)}, boxmaxs = {BFIXED(16,0), BFIXED(16,0), BFIXED(8,0)};
	bsp_trace_t trace;

	modelnum = reach->facenum & 0x0000FFFF;
	//get some bsp model info
	AAS_BSPModelMinsMaxsOrigin(modelnum, angles, mins, maxs, NULL);
	//
	if (!AAS_OriginOfMoverWithModelNum(modelnum, modelorigin))
	{
		botimport.Print(PRT_MESSAGE, "no entity with model %d\n", modelnum);
		return qfalse;
	} //end if
	//
	for (i = 0; i < 2; i++)
	{
		if (origin[i] > modelorigin[i] + maxs[i] + BFIXED(16,0)) return qfalse;
		if (origin[i] < modelorigin[i] + mins[i] - BFIXED(16,0)) return qfalse;
	} //end for
	//
	VectorCopy(origin, org);
	org[2] += BFIXED(24,0);
	VectorCopy(origin, end);
	end[2] -= BFIXED(48,0);
	//
	trace = AAS_Trace(org, boxmins, boxmaxs, end, entnum, CONTENTS_SOLID|CONTENTS_PLAYERCLIP);
	if (!trace.startsolid && !trace.allsolid)
	{
		//NOTE: the reachability face number is the model number of the elevator
		if (trace.ent != ENTITYNUM_NONE && AAS_EntityModelNum(trace.ent) == modelnum)
		{
			return qtrue;
		} //end if
	} //end if
	return qfalse;
} //end of the function BotOnMover
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int MoverDown(aas_reachability_t *reach)
{
	int modelnum;
	bvec3_t mins, maxs, origin;
	avec3_t angles = {AFIXED_0, AFIXED_0, AFIXED_0};

	modelnum = reach->facenum & 0x0000FFFF;
	//get some bsp model info
	AAS_BSPModelMinsMaxsOrigin(modelnum, angles, mins, maxs, origin);
	//
	if (!AAS_OriginOfMoverWithModelNum(modelnum, origin))
	{
		botimport.Print(PRT_MESSAGE, "no entity with model %d\n", modelnum);
		return qfalse;
	} //end if
	//if the top of the plat is below the reachability start point
	if (origin[2] + maxs[2] < reach->start[2]) return qtrue;
	return qfalse;
} //end of the function MoverDown
//========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//========================================================================
void BotSetBrushModelTypes(void)
{
	int ent, modelnum;
	char classname[MAX_EPAIRKEY], model[MAX_EPAIRKEY];

	Com_Memset(modeltypes, 0, MAX_MODELS * sizeof(int));
	//
	for (ent = AAS_NextBSPEntity(0); ent; ent = AAS_NextBSPEntity(ent))
	{
		if (!AAS_ValueForBSPEpairKey(ent, "classname", classname, MAX_EPAIRKEY)) continue;
		if (!AAS_ValueForBSPEpairKey(ent, "model", model, MAX_EPAIRKEY)) continue;
		if (model[0]) modelnum = atoi(model+1);
		else modelnum = 0;

		if (modelnum < 0 || modelnum > MAX_MODELS)
		{
			botimport.Print(PRT_MESSAGE, "entity %s model number out of range\n", classname);
			continue;
		} //end if

		if (!Q_stricmp(classname, "func_bobbing"))
			modeltypes[modelnum] = MODELTYPE_FUNC_BOB;
		else if (!Q_stricmp(classname, "func_plat"))
			modeltypes[modelnum] = MODELTYPE_FUNC_PLAT;
		else if (!Q_stricmp(classname, "func_door"))
			modeltypes[modelnum] = MODELTYPE_FUNC_DOOR;
		else if (!Q_stricmp(classname, "func_static"))
			modeltypes[modelnum] = MODELTYPE_FUNC_STATIC;
	} //end for
} //end of the function BotSetBrushModelTypes
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotOnTopOfEntity(bot_movestate_t *ms)
{
	bvec3_t mins, maxs, end;
	avec3_t up = {AFIXED_0, AFIXED_0, AFIXED_1};
	bsp_trace_t trace;

	AAS_PresenceTypeBoundingBox(ms->presencetype, mins, maxs);
	FIXED_VEC3MA_R(ms->origin, -BFIXED(3,0), up, end);
	trace = AAS_Trace(ms->origin, mins, maxs, end, ms->entitynum, CONTENTS_SOLID|CONTENTS_PLAYERCLIP);
	if (!trace.startsolid && (trace.ent != ENTITYNUM_WORLD && trace.ent != ENTITYNUM_NONE) )
	{
		return trace.ent;
	} //end if
	return -1;
} //end of the function BotOnTopOfEntity
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotValidTravel(bvec3_t origin, aas_reachability_t *reach, int travelflags)
{
	//if the reachability uses an unwanted travel type
	if (AAS_TravelFlagForType(reach->traveltype) & ~travelflags) return qfalse;
	//don't go into areas with bad travel types
	if (AAS_AreaContentsTravelFlags(reach->areanum) & ~travelflags) return qfalse;
	return qtrue;
} //end of the function BotValidTravel
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotAddToAvoidReach(bot_movestate_t *ms, int number, gfixed avoidtime)
{
	int i;

	for (i = 0; i < MAX_AVOIDREACH; i++)
	{
		if (ms->avoidreach[i] == number)
		{
			if (ms->avoidreachtimes[i] > AAS_Time()) ms->avoidreachtries[i]++;
			else ms->avoidreachtries[i] = 1;
			ms->avoidreachtimes[i] = AAS_Time() + avoidtime;
			return;
		} //end if
	} //end for
	//add the reachability to the reachabilities to avoid for a while
	for (i = 0; i < MAX_AVOIDREACH; i++)
	{
		if (ms->avoidreachtimes[i] < AAS_Time())
		{
			ms->avoidreach[i] = number;
			ms->avoidreachtimes[i] = AAS_Time() + avoidtime;
			ms->avoidreachtries[i] = 1;
			return;
		} //end if
	} //end for
} //end of the function BotAddToAvoidReach
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bfixed DistanceFromLineSquared(bvec3_t p, bvec3_t lp1, bvec3_t lp2)
{
	bvec3_t proj, dir;
	int j;

	AAS_ProjectPointOntoVector(p, lp1, lp2, proj);
	for (j = 0; j < 3; j++)
		if ((proj[j] > lp1[j] && proj[j] > lp2[j]) ||
			(proj[j] < lp1[j] && proj[j] < lp2[j]))
			break;
	if (j < 3) {
		if (FIXED_ABS(proj[j] - lp1[j]) < FIXED_ABS(proj[j] - lp2[j]))
			VectorSubtract(p, lp1, dir);
		else
			VectorSubtract(p, lp2, dir);
		return FIXED_VEC3LEN_SQ(dir);
	}
	VectorSubtract(p, proj, dir);
	return FIXED_VEC3LEN_SQ(dir);
} //end of the function DistanceFromLineSquared
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bfixed VectorDistanceSquared(bvec3_t p1, bvec3_t p2)
{
	bvec3_t dir;
	VectorSubtract(p2, p1, dir);
	return FIXED_VEC3LEN_SQ(dir);
} //end of the function VectorDistanceSquared
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotAvoidSpots(bvec3_t origin, aas_reachability_t *reach, bot_avoidspot_t *avoidspots, int numavoidspots)
{
	int checkbetween, i, type;
	bfixed squareddist, squaredradius;

	switch(reach->traveltype & TRAVELTYPE_MASK)
	{
		case TRAVEL_WALK: checkbetween = qtrue; break;
		case TRAVEL_CROUCH: checkbetween = qtrue; break;
		case TRAVEL_BARRIERJUMP: checkbetween = qtrue; break;
		case TRAVEL_LADDER: checkbetween = qtrue; break;
		case TRAVEL_WALKOFFLEDGE: checkbetween = qfalse; break;
		case TRAVEL_JUMP: checkbetween = qfalse; break;
		case TRAVEL_SWIM: checkbetween = qtrue; break;
		case TRAVEL_WATERJUMP: checkbetween = qtrue; break;
		case TRAVEL_TELEPORT: checkbetween = qfalse; break;
		case TRAVEL_ELEVATOR: checkbetween = qfalse; break;
		case TRAVEL_GRAPPLEHOOK: checkbetween = qfalse; break;
		case TRAVEL_ROCKETJUMP: checkbetween = qfalse; break;
		case TRAVEL_BFGJUMP: checkbetween = qfalse; break;
		case TRAVEL_JUMPPAD: checkbetween = qfalse; break;
		case TRAVEL_FUNCBOB: checkbetween = qfalse; break;
		default: checkbetween = qtrue; break;
	} //end switch

	type = AVOID_CLEAR;
	for (i = 0; i < numavoidspots; i++)
	{
		squaredradius = Square(avoidspots[i].radius);
		squareddist = DistanceFromLineSquared(avoidspots[i].origin, origin, reach->start);
		// if moving towards the avoid spot
		if (squareddist < squaredradius &&
			VectorDistanceSquared(avoidspots[i].origin, origin) > squareddist)
		{
			type = avoidspots[i].type;
		} //end if
		else if (checkbetween) {
			squareddist = DistanceFromLineSquared(avoidspots[i].origin, reach->start, reach->end);
			// if moving towards the avoid spot
			if (squareddist < squaredradius &&
				VectorDistanceSquared(avoidspots[i].origin, reach->start) > squareddist)
			{
				type = avoidspots[i].type;
			} //end if
		} //end if
		else
		{
			VectorDistanceSquared(avoidspots[i].origin, reach->end);
			// if the reachability leads closer to the avoid spot
			if (squareddist < squaredradius && 
				VectorDistanceSquared(avoidspots[i].origin, reach->start) > squareddist)
			{
				type = avoidspots[i].type;
			} //end if
		} //end else
		if (type == AVOID_ALWAYS)
			return type;
	} //end for
	return type;
} //end of the function BotAvoidSpots
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotAddAvoidSpot(int movestate, bvec3_t origin, bfixed radius, int type)
{
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle(movestate);
	if (!ms) return;
	if (type == AVOID_CLEAR)
	{
		ms->numavoidspots = 0;
		return;
	} //end if

	if (ms->numavoidspots >= MAX_AVOIDSPOTS)
		return;
	VectorCopy(origin, ms->avoidspots[ms->numavoidspots].origin);
	ms->avoidspots[ms->numavoidspots].radius = radius;
	ms->avoidspots[ms->numavoidspots].type = type;
	ms->numavoidspots++;
} //end of the function BotAddAvoidSpot
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotGetReachabilityToGoal(bvec3_t origin, int areanum,
									  int lastgoalareanum, int lastareanum,
									  int *avoidreach, gfixed *avoidreachtimes, int *avoidreachtries,
									  bot_goal_t *goal, int travelflags, int movetravelflags,
									  struct bot_avoidspot_s *avoidspots, int numavoidspots, int *flags)
{
	int i, t, besttime, bestreachnum, reachnum;
	aas_reachability_t reach;

	//if not in a valid area
	if (!areanum) return 0;
	//
	if (AAS_AreaDoNotEnter(areanum) || AAS_AreaDoNotEnter(goal->areanum))
	{
		travelflags |= TFL_DONOTENTER;
		movetravelflags |= TFL_DONOTENTER;
	} //end if
	//use the routing to find the next area to go to
	besttime = 0;
	bestreachnum = 0;
	//
	for (reachnum = AAS_NextAreaReachability(areanum, 0); reachnum;
		reachnum = AAS_NextAreaReachability(areanum, reachnum))
	{
#ifdef AVOIDREACH
		//check if it isn't an reachability to avoid
		for (i = 0; i < MAX_AVOIDREACH; i++)
		{
			if (avoidreach[i] == reachnum && avoidreachtimes[i] >= AAS_Time()) break;
		} //end for
		if (i != MAX_AVOIDREACH && avoidreachtries[i] > AVOIDREACH_TRIES)
		{
#ifdef DEBUG
			if (bl_bot_developer)
			{
				botimport.Print(PRT_MESSAGE, "avoiding reachability %d\n", avoidreach[i]);
			} //end if
#endif //DEBUG
			continue;
		} //end if
#endif //AVOIDREACH
		//get the reachability from the number
		AAS_ReachabilityFromNum(reachnum, &reach);
		//NOTE: do not go back to the previous area if the goal didn't change
		//NOTE: is this actually avoidance of local routing minima between two areas???
		if (lastgoalareanum == goal->areanum && reach.areanum == lastareanum) continue;
		//if (AAS_AreaContentsTravelFlags(reach.areanum) & ~travelflags) continue;
		//if the travel isn't valid
		if (!BotValidTravel(origin, &reach, movetravelflags)) continue;
		//get the travel time
		t = AAS_AreaTravelTimeToGoalArea(reach.areanum, reach.end, goal->areanum, travelflags);
		//if the goal area isn't reachable from the reachable area
		if (!t) continue;
		//if the bot should not use this reachability to avoid bad spots
		if (BotAvoidSpots(origin, &reach, avoidspots, numavoidspots)) {
			if (flags) {
				*flags |= MOVERESULT_BLOCKEDBYAVOIDSPOT;
			}
			continue;
		}
		//add the travel time towards the area
		t += reach.traveltime;// + AAS_AreaTravelTime(areanum, origin, reach.start);
		//if the travel time is better than the ones already found
		if (!besttime || t < besttime)
		{
			besttime = t;
			bestreachnum = reachnum;
		} //end if
	} //end for
	//
	return bestreachnum;
} //end of the function BotGetReachabilityToGoal
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int BotAddToTarget(bvec3_t start, bvec3_t end, bfixed maxdist, bfixed *dist, bvec3_t target)
{
	avec3_t dir;
	bvec3_t tmp;
	bfixed curdist;

	VectorSubtract(end, start, tmp);
	curdist = VectorNormalizeB2A(tmp,dir);
	if (*dist + curdist < maxdist)
	{
		VectorCopy(end, target);
		*dist += curdist;
		return qfalse;
	} //end if
	else
	{
		FIXED_VEC3MA_R(start, maxdist - *dist, dir, target);
		*dist = maxdist;
		return qtrue;
	} //end else
} //end of the function BotAddToTarget

int BotMovementViewTarget(int movestate, bot_goal_t *goal, int travelflags, bfixed lookahead, bvec3_t target)
{
	aas_reachability_t reach;
	int reachnum, lastareanum;
	bot_movestate_t *ms;
	bvec3_t end;
	bfixed dist;

	ms = BotMoveStateFromHandle(movestate);
	if (!ms) return qfalse;
	reachnum = 0;
	//if the bot has no goal or no last reachability
	if (!ms->lastreachnum || !goal) return qfalse;

	reachnum = ms->lastreachnum;
	VectorCopy(ms->origin, end);
	lastareanum = ms->lastareanum;
	dist = BFIXED_0;
	while(reachnum && dist < lookahead)
	{
		AAS_ReachabilityFromNum(reachnum, &reach);
		if (BotAddToTarget(end, reach.start, lookahead, &dist, target)) return qtrue;
		//never look beyond teleporters
		if ((reach.traveltype & TRAVELTYPE_MASK) == TRAVEL_TELEPORT) return qtrue;
		//never look beyond the weapon jump point
		if ((reach.traveltype & TRAVELTYPE_MASK) == TRAVEL_ROCKETJUMP) return qtrue;
		if ((reach.traveltype & TRAVELTYPE_MASK) == TRAVEL_BFGJUMP) return qtrue;
		//don't add jump pad distances
		if ((reach.traveltype & TRAVELTYPE_MASK) != TRAVEL_JUMPPAD &&
			(reach.traveltype & TRAVELTYPE_MASK) != TRAVEL_ELEVATOR &&
			(reach.traveltype & TRAVELTYPE_MASK) != TRAVEL_FUNCBOB)
		{
			if (BotAddToTarget(reach.start, reach.end, lookahead, &dist, target)) return qtrue;
		} //end if
		reachnum = BotGetReachabilityToGoal(reach.end, reach.areanum,
						ms->lastgoalareanum, lastareanum,
							ms->avoidreach, ms->avoidreachtimes, ms->avoidreachtries,
									goal, travelflags, travelflags, NULL, 0, NULL);
		VectorCopy(reach.end, end);
		lastareanum = reach.areanum;
		if (lastareanum == goal->areanum)
		{
			BotAddToTarget(reach.end, goal->origin, lookahead, &dist, target);
			return qtrue;
		} //end if
	} //end while
	//
	return qfalse;
} //end of the function BotMovementViewTarget
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotVisible(int ent, bvec3_t eye, bvec3_t target)
{
	bsp_trace_t trace;

	trace = AAS_Trace(eye, NULL, NULL, target, ent, CONTENTS_SOLID|CONTENTS_PLAYERCLIP);
	if (trace.fraction >= GFIXED_1) return qtrue;
	return qfalse;
} //end of the function BotVisible
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotPredictVisiblePosition(bvec3_t origin, int areanum, bot_goal_t *goal, int travelflags, bvec3_t target)
{
	aas_reachability_t reach;
	int reachnum, lastgoalareanum, lastareanum, i;
	int avoidreach[MAX_AVOIDREACH];
	gfixed avoidreachtimes[MAX_AVOIDREACH];
	int avoidreachtries[MAX_AVOIDREACH];
	bvec3_t end;

	//if the bot has no goal or no last reachability
	if (!goal) return qfalse;
	//if the areanum is not valid
	if (!areanum) return qfalse;
	//if the goal areanum is not valid
	if (!goal->areanum) return qfalse;

	Com_Memset(avoidreach, 0, MAX_AVOIDREACH * sizeof(int));
	lastgoalareanum = goal->areanum;
	lastareanum = areanum;
	VectorCopy(origin, end);
	//only do 20 hops
	for (i = 0; i < 20 && (areanum != goal->areanum); i++)
	{
		//
		reachnum = BotGetReachabilityToGoal(end, areanum,
						lastgoalareanum, lastareanum,
							avoidreach, avoidreachtimes, avoidreachtries,
									goal, travelflags, travelflags, NULL, 0, NULL);
		if (!reachnum) return qfalse;
		AAS_ReachabilityFromNum(reachnum, &reach);
		//
		if (BotVisible(goal->entitynum, goal->origin, reach.start))
		{
			VectorCopy(reach.start, target);
			return qtrue;
		} //end if
		//
		if (BotVisible(goal->entitynum, goal->origin, reach.end))
		{
			VectorCopy(reach.end, target);
			return qtrue;
		} //end if
		//
		if (reach.areanum == goal->areanum)
		{
			VectorCopy(reach.end, target);
			return qtrue;
		} //end if
		//
		lastareanum = areanum;
		areanum = reach.areanum;
		VectorCopy(reach.end, end);
		//
	} //end while
	//
	return qfalse;
} //end of the function BotPredictVisiblePosition
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void MoverBottomCenter(aas_reachability_t *reach, bvec3_t bottomcenter)
{
	int modelnum;
	bvec3_t mins, maxs, origin, mids;
	avec3_t angles = {AFIXED_0, AFIXED_0, AFIXED_0};

	modelnum = reach->facenum & 0x0000FFFF;
	//get some bsp model info
	AAS_BSPModelMinsMaxsOrigin(modelnum, angles, mins, maxs, origin);
	//
	if (!AAS_OriginOfMoverWithModelNum(modelnum, origin))
	{
		botimport.Print(PRT_MESSAGE, "no entity with model %d\n", modelnum);
	} //end if
	//get a point just above the plat in the bottom position
	VectorAdd(mins, maxs, mids);
	FIXED_VEC3MA(origin, BFIXED(0,5), mids, bottomcenter);
	bottomcenter[2] = reach->start[2];
} //end of the function MoverBottomCenter
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bfixed BotGapDistance(bvec3_t origin, avec3_t hordir, int entnum)
{
	bfixed dist, startz;
	bvec3_t start, end;
	aas_trace_t trace;

	//do gap checking
	startz = origin[2];
	//this enables walking down stairs more fluidly
	{
		VectorCopy(origin, start);
		VectorCopy(origin, end);
		end[2] -= BFIXED(60,0);
		trace = AAS_TraceClientBBox(start, end, PRESENCE_CROUCH, entnum);
		if (trace.fraction >= GFIXED_1) return BFIXED_1;
		startz = trace.endpos[2] + BFIXED_1;
	}
	//
	for (dist = BFIXED(8,0); dist <= BFIXED(100,0); dist += BFIXED(8,0))
	{
		FIXED_VEC3MA_R(origin, dist, hordir, start);
		start[2] = startz + BFIXED(24,0);
		VectorCopy(start, end);
		end[2] -= BFIXED(48,0) + MAKE_BFIXED(sv_maxbarrier->value);
		trace = AAS_TraceClientBBox(start, end, PRESENCE_CROUCH, entnum);
		//if solid is found the bot can't walk any further and fall into a gap
		if (!trace.startsolid)
		{
			//if it is a gap
			if (trace.endpos[2] < startz - MAKE_BFIXED(sv_maxstep->value) - BFIXED(8,0))
			{
				VectorCopy(trace.endpos, end);
				end[2] -= BFIXED(20,0);
				if (AAS_PointContents(end) & CONTENTS_WATER) break;
				//if a gap is found slow down
				//botimport.Print(PRT_MESSAGE, "gap at %f\n", FIXED_TO_DOUBLE(dist));
				return dist;
			} //end if
			startz = trace.endpos[2];
		} //end if
	} //end for
	return BFIXED_0;
} //end of the function BotGapDistance
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotCheckBarrierJump(bot_movestate_t *ms, avec3_t dir, bfixed speed)
{
	bvec3_t start, end;
	avec3_t hordir;
	aas_trace_t trace;

	VectorCopy(ms->origin, end);
	end[2] += MAKE_BFIXED(sv_maxbarrier->value);
	//trace right up
	trace = AAS_TraceClientBBox(ms->origin, end, PRESENCE_NORMAL, ms->entitynum);
	//this shouldn't happen... but we check anyway
	if (trace.startsolid) return qfalse;
	//if very low ceiling it isn't possible to jump up to a barrier
	if (trace.endpos[2] - ms->origin[2] < MAKE_BFIXED(sv_maxstep->value)) return qfalse;
	//
	hordir[0] = dir[0];
	hordir[1] = dir[1];
	hordir[2] = AFIXED_0;
	VectorNormalize(hordir);
	FIXED_VEC3MA_R(ms->origin, MAKE_BFIXED(ms->thinktime) * speed * BFIXED(0,5), hordir, end);
	VectorCopy(trace.endpos, start);
	end[2] = trace.endpos[2];
	//trace from previous trace end pos horizontally in the move direction
	trace = AAS_TraceClientBBox(start, end, PRESENCE_NORMAL, ms->entitynum);
	//again this shouldn't happen
	if (trace.startsolid) return qfalse;
	//
	VectorCopy(trace.endpos, start);
	VectorCopy(trace.endpos, end);
	end[2] = ms->origin[2];
	//trace down from the previous trace end pos
	trace = AAS_TraceClientBBox(start, end, PRESENCE_NORMAL, ms->entitynum);
	//if solid
	if (trace.startsolid) return qfalse;
	//if no obstacle at all
	if (trace.fraction >= GFIXED_1) return qfalse;
	//if less than the maximum step height
	if (trace.endpos[2] - ms->origin[2] < MAKE_BFIXED(sv_maxstep->value)) return qfalse;
	//
	EA_Jump(ms->client);
	EA_Move(ms->client, hordir, speed);
	ms->moveflags |= MFL_BARRIERJUMP;
	//there is a barrier
	return qtrue;
} //end of the function BotCheckBarrierJump
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotSwimInDirection(bot_movestate_t *ms, avec3_t dir, bfixed speed, int type)
{
	avec3_t normdir;

	VectorCopy(dir, normdir);
	VectorNormalize(normdir);
	EA_Move(ms->client, normdir, speed);
	return qtrue;
} //end of the function BotSwimInDirection
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotWalkInDirection(bot_movestate_t *ms, avec3_t dir, bfixed speed, int type)
{
	avec3_t hordir, atmpdir;
	bvec3_t cmdmove, velocity, origin, btmpdir;
	int presencetype, maxframes, cmdframes, stopevent;
	aas_clientmove_t move;
	bfixed dist;

	if (AAS_OnGround(ms->origin, ms->presencetype, ms->entitynum)) ms->moveflags |= MFL_ONGROUND;
	//if the bot is on the ground
	if (ms->moveflags & MFL_ONGROUND)
	{
		//if there is a barrier the bot can jump on
		if (BotCheckBarrierJump(ms, dir, speed)) return qtrue;
		//remove barrier jump flag
		ms->moveflags &= ~MFL_BARRIERJUMP;
		//get the presence type for the movement
		if ((type & MOVE_CROUCH) && !(type & MOVE_JUMP)) presencetype = PRESENCE_CROUCH;
		else presencetype = PRESENCE_NORMAL;
		//horizontal direction
		hordir[0] = dir[0];
		hordir[1] = dir[1];
		hordir[2] = AFIXED_0;
		VectorNormalize(hordir);
		//if the bot is not supposed to jump
		if (!(type & MOVE_JUMP))
		{
			//if there is a gap, try to jump over it
			if (BotGapDistance(ms->origin, hordir, ms->entitynum) > BFIXED_0) type |= MOVE_JUMP;
		} //end if
		//get command movement
		FIXED_VEC3SCALE_R(hordir, speed, cmdmove);
		VectorCopy(ms->velocity, velocity);
		//
		if (type & MOVE_JUMP)
		{
			//botimport.Print(PRT_MESSAGE, "trying jump\n");
			cmdmove[2] = BFIXED(400,0);
			maxframes = PREDICTIONTIME_JUMP * 10;
			cmdframes = 1;
			stopevent = PSE_HITGROUND|PSE_HITGROUNDDAMAGE|
						PSE_ENTERWATER|PSE_ENTERSLIME|PSE_ENTERLAVA;
		} //end if
		else
		{
			maxframes = 2;
			cmdframes = 2;
			stopevent = PSE_HITGROUNDDAMAGE|
						PSE_ENTERWATER|PSE_ENTERSLIME|PSE_ENTERLAVA;
		} //end else
		//AAS_ClearShownDebugLines();
		//
		VectorCopy(ms->origin, origin);
		origin[2] += BFIXED(0,5);
		AAS_PredictClientMovement(&move, ms->entitynum, origin, presencetype, qtrue,
									velocity, cmdmove, cmdframes, maxframes, GFIXED(0,1),
									stopevent, 0, qfalse);//qtrue);
		//if prediction time wasn't enough to fully predict the movement
		if (move.frames >= maxframes && (type & MOVE_JUMP))
		{
			//botimport.Print(PRT_MESSAGE, "client %d: max prediction frames\n", ms->client);
			return qfalse;
		} //end if
		//don't enter slime or lava and don't fall from too high
		if (move.stopevent & (PSE_ENTERSLIME|PSE_ENTERLAVA|PSE_HITGROUNDDAMAGE))
		{
			//botimport.Print(PRT_MESSAGE, "client %d: would be hurt ", ms->client);
			//if (move.stopevent & PSE_ENTERSLIME) botimport.Print(PRT_MESSAGE, "slime\n");
			//if (move.stopevent & PSE_ENTERLAVA) botimport.Print(PRT_MESSAGE, "lava\n");
			//if (move.stopevent & PSE_HITGROUNDDAMAGE) botimport.Print(PRT_MESSAGE, "hitground\n");
			return qfalse;
		} //end if
		//if ground was hit
		if (move.stopevent & PSE_HITGROUND)
		{
			//check for nearby gap
			VectorNormalizeB2A(move.velocity, atmpdir);
			dist = BotGapDistance(move.endpos, atmpdir, ms->entitynum);
			if (dist > BFIXED_0) return qfalse;
			//
			dist = BotGapDistance(move.endpos, hordir, ms->entitynum);
			if (dist > BFIXED_0) return qfalse;
		} //end if
		//get horizontal movement
		btmpdir[0] = move.endpos[0] - ms->origin[0];
		btmpdir[1] = move.endpos[1] - ms->origin[1];
		btmpdir[2] = BFIXED_0;
		//
		//AAS_DrawCross(move.endpos, 4, LINECOLOR_BLUE);
		//the bot is blocked by something
		if (FIXED_VEC3LEN(btmpdir) < FIXED_DIVPOW2(speed * MAKE_BFIXED(ms->thinktime),1)) return qfalse;
		//perform the movement
		if (type & MOVE_JUMP) EA_Jump(ms->client);
		if (type & MOVE_CROUCH) EA_Crouch(ms->client);
		EA_Move(ms->client, hordir, speed);
		//movement was succesfull
		return qtrue;
	} //end if
	else
	{
		if (ms->moveflags & MFL_BARRIERJUMP)
		{
			//if near the top or going down
			if (ms->velocity[2] < BFIXED(50,0))
			{
				EA_Move(ms->client, dir, speed);
			} //end if
		} //end if
		//FIXME: do air control to avoid hazards
		return qtrue;
	} //end else
} //end of the function BotWalkInDirection
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotMoveInDirection(int movestate, avec3_t dir, bfixed speed, int type)
{
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle(movestate);
	if (!ms) return qfalse;
	//if swimming
	if (AAS_Swimming(ms->origin))
	{
		return BotSwimInDirection(ms, dir, speed, type);
	} //end if
	else
	{
		return BotWalkInDirection(ms, dir, speed, type);
	} //end else
} //end of the function BotMoveInDirection
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int Intersection(bvec2_t p1, bvec2_t p2, bvec2_t p3, bvec2_t p4, bvec2_t out)
{
   bfixed x1, dx1, dy1, x2, dx2, dy2, d;

   dx1 = p2[0] - p1[0];
   dy1 = p2[1] - p1[1];
   dx2 = p4[0] - p3[0];
   dy2 = p4[1] - p3[1];

   d = dy1 * dx2 - dx1 * dy2;
   if (d != BFIXED_0)
   {
      x1 = p1[1] * dx1 - p1[0] * dy1;
      x2 = p3[1] * dx2 - p3[0] * dy2;
      out[0] = FIXED_SNAP ((dx1 * x2 - dx2 * x1) / d);
      out[1] = FIXED_SNAP ((dy1 * x2 - dy2 * x1) / d);
		return qtrue;
   } //end if
   else
   {
      return qfalse;
   } //end else
} //end of the function Intersection
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotCheckBlocked(bot_movestate_t *ms, avec3_t dir, int checkbottom, bot_moveresult_t *result)
{
	bvec3_t mins, maxs, end;
	avec3_t up = {AFIXED_0, AFIXED_0, AFIXED_1};
	bsp_trace_t trace;

	//test for entities obstructing the bot's path
	AAS_PresenceTypeBoundingBox(ms->presencetype, mins, maxs);
	//
	if (FIXED_ABS(FIXED_VEC3DOT(dir, up)) < AFIXED(0,7))
	{
		mins[2] += MAKE_BFIXED(sv_maxstep->value); //if the bot can step on
		maxs[2] -= BFIXED(10,0); //a little lower to avoid low ceiling
	} //end if
	FIXED_VEC3MA_R(ms->origin, BFIXED(3,0), dir, end);
	trace = AAS_Trace(ms->origin, mins, maxs, end, ms->entitynum, CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY);
	//if not started in solid and not hitting the world entity
	if (!trace.startsolid && (trace.ent != ENTITYNUM_WORLD && trace.ent != ENTITYNUM_NONE) )
	{
		result->blocked = qtrue;
		result->blockentity = trace.ent;
#ifdef DEBUG
		//botimport.Print(PRT_MESSAGE, "%d: BotCheckBlocked: I'm blocked\n", ms->client);
#endif //DEBUG
	} //end if
	//if not in an area with reachability
	else if (checkbottom && !AAS_AreaReachability(ms->areanum))
	{
		//check if the bot is standing on something
		AAS_PresenceTypeBoundingBox(ms->presencetype, mins, maxs);
		FIXED_VEC3MA_R(ms->origin, -BFIXED(3,0), up, end);
		trace = AAS_Trace(ms->origin, mins, maxs, end, ms->entitynum, CONTENTS_SOLID|CONTENTS_PLAYERCLIP);
		if (!trace.startsolid && (trace.ent != ENTITYNUM_WORLD && trace.ent != ENTITYNUM_NONE) )
		{
			result->blocked = qtrue;
			result->blockentity = trace.ent;
			result->flags |= MOVERESULT_ONTOPOFOBSTACLE;
#ifdef DEBUG
			//botimport.Print(PRT_MESSAGE, "%d: BotCheckBlocked: I'm blocked\n", ms->client);
#endif //DEBUG
		} //end if
	} //end else
} //end of the function BotCheckBlocked
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotClearMoveResult(bot_moveresult_t *moveresult)
{
	moveresult->failure = qfalse;
	moveresult->type = 0;
	moveresult->blocked = qfalse;
	moveresult->blockentity = 0;
	moveresult->traveltype = 0;
	moveresult->flags = 0;
} //end of the function BotClearMoveResult
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Walk(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bfixed dist, speed;
	avec3_t hordir;
	bvec3_t tmp;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//first walk straight to the reachability start
	tmp[0] = reach->start[0] - ms->origin[0];
	tmp[1] = reach->start[1] - ms->origin[1];
	tmp[2] = BFIXED_0;
	dist = VectorNormalizeB2A(tmp,hordir);
	//
	BotCheckBlocked(ms, hordir, qtrue, &result);
	//
	if (dist < BFIXED(10,0))
	{
		//walk straight to the reachability end
		tmp[0] = reach->end[0] - ms->origin[0];
		tmp[1] = reach->end[1] - ms->origin[1];
		tmp[2] = BFIXED_0;
		dist = VectorNormalizeB2A(tmp,hordir);
	} //end if
	//if going towards a crouch area
	if (!(AAS_AreaPresenceType(reach->areanum) & PRESENCE_NORMAL))
	{
		//if pretty close to the reachable area
		if (dist < BFIXED(20,0)) EA_Crouch(ms->client);
	} //end if
	//
	dist = BotGapDistance(ms->origin, hordir, ms->entitynum);
	//
	if (ms->moveflags & MFL_WALK)
	{
		if (dist > BFIXED_0) speed = BFIXED(200,0) - (BFIXED(180,0) - BFIXED_1 * dist);
		else speed = BFIXED(200,0);
		EA_Walk(ms->client);
	} //end if
	else
	{
		if (dist >BFIXED_0) speed = BFIXED(400,0) - (BFIXED(360,0) - BFIXED(2,0) * dist);
		else speed = BFIXED(400,0);
	} //end else
	//elemantary action move in direction
	EA_Move(ms->client, hordir, speed);
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_Walk
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_Walk(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bvec3_t tmp;
	avec3_t hordir;
	bfixed dist, speed;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//if not on the ground and changed areas... don't walk back!!
	//(doesn't seem to help)
	/*
	ms->areanum = BotFuzzyPointReachabilityArea(ms->origin);
	if (ms->areanum == reach->areanum)
	{
#ifdef DEBUG
		botimport.Print(PRT_MESSAGE, "BotFinishTravel_Walk: already in reach area\n");
#endif //DEBUG
		return result;
	} //end if*/
	//go straight to the reachability end
	tmp[0] = reach->end[0] - ms->origin[0];
	tmp[1] = reach->end[1] - ms->origin[1];
	tmp[2] = BFIXED_0;
	dist = VectorNormalizeB2A(tmp,hordir);
	//
	if (dist > BFIXED(100,0)) dist = BFIXED(100,0);
	speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(3,0) * dist);
	//
	EA_Move(ms->client, hordir, speed);
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotFinishTravel_Walk
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Crouch(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bfixed speed;
	avec3_t hordir;
	bvec3_t tmp;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//
	speed = BFIXED(400,0);
	//walk straight to reachability end
	tmp[0] = reach->end[0] - ms->origin[0];
	tmp[1] = reach->end[1] - ms->origin[1];
	tmp[2] = BFIXED_0;
	VectorNormalizeB2A(tmp,hordir);
	//
	BotCheckBlocked(ms, hordir, qtrue, &result);
	//elemantary actions
	EA_Crouch(ms->client);
	EA_Move(ms->client, hordir, speed);
	//
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_Crouch
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_BarrierJump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bfixed dist, speed;
	avec3_t hordir;
	bvec3_t tmp;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//walk straight to reachability start
	tmp[0] = reach->start[0] - ms->origin[0];
	tmp[1] = reach->start[1] - ms->origin[1];
	tmp[2] = BFIXED_0;
	dist = VectorNormalizeB2A(tmp,hordir);
	//
	BotCheckBlocked(ms, hordir, qtrue, &result);
	//if pretty close to the barrier
	if (dist < BFIXED(9,0))
	{
		EA_Jump(ms->client);
	} //end if
	else
	{
		if (dist > BFIXED(60,0)) dist = BFIXED(60,0);
		speed = BFIXED(360,0) - (BFIXED(360,0) - BFIXED(6,0) * dist);
		EA_Move(ms->client, hordir, speed);
	} //end else
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_BarrierJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_BarrierJump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bfixed dist;
	avec3_t hordir;
	bvec3_t tmp;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//if near the top or going down
	if (ms->velocity[2] < BFIXED(250,0))
	{
		tmp[0] = reach->end[0] - ms->origin[0];
		tmp[1] = reach->end[1] - ms->origin[1];
		tmp[2] = BFIXED_0;
		dist = VectorNormalizeB2A(tmp,hordir);
		//
		BotCheckBlocked(ms, hordir, qtrue, &result);
		//
		EA_Move(ms->client, hordir, BFIXED(400,0));
		VectorCopy(hordir, result.movedir);
	} //end if
	//
	return result;
} //end of the function BotFinishTravel_BarrierJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Swim(bot_movestate_t *ms, aas_reachability_t *reach)
{
	avec3_t dir;
	bvec3_t tmp;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//swim straight to reachability end
	VectorSubtract(reach->start, ms->origin, tmp);
	VectorNormalizeB2A(tmp,dir);
	//
	BotCheckBlocked(ms, dir, qtrue, &result);
	//elementary actions
	EA_Move(ms->client, dir, BFIXED(400,0));
	//
	VectorCopy(dir, result.movedir);
	Vector2Angles(dir, result.ideal_viewangles);
	result.flags |= MOVERESULT_SWIMVIEW;
	//
	return result;
} //end of the function BotTravel_Swim
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_WaterJump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bvec3_t bdir, tmp1;
	avec3_t adir, hordir;
	bfixed dist;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//swim straight to reachability end
	VectorSubtract(reach->end, ms->origin, bdir);
	VectorCopy(bdir, tmp1);
	tmp1[2] = BFIXED_0;
	bdir[2] += BFIXED(15,0) + crandom_b() * BFIXED(40,0);
	//botimport.Print(PRT_MESSAGE, "BotTravel_WaterJump: dir[2] = %f\n", FIXED_TO_DOUBLE(dir[2]));
	VectorNormalizeB2A(bdir,adir);
	dist = VectorNormalizeB2A(tmp1,hordir);
	//elemantary actions
	//EA_Move(ms->client, adir, 400);
	EA_MoveForward(ms->client);
	//move up if close to the actual out of water jump spot
	if (dist < BFIXED(40,0)) EA_MoveUp(ms->client);
	//set the ideal view angles
	Vector2Angles(adir, result.ideal_viewangles);
	result.flags |= MOVERESULT_MOVEMENTVIEW;
	//
	VectorCopy(adir, result.movedir);
	//
	return result;
} //end of the function BotTravel_WaterJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_WaterJump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bvec3_t bdir;
	avec3_t adir;
	bvec3_t pnt;
	bfixed dist;
	bot_moveresult_t result;

	//botimport.Print(PRT_MESSAGE, "BotFinishTravel_WaterJump\n");
	BotClearMoveResult(&result);
	//if waterjumping there's nothing to do
	if (ms->moveflags & MFL_WATERJUMP) return result;
	//if not touching any water anymore don't do anything
	//otherwise the bot sometimes keeps jumping?
	VectorCopy(ms->origin, pnt);
	pnt[2] -= BFIXED(32,0);	//extra for q2dm4 near red armor/mega health
	if (!(AAS_PointContents(pnt) & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER))) return result;
	//swim straight to reachability end
	VectorSubtract(reach->end, ms->origin, bdir);
	bdir[0] += crandom_b() * BFIXED(10,0);
	bdir[1] += crandom_b() * BFIXED(10,0);
	bdir[2] += BFIXED(70,0) + crandom_b() * BFIXED(10,0);
	dist = VectorNormalizeB2A(bdir,adir);
	//elemantary actions
	EA_Move(ms->client, adir, BFIXED(400,0));
	//set the ideal view angles
	Vector2Angles(adir, result.ideal_viewangles);
	result.flags |= MOVERESULT_MOVEMENTVIEW;
	//
	VectorCopy(adir, result.movedir);
	//
	return result;
} //end of the function BotFinishTravel_WaterJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_WalkOffLedge(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bvec3_t tmp;
	avec3_t hordir, dir;
	bfixed dist, speed, reachhordist;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//check if the bot is blocked by anything
	VectorSubtract(reach->start, ms->origin, tmp);
	VectorNormalizeB2A(tmp, dir);
	BotCheckBlocked(ms, dir, qtrue, &result);
	//if the reachability start and end are practially above each other
	VectorSubtract(reach->end, reach->start, tmp);
	tmp[2] = BFIXED_0;
	reachhordist = FIXED_VEC3LEN(tmp);
	//walk straight to the reachability start
	tmp[0] = reach->start[0] - ms->origin[0];
	tmp[1] = reach->start[1] - ms->origin[1];
	tmp[2] = BFIXED_0;
	dist = VectorNormalizeB2A(tmp,hordir);
	//if pretty close to the start focus on the reachability end
	if (dist < BFIXED(48,0))
	{
		tmp[0] = reach->end[0] - ms->origin[0];
		tmp[1] = reach->end[1] - ms->origin[1];
		tmp[2] = BFIXED_0;
		VectorNormalizeB2A(tmp,hordir);
		//
		if (reachhordist < BFIXED(20,0))
		{
			speed = BFIXED(100,0);
		} //end if
		else if (!AAS_HorizontalVelocityForJump(BFIXED_0, reach->start, reach->end, &speed))
		{
			speed = BFIXED(400,0);
		} //end if
	} //end if
	else
	{
		if (reachhordist < BFIXED(20,0))
		{
			if (dist > BFIXED(64,0)) dist = BFIXED(64,0);
			speed = BFIXED(400,0) - (BFIXED(256,0) - BFIXED(4,0) * dist);
		} //end if
		else
		{
			speed = BFIXED(400,0);
		} //end else
	} //end else
	//
	BotCheckBlocked(ms, hordir, qtrue, &result);
	//elemantary action
	EA_Move(ms->client, hordir, speed);
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_WalkOffLedge
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotAirControl(bvec3_t origin, bvec3_t velocity, bvec3_t goal, avec3_t dir, bfixed *speed)
{
	bvec3_t org, vel, tmp;
	bfixed dist;
	int i;

	VectorCopy(origin, org);
	FIXED_VEC3SCALE(velocity, BFIXED(0,1), vel);
	for (i = 0; i < 50; i++)
	{
		vel[2] -= MAKE_BFIXED(sv_gravity->value * LFIXED(0,01));
		//if going down and next position would be below the goal
		if (vel[2] < BFIXED_0 && org[2] + vel[2] < goal[2])
		{
			FIXED_VEC3SCALE(vel, (goal[2] - org[2]) / vel[2], vel);
			VectorAdd(org, vel, org);
			VectorSubtract(goal, org, tmp);
			dist = VectorNormalizeB2A(tmp,dir);
			if (dist > BFIXED(32,0)) dist = BFIXED(32,0);
			*speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(13,0) * dist);
			return qtrue;
		} //end if
		else
		{
			VectorAdd(org, vel, org);
		} //end else
	} //end for
	VectorSet(dir, AFIXED_0, AFIXED_0, AFIXED_0);
	*speed = BFIXED(400,0);
	return qfalse;
} //end of the function BotAirControl
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_WalkOffLedge(bot_movestate_t *ms, aas_reachability_t *reach)
{
	avec3_t hordir,av;
	bvec3_t tmp , end, v;
	avec3_t adir;
	bvec3_t bdir;
	bfixed dist, speed;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//
	VectorSubtract(reach->end, ms->origin, bdir);
	VectorNormalizeB2A(bdir,adir);
	BotCheckBlocked(ms, adir, qtrue, &result);
	//
	VectorSubtract(reach->end, ms->origin, v);
	v[2] = BFIXED_0;
	dist = VectorNormalizeB2A(v,av);
	if (dist > BFIXED(16,0)) FIXED_VEC3MA_R(reach->end, BFIXED(16,0), av, end);
	else VectorCopy(reach->end, end);
	//
	if (!BotAirControl(ms->origin, ms->velocity, end, hordir, &speed))
	{
		//go straight to the reachability end
		VectorCopy(bdir, tmp);
		tmp[2] = BFIXED_0;
		//
		dist = VectorNormalizeB2A(tmp,hordir);
		speed = BFIXED(400,0);
	} //end if
	//
	EA_Move(ms->client, hordir, speed);
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotFinishTravel_WalkOffLedge
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
/*
bot_moveresult_t BotTravel_Jump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	avec3_t hordir;
	bfixed dist, gapdist, speed, horspeed, sv_jumpvel;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//
	sv_jumpvel = botlibglobals.sv_jumpvel->value;
	//walk straight to the reachability start
	hordir[0] = reach->start[0] - ms->origin[0];
	hordir[1] = reach->start[1] - ms->origin[1];
	hordir[2] = 0;
	dist = VectorNormalize(hordir);
	//
	speed = 350;
	//
	gapdist = BotGapDistance(ms, hordir, ms->entitynum);
	//if pretty close to the start focus on the reachability end
	if (dist < 50 || (gapdist && gapdist < 50))
	{
		//NOTE: using max speed (400) works best
		//if (AAS_HorizontalVelocityForJump(sv_jumpvel, ms->origin, reach->end, &horspeed))
		//{
		//	speed = horspeed * 400 / botlibglobals.sv_maxwalkvelocity->value;
		//} //end if
		hordir[0] = reach->end[0] - ms->origin[0];
		hordir[1] = reach->end[1] - ms->origin[1];
		VectorNormalize(hordir);
		//elemantary action jump
		EA_Jump(ms->client);
		//
		ms->jumpreach = ms->lastreachnum;
		speed = 600;
	} //end if
	else
	{
		if (AAS_HorizontalVelocityForJump(sv_jumpvel, reach->start, reach->end, &horspeed))
		{
			speed = horspeed * 400 / botlibglobals.sv_maxwalkvelocity->value;
		} //end if
	} //end else
	//elemantary action
	EA_Move(ms->client, hordir, speed);
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_Jump*/
/*
bot_moveresult_t BotTravel_Jump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	avec3_t hordir, dir1, dir2, mins, maxs, start, end;
	bfixed dist1, dist2, speed;
	bot_moveresult_t result;
	bsp_trace_t trace;

	BotClearMoveResult(&result);
	//
	hordir[0] = reach->start[0] - reach->end[0];
	hordir[1] = reach->start[1] - reach->end[1];
	hordir[2] = 0;
	VectorNormalize(hordir);
	//
	VectorCopy(reach->start, start);
	start[2] += 1;
	//minus back the bouding box size plus 16
	FIXED_VEC3MA(reach->start, BFIXED(80,0), hordir, end);
	//
	AAS_PresenceTypeBoundingBox(PRESENCE_NORMAL, mins, maxs);
	//check for solids
	trace = AAS_Trace(start, mins, maxs, end, ms->entitynum, MASK_PLAYERSOLID);
	if (trace.startsolid) VectorCopy(start, trace.endpos);
	//check for a gap
	for (dist1 = 0; dist1 < 80; dist1 += 10)
	{
		FIXED_VEC3MA(start, dist1+BFIXED(10,0), hordir, end);
		end[2] += 1;
		if (AAS_PointAreaNum(end) != ms->reachareanum) break;
	} //end for
	if (dist1 < 80) FIXED_VEC3MA(reach->start, dist1, hordir, trace.endpos);
//	dist1 = BotGapDistance(start, hordir, ms->entitynum);
//	if (dist1 && dist1 <= trace.fraction * 80) FIXED_VEC3MA(reach->start, dist1-BFIXED(20,0), hordir, trace.endpos);
	//
	VectorSubtract(ms->origin, reach->start, dir1);
	dir1[2] = 0;
	dist1 = VectorNormalize(dir1);
	VectorSubtract(ms->origin, trace.endpos, dir2);
	dir2[2] = 0;
	dist2 = VectorNormalize(dir2);
	//if just before the reachability start
	if (FIXED_VEC3DOT(dir1, dir2) < -BFIXED(0,8) || dist2 < 5)
	{
		//botimport.Print(PRT_MESSAGE, "between jump start and run to point\n");
		hordir[0] = reach->end[0] - ms->origin[0];
		hordir[1] = reach->end[1] - ms->origin[1];
		hordir[2] = 0;
		VectorNormalize(hordir);
		//elemantary action jump
		if (dist1 < 24) EA_Jump(ms->client);
		else if (dist1 < 32) EA_DelayedJump(ms->client);
		EA_Move(ms->client, hordir, 600);
		//
		ms->jumpreach = ms->lastreachnum;
	} //end if
	else
	{
		//botimport.Print(PRT_MESSAGE, "going towards run to point\n");
		hordir[0] = trace.endpos[0] - ms->origin[0];
		hordir[1] = trace.endpos[1] - ms->origin[1];
		hordir[2] = 0;
		VectorNormalize(hordir);
		//
		if (dist2 > 80) dist2 = 80;
		speed = 400 - (400 - 5 * dist2);
		EA_Move(ms->client, hordir, speed);
	} //end else
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_Jump*/
//*
bot_moveresult_t BotTravel_Jump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	avec3_t hordir, dir1, dir2;
	bvec3_t runstart,tmp, start, end;
	bfixed dist1, dist2, speed;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//
	AAS_JumpReachRunStart(reach, runstart);
	//*
	tmp[0] = runstart[0] - reach->start[0];
	tmp[1] = runstart[1] - reach->start[1];
	tmp[2] = BFIXED_0;
	VectorNormalizeB2A(tmp,hordir);
	//
	VectorCopy(reach->start, start);
	start[2] += BFIXED_1;
	FIXED_VEC3MA_R(reach->start, BFIXED(80,0), hordir, runstart);
	//check for a gap
	for (dist1 = BFIXED_0; dist1 < BFIXED(80,0); dist1 += BFIXED(10,0))
	{
		FIXED_VEC3MA_R(start, dist1+BFIXED(10,0), hordir, end);
		end[2] += BFIXED_1;
		if (AAS_PointAreaNum(end) != ms->reachareanum) break;
	} //end for
	if (dist1 < BFIXED(80,0)) FIXED_VEC3MA_R(reach->start, dist1, hordir, runstart);
	//
	VectorSubtract(ms->origin, reach->start, tmp);
	tmp[2] = BFIXED_0;
	dist1 = VectorNormalizeB2A(tmp,dir1);
	VectorSubtract(ms->origin, runstart, tmp);
	tmp[2] = BFIXED_0;
	dist2 = VectorNormalizeB2A(tmp,dir2);
	//if just before the reachability start
	if (FIXED_VEC3DOT(dir1, dir2) < -AFIXED(0,8) || dist2 < BFIXED(5,0))
	{
//		botimport.Print(PRT_MESSAGE, "between jump start and run start point\n");
		tmp[0] = reach->end[0] - ms->origin[0];
		tmp[1] = reach->end[1] - ms->origin[1];
		tmp[2] = BFIXED_0;
		VectorNormalizeB2A(tmp,hordir);
		//elemantary action jump
		if (dist1 < BFIXED(24,0)) EA_Jump(ms->client);
		else if (dist1 < BFIXED(32,0)) EA_DelayedJump(ms->client);
		EA_Move(ms->client, hordir, BFIXED(600,0));
		//
		ms->jumpreach = ms->lastreachnum;
	} //end if
	else
	{
//		botimport.Print(PRT_MESSAGE, "going towards run start point\n");
		tmp[0] = runstart[0] - ms->origin[0];
		tmp[1] = runstart[1] - ms->origin[1];
		tmp[2] = BFIXED_0;
		VectorNormalizeB2A(tmp,hordir);
		//
		if (dist2 > BFIXED(80,0)) dist2 = BFIXED(80,0);
		speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(5,0) * dist2);
		EA_Move(ms->client, hordir, speed);
	} //end else
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_Jump*/
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_Jump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bvec3_t tmp;
	avec3_t hordir, hordir2;
	bfixed speed, dist;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//if not jumped yet
	if (!ms->jumpreach) return result;
	//go straight to the reachability end
	tmp[0] = reach->end[0] - ms->origin[0];
	tmp[1] = reach->end[1] - ms->origin[1];
	tmp[2] = BFIXED_0;
	dist = VectorNormalizeB2A(tmp,hordir);
	//
	tmp[0] = reach->end[0] - reach->start[0];
	tmp[1] = reach->end[1] - reach->start[1];
	tmp[2] = BFIXED_0;
	VectorNormalizeB2A(tmp,hordir2);
	//
	if (FIXED_VEC3DOT(hordir, hordir2) < -AFIXED(0,5) && dist < BFIXED(24,0)) return result;
	//always use max speed when traveling through the air
	speed = BFIXED(800,0);
	//
	EA_Move(ms->client, hordir, speed);
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotFinishTravel_Jump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Ladder(bot_movestate_t *ms, aas_reachability_t *reach)
{
	//bfixed dist, speed;
	avec3_t dir, viewdir;//, hordir;
	bvec3_t tmp;
	avec3_t origin = {AFIXED_0, AFIXED_0, AFIXED_0};
//	avec3_t up = {AFIXED_0, AFIXED_0, AFIXED_1};
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//
//	if ((ms->moveflags & MFL_AGAINSTLADDER))
		//NOTE: not a good idea for ladders starting in water
		// || !(ms->moveflags & MFL_ONGROUND))
	{
		//botimport.Print(PRT_MESSAGE, "against ladder or not on ground\n");
		VectorSubtract(reach->end, ms->origin, tmp);
		VectorNormalizeB2A(tmp,dir);
		//set the ideal view angles, facing the ladder up or down
		viewdir[0] = dir[0];
		viewdir[1] = dir[1];
		viewdir[2] = AFIXED(3,0) * dir[2];
		Vector2Angles(viewdir, result.ideal_viewangles);
		//elemantary action
		EA_Move(ms->client, origin, BFIXED_0);
		EA_MoveForward(ms->client);
		//set movement view flag so the AI can see the view is focussed
		result.flags |= MOVERESULT_MOVEMENTVIEW;
	} //end if
/*	else
	{
		//botimport.Print(PRT_MESSAGE, "moving towards ladder\n");
		VectorSubtract(reach->end, ms->origin, dir);
		//make sure the horizontal movement is large anough
		VectorCopy(dir, hordir);
		hordir[2] = 0;
		dist = VectorNormalize(hordir);
		//
		dir[0] = hordir[0];
		dir[1] = hordir[1];
		if (dir[2] > 0) dir[2] = 1;
		else dir[2] = -1;
		if (dist > 50) dist = 50;
		speed = 400 - (200 - 4 * dist);
		EA_Move(ms->client, dir, speed);
	} //end else*/
	//save the movement direction
	VectorCopy(dir, result.movedir);
	//
	return result;
} //end of the function BotTravel_Ladder
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Teleport(bot_movestate_t *ms, aas_reachability_t *reach)
{
	avec3_t hordir;
	bfixed dist;
	bvec3_t tmp;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//if the bot is being teleported
	if (ms->moveflags & MFL_TELEPORTED) return result;

	//walk straight to center of the teleporter
	VectorSubtract(reach->start, ms->origin, tmp);
	if (!(ms->moveflags & MFL_SWIMMING)) tmp[2] = BFIXED_0;
	dist = VectorNormalizeB2A(tmp,hordir);
	//
	BotCheckBlocked(ms, hordir, qtrue, &result);

	if (dist < BFIXED(30,0)) EA_Move(ms->client, hordir, BFIXED(200,0));
	else EA_Move(ms->client, hordir, BFIXED(400,0));

	if (ms->moveflags & MFL_SWIMMING) result.flags |= MOVERESULT_SWIMVIEW;

	VectorCopy(hordir, result.movedir);
	return result;
} //end of the function BotTravel_Teleport
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Elevator(bot_movestate_t *ms, aas_reachability_t *reach)
{
	avec3_t dir, dir1, dir2, hordir;
	bvec3_t tmp, bottomcenter, bdir;
	bfixed dist, dist1, dist2, speed;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//if standing on the plat
	if (BotOnMover(ms->origin, ms->entitynum, reach))
	{
#ifdef DEBUG_ELEVATOR
		botimport.Print(PRT_MESSAGE, "bot on elevator\n");
#endif //DEBUG_ELEVATOR
		//if vertically not too far from the end point
		if (FIXED_ABS(ms->origin[2] - reach->end[2]) < MAKE_BFIXED(sv_maxbarrier->value))
		{
#ifdef DEBUG_ELEVATOR
			botimport.Print(PRT_MESSAGE, "bot moving to end\n");
#endif //DEBUG_ELEVATOR
			//move to the end point
			VectorSubtract(reach->end, ms->origin, tmp);
			tmp[2] = BFIXED_0;
			VectorNormalizeB2A(tmp,hordir);
			if (!BotCheckBarrierJump(ms, hordir, BFIXED(100,0)))
			{
				EA_Move(ms->client, hordir, BFIXED(400,0));
			} //end if
			VectorCopy(hordir, result.movedir);
		} //end else
		//if not really close to the center of the elevator
		else
		{
			MoverBottomCenter(reach, bottomcenter);
			VectorSubtract(bottomcenter, ms->origin, tmp);
			tmp[2] = BFIXED_0;
			dist = VectorNormalizeB2A(tmp,hordir);
			//
			if (dist > BFIXED(10,0))
			{
#ifdef DEBUG_ELEVATOR
				botimport.Print(PRT_MESSAGE, "bot moving to center\n");
#endif //DEBUG_ELEVATOR
				//move to the center of the plat
				if (dist > BFIXED(100,0)) dist = BFIXED(100,0);
				speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(4,0) * dist);
				//
				EA_Move(ms->client, hordir, speed);
				VectorCopy(hordir, result.movedir);
			} //end if
		} //end else
	} //end if
	else
	{
#ifdef DEBUG_ELEVATOR
		botimport.Print(PRT_MESSAGE, "bot not on elevator\n");
#endif //DEBUG_ELEVATOR
		//if very near the reachability end
		VectorSubtract(reach->end, ms->origin, bdir);
		dist = FIXED_VEC3LEN(bdir);
		if (dist < BFIXED(64,0))
		{
			if (dist > BFIXED(60,0)) dist = BFIXED(60,0);
			speed = BFIXED(360,0) - (BFIXED(360,0) - BFIXED(6,0) * dist);
			//
			if ((ms->moveflags & MFL_SWIMMING) || !BotCheckBarrierJump(ms, dir, BFIXED(50,0)))
			{
				if (speed > BFIXED(5,0)) EA_Move(ms->client, dir, speed);
			} //end if
			VectorCopy(dir, result.movedir);
			//
			if (ms->moveflags & MFL_SWIMMING) result.flags |= MOVERESULT_SWIMVIEW;
			//stop using this reachability
			ms->reachability_time = GFIXED_0;
			return result;
		} //end if
		//get direction and distance to reachability start
		VectorSubtract(reach->start, ms->origin, tmp);
		if (!(ms->moveflags & MFL_SWIMMING)) tmp[2] = BFIXED_0;
		dist1 = VectorNormalizeB2A(tmp, dir1);
		//if the elevator isn't down
		if (!MoverDown(reach))
		{
#ifdef DEBUG_ELEVATOR
			botimport.Print(PRT_MESSAGE, "elevator not down\n");
#endif //DEBUG_ELEVATOR
			dist = dist1;
			VectorCopy(dir1, dir);
			//
			BotCheckBlocked(ms, dir, qfalse, &result);
			//
			if (dist > BFIXED(60,0)) dist = BFIXED(60,0);
			speed = BFIXED(360,0) - (BFIXED(360,0) - BFIXED(6,0) * dist);
			//
			if (!(ms->moveflags & MFL_SWIMMING) && !BotCheckBarrierJump(ms, dir, BFIXED(50,0)))
			{
				if (speed > BFIXED(5,0)) EA_Move(ms->client, dir, speed);
			} //end if
			VectorCopy(dir, result.movedir);
			//
			if (ms->moveflags & MFL_SWIMMING) result.flags |= MOVERESULT_SWIMVIEW;
			//this isn't a failure... just wait till the elevator comes down
			result.type = RESULTTYPE_ELEVATORUP;
			result.flags |= MOVERESULT_WAITING;
			return result;
		} //end if
		//get direction and distance to elevator bottom center
		MoverBottomCenter(reach, bottomcenter);
		VectorSubtract(bottomcenter, ms->origin, tmp);
		if (!(ms->moveflags & MFL_SWIMMING)) tmp[2] = BFIXED_0;
		dist2 = VectorNormalizeB2A(tmp,dir2);
		//if very close to the reachability start or
		//closer to the elevator center or
		//between reachability start and elevator center
		if (dist1 < BFIXED(20,0) || dist2 < dist1 || FIXED_VEC3DOT(dir1, dir2) < AFIXED_0)
		{
#ifdef DEBUG_ELEVATOR
			botimport.Print(PRT_MESSAGE, "bot moving to center\n");
#endif //DEBUG_ELEVATOR
			dist = dist2;
			VectorCopy(dir2, dir);
		} //end if
		else //closer to the reachability start
		{
#ifdef DEBUG_ELEVATOR
			botimport.Print(PRT_MESSAGE, "bot moving to start\n");
#endif //DEBUG_ELEVATOR
			dist = dist1;
			VectorCopy(dir1, dir);
		} //end else
		//
		BotCheckBlocked(ms, dir, qfalse, &result);
		//
		if (dist > BFIXED(60,0)) dist = BFIXED(60,0);
		speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(6,0) * dist);
		//
		if (!(ms->moveflags & MFL_SWIMMING) && !BotCheckBarrierJump(ms, dir, BFIXED(50,0)))
		{
			EA_Move(ms->client, dir, speed);
		} //end if
		VectorCopy(dir, result.movedir);
		//
		if (ms->moveflags & MFL_SWIMMING) result.flags |= MOVERESULT_SWIMVIEW;
	} //end else
	return result;
} //end of the function BotTravel_Elevator
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_Elevator(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bvec3_t bottomcenter;
	bvec3_t bdtmp,tdtmp;
	avec3_t bottomdir, topdir;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//
	MoverBottomCenter(reach, bottomcenter);
	VectorSubtract(bottomcenter, ms->origin, bdtmp);
	//
	VectorSubtract(reach->end, ms->origin, tdtmp);
	//
	if (FIXED_ABS(bdtmp[2]) < FIXED_ABS(tdtmp[2]))
	{
		VectorNormalizeB2A(bdtmp,bottomdir);
		EA_Move(ms->client, bottomdir, BFIXED(300,0));
	} //end if
	else
	{
		VectorNormalizeB2A(tdtmp,topdir);
		EA_Move(ms->client, topdir, BFIXED(300,0));
	} //end else
	return result;
} //end of the function BotFinishTravel_Elevator
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotFuncBobStartEnd(aas_reachability_t *reach, bvec3_t start, bvec3_t end, bvec3_t origin)
{
	int spawnflags, modelnum;
	bvec3_t mins, maxs, mid;
	avec3_t angles = {AFIXED_0, AFIXED_0, AFIXED_0};
	int num0, num1;

	modelnum = reach->facenum & 0x0000FFFF;
	if (!AAS_OriginOfMoverWithModelNum(modelnum, origin))
	{
		botimport.Print(PRT_MESSAGE, "BotFuncBobStartEnd: no entity with model %d\n", modelnum);
		VectorSet(start, BFIXED_0, BFIXED_0, BFIXED_0);
		VectorSet(end, BFIXED_0, BFIXED_0, BFIXED_0);
		return;
	} //end if
	AAS_BSPModelMinsMaxsOrigin(modelnum, angles, mins, maxs, NULL);
	VectorAdd(mins, maxs, mid);
	FIXED_VEC3SCALE(mid, BFIXED(0,5), mid);
	VectorCopy(mid, start);
	VectorCopy(mid, end);
	spawnflags = reach->facenum >> 16;
	num0 = reach->edgenum >> 16;
	if (num0 > 0x00007FFF) num0 |= 0xFFFF0000;
	num1 = reach->edgenum & 0x0000FFFF;
	if (num1 > 0x00007FFF) num1 |= 0xFFFF0000;
	if (spawnflags & 1)
	{
		start[0] = MAKE_BFIXED(num0);
		end[0] = MAKE_BFIXED(num1);
		//
		origin[0] += mid[0];
		origin[1] = mid[1];
		origin[2] = mid[2];
	} //end if
	else if (spawnflags & 2)
	{
		start[1] = MAKE_BFIXED(num0);
		end[1] = MAKE_BFIXED(num1);
		//
		origin[0] = mid[0];
		origin[1] += mid[1];
		origin[2] = mid[2];
	} //end else if
	else
	{
		start[2] = MAKE_BFIXED(num0);
		end[2] = MAKE_BFIXED(num1);
		//
		origin[0] = mid[0];
		origin[1] = mid[1];
		origin[2] += mid[2];
	} //end else
} //end of the function BotFuncBobStartEnd
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_FuncBobbing(bot_movestate_t *ms, aas_reachability_t *reach)
{
	avec3_t dir, dir1, dir2, hordir;
	bvec3_t bottomcenter, bob_start, bob_end, bob_origin, tmp;
	bfixed dist, dist1, dist2, speed;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//
	BotFuncBobStartEnd(reach, bob_start, bob_end, bob_origin);
	//if standing ontop of the func_bobbing
	if (BotOnMover(ms->origin, ms->entitynum, reach))
	{
#ifdef DEBUG_FUNCBOB
		botimport.Print(PRT_MESSAGE, "bot on func_bobbing\n");
#endif
		//if near end point of reachability
		VectorSubtract(bob_origin, bob_end, tmp);
		if (FIXED_VEC3LEN(tmp) < BFIXED(24,0))
		{
#ifdef DEBUG_FUNCBOB
			botimport.Print(PRT_MESSAGE, "bot moving to reachability end\n");
#endif
			//move to the end point
			VectorSubtract(reach->end, ms->origin, tmp);
			tmp[2] = BFIXED_0;
			VectorNormalizeB2A(tmp,hordir);
			if (!BotCheckBarrierJump(ms, hordir, BFIXED(100,0)))
			{
				EA_Move(ms->client, hordir, BFIXED(400,0));
			} //end if
			VectorCopy(hordir, result.movedir);
		} //end else
		//if not really close to the center of the elevator
		else
		{
			MoverBottomCenter(reach, bottomcenter);
			VectorSubtract(bottomcenter, ms->origin, tmp);
			tmp[2] = BFIXED_0;
			dist = VectorNormalizeB2A(tmp,hordir);
			//
			if (dist > BFIXED(10,0))
			{
#ifdef DEBUG_FUNCBOB
				botimport.Print(PRT_MESSAGE, "bot moving to func_bobbing center\n");
#endif
				//move to the center of the plat
				if (dist > BFIXED(100,0)) dist = BFIXED(100,0);
				speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(4,0) * dist);
				//
				EA_Move(ms->client, hordir, speed);
				VectorCopy(hordir, result.movedir);
			} //end if
		} //end else
	} //end if
	else
	{
#ifdef DEBUG_FUNCBOB
		botimport.Print(PRT_MESSAGE, "bot not ontop of func_bobbing\n");
#endif
		//if very near the reachability end
		VectorSubtract(reach->end, ms->origin, tmp);
		dist = FIXED_VEC3LEN(tmp);
		if (dist < BFIXED(64,0))
		{
#ifdef DEBUG_FUNCBOB
			botimport.Print(PRT_MESSAGE, "bot moving to end\n");
#endif
			VectorNormalizeB2A(tmp,dir);

			if (dist > BFIXED(60,0)) dist = BFIXED(60,0);
			speed = BFIXED(360,0) - (BFIXED(360,0) - BFIXED(6,0) * dist);
			//if swimming or no barrier jump
			if ((ms->moveflags & MFL_SWIMMING) || !BotCheckBarrierJump(ms, dir, BFIXED(50,0)))
			{
				if (speed > BFIXED(5,0)) EA_Move(ms->client, dir, speed);
			} //end if
			VectorCopy(dir, result.movedir);
			//
			if (ms->moveflags & MFL_SWIMMING) result.flags |= MOVERESULT_SWIMVIEW;
			//stop using this reachability
			ms->reachability_time = GFIXED_0;
			return result;
		} //end if
		//get direction and distance to reachability start
		VectorSubtract(reach->start, ms->origin, tmp);
		if (!(ms->moveflags & MFL_SWIMMING)) tmp[2] = BFIXED_0;
		dist1 = VectorNormalizeB2A(tmp,dir1);
		//if func_bobbing is Not it's start position
		VectorSubtract(bob_origin, bob_start, tmp);
		if (FIXED_VEC3LEN(tmp) > BFIXED(16,0))
		{
#ifdef DEBUG_FUNCBOB
			botimport.Print(PRT_MESSAGE, "func_bobbing not at start\n");
#endif
			dist = dist1;
			VectorCopy(dir1, dir);
			//
			BotCheckBlocked(ms, dir, qfalse, &result);
			//
			if (dist > BFIXED(60,0)) dist = BFIXED(60,0);
			speed = BFIXED(360,0) - (BFIXED(360,0) - BFIXED(6,0) * dist);
			//
			if (!(ms->moveflags & MFL_SWIMMING) && !BotCheckBarrierJump(ms, dir, BFIXED(50,0)))
			{
				if (speed > BFIXED(5,0)) EA_Move(ms->client, dir, speed);
			} //end if
			VectorCopy(dir, result.movedir);
			//
			if (ms->moveflags & MFL_SWIMMING) result.flags |= MOVERESULT_SWIMVIEW;
			//this isn't a failure... just wait till the func_bobbing arrives
			result.type = RESULTTYPE_WAITFORFUNCBOBBING;
			result.flags |= MOVERESULT_WAITING;
			return result;
		} //end if
		//get direction and distance to func_bob bottom center
		MoverBottomCenter(reach, bottomcenter);
		VectorSubtract(bottomcenter, ms->origin, tmp);
		if (!(ms->moveflags & MFL_SWIMMING)) tmp[2] = BFIXED_0;
		dist2 = VectorNormalizeB2A(tmp,dir2);
		//if very close to the reachability start or
		//closer to the elevator center or
		//between reachability start and func_bobbing center
		if (dist1 < BFIXED(20,0) || dist2 < dist1 || FIXED_VEC3DOT(dir1, dir2) < AFIXED_0)
		{
#ifdef DEBUG_FUNCBOB
			botimport.Print(PRT_MESSAGE, "bot moving to func_bobbing center\n");
#endif
			dist = dist2;
			VectorCopy(dir2, dir);
		} //end if
		else //closer to the reachability start
		{
#ifdef DEBUG_FUNCBOB
			botimport.Print(PRT_MESSAGE, "bot moving to reachability start\n");
#endif
			dist = dist1;
			VectorCopy(dir1, dir);
		} //end else
		//
		BotCheckBlocked(ms, dir, qfalse, &result);
		//
		if (dist > BFIXED(60,0)) dist = BFIXED(60,0);
		speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(6,0) * dist);
		//
		if (!(ms->moveflags & MFL_SWIMMING) && !BotCheckBarrierJump(ms, dir, BFIXED(50,0)))
		{
			EA_Move(ms->client, dir, speed);
		} //end if
		VectorCopy(dir, result.movedir);
		//
		if (ms->moveflags & MFL_SWIMMING) result.flags |= MOVERESULT_SWIMVIEW;
	} //end else
	return result;
} //end of the function BotTravel_FuncBobbing
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_FuncBobbing(bot_movestate_t *ms, aas_reachability_t *reach)
{
	avec3_t dir, hordir;
	bvec3_t bob_origin, bob_start, bob_end, bottomcenter,tmp;
	bot_moveresult_t result;
	bfixed dist, speed;

	BotClearMoveResult(&result);
	//
	BotFuncBobStartEnd(reach, bob_start, bob_end, bob_origin);
	//
	VectorSubtract(bob_origin, bob_end, tmp);
	dist = FIXED_VEC3LEN(tmp);
	//if the func_bobbing is near the end
	if (dist < BFIXED(16,0))
	{
		VectorNormalizeB2A(tmp,dir);
		VectorSubtract(reach->end, ms->origin, tmp);
		if (!(ms->moveflags & MFL_SWIMMING)) tmp[2] = BFIXED_0;
		dist = VectorNormalizeB2A(tmp,hordir);
		//
		if (dist > BFIXED(60,0)) dist = BFIXED(60,0);
		speed = BFIXED(360,0) - (BFIXED(360,0) - BFIXED(6,0) * dist);
		//
		if (speed > BFIXED(5,0)) EA_Move(ms->client, dir, speed);
		VectorCopy(dir, result.movedir);
		//
		if (ms->moveflags & MFL_SWIMMING) result.flags |= MOVERESULT_SWIMVIEW;
	} //end if
	else
	{
		MoverBottomCenter(reach, bottomcenter);
		VectorSubtract(bottomcenter, ms->origin, tmp);
		if (!(ms->moveflags & MFL_SWIMMING)) tmp[2] = BFIXED_0;
		dist = VectorNormalizeB2A(tmp,hordir);
		//
		if (dist > BFIXED(5,0))
		{
			//move to the center of the plat
			if (dist > BFIXED(100,0)) dist = BFIXED(100,0);
			speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(4,0) * dist);
			//
			EA_Move(ms->client, hordir, speed);
			VectorCopy(hordir, result.movedir);
		} //end if
	} //end else
	return result;
} //end of the function BotFinishTravel_FuncBobbing
//===========================================================================
// 0  no valid grapple hook visible
// 1  the grapple hook is still flying
// 2  the grapple hooked into a wall
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int GrappleState(bot_movestate_t *ms, aas_reachability_t *reach)
{
	int i;
	aas_entityinfo_t entinfo;

	//if the grapple hook is pulling
	if (ms->moveflags & MFL_GRAPPLEPULL)
		return 2;
	//check for a visible grapple missile entity
	//or visible grapple entity
	for (i = AAS_NextEntity(0); i; i = AAS_NextEntity(i))
	{
		if (AAS_EntityType(i) == FIXED_TO_INT(entitytypemissile->value))
		{
			AAS_EntityInfo(i, &entinfo);
			if (entinfo.weapon == FIXED_TO_INT(weapindex_grapple->value))
			{
				return 1;
			} //end if
		} //end if
	} //end for
	//no valid grapple at all
	return 0;
} //end of the function GrappleState
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotResetGrapple(bot_movestate_t *ms)
{
	aas_reachability_t reach;

	AAS_ReachabilityFromNum(ms->lastreachnum, &reach);
	//if not using the grapple hook reachability anymore
	if ((reach.traveltype & TRAVELTYPE_MASK) != TRAVEL_GRAPPLEHOOK)
	{
		if ((ms->moveflags & MFL_ACTIVEGRAPPLE) || FIXED_NOT_ZERO(ms->grapplevisible_time))
		{
			if (FIXED_NOT_ZERO(offhandgrapple->value))
				EA_Command(ms->client, cmd_grappleoff->string);
			ms->moveflags &= ~MFL_ACTIVEGRAPPLE;
			ms->grapplevisible_time = GFIXED_0;
#ifdef DEBUG_GRAPPLE
			botimport.Print(PRT_MESSAGE, "reset grapple\n");
#endif //DEBUG_GRAPPLE
		} //end if
	} //end if
} //end of the function BotResetGrapple
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Grapple(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bot_moveresult_t result;
	bfixed dist, speed;
	avec3_t dir;
	bvec3_t tmp, org, viewdir;
	int state, areanum;
	bsp_trace_t trace;

#ifdef DEBUG_GRAPPLE
	static int debugline;
	if (!debugline) debugline = botimport.DebugLineCreate();
	botimport.DebugLineShow(debugline, reach->start, reach->end, LINECOLOR_BLUE);
#endif //DEBUG_GRAPPLE

	BotClearMoveResult(&result);
	//
	if (ms->moveflags & MFL_GRAPPLERESET)
	{
		if (FIXED_NOT_ZERO(offhandgrapple->value))
			EA_Command(ms->client, cmd_grappleoff->string);
		ms->moveflags &= ~MFL_ACTIVEGRAPPLE;
		return result;
	} //end if
	//
	if (!FIXED_TO_INT(offhandgrapple->value))
	{
		result.weapon = FIXED_TO_INT(weapindex_grapple->value);
		result.flags |= MOVERESULT_MOVEMENTWEAPON;
	} //end if
	//
	if (ms->moveflags & MFL_ACTIVEGRAPPLE)
	{
#ifdef DEBUG_GRAPPLE
		botimport.Print(PRT_MESSAGE, "BotTravel_Grapple: active grapple\n");
#endif //DEBUG_GRAPPLE
		//
		state = GrappleState(ms, reach);
		//
		VectorSubtract(reach->end, ms->origin, tmp);
		tmp[2] = BFIXED_0;
		dist = FIXED_VEC3LEN(tmp);
		//if very close to the grapple end or the grappled is hooked and
		//the bot doesn't get any closer
		if (state && dist < BFIXED(48,0))
		{
			if (ms->lastgrappledist - dist < BFIXED_1)
			{
#ifdef DEBUG_GRAPPLE
				botimport.Print(PRT_ERROR, "grapple normal end\n");
#endif //DEBUG_GRAPPLE
				if (FIXED_NOT_ZERO(offhandgrapple->value))
					EA_Command(ms->client, cmd_grappleoff->string);
				ms->moveflags &= ~MFL_ACTIVEGRAPPLE;
				ms->moveflags |= MFL_GRAPPLERESET;
				ms->reachability_time = GFIXED_0;	//end the reachability
				return result;
			} //end if
		} //end if
		//if no valid grapple at all, or the grapple hooked and the bot
		//isn't moving anymore
		else if (!state || (state == 2 && dist > ms->lastgrappledist - BFIXED(2,0)))
		{
			if (ms->grapplevisible_time < AAS_Time() - GFIXED(0,4))
			{
#ifdef DEBUG_GRAPPLE
				botimport.Print(PRT_ERROR, "grapple not visible\n");
#endif //DEBUG_GRAPPLE
				if (FIXED_NOT_ZERO(offhandgrapple->value))
					EA_Command(ms->client, cmd_grappleoff->string);
				ms->moveflags &= ~MFL_ACTIVEGRAPPLE;
				ms->moveflags |= MFL_GRAPPLERESET;
				ms->reachability_time = GFIXED_0;	//end the reachability
				return result;
			} //end if
		} //end if
		else
		{
			ms->grapplevisible_time = AAS_Time();
		} //end else
		//
		if (!FIXED_TO_INT(offhandgrapple->value))
		{
			EA_Attack(ms->client);
		} //end if
		//remember the current grapple distance
		ms->lastgrappledist = dist;
	} //end if
	else
	{
#ifdef DEBUG_GRAPPLE
		botimport.Print(PRT_MESSAGE, "BotTravel_Grapple: inactive grapple\n");
#endif //DEBUG_GRAPPLE
		//
		ms->grapplevisible_time = AAS_Time();
		//
		VectorSubtract(reach->start, ms->origin, tmp);
		if (!(ms->moveflags & MFL_SWIMMING)) tmp[2] = BFIXED_0;
		dist = VectorNormalizeB2A(tmp,dir);
		
		VectorAdd(ms->origin, ms->viewoffset, org);
		
		VectorSubtract(reach->end, org, viewdir);
		//
		Vector2Angles(viewdir, result.ideal_viewangles);
		result.flags |= MOVERESULT_MOVEMENTVIEW;
		//
		if (dist < BFIXED(5,0) &&
			FIXED_ABS(AngleDiff(result.ideal_viewangles[0], ms->viewangles[0])) < AFIXED(2,0) &&
			FIXED_ABS(AngleDiff(result.ideal_viewangles[1], ms->viewangles[1])) < AFIXED(2,0))
		{
#ifdef DEBUG_GRAPPLE
			botimport.Print(PRT_MESSAGE, "BotTravel_Grapple: activating grapple\n");
#endif //DEBUG_GRAPPLE
			//check if the grapple missile path is clear
			VectorAdd(ms->origin, ms->viewoffset, org);
			trace = AAS_Trace(org, NULL, NULL, reach->end, ms->entitynum, CONTENTS_SOLID);
			VectorSubtract(reach->end, trace.endpos, tmp);
			if (FIXED_VEC3LEN(tmp) > BFIXED(16,0))
			{
				result.failure = qtrue;
				return result;
			} //end if
			//activate the grapple
			if (FIXED_NOT_ZERO(offhandgrapple->value))
			{
				EA_Command(ms->client, cmd_grappleon->string);
			} //end if
			else
			{
				EA_Attack(ms->client);
			} //end else
			ms->moveflags |= MFL_ACTIVEGRAPPLE;
			ms->lastgrappledist = BFIXED(999999,0);
		} //end if
		else
		{
			if (dist < BFIXED(70,0)) speed = BFIXED(300,0) - (BFIXED(300,0) - BFIXED(4,0) * dist);
			else speed = BFIXED(400,0);
			//
			BotCheckBlocked(ms, dir, qtrue, &result);
			//elemantary action move in direction
			EA_Move(ms->client, dir, speed);
			VectorCopy(dir, result.movedir);
		} //end else
		//if in another area before actually grappling
		areanum = AAS_PointAreaNum(ms->origin);
		if (areanum && areanum != ms->reachareanum) ms->reachability_time = GFIXED_0;
	} //end else
	return result;
} //end of the function BotTravel_Grapple
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:			-
//===========================================================================
bot_moveresult_t BotTravel_RocketJump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bvec3_t tmp;
	avec3_t hordir;
	bfixed dist, speed;
	bot_moveresult_t result;

	//botimport.Print(PRT_MESSAGE, "BotTravel_RocketJump: bah\n");
	BotClearMoveResult(&result);
	//
	tmp[0] = reach->start[0] - ms->origin[0];
	tmp[1] = reach->start[1] - ms->origin[1];
	tmp[2] = BFIXED_0;
	//
	dist = VectorNormalizeB2A(tmp,hordir);
	//look in the movement direction
	Vector2Angles(hordir, result.ideal_viewangles);
	//look straight down
	result.ideal_viewangles[PITCH] = AFIXED(90,0);
	//
	if (dist < BFIXED(5,0) &&
			FIXED_ABS(AngleDiff(result.ideal_viewangles[0], ms->viewangles[0])) < AFIXED(5,0) &&
			FIXED_ABS(AngleDiff(result.ideal_viewangles[1], ms->viewangles[1])) < AFIXED(5,0) )
	{
		//botimport.Print(PRT_MESSAGE, "between jump start and run start point\n");
		tmp[0] = reach->end[0] - ms->origin[0];
		tmp[1] = reach->end[1] - ms->origin[1];
		tmp[2] = BFIXED_0;
		VectorNormalizeB2A(tmp,hordir);
		//elemantary action jump
		EA_Jump(ms->client);
		EA_Attack(ms->client);
		EA_Move(ms->client, hordir, BFIXED(800,0));
		//
		ms->jumpreach = ms->lastreachnum;
	} //end if
	else
	{
		if (dist > BFIXED(80,0)) dist = BFIXED(80,0);
		speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(5,0) * dist);
		EA_Move(ms->client, hordir, speed);
	} //end else
	//look in the movement direction
	Vector2Angles(hordir, result.ideal_viewangles);
	//look straight down
	result.ideal_viewangles[PITCH] = AFIXED(90,0);
	//set the view angles directly
	EA_View(ms->client, result.ideal_viewangles);
	//view is important for the movment
	result.flags |= MOVERESULT_MOVEMENTVIEWSET;
	//select the rocket launcher
	EA_SelectWeapon(ms->client, FIXED_TO_INT(weapindex_rocketlauncher->value));
	//weapon is used for movement
	result.weapon = FIXED_TO_INT(weapindex_rocketlauncher->value);
	result.flags |= MOVERESULT_MOVEMENTWEAPON;
	//
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_RocketJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_BFGJump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bvec3_t tmp;
	avec3_t hordir;
	bfixed dist, speed;
	bot_moveresult_t result;

	//botimport.Print(PRT_MESSAGE, "BotTravel_BFGJump: bah\n");
	BotClearMoveResult(&result);
	//
	tmp[0] = reach->start[0] - ms->origin[0];
	tmp[1] = reach->start[1] - ms->origin[1];
	tmp[2] = BFIXED_0;
	//
	dist = VectorNormalizeB2A(tmp,hordir);
	//
	if (dist < BFIXED(5,0) &&
			FIXED_ABS(AngleDiff(result.ideal_viewangles[0], ms->viewangles[0])) < AFIXED(5,0) &&
			FIXED_ABS(AngleDiff(result.ideal_viewangles[1], ms->viewangles[1])) < AFIXED(5,0))
	{
		//botimport.Print(PRT_MESSAGE, "between jump start and run start point\n");
		tmp[0] = reach->end[0] - ms->origin[0];
		tmp[1] = reach->end[1] - ms->origin[1];
		tmp[2] = BFIXED_0;
		VectorNormalizeB2A(tmp,hordir);
		//elemantary action jump
		EA_Jump(ms->client);
		EA_Attack(ms->client);
		EA_Move(ms->client, hordir, BFIXED(800,0));
		//
		ms->jumpreach = ms->lastreachnum;
	} //end if
	else
	{
		if (dist > BFIXED(80,0)) dist = BFIXED(80,0);
		speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(5,0) * dist);
		EA_Move(ms->client, hordir, speed);
	} //end else
	//look in the movement direction
	Vector2Angles(hordir, result.ideal_viewangles);
	//look straight down
	result.ideal_viewangles[PITCH] = AFIXED(90,0);
	//set the view angles directly
	EA_View(ms->client, result.ideal_viewangles);
	//view is important for the movment
	result.flags |= MOVERESULT_MOVEMENTVIEWSET;
	//select the rocket launcher
	EA_SelectWeapon(ms->client, FIXED_TO_INT(weapindex_bfg10k->value));
	//weapon is used for movement
	result.weapon = FIXED_TO_INT(weapindex_bfg10k->value);
	result.flags |= MOVERESULT_MOVEMENTWEAPON;
	//
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_BFGJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_WeaponJump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bvec3_t tmp;
	avec3_t hordir;
	bfixed speed;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//if not jumped yet
	if (!ms->jumpreach) return result;
	/*
	//go straight to the reachability end
	hordir[0] = reach->end[0] - ms->origin[0];
	hordir[1] = reach->end[1] - ms->origin[1];
	hordir[2] = 0;
	VectorNormalize(hordir);
	//always use max speed when traveling through the air
	EA_Move(ms->client, hordir, 800);
	VectorCopy(hordir, result.movedir);
	*/
	//
	if (!BotAirControl(ms->origin, ms->velocity, reach->end, hordir, &speed))
	{
		//go straight to the reachability end
		VectorSubtract(reach->end, ms->origin, tmp);
		tmp[2] = BFIXED_0;
		VectorNormalizeB2A(tmp,hordir);
		speed = BFIXED(400,0);
	} //end if
	//
	EA_Move(ms->client, hordir, speed);
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotFinishTravel_WeaponJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_JumpPad(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bfixed dist, speed;
	bvec3_t tmp;
	avec3_t hordir;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//first walk straight to the reachability start
	tmp[0] = reach->start[0] - ms->origin[0];
	tmp[1] = reach->start[1] - ms->origin[1];
	tmp[2] = BFIXED_0;
	dist = VectorNormalizeB2A(tmp,hordir);
	//
	BotCheckBlocked(ms, hordir, qtrue, &result);
	speed = BFIXED(400,0);
	//elemantary action move in direction
	EA_Move(ms->client, hordir, speed);
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_JumpPad
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_JumpPad(bot_movestate_t *ms, aas_reachability_t *reach)
{
	bfixed speed;
	bvec3_t tmp;
	avec3_t hordir;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	if (!BotAirControl(ms->origin, ms->velocity, reach->end, hordir, &speed))
	{
		tmp[0] = reach->end[0] - ms->origin[0];
		tmp[1] = reach->end[1] - ms->origin[1];
		tmp[2] = BFIXED_0;
		VectorNormalizeB2A(tmp,hordir);
		speed = BFIXED(400,0);
	} //end if
	BotCheckBlocked(ms, hordir, qtrue, &result);
	//elemantary action move in direction
	EA_Move(ms->client, hordir, speed);
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotFinishTravel_JumpPad
//===========================================================================
// time before the reachability times out
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int BotReachabilityTime(aas_reachability_t *reach)
{
	switch(reach->traveltype & TRAVELTYPE_MASK)
	{
		case TRAVEL_WALK: return 5;
		case TRAVEL_CROUCH: return 5;
		case TRAVEL_BARRIERJUMP: return 5;
		case TRAVEL_LADDER: return 6;
		case TRAVEL_WALKOFFLEDGE: return 5;
		case TRAVEL_JUMP: return 5;
		case TRAVEL_SWIM: return 5;
		case TRAVEL_WATERJUMP: return 5;
		case TRAVEL_TELEPORT: return 5;
		case TRAVEL_ELEVATOR: return 10;
		case TRAVEL_GRAPPLEHOOK: return 8;
		case TRAVEL_ROCKETJUMP: return 6;
		case TRAVEL_BFGJUMP: return 6;
		case TRAVEL_JUMPPAD: return 10;
		case TRAVEL_FUNCBOB: return 10;
		default:
		{
			botimport.Print(PRT_ERROR, "travel type %d not implemented yet\n", reach->traveltype);
			return 8;
		} //end case
	} //end switch
} //end of the function BotReachabilityTime
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotMoveInGoalArea(bot_movestate_t *ms, bot_goal_t *goal)
{
	bot_moveresult_t result;
	bvec3_t tmp;
	avec3_t dir;
	bfixed dist, speed;

#ifdef DEBUG
	//botimport.Print(PRT_MESSAGE, "%s: moving straight to goal\n", ClientName(ms->entitynum-1));
	//AAS_ClearShownDebugLines();
	//AAS_DebugLine(ms->origin, goal->origin, LINECOLOR_RED);
#endif //DEBUG
	BotClearMoveResult(&result);
	//walk straight to the goal origin
	tmp[0] = goal->origin[0] - ms->origin[0];
	tmp[1] = goal->origin[1] - ms->origin[1];
	if (ms->moveflags & MFL_SWIMMING)
	{
		tmp[2] = goal->origin[2] - ms->origin[2];
		result.traveltype = TRAVEL_SWIM;
	} //end if
	else
	{
		tmp[2] = BFIXED_0;
		result.traveltype = TRAVEL_WALK;
	} //endif
	//
	dist = VectorNormalizeB2A(tmp,dir);
	if (dist > BFIXED(100,0)) dist = BFIXED(100,0);
	speed = BFIXED(400,0) - (BFIXED(400,0) - BFIXED(4,0) * dist);
	if (speed < BFIXED(10,0)) speed = BFIXED_0;
	//
	BotCheckBlocked(ms, dir, qtrue, &result);
	//elemantary action move in direction
	EA_Move(ms->client, dir, speed);
	VectorCopy(dir, result.movedir);
	//
	if (ms->moveflags & MFL_SWIMMING)
	{
		Vector2Angles(dir, result.ideal_viewangles);
		result.flags |= MOVERESULT_SWIMVIEW;
	} //end if
	//if (!debugline) debugline = botimport.DebugLineCreate();
	//botimport.DebugLineShow(debugline, ms->origin, goal->origin, LINECOLOR_BLUE);
	//
	ms->lastreachnum = 0;
	ms->lastareanum = 0;
	ms->lastgoalareanum = goal->areanum;
	VectorCopy(ms->origin, ms->lastorigin);
	//
	return result;
} //end of the function BotMoveInGoalArea
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotMoveToGoal(bot_moveresult_t *result, int movestate, bot_goal_t *goal, int travelflags)
{
	int reachnum, lastreachnum, foundjumppad, ent, resultflags;
	aas_reachability_t reach, lastreach;
	bot_movestate_t *ms;
	//bvec3_t mins, maxs, up = {0, 0, 1};
	//bsp_trace_t trace;
	//static int debugline;


	BotClearMoveResult(result);
	//
	ms = BotMoveStateFromHandle(movestate);
	if (!ms) return;
	//reset the grapple before testing if the bot has a valid goal
	//because the bot could loose all it's goals when stuck to a wall
	BotResetGrapple(ms);
	//
	if (!goal)
	{
#ifdef DEBUG
		botimport.Print(PRT_MESSAGE, "client %d: movetogoal -> no goal\n", ms->client);
#endif //DEBUG
		result->failure = qtrue;
		return;
	} //end if
	//botimport.Print(PRT_MESSAGE, "numavoidreach = %d\n", ms->numavoidreach);
	//remove some of the move flags
	ms->moveflags &= ~(MFL_SWIMMING|MFL_AGAINSTLADDER);
	//set some of the move flags
	//NOTE: the MFL_ONGROUND flag is also set in the higher AI
	if (AAS_OnGround(ms->origin, ms->presencetype, ms->entitynum)) ms->moveflags |= MFL_ONGROUND;
	//
	if (ms->moveflags & MFL_ONGROUND)
	{
		int modeltype, modelnum;

		ent = BotOnTopOfEntity(ms);

		if (ent != -1)
		{
			modelnum = AAS_EntityModelindex(ent);
			if (modelnum >= 0 && modelnum < MAX_MODELS)
			{
				modeltype = modeltypes[modelnum];

				if (modeltype == MODELTYPE_FUNC_PLAT)
				{
					AAS_ReachabilityFromNum(ms->lastreachnum, &reach);
					//if the bot is Not using the elevator
					if ((reach.traveltype & TRAVELTYPE_MASK) != TRAVEL_ELEVATOR ||
						//NOTE: the face number is the plat model number
						(reach.facenum & 0x0000FFFF) != modelnum)
					{
						reachnum = AAS_NextModelReachability(0, modelnum);
						if (reachnum)
						{
							//botimport.Print(PRT_MESSAGE, "client %d: accidentally ended up on func_plat\n", ms->client);
							AAS_ReachabilityFromNum(reachnum, &reach);
							ms->lastreachnum = reachnum;
							ms->reachability_time = AAS_Time() + MAKE_GFIXED(BotReachabilityTime(&reach));
						} //end if
						else
						{
							if (bl_bot_developer)
							{
								botimport.Print(PRT_MESSAGE, "client %d: on func_plat without reachability\n", ms->client);
							} //end if
							result->blocked = qtrue;
							result->blockentity = ent;
							result->flags |= MOVERESULT_ONTOPOFOBSTACLE;
							return;
						} //end else
					} //end if
					result->flags |= MOVERESULT_ONTOPOF_ELEVATOR;
				} //end if
				else if (modeltype == MODELTYPE_FUNC_BOB)
				{
					AAS_ReachabilityFromNum(ms->lastreachnum, &reach);
					//if the bot is Not using the func bobbing
					if ((reach.traveltype & TRAVELTYPE_MASK) != TRAVEL_FUNCBOB ||
						//NOTE: the face number is the func_bobbing model number
						(reach.facenum & 0x0000FFFF) != modelnum)
					{
						reachnum = AAS_NextModelReachability(0, modelnum);
						if (reachnum)
						{
							//botimport.Print(PRT_MESSAGE, "client %d: accidentally ended up on func_bobbing\n", ms->client);
							AAS_ReachabilityFromNum(reachnum, &reach);
							ms->lastreachnum = reachnum;
							ms->reachability_time = AAS_Time() + MAKE_GFIXED(BotReachabilityTime(&reach));
						} //end if
						else
						{
							if (bl_bot_developer)
							{
								botimport.Print(PRT_MESSAGE, "client %d: on func_bobbing without reachability\n", ms->client);
							} //end if
							result->blocked = qtrue;
							result->blockentity = ent;
							result->flags |= MOVERESULT_ONTOPOFOBSTACLE;
							return;
						} //end else
					} //end if
					result->flags |= MOVERESULT_ONTOPOF_FUNCBOB;
				} //end if
				else if (modeltype == MODELTYPE_FUNC_STATIC || modeltype == MODELTYPE_FUNC_DOOR)
				{
					// check if ontop of a door bridge ?
					ms->areanum = BotFuzzyPointReachabilityArea(ms->origin);
					// if not in a reachability area
					if (!AAS_AreaReachability(ms->areanum))
					{
						result->blocked = qtrue;
						result->blockentity = ent;
						result->flags |= MOVERESULT_ONTOPOFOBSTACLE;
						return;
					} //end if
				} //end else if
				else
				{
					result->blocked = qtrue;
					result->blockentity = ent;
					result->flags |= MOVERESULT_ONTOPOFOBSTACLE;
					return;
				} //end else
			} //end if
		} //end if
	} //end if
	//if swimming
	if (AAS_Swimming(ms->origin)) ms->moveflags |= MFL_SWIMMING;
	//if against a ladder
	if (AAS_AgainstLadder(ms->origin)) ms->moveflags |= MFL_AGAINSTLADDER;
	//if the bot is on the ground, swimming or against a ladder
	if (ms->moveflags & (MFL_ONGROUND|MFL_SWIMMING|MFL_AGAINSTLADDER))
	{
		//botimport.Print(PRT_MESSAGE, "%s: onground, swimming or against ladder\n", ClientName(ms->entitynum-1));
		//
		AAS_ReachabilityFromNum(ms->lastreachnum, &lastreach);
		//reachability area the bot is in
		//ms->areanum = BotReachabilityArea(ms->origin, ((lastreach.traveltype & TRAVELTYPE_MASK) != TRAVEL_ELEVATOR));
		ms->areanum = BotFuzzyPointReachabilityArea(ms->origin);
		//
		if ( !ms->areanum )
		{
			result->failure = qtrue;
			result->blocked = qtrue;
			result->blockentity = 0;
			result->type = RESULTTYPE_INSOLIDAREA;
			return;
		} //end if
		//if the bot is in the goal area
		if (ms->areanum == goal->areanum)
		{
			*result = BotMoveInGoalArea(ms, goal);
			return;
		} //end if
		//assume we can use the reachability from the last frame
		reachnum = ms->lastreachnum;
		//if there is a last reachability
		if (reachnum)
		{
			AAS_ReachabilityFromNum(reachnum, &reach);
			//check if the reachability is still valid
			if (!(AAS_TravelFlagForType(reach.traveltype) & travelflags))
			{
				reachnum = 0;
			} //end if
			//special grapple hook case
			else if ((reach.traveltype & TRAVELTYPE_MASK) == TRAVEL_GRAPPLEHOOK)
			{
				if (ms->reachability_time < AAS_Time() ||
					(ms->moveflags & MFL_GRAPPLERESET))
				{
					reachnum = 0;
				} //end if
			} //end if
			//special elevator case
			else if ((reach.traveltype & TRAVELTYPE_MASK) == TRAVEL_ELEVATOR ||
				(reach.traveltype & TRAVELTYPE_MASK) == TRAVEL_FUNCBOB)
			{
				if ((result->flags & MOVERESULT_ONTOPOF_FUNCBOB) ||
					(result->flags & MOVERESULT_ONTOPOF_FUNCBOB))
				{
					ms->reachability_time = AAS_Time() + GFIXED(5,0);
				} //end if
				//if the bot was going for an elevator and reached the reachability area
				if (ms->areanum == reach.areanum ||
					ms->reachability_time < AAS_Time())
				{
					reachnum = 0;
				} //end if
			} //end if
			else
			{
#ifdef DEBUG
				if (bl_bot_developer)
				{
					if (ms->reachability_time < AAS_Time())
					{
						botimport.Print(PRT_MESSAGE, "client %d: reachability timeout in ", ms->client);
						AAS_PrintTravelType(reach.traveltype & TRAVELTYPE_MASK);
						botimport.Print(PRT_MESSAGE, "\n");
					} //end if
					/*
					if (ms->lastareanum != ms->areanum)
					{
						botimport.Print(PRT_MESSAGE, "changed from area %d to %d\n", ms->lastareanum, ms->areanum);
					} //end if*/
				} //end if
#endif //DEBUG
				//if the goal area changed or the reachability timed out
				//or the area changed
				if (ms->lastgoalareanum != goal->areanum ||
						ms->reachability_time < AAS_Time() ||
						ms->lastareanum != ms->areanum)
				{
					reachnum = 0;
					//botimport.Print(PRT_MESSAGE, "area change or timeout\n");
				} //end else if
			} //end else
		} //end if
		resultflags = 0;
		//if the bot needs a new reachability
		if (!reachnum)
		{
			//if the area has no reachability links
			if (!AAS_AreaReachability(ms->areanum))
			{
#ifdef DEBUG
				if (bl_bot_developer)
				{
					botimport.Print(PRT_MESSAGE, "area %d no reachability\n", ms->areanum);
				} //end if
#endif //DEBUG
			} //end if
			//get a new reachability leading towards the goal
			reachnum = BotGetReachabilityToGoal(ms->origin, ms->areanum,
								ms->lastgoalareanum, ms->lastareanum,
											ms->avoidreach, ms->avoidreachtimes, ms->avoidreachtries,
														goal, travelflags, travelflags,
																ms->avoidspots, ms->numavoidspots, &resultflags);
			//the area number the reachability starts in
			ms->reachareanum = ms->areanum;
			//reset some state variables
			ms->jumpreach = 0;						//for TRAVEL_JUMP
			ms->moveflags &= ~MFL_GRAPPLERESET;	//for TRAVEL_GRAPPLEHOOK
			//if there is a reachability to the goal
			if (reachnum)
			{
				AAS_ReachabilityFromNum(reachnum, &reach);
				//set a timeout for this reachability
				ms->reachability_time = AAS_Time() + MAKE_GFIXED(BotReachabilityTime(&reach));
				//
#ifdef AVOIDREACH
				//add the reachability to the reachabilities to avoid for a while
				BotAddToAvoidReach(ms, reachnum, GFIXED(AVOIDREACH_TIME,0));
#endif //AVOIDREACH
			} //end if
#ifdef DEBUG
			
			else if (bl_bot_developer)
			{
				botimport.Print(PRT_MESSAGE, "goal not reachable\n");
				Com_Memset(&reach, 0, sizeof(aas_reachability_t)); //make compiler happy
			} //end else
			if (bl_bot_developer)
			{
				//if still going for the same goal
				if (ms->lastgoalareanum == goal->areanum)
				{
					if (ms->lastareanum == reach.areanum)
					{
						botimport.Print(PRT_MESSAGE, "same goal, going back to previous area\n");
					} //end if
				} //end if
			} //end if
#endif //DEBUG
		} //end else
		//
		ms->lastreachnum = reachnum;
		ms->lastgoalareanum = goal->areanum;
		ms->lastareanum = ms->areanum;
		//if the bot has a reachability
		if (reachnum)
		{
			//get the reachability from the number
			AAS_ReachabilityFromNum(reachnum, &reach);
			result->traveltype = reach.traveltype;
			//
#ifdef DEBUG_AI_MOVE
			AAS_ClearShownDebugLines();
			AAS_PrintTravelType(reach.traveltype & TRAVELTYPE_MASK);
			AAS_ShowReachability(&reach);
#endif //DEBUG_AI_MOVE
			//
#ifdef DEBUG
			//botimport.Print(PRT_MESSAGE, "client %d: ", ms->client);
			//AAS_PrintTravelType(reach.traveltype);
			//botimport.Print(PRT_MESSAGE, "\n");
#endif //DEBUG
			switch(reach.traveltype & TRAVELTYPE_MASK)
			{
				case TRAVEL_WALK: *result = BotTravel_Walk(ms, &reach); break;
				case TRAVEL_CROUCH: *result = BotTravel_Crouch(ms, &reach); break;
				case TRAVEL_BARRIERJUMP: *result = BotTravel_BarrierJump(ms, &reach); break;
				case TRAVEL_LADDER: *result = BotTravel_Ladder(ms, &reach); break;
				case TRAVEL_WALKOFFLEDGE: *result = BotTravel_WalkOffLedge(ms, &reach); break;
				case TRAVEL_JUMP: *result = BotTravel_Jump(ms, &reach); break;
				case TRAVEL_SWIM: *result = BotTravel_Swim(ms, &reach); break;
				case TRAVEL_WATERJUMP: *result = BotTravel_WaterJump(ms, &reach); break;
				case TRAVEL_TELEPORT: *result = BotTravel_Teleport(ms, &reach); break;
				case TRAVEL_ELEVATOR: *result = BotTravel_Elevator(ms, &reach); break;
				case TRAVEL_GRAPPLEHOOK: *result = BotTravel_Grapple(ms, &reach); break;
				case TRAVEL_ROCKETJUMP: *result = BotTravel_RocketJump(ms, &reach); break;
				case TRAVEL_BFGJUMP: *result = BotTravel_BFGJump(ms, &reach); break;
				case TRAVEL_JUMPPAD: *result = BotTravel_JumpPad(ms, &reach); break;
				case TRAVEL_FUNCBOB: *result = BotTravel_FuncBobbing(ms, &reach); break;
				default:
				{
					botimport.Print(PRT_FATAL, "travel type %d not implemented yet\n", (reach.traveltype & TRAVELTYPE_MASK));
					break;
				} //end case
			} //end switch
			result->traveltype = reach.traveltype;
			result->flags |= resultflags;
		} //end if
		else
		{
			result->failure = qtrue;
			result->flags |= resultflags;
			Com_Memset(&reach, 0, sizeof(aas_reachability_t));
		} //end else
#ifdef DEBUG
		if (bl_bot_developer)
		{
			if (result->failure)
			{
				botimport.Print(PRT_MESSAGE, "client %d: movement failure in ", ms->client);
				AAS_PrintTravelType(reach.traveltype & TRAVELTYPE_MASK);
				botimport.Print(PRT_MESSAGE, "\n");
			} //end if
		} //end if
#endif //DEBUG
	} //end if
	else
	{
		int i, numareas, areas[16];
		bvec3_t end;

		//special handling of jump pads when the bot uses a jump pad without knowing it
		foundjumppad = qfalse;
		FIXED_VEC3MA(ms->origin, -BFIXED(2,0) * MAKE_BFIXED(ms->thinktime), ms->velocity, end);
		numareas = AAS_TraceAreas(ms->origin, end, areas, NULL, 16);
		for (i = numareas-1; i >= 0; i--)
		{
			if (AAS_AreaJumpPad(areas[i]))
			{
				//botimport.Print(PRT_MESSAGE, "client %d used a jumppad without knowing, area %d\n", ms->client, areas[i]);
				foundjumppad = qtrue;
				lastreachnum = BotGetReachabilityToGoal(end, areas[i],
							ms->lastgoalareanum, ms->lastareanum,
							ms->avoidreach, ms->avoidreachtimes, ms->avoidreachtries,
							goal, travelflags, TFL_JUMPPAD, ms->avoidspots, ms->numavoidspots, NULL);
				if (lastreachnum)
				{
					ms->lastreachnum = lastreachnum;
					ms->lastareanum = areas[i];
					//botimport.Print(PRT_MESSAGE, "found jumppad reachability\n");
					break;
				} //end if
				else
				{
					for (lastreachnum = AAS_NextAreaReachability(areas[i], 0); lastreachnum;
						lastreachnum = AAS_NextAreaReachability(areas[i], lastreachnum))
					{
						//get the reachability from the number
						AAS_ReachabilityFromNum(lastreachnum, &reach);
						if ((reach.traveltype & TRAVELTYPE_MASK) == TRAVEL_JUMPPAD)
						{
							ms->lastreachnum = lastreachnum;
							ms->lastareanum = areas[i];
							//botimport.Print(PRT_MESSAGE, "found jumppad reachability hard!!\n");
							break;
						} //end if
					} //end for
					if (lastreachnum) break;
				} //end else
			} //end if
		} //end for
		if (bl_bot_developer)
		{
			//if a jumppad is found with the trace but no reachability is found
			if (foundjumppad && !ms->lastreachnum)
			{
				botimport.Print(PRT_MESSAGE, "client %d didn't find jumppad reachability\n", ms->client);
			} //end if
		} //end if
		//
		if (ms->lastreachnum)
		{
			//botimport.Print(PRT_MESSAGE, "%s: NOT onground, swimming or against ladder\n", ClientName(ms->entitynum-1));
			AAS_ReachabilityFromNum(ms->lastreachnum, &reach);
			result->traveltype = reach.traveltype;
#ifdef DEBUG
			//botimport.Print(PRT_MESSAGE, "client %d finish: ", ms->client);
			//AAS_PrintTravelType(reach.traveltype & TRAVELTYPE_MASK);
			//botimport.Print(PRT_MESSAGE, "\n");
#endif //DEBUG
			//
			switch(reach.traveltype & TRAVELTYPE_MASK)
			{
				case TRAVEL_WALK: *result = BotTravel_Walk(ms, &reach); break;//BotFinishTravel_Walk(ms, &reach); break;
				case TRAVEL_CROUCH: /*do nothing*/ break;
				case TRAVEL_BARRIERJUMP: *result = BotFinishTravel_BarrierJump(ms, &reach); break;
				case TRAVEL_LADDER: *result = BotTravel_Ladder(ms, &reach); break;
				case TRAVEL_WALKOFFLEDGE: *result = BotFinishTravel_WalkOffLedge(ms, &reach); break;
				case TRAVEL_JUMP: *result = BotFinishTravel_Jump(ms, &reach); break;
				case TRAVEL_SWIM: *result = BotTravel_Swim(ms, &reach); break;
				case TRAVEL_WATERJUMP: *result = BotFinishTravel_WaterJump(ms, &reach); break;
				case TRAVEL_TELEPORT: /*do nothing*/ break;
				case TRAVEL_ELEVATOR: *result = BotFinishTravel_Elevator(ms, &reach); break;
				case TRAVEL_GRAPPLEHOOK: *result = BotTravel_Grapple(ms, &reach); break;
				case TRAVEL_ROCKETJUMP:
				case TRAVEL_BFGJUMP: *result = BotFinishTravel_WeaponJump(ms, &reach); break;
				case TRAVEL_JUMPPAD: *result = BotFinishTravel_JumpPad(ms, &reach); break;
				case TRAVEL_FUNCBOB: *result = BotFinishTravel_FuncBobbing(ms, &reach); break;
				default:
				{
					botimport.Print(PRT_FATAL, "(last) travel type %d not implemented yet\n", (reach.traveltype & TRAVELTYPE_MASK));
					break;
				} //end case
			} //end switch
			result->traveltype = reach.traveltype;
#ifdef DEBUG
			if (bl_bot_developer)
			{
				if (result->failure)
				{
					botimport.Print(PRT_MESSAGE, "client %d: movement failure in finish ", ms->client);
					AAS_PrintTravelType(reach.traveltype & TRAVELTYPE_MASK);
					botimport.Print(PRT_MESSAGE, "\n");
				} //end if
			} //end if
#endif //DEBUG
		} //end if
	} //end else
	//FIXME: is it right to do this here?
	if (result->blocked) ms->reachability_time -= GFIXED(10,0) * ms->thinktime;
	//copy the last origin
	VectorCopy(ms->origin, ms->lastorigin);
	//return the movement result
	return;
} //end of the function BotMoveToGoal
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotResetAvoidReach(int movestate)
{
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle(movestate);
	if (!ms) return;
	Com_Memset(ms->avoidreach, 0, MAX_AVOIDREACH * sizeof(int));
	Com_Memset(ms->avoidreachtimes, 0, MAX_AVOIDREACH * sizeof(bfixed));
	Com_Memset(ms->avoidreachtries, 0, MAX_AVOIDREACH * sizeof(int));
} //end of the function BotResetAvoidReach
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotResetLastAvoidReach(int movestate)
{
	int i, latest;
	gfixed latesttime;
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle(movestate);
	if (!ms) return;
	latesttime = GFIXED_0;
	latest = 0;
	for (i = 0; i < MAX_AVOIDREACH; i++)
	{
		if (ms->avoidreachtimes[i] > latesttime)
		{
			latesttime = ms->avoidreachtimes[i];
			latest = i;
		} //end if
	} //end for
	if (FIXED_NOT_ZERO(latesttime))
	{
		ms->avoidreachtimes[latest] = GFIXED_0;
		if (ms->avoidreachtries[i] > 0) ms->avoidreachtries[latest]--;
	} //end if
} //end of the function BotResetLastAvoidReach
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotResetMoveState(int movestate)
{
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle(movestate);
	if (!ms) return;
	Com_Memset(ms, 0, sizeof(bot_movestate_t));
} //end of the function BotResetMoveState
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotSetupMoveAI(void)
{
	BotSetBrushModelTypes();
	sv_maxstep = LibVar("sv_step", "18");
	sv_maxbarrier = LibVar("sv_maxbarrier", "32");
	sv_gravity = LibVar("sv_gravity", "800");
	weapindex_rocketlauncher = LibVar("weapindex_rocketlauncher", "5");
	weapindex_bfg10k = LibVar("weapindex_bfg10k", "9");
	weapindex_grapple = LibVar("weapindex_grapple", "10");
	entitytypemissile = LibVar("entitytypemissile", "3");
	offhandgrapple = LibVar("offhandgrapple", "0");
	cmd_grappleon = LibVar("cmd_grappleon", "grappleon");
	cmd_grappleoff = LibVar("cmd_grappleoff", "grappleoff");
	return BLERR_NOERROR;
} //end of the function BotSetupMoveAI
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotShutdownMoveAI(void)
{
	int i;

	for (i = 1; i <= MAX_CLIENTS; i++)
	{
		if (botmovestates[i])
		{
			FreeMemory(botmovestates[i]);
			botmovestates[i] = NULL;
		} //end if
	} //end for
} //end of the function BotShutdownMoveAI


