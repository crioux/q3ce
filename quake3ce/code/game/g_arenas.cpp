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
//
// g_arenas.c
//
#include"game_pch.h"


gentity_t	*podium1;
gentity_t	*podium2;
gentity_t	*podium3;


/*
==================
UpdateTournamentInfo
==================
*/
void UpdateTournamentInfo( void ) {
	int			i;
	gentity_t	*player;
	int			playerClientNum;
	int			n, accuracy, perfect,	msglen;
	int			buflen;
#ifdef MISSIONPACK // bk001205
  int score1, score2;
	qboolean won;
#endif
	char		buf[32];
	char		msg[MAX_STRING_CHARS];

	// find the real player
	player = NULL;
	for (i = 0; i < level.maxclients; i++ ) {
		player = &g_entities[i];
		if ( !player->inuse ) {
			continue;
		}
		if ( !( player->r.svFlags & SVF_BOT ) ) {
			break;
		}
	}
	// this should never happen!
	if ( !player || i == level.maxclients ) {
		return;
	}
	playerClientNum = i;

	CalculateRanks();

	if ( level.clients[playerClientNum].sess.sessionTeam == TEAM_SPECTATOR ) {
#ifdef MISSIONPACK
		Com_sprintf( msg, sizeof(msg), "postgame %i %i 0 0 0 0 0 0 0 0 0 0 0", level.numNonSpectatorClients, playerClientNum );
#else
		Com_sprintf( msg, sizeof(msg), "postgame %i %i 0 0 0 0 0 0", level.numNonSpectatorClients, playerClientNum );
#endif
	}
	else {
		if( player->client->accuracy_shots ) {
			accuracy = player->client->accuracy_hits * 100 / player->client->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
#ifdef MISSIONPACK
		won = qfalse;
		if (g_gametype.integer >= GT_CTF) {
			score1 = level.teamScores[TEAM_RED];
			score2 = level.teamScores[TEAM_BLUE];
			if (level.clients[playerClientNum].sess.sessionTeam	== TEAM_RED) {
				won = (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE]);
			} else {
				won = (level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED]);
			}
		} else {
			if (&level.clients[playerClientNum] == &level.clients[ level.sortedClients[0] ]) {
				won = qtrue;
				score1 = level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE];
				score2 = level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE];
			} else {
				score2 = level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE];
				score1 = level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE];
			}
		}
		if (won && player->client->ps.persistant[PERS_KILLED] == 0) {
			perfect = 1;
		} else {
			perfect = 0;
		}
		Com_sprintf( msg, sizeof(msg), "postgame %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.numNonSpectatorClients, playerClientNum, accuracy,
			player->client->ps.persistant[PERS_IMPRESSIVE_COUNT], player->client->ps.persistant[PERS_EXCELLENT_COUNT],player->client->ps.persistant[PERS_DEFEND_COUNT],
			player->client->ps.persistant[PERS_ASSIST_COUNT], player->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT], player->client->ps.persistant[PERS_SCORE],
			perfect, score1, score2, level.time, player->client->ps.persistant[PERS_CAPTURES] );

#else
		perfect = ( level.clients[playerClientNum].ps.persistant[PERS_RANK] == 0 && player->client->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;
		Com_sprintf( msg, sizeof(msg), "postgame %i %i %i %i %i %i %i %i", level.numNonSpectatorClients, playerClientNum, accuracy,
			player->client->ps.persistant[PERS_IMPRESSIVE_COUNT], player->client->ps.persistant[PERS_EXCELLENT_COUNT],
			player->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT], player->client->ps.persistant[PERS_SCORE],
			perfect );
#endif
	}

	msglen = strlen( msg );
	for( i = 0; i < level.numNonSpectatorClients; i++ ) {
		n = level.sortedClients[i];
		Com_sprintf( buf, sizeof(buf), " %i %i %i", n, level.clients[n].ps.persistant[PERS_RANK], level.clients[n].ps.persistant[PERS_SCORE] );
		buflen = strlen( buf );
		if( msglen + buflen + 1 >= sizeof(msg) ) {
			break;
		}
		strcat( msg, buf );
	}
	_G_trap_SendConsoleCommand( EXEC_APPEND, msg );
}


static gentity_t *SpawnModelOnVictoryPad( gentity_t *pad, bvec3_t offset, gentity_t *ent, int place ) {
	gentity_t	*body;
	bvec3_t		vec;
	avec3_t		f, r, u;

	body = G_Spawn();
	if ( !body ) {
		G_Printf( S_COLOR_RED "ERROR: out of gentities\n" );
		return NULL;
	}

	body->classname = ent->client->pers.netname;
	body->client = ent->client;
	body->s = ent->s;
	body->s.eType = ET_PLAYER;		// could be ET_INVISIBLE
	body->s.eFlags = 0;				// clear EF_TALK, etc
	body->s.powerups = 0;			// clear powerups
	body->s.loopSound = 0;			// clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = GFIXED_0;		// don't bounce
	body->s.event = 0;
	body->s.pos.trType = TR_STATIONARY;
	body->s.groundEntityNum = ENTITYNUM_WORLD;
	body->s.legsAnim = LEGS_IDLE;
	body->s.torsoAnim = TORSO_STAND;
	if( body->s.weapon == WP_NONE ) {
		body->s.weapon = WP_MACHINEGUN;
	}
	if( body->s.weapon == WP_GAUNTLET) {
		body->s.torsoAnim = TORSO_STAND2;
	}
	body->s.event = 0;
	body->r.svFlags = ent->r.svFlags;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);
	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.contents = CONTENTS_BODY;
	body->r.ownerNum = ent->r.ownerNum;
	body->takedamage = qfalse;

	VectorSubtract( level.intermission_origin, pad->r.currentOrigin, vec );
	vectoangles( vec, body->s.apos.trBase );
	body->s.apos.trBase[PITCH] = AFIXED_0;
	body->s.apos.trBase[ROLL] = AFIXED_0;

	AngleVectors( body->s.apos.trBase, f, r, u );
	FIXED_VEC3MA_R( pad->r.currentOrigin, offset[0], f, vec );
	FIXED_VEC3MA_R( vec, offset[1], r, vec );
	FIXED_VEC3MA_R( vec, offset[2], u, vec );

	G_SetOrigin( body, vec );

	_G_trap_LinkEntity (body);

	body->count = place;

	return body;
}


static void CelebrateStop( gentity_t *player ) {
	int		anim;

	if( player->s.weapon == WP_GAUNTLET) {
		anim = TORSO_STAND2;
	}
	else {
		anim = TORSO_STAND;
	}
	player->s.torsoAnim = ( ( player->s.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
}


#define	TIMER_GESTURE	(34*66+50)
static void CelebrateStart( gentity_t *player ) {
	player->s.torsoAnim = ( ( player->s.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | TORSO_GESTURE;
	player->nextthink = level.time + TIMER_GESTURE;
	player->think = CelebrateStop;

	/*
	player->client->ps.events[player->client->ps.eventSequence & (MAX_PS_EVENTS-1)] = EV_TAUNT;
	player->client->ps.eventParms[player->client->ps.eventSequence & (MAX_PS_EVENTS-1)] = 0;
	player->client->ps.eventSequence++;
	*/
	G_AddEvent(player, EV_TAUNT, 0);
}


static bvec3_t	offsetFirst  = {BFIXED_0, BFIXED_0, BFIXED(74,0)};
static bvec3_t	offsetSecond = {-BFIXED(10,0), BFIXED(60,0), BFIXED(54,0)};
static bvec3_t	offsetThird  = {-BFIXED(19,0), -BFIXED(60,0), BFIXED(45,0)};

static void PodiumPlacementThink( gentity_t *podium ) {
	avec3_t		avec;
	bvec3_t		bvec;
	bvec3_t		origin;
	avec3_t		f, r, u;

	podium->nextthink = level.time + 100;

	AngleVectors( level.intermission_angle, avec, NULL, NULL );
	FIXED_VEC3MA_R( level.intermission_origin,MAKE_BFIXED(_G_trap_Cvar_VariableIntegerValue( "g_podiumDist" )), bvec, origin );
	origin[2] -= MAKE_BFIXED(_G_trap_Cvar_VariableIntegerValue( "g_podiumDrop" ));
	G_SetOrigin( podium, origin );

	if( podium1 ) {

		VectorSubtract( level.intermission_origin, podium->r.currentOrigin, bvec );
		vectoangles( bvec, podium1->s.apos.trBase );
		podium1->s.apos.trBase[PITCH] = AFIXED_0;
		podium1->s.apos.trBase[ROLL] = AFIXED_0;

		AngleVectors( podium1->s.apos.trBase, f, r, u );
		FIXED_VEC3MA_R( podium->r.currentOrigin, offsetFirst[0], f, bvec );
		FIXED_VEC3MA_R( bvec, offsetFirst[1], r, bvec );
		FIXED_VEC3MA_R( bvec, offsetFirst[2], u, bvec );

		G_SetOrigin( podium1, bvec );
	}

	if( podium2 ) {
		VectorSubtract( level.intermission_origin, podium->r.currentOrigin, bvec );
		vectoangles( bvec, podium2->s.apos.trBase );
		podium2->s.apos.trBase[PITCH] = AFIXED_0;
		podium2->s.apos.trBase[ROLL] = AFIXED_0;

		AngleVectors( podium2->s.apos.trBase, f, r, u );
		FIXED_VEC3MA_R( podium->r.currentOrigin, offsetSecond[0], f, bvec );
		FIXED_VEC3MA_R( bvec, offsetSecond[1], r, bvec );
		FIXED_VEC3MA_R( bvec, offsetSecond[2], u, bvec );

		G_SetOrigin( podium2, bvec );
	}

	if( podium3 ) {
		VectorSubtract( level.intermission_origin, podium->r.currentOrigin, bvec );
		vectoangles( bvec, podium3->s.apos.trBase );
		podium3->s.apos.trBase[PITCH] = AFIXED_0;
		podium3->s.apos.trBase[ROLL] = AFIXED_0;

		AngleVectors( podium3->s.apos.trBase, f, r, u );
		FIXED_VEC3MA_R( podium->r.currentOrigin, offsetThird[0], f, bvec );
		FIXED_VEC3MA_R( bvec, offsetThird[1], r, bvec );
		FIXED_VEC3MA_R( bvec, offsetThird[2], u, bvec );

		G_SetOrigin( podium3, bvec );
	}
}


static gentity_t *SpawnPodium( void ) {
	gentity_t	*podium;
	avec3_t		avec;
	bvec3_t		bvec;
	bvec3_t		origin;

	podium = G_Spawn();
	if ( !podium ) {
		return NULL;
	}

	podium->classname = "podium";
	podium->s.eType = ET_GENERAL;
	podium->s.number = podium - g_entities;
	podium->clipmask = CONTENTS_SOLID;
	podium->r.contents = CONTENTS_SOLID;
	podium->s.modelindex = G_ModelIndex( SP_PODIUM_MODEL );

	AngleVectors( level.intermission_angle, avec, NULL, NULL );
	FIXED_VEC3MA_R( level.intermission_origin, MAKE_BFIXED(_G_trap_Cvar_VariableIntegerValue( "g_podiumDist" )), avec, origin );
	origin[2] -= MAKE_BFIXED(_G_trap_Cvar_VariableIntegerValue( "g_podiumDrop" ));
	G_SetOrigin( podium, origin );

	VectorSubtract( level.intermission_origin, podium->r.currentOrigin, bvec );
	podium->s.apos.trBase[YAW] = vectoyaw( bvec );
	_G_trap_LinkEntity (podium);

	podium->think = PodiumPlacementThink;
	podium->nextthink = level.time + 100;
	return podium;
}


/*
==================
SpawnModelsOnVictoryPads
==================
*/
void SpawnModelsOnVictoryPads( void ) {
	gentity_t	*player;
	gentity_t	*podium;

	podium1 = NULL;
	podium2 = NULL;
	podium3 = NULL;

	podium = SpawnPodium();

	player = SpawnModelOnVictoryPad( podium, offsetFirst, &g_entities[level.sortedClients[0]],
				level.clients[ level.sortedClients[0] ].ps.persistant[PERS_RANK] &~ RANK_TIED_FLAG );
	if ( player ) {
		player->nextthink = level.time + 2000;
		player->think = CelebrateStart;
		podium1 = player;
	}

	player = SpawnModelOnVictoryPad( podium, offsetSecond, &g_entities[level.sortedClients[1]],
				level.clients[ level.sortedClients[1] ].ps.persistant[PERS_RANK] &~ RANK_TIED_FLAG );
	if ( player ) {
		podium2 = player;
	}

	if ( level.numNonSpectatorClients > 2 ) {
		player = SpawnModelOnVictoryPad( podium, offsetThird, &g_entities[level.sortedClients[2]],
				level.clients[ level.sortedClients[2] ].ps.persistant[PERS_RANK] &~ RANK_TIED_FLAG );
		if ( player ) {
			podium3 = player;
		}
	}
}


/*
===============
Svcmd_AbortPodium_f
===============
*/
void Svcmd_AbortPodium_f( void ) {
	if( g_gametype.integer != GT_SINGLE_PLAYER ) {
		return;
	}

	if( podium1 ) {
		podium1->nextthink = level.time;
		podium1->think = CelebrateStop;
	}
}
