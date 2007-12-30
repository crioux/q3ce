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
// sv_game.c -- interface to the game dll

#include"server_pch.h"

botlib_export_t	*botlib_export;

void SV_GameError( const char *string ) {
	Com_Error( ERR_DROP, "%s", string );
}

void SV_GamePrint( const char *string ) {
	Com_Printf( "%s", string );
}

// these functions must be used instead of pointer arithmetic, because
// the game allocates gentities with private information after the server shared part
int	SV_NumForGentity( sharedEntity_t *ent ) {
	int		num;

	num = ( (byte *)ent - (byte *)sv.gentities ) / sv.gentitySize;

	return num;
}

sharedEntity_t *SV_GentityNum( int num ) {
	sharedEntity_t *ent;

	ent = (sharedEntity_t *)((byte *)sv.gentities + sv.gentitySize*(num));

	return ent;
}

playerState_t *SV_GameClientNum( int num ) {
	playerState_t	*ps;

	ps = (playerState_t *)((byte *)sv.gameClients + sv.gameClientSize*(num));

	return ps;
}

svEntity_t	*SV_SvEntityForGentity( sharedEntity_t *gEnt ) {
	if ( !gEnt || gEnt->s.number < 0 || gEnt->s.number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	return &sv.svEntities[ gEnt->s.number ];
}

sharedEntity_t *SV_GEntityForSvEntity( svEntity_t *svEnt ) {
	int		num;

	num = svEnt - sv.svEntities;
	return SV_GentityNum( num );
}

/*
===============
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand( int clientNum, const char *text ) {
	if ( clientNum == -1 ) {
		SV_SendServerCommand( NULL, "%s", text );
	} else {
		if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
			return;
		}
		SV_SendServerCommand( svs.clients + clientNum, "%s", text );	
	}
}


/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/
void SV_GameDropClient( int clientNum, const char *reason ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	SV_DropClient( svs.clients + clientNum, reason );	
}


/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel( sharedEntity_t *ent, const char *name ) {
	clipHandle_t	h;
	bvec3_t			mins, maxs;

	if (!name) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
	}

	if (name[0] != '*') {
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
	}


	ent->s.modelindex = atoi( name + 1 );

	h = CM_InlineModel( ent->s.modelindex );
	CM_ModelBounds( h, mins, maxs );
	VectorCopy (mins, ent->r.mins);
	VectorCopy (maxs, ent->r.maxs);
	ent->r.bmodel = qtrue;

	ent->r.contents = -1;		// we don't know exactly what is in the brushes

	SV_LinkEntity( ent );		// FIXME: remove
}



/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
qboolean SV_inPVS (const bvec3_t p1, const bvec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);
	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return qfalse;
	if (!CM_AreasConnected (area1, area2))
		return qfalse;		// a door blocks sight
	return qtrue;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
qboolean SV_inPVSIgnorePortals( const bvec3_t p1, const bvec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);

	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return qfalse;

	return qtrue;
}


/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState( sharedEntity_t *ent, qboolean open ) {
	svEntity_t	*svEnt;

	svEnt = SV_SvEntityForGentity( ent );
	if ( svEnt->areanum2 == -1 ) {
		return;
	}
	CM_AdjustAreaPortalState( svEnt->areanum, svEnt->areanum2, open );
}


/*
==================
SV_GameAreaEntities
==================
*/
qboolean	SV_EntityContact( const bvec3_t mins, const bvec3_t maxs, const sharedEntity_t *gEnt, int capsule ) {
	const bfixed	*origin;
	const afixed	*angles;
	clipHandle_t	ch;
	trace_t			trace;

	// check for exact collision
	origin = gEnt->r.currentOrigin;
	angles = gEnt->r.currentAngles;

	ch = SV_ClipHandleForEntity( gEnt );
	CM_TransformedBoxTrace ( &trace, bvec3_origin, bvec3_origin, mins, maxs,
		ch, -1, origin, angles, capsule );

	return trace.startsolid;
}


/*
===============
SV_GetServerinfo

===============
*/
void SV_GetServerinfo( char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
	}
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
}

/*
===============
SV_LocateGameData

===============
*/
void SV_LocateGameData( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t,
					   playerState_t *clients, int sizeofGameClient ) {
	sv.gentities = gEnts;
	sv.gentitySize = sizeofGEntity_t;
	sv.num_entities = numGEntities;

	sv.gameClients = clients;
	sv.gameClientSize = sizeofGameClient;
}


/*
===============
SV_GetUsercmd

===============
*/
void SV_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = svs.clients[clientNum].lastUsercmd;
}

//==============================================

static int	FloatAsInt( float f ) {
	union
	{
	    int i;
	    float f;
	} temp;
	
	temp.f = f;
	return temp.i;
}

/*
====================
SV_GameSystemCalls

The module is making a system call
====================
*/
//rcg010207 - see my comments in VM_DllSyscall(), in qcommon/vm.c ...
#if ((defined __linux__) && (defined __powerpc__))
#define VMA(x) ((void *) args[x])
#else
#define	VMA(x) VM_ArgPtr(args[x])
#endif

#define	VMF(x)	((float *)args)[x]

SysCallArg SV_GameSystemCalls( int callnum, SysCallArgs &args) {
	switch( callnum ) {
	case G_PRINT:
		Com_Printf( "%s", (const void *)args[0] );
		return SysCallArg();
	case G_ERROR:
		Com_Error( ERR_DROP, "%s", (const void *)args[0] );
		return SysCallArg();
	case G_MILLISECONDS:
		return SysCallArg::Int(Sys_Milliseconds());
	case G_CVAR_REGISTER:
		Cvar_Register( args[0], args[1], args[2], args[3] ); 
		return SysCallArg();
	case G_CVAR_UPDATE:
		Cvar_Update( args[0] );
		return SysCallArg();
	case G_CVAR_SET:
		Cvar_Set( args[0], args[1] );
		return SysCallArg();
	case G_CVAR_VARIABLE_INTEGER_VALUE:
		return SysCallArg::Int(Cvar_VariableIntegerValue( args[0] ));
	case G_CVAR_VARIABLE_STRING_BUFFER:
		Cvar_VariableStringBuffer( args[0], args[1], args[2] );
		return SysCallArg();
	case G_ARGC:
		return SysCallArg::Int(Cmd_Argc());
	case G_ARGV:
		Cmd_ArgvBuffer( args[0],args[1],args[2] );
		return SysCallArg();
	case G_SEND_CONSOLE_COMMAND:
		Cbuf_ExecuteText( args[0], args[1] );
		return SysCallArg();

	case G_FS_FOPEN_FILE:
		return SysCallArg::Int(FS_FOpenFileByMode( args[0],args[1],(fsMode_t)(int)args[2] ));
	case G_FS_READ:
		FS_Read2( args[0],args[1],args[2]);
		return SysCallArg();
	case G_FS_WRITE:
		FS_Write( args[0],args[1],args[2]);
		return SysCallArg();
	case G_FS_FCLOSE_FILE:
		FS_FCloseFile( args[0] );
		return SysCallArg();
	case G_FS_GETFILELIST:
		return SysCallArg::Int(FS_GetFileList( args[0],args[1],args[2],args[3] ));
	case G_FS_SEEK:
		return SysCallArg::Int(FS_Seek( args[0],(int)args[1],(int)args[2] ));

	case G_LOCATE_GAME_DATA:
		SV_LocateGameData(args[0],args[1],args[2],args[3],args[4]);
		return SysCallArg();
	case G_DROP_CLIENT:
		SV_GameDropClient( args[0],args[1] );
		return SysCallArg();
	case G_SEND_SERVER_COMMAND:
		SV_GameSendServerCommand(args[0],args[1]);
		return SysCallArg();
	case G_LINKENTITY:
		SV_LinkEntity(args[0] );
		return SysCallArg();
	case G_UNLINKENTITY:
		SV_UnlinkEntity( args[0] );
		return SysCallArg();
	case G_ENTITIES_IN_BOX:
		return SysCallArg::Int(SV_AreaEntities( args[0],args[1],args[2],args[3] ));
	case G_ENTITY_CONTACT:
		return SysCallArg::Int(SV_EntityContact( args[0],args[1],args[2],/*int capsule*/ qfalse ));
	case G_ENTITY_CONTACTCAPSULE:
		return SysCallArg::Int(SV_EntityContact( args[0],args[1],args[2], /*int capsule*/ qtrue ));
	case G_TRACE:
		SV_Trace( args[0],args[1],args[2],args[3],args[4],args[5],args[6], /*int capsule*/ qfalse );
		return SysCallArg();
	case G_TRACECAPSULE:
		SV_Trace( args[0],args[1],args[2],args[3],args[4],args[5],args[6], /*int capsule*/ qtrue );
		return SysCallArg();
	case G_POINT_CONTENTS:
		return SysCallArg::Int(SV_PointContents( args[0],args[1]));
	case G_SET_BRUSH_MODEL:
		SV_SetBrushModel( args[0],args[1]);
		return SysCallArg();
	case G_IN_PVS:
		return SysCallArg::Int(SV_inPVS( args[0],args[1]));
	case G_IN_PVS_IGNORE_PORTALS:
		return SysCallArg::Int(SV_inPVSIgnorePortals( args[0],args[1] ));

	case G_SET_CONFIGSTRING:
		SV_SetConfigstring( args[0],args[1]);
		return SysCallArg();
	case G_GET_CONFIGSTRING:
		SV_GetConfigstring( args[0],args[1],args[2]);
		return SysCallArg();
	case G_SET_USERINFO:
		SV_SetUserinfo( args[0],args[1] );
		return SysCallArg();
	case G_GET_USERINFO:
		SV_GetUserinfo( args[0],args[1],args[2] );
		return SysCallArg();
	case G_GET_SERVERINFO:
		SV_GetServerinfo( args[0],args[1] );
		return SysCallArg();
	case G_ADJUST_AREA_PORTAL_STATE:
		SV_AdjustAreaPortalState( args[0],args[1] );
		return SysCallArg();
	case G_AREAS_CONNECTED:
		return SysCallArg::Int(CM_AreasConnected( args[0],args[1] ));

	case G_BOT_ALLOCATE_CLIENT:
		return SysCallArg::Int(SV_BotAllocateClient());
	case G_BOT_FREE_CLIENT:
		SV_BotFreeClient( args[0] );
		return SysCallArg();

	case G_GET_USERCMD:
		SV_GetUsercmd( args[0],args[1] );
		return SysCallArg();
	case G_GET_ENTITY_TOKEN:
		{
			const char	*s;

			s = COM_Parse( &sv.entityParsePoint );
			Q_strncpyz( args[0], s, args[1] );
			if ( !sv.entityParsePoint && !s[0] ) {
				return SysCallArg::Int(qfalse);
			} else {
				return SysCallArg::Int(qtrue);
			}
		}

	case G_DEBUG_POLYGON_CREATE:
		return SysCallArg::Int(BotImport_DebugPolygonCreate( args[0],args[1],args[2] ));
	case G_DEBUG_POLYGON_DELETE:
		BotImport_DebugPolygonDelete( args[0] );
		return SysCallArg();
	case G_REAL_TIME:
		return SysCallArg::Int(Com_RealTime( args[0] ));
	case G_SNAPVECTOR:
		Sys_SnapVector( args[0] );
		return SysCallArg();

		//====================================

	case BOTLIB_SETUP:
		return SysCallArg::Int(SV_BotLibSetup());
	case BOTLIB_SHUTDOWN:
		return SysCallArg::Int(SV_BotLibShutdown());
	case BOTLIB_LIBVAR_SET:
		return SysCallArg::Int(botlib_export->BotLibVarSet( args[0], args[1] ));
	case BOTLIB_LIBVAR_GET:
		return SysCallArg::Int(botlib_export->BotLibVarGet( args[0], args[1], args[2] ));

	case BOTLIB_PC_ADD_GLOBAL_DEFINE:
		return SysCallArg::Int(botlib_export->PC_AddGlobalDefine( args[0] ));
	case BOTLIB_PC_LOAD_SOURCE:
		return SysCallArg::Int(botlib_export->PC_LoadSourceHandle( args[0] ));
	case BOTLIB_PC_FREE_SOURCE:
		return SysCallArg::Int(botlib_export->PC_FreeSourceHandle( args[0] ));
	case BOTLIB_PC_READ_TOKEN:
		return SysCallArg::Int(botlib_export->PC_ReadTokenHandle( args[0],args[1] ));
	case BOTLIB_PC_SOURCE_FILE_AND_LINE:
		return SysCallArg::Int(botlib_export->PC_SourceFileAndLine( args[0], args[1], args[2] ));

	case BOTLIB_START_FRAME:
		return SysCallArg::Int(botlib_export->BotLibStartFrame( args[0] ));
	case BOTLIB_LOAD_MAP:
		return SysCallArg::Int(botlib_export->BotLibLoadMap( args[0] ));
	case BOTLIB_UPDATENTITY:
		return SysCallArg::Int(botlib_export->BotLibUpdateEntity( args[0],args[1] ));
	case BOTLIB_TEST:
		return SysCallArg::Int(botlib_export->Test( args[0],args[1],args[2],args[3] ));

	case BOTLIB_GET_SNAPSHOT_ENTITY:
		return SysCallArg::Int(SV_BotGetSnapshotEntity( args[0],args[1] ));
	case BOTLIB_GET_CONSOLE_MESSAGE:
		return SysCallArg::Int(SV_BotGetConsoleMessage( args[0],args[1],args[2] ));
	case BOTLIB_USER_COMMAND:
		SV_ClientThink( &svs.clients[args[0]], args[1] );
		return SysCallArg();

	case BOTLIB_AAS_BBOX_AREAS:
		return SysCallArg::Int(botlib_export->aas.AAS_BBoxAreas( args[0],args[1],args[2],args[3] ));
	case BOTLIB_AAS_AREA_INFO:
		return SysCallArg::Int(botlib_export->aas.AAS_AreaInfo( args[0], args[1] ));
	case BOTLIB_AAS_ALTERNATIVE_ROUTE_GOAL:
		return SysCallArg::Int(botlib_export->aas.AAS_AlternativeRouteGoals( args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7] ));
	case BOTLIB_AAS_ENTITY_INFO:
		botlib_export->aas.AAS_EntityInfo( args[0],args[1] );
		return SysCallArg();

	case BOTLIB_AAS_INITIALIZED:
		return SysCallArg::Int(botlib_export->aas.AAS_Initialized());
	case BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX:
		botlib_export->aas.AAS_PresenceTypeBoundingBox( args[0],args[1],args[2] );
		return SysCallArg();
	case BOTLIB_AAS_TIME:
		return SysCallArg::Fixed( botlib_export->aas.AAS_Time() );

	case BOTLIB_AAS_POINT_AREA_NUM:
		return SysCallArg::Int(botlib_export->aas.AAS_PointAreaNum( args[0] ));
	case BOTLIB_AAS_POINT_REACHABILITY_AREA_INDEX:
		return SysCallArg::Int(botlib_export->aas.AAS_PointReachabilityAreaIndex( args[0] ));
	case BOTLIB_AAS_TRACE_AREAS:
		return SysCallArg::Int(botlib_export->aas.AAS_TraceAreas( args[0],args[1],args[2],args[3],args[4] ));

	case BOTLIB_AAS_POINT_CONTENTS:
		return SysCallArg::Int(botlib_export->aas.AAS_PointContents( args[0] ));
	case BOTLIB_AAS_NEXT_BSP_ENTITY:
		return SysCallArg::Int(botlib_export->aas.AAS_NextBSPEntity( args[0] ));
	case BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY:
		return SysCallArg::Int(botlib_export->aas.AAS_ValueForBSPEpairKey( args[0], args[1],args[2],args[3]));
	case BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY:
		return SysCallArg::Int(botlib_export->aas.AAS_VectorForBSPEpairKey( args[0],args[1],args[2] ));
	case BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY:
		return SysCallArg::Int(botlib_export->aas.AAS_FloatForBSPEpairKey( args[0],args[1],args[2] ));
	case BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY:
		return SysCallArg::Int(botlib_export->aas.AAS_IntForBSPEpairKey( args[0],args[1],args[2] ));

	case BOTLIB_AAS_AREA_REACHABILITY:
		return SysCallArg::Int(botlib_export->aas.AAS_AreaReachability( args[0] ));

	case BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA:
		return SysCallArg::Int(botlib_export->aas.AAS_AreaTravelTimeToGoalArea( args[0],args[1],args[2],args[3] ));
	case BOTLIB_AAS_ENABLE_ROUTING_AREA:
		return SysCallArg::Int(botlib_export->aas.AAS_EnableRoutingArea( args[0], args[1] ));
	case BOTLIB_AAS_PREDICT_ROUTE:
		return SysCallArg::Int(botlib_export->aas.AAS_PredictRoute(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10] ));

	case BOTLIB_AAS_SWIMMING:
		return SysCallArg::Int(botlib_export->aas.AAS_Swimming( args[0] ));
	case BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT:
		return SysCallArg::Int(botlib_export->aas.AAS_PredictClientMovement( args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11],args[12] ));

	case BOTLIB_EA_SAY:
		botlib_export->ea.EA_Say( args[0],args[1] );
		return SysCallArg();
	case BOTLIB_EA_SAY_TEAM:
		botlib_export->ea.EA_SayTeam( args[0],args[1] );
		return SysCallArg();
	case BOTLIB_EA_COMMAND:
		botlib_export->ea.EA_Command( args[0],args[1] );
		return SysCallArg();

	case BOTLIB_EA_ACTION:
		botlib_export->ea.EA_Action( args[0],args[1] );
		break;
	case BOTLIB_EA_GESTURE:
		botlib_export->ea.EA_Gesture( args[0] );
		return SysCallArg();
	case BOTLIB_EA_TALK:
		botlib_export->ea.EA_Talk( args[0] );
		return SysCallArg();
	case BOTLIB_EA_ATTACK:
		botlib_export->ea.EA_Attack( args[0] );
		return SysCallArg();
	case BOTLIB_EA_USE:
		botlib_export->ea.EA_Use( args[0] );
		return SysCallArg();
	case BOTLIB_EA_RESPAWN:
		botlib_export->ea.EA_Respawn( args[0] );
		return SysCallArg();
	case BOTLIB_EA_CROUCH:
		botlib_export->ea.EA_Crouch( args[0] );
		return SysCallArg();
	case BOTLIB_EA_MOVE_UP:
		botlib_export->ea.EA_MoveUp( args[0] );
		return SysCallArg();
	case BOTLIB_EA_MOVE_DOWN:
		botlib_export->ea.EA_MoveDown( args[0] );
		return SysCallArg();
	case BOTLIB_EA_MOVE_FORWARD:
		botlib_export->ea.EA_MoveForward( args[0] );
		return SysCallArg();
	case BOTLIB_EA_MOVE_BACK:
		botlib_export->ea.EA_MoveBack( args[0] );
		return SysCallArg();
	case BOTLIB_EA_MOVE_LEFT:
		botlib_export->ea.EA_MoveLeft( args[0] );
		return SysCallArg();
	case BOTLIB_EA_MOVE_RIGHT:
		botlib_export->ea.EA_MoveRight( args[0] );
		return SysCallArg();

	case BOTLIB_EA_SELECT_WEAPON:
		botlib_export->ea.EA_SelectWeapon( args[0], args[1] );
		return SysCallArg();
	case BOTLIB_EA_JUMP:
		botlib_export->ea.EA_Jump( args[0] );
		return SysCallArg();
	case BOTLIB_EA_DELAYED_JUMP:
		botlib_export->ea.EA_DelayedJump( args[0] );
		return SysCallArg();
	case BOTLIB_EA_MOVE:
		botlib_export->ea.EA_Move(args[0], args[1],args[2]);
		return SysCallArg();
	case BOTLIB_EA_VIEW:
		botlib_export->ea.EA_View(args[0],args[1] );
		return SysCallArg();

	case BOTLIB_EA_END_REGULAR:
		botlib_export->ea.EA_EndRegular(args[0],args[1]);
		return SysCallArg();
	case BOTLIB_EA_GET_INPUT:
		botlib_export->ea.EA_GetInput(args[0],args[1],args[2] );
		return SysCallArg();
	case BOTLIB_EA_RESET_INPUT:
		botlib_export->ea.EA_ResetInput( args[0]);
		return SysCallArg();

	case BOTLIB_AI_LOAD_CHARACTER:
		return SysCallArg::Int(botlib_export->ai.BotLoadCharacter(args[0],args[1] ));
	case BOTLIB_AI_FREE_CHARACTER:
		botlib_export->ai.BotFreeCharacter( args[0]);
		return SysCallArg();
	case BOTLIB_AI_CHARACTERISTIC_FLOAT:
		return SysCallArg::Fixed( botlib_export->ai.Characteristic_Float( args[0],args[1] ) );
	case BOTLIB_AI_CHARACTERISTIC_BFLOAT:
		return SysCallArg::Fixed( botlib_export->ai.Characteristic_BFloat( args[0],args[1],args[2],args[3] ) );
	case BOTLIB_AI_CHARACTERISTIC_INTEGER:
		return SysCallArg::Int(botlib_export->ai.Characteristic_Integer( args[0],args[1] ) );
	case BOTLIB_AI_CHARACTERISTIC_BINTEGER:
		return SysCallArg::Int(botlib_export->ai.Characteristic_BInteger( args[0],args[1],args[2],args[3] ));
	case BOTLIB_AI_CHARACTERISTIC_STRING:
		botlib_export->ai.Characteristic_String( args[0],args[1],args[2],args[3] );
		return SysCallArg();

	case BOTLIB_AI_ALLOC_CHAT_STATE:
		return SysCallArg::Int(botlib_export->ai.BotAllocChatState());
	case BOTLIB_AI_FREE_CHAT_STATE:
		botlib_export->ai.BotFreeChatState( args[0] );
		return SysCallArg();
	case BOTLIB_AI_QUEUE_CONSOLE_MESSAGE:
		botlib_export->ai.BotQueueConsoleMessage( args[0],args[1],args[2] );
		return SysCallArg();
	case BOTLIB_AI_REMOVE_CONSOLE_MESSAGE:
		botlib_export->ai.BotRemoveConsoleMessage( args[0],args[1] );
		return SysCallArg();
	case BOTLIB_AI_NEXT_CONSOLE_MESSAGE:
		return SysCallArg::Int(botlib_export->ai.BotNextConsoleMessage( args[0],args[1] ));
	case BOTLIB_AI_NUM_CONSOLE_MESSAGE:
		return SysCallArg::Int(botlib_export->ai.BotNumConsoleMessages( args[0] ));
	case BOTLIB_AI_INITIAL_CHAT:
		botlib_export->ai.BotInitialChat( args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10]);
		return SysCallArg();
	case BOTLIB_AI_NUM_INITIAL_CHATS:
		return SysCallArg::Int(botlib_export->ai.BotNumInitialChats( args[0],args[1] ));
	case BOTLIB_AI_REPLY_CHAT:
		return SysCallArg::Int(botlib_export->ai.BotReplyChat( args[0], args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11] ));
	case BOTLIB_AI_CHAT_LENGTH:
		return SysCallArg::Int(botlib_export->ai.BotChatLength( args[0] ));
	case BOTLIB_AI_ENTER_CHAT:
		botlib_export->ai.BotEnterChat( args[0], args[1], args[2] );
		return SysCallArg();
	case BOTLIB_AI_GET_CHAT_MESSAGE:
		botlib_export->ai.BotGetChatMessage( args[0], args[1], args[2] );
		return SysCallArg();
	case BOTLIB_AI_STRING_CONTAINS:
		return SysCallArg::Int(botlib_export->ai.StringContains( args[0], args[1], args[2] ));
	case BOTLIB_AI_FIND_MATCH:
		return SysCallArg::Int(botlib_export->ai.BotFindMatch( args[0], args[1], (int)args[2] ));
	case BOTLIB_AI_MATCH_VARIABLE:
		botlib_export->ai.BotMatchVariable( args[0], args[1], args[2], args[3] );
		return SysCallArg();
	case BOTLIB_AI_UNIFY_WHITE_SPACES:
		botlib_export->ai.UnifyWhiteSpaces( args[0] );
		return SysCallArg();
	case BOTLIB_AI_REPLACE_SYNONYMS:
		botlib_export->ai.BotReplaceSynonyms( args[0], (int)args[1] );
		return SysCallArg();
	case BOTLIB_AI_LOAD_CHAT_FILE:
		return SysCallArg::Int(botlib_export->ai.BotLoadChatFile( args[0], args[1], args[2] ));
	case BOTLIB_AI_SET_CHAT_GENDER:
		botlib_export->ai.BotSetChatGender( args[0], args[1] );
		return SysCallArg();
	case BOTLIB_AI_SET_CHAT_NAME:
		botlib_export->ai.BotSetChatName( args[0], args[1], args[2] );
		return SysCallArg();

	case BOTLIB_AI_RESET_GOAL_STATE:
		botlib_export->ai.BotResetGoalState( args[0] );
		return SysCallArg();
	case BOTLIB_AI_RESET_AVOID_GOALS:
		botlib_export->ai.BotResetAvoidGoals( args[0] );
		return SysCallArg();
	case BOTLIB_AI_REMOVE_FROM_AVOID_GOALS:
		botlib_export->ai.BotRemoveFromAvoidGoals( args[0], args[1] );
		return SysCallArg();
	case BOTLIB_AI_PUSH_GOAL:
		botlib_export->ai.BotPushGoal( args[0], args[1] );
		return SysCallArg();
	case BOTLIB_AI_POP_GOAL:
		botlib_export->ai.BotPopGoal( args[0] );
		return SysCallArg();
	case BOTLIB_AI_EMPTY_GOAL_STACK:
		botlib_export->ai.BotEmptyGoalStack( args[0] );
		return SysCallArg();
	case BOTLIB_AI_DUMP_AVOID_GOALS:
		botlib_export->ai.BotDumpAvoidGoals( args[0] );
		return SysCallArg();
	case BOTLIB_AI_DUMP_GOAL_STACK:
		botlib_export->ai.BotDumpGoalStack( args[0] );
		return SysCallArg();
	case BOTLIB_AI_GOAL_NAME:
		botlib_export->ai.BotGoalName( args[0], args[1], args[2] );
		return SysCallArg();
	case BOTLIB_AI_GET_TOP_GOAL:
		return SysCallArg::Int(botlib_export->ai.BotGetTopGoal( args[0], args[1] ));
	case BOTLIB_AI_GET_SECOND_GOAL:
		return SysCallArg::Int(botlib_export->ai.BotGetSecondGoal( args[0], args[1] ));
	case BOTLIB_AI_CHOOSE_LTG_ITEM:
		return SysCallArg::Int(botlib_export->ai.BotChooseLTGItem( args[0], args[1], args[2], args[3] ));
	case BOTLIB_AI_CHOOSE_NBG_ITEM:
		return SysCallArg::Int(botlib_export->ai.BotChooseNBGItem( args[0], args[1], args[2], args[3], args[4], args[5] ));
	case BOTLIB_AI_TOUCHING_GOAL:
		return SysCallArg::Int(botlib_export->ai.BotTouchingGoal( args[0], args[1] ));
	case BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE:
		return SysCallArg::Int(botlib_export->ai.BotItemGoalInVisButNotVisible( args[0], args[1], args[2], args[3] ));
	case BOTLIB_AI_GET_LEVEL_ITEM_GOAL:
		return SysCallArg::Int(botlib_export->ai.BotGetLevelItemGoal( args[0], args[1], args[2] ));
	case BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL:
		return SysCallArg::Int(botlib_export->ai.BotGetNextCampSpotGoal( args[0], args[1] ));
	case BOTLIB_AI_GET_MAP_LOCATION_GOAL:
		return SysCallArg::Int(botlib_export->ai.BotGetMapLocationGoal( args[0], args[1] ));
	case BOTLIB_AI_AVOID_GOAL_TIME:
		return SysCallArg::Fixed( botlib_export->ai.BotAvoidGoalTime( args[0], args[1] ) );
	case BOTLIB_AI_SET_AVOID_GOAL_TIME:
		botlib_export->ai.BotSetAvoidGoalTime( args[0], args[1], args[2]);
		return SysCallArg();
	case BOTLIB_AI_INIT_LEVEL_ITEMS:
		botlib_export->ai.BotInitLevelItems();
		return SysCallArg();
	case BOTLIB_AI_UPDATE_ENTITY_ITEMS:
		botlib_export->ai.BotUpdateEntityItems();
		return SysCallArg();
	case BOTLIB_AI_LOAD_ITEM_WEIGHTS:
		return SysCallArg::Int(botlib_export->ai.BotLoadItemWeights( args[0], args[1] ));
	case BOTLIB_AI_FREE_ITEM_WEIGHTS:
		botlib_export->ai.BotFreeItemWeights( args[0] );
		return SysCallArg();
	case BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC:
		botlib_export->ai.BotInterbreedGoalFuzzyLogic( args[0], args[1], args[2] );
		return SysCallArg();
	case BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC:
		botlib_export->ai.BotSaveGoalFuzzyLogic( args[0], args[1] );
		return SysCallArg();
	case BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC:
		botlib_export->ai.BotMutateGoalFuzzyLogic( args[0], args[1] );
		return SysCallArg();
	case BOTLIB_AI_ALLOC_GOAL_STATE:
		return SysCallArg::Int(botlib_export->ai.BotAllocGoalState( args[0] ));
	case BOTLIB_AI_FREE_GOAL_STATE:
		botlib_export->ai.BotFreeGoalState( args[0] );
		return SysCallArg();

	case BOTLIB_AI_RESET_MOVE_STATE:
		botlib_export->ai.BotResetMoveState( args[0] );
		return SysCallArg();
	case BOTLIB_AI_ADD_AVOID_SPOT:
		botlib_export->ai.BotAddAvoidSpot( args[0], args[1], args[2], args[3] );
		return SysCallArg();
	case BOTLIB_AI_MOVE_TO_GOAL:
		botlib_export->ai.BotMoveToGoal( args[0], args[1], args[2], args[3] );
		return SysCallArg();
	case BOTLIB_AI_MOVE_IN_DIRECTION:
		return SysCallArg::Int(botlib_export->ai.BotMoveInDirection( args[0], args[1], args[2], args[3] ));
	case BOTLIB_AI_RESET_AVOID_REACH:
		botlib_export->ai.BotResetAvoidReach( args[0] );
		return SysCallArg();
	case BOTLIB_AI_RESET_LAST_AVOID_REACH:
		botlib_export->ai.BotResetLastAvoidReach( args[0] );
		return SysCallArg();
	case BOTLIB_AI_REACHABILITY_AREA:
		return SysCallArg::Int(botlib_export->ai.BotReachabilityArea( args[0], args[1] ));
	case BOTLIB_AI_MOVEMENT_VIEW_TARGET:
		return SysCallArg::Int(botlib_export->ai.BotMovementViewTarget( args[0], args[1], args[2], args[3], args[4] ));
	case BOTLIB_AI_PREDICT_VISIBLE_POSITION:
		return SysCallArg::Int(botlib_export->ai.BotPredictVisiblePosition( args[0], args[1], args[2], args[3], args[4] ));
	case BOTLIB_AI_ALLOC_MOVE_STATE:
		return SysCallArg::Int(botlib_export->ai.BotAllocMoveState());
	case BOTLIB_AI_FREE_MOVE_STATE:
		botlib_export->ai.BotFreeMoveState( args[0] );
		return SysCallArg();
	case BOTLIB_AI_INIT_MOVE_STATE:
		botlib_export->ai.BotInitMoveState( args[0], args[1] );
		return SysCallArg();

	case BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON:
		return SysCallArg::Int(botlib_export->ai.BotChooseBestFightWeapon( args[0], args[1] ));
	case BOTLIB_AI_GET_WEAPON_INFO:
		botlib_export->ai.BotGetWeaponInfo( args[0], args[1], args[2] );
		return SysCallArg();
	case BOTLIB_AI_LOAD_WEAPON_WEIGHTS:
		return SysCallArg::Int(botlib_export->ai.BotLoadWeaponWeights( args[0], args[1] ));
	case BOTLIB_AI_ALLOC_WEAPON_STATE:
		return SysCallArg::Int(botlib_export->ai.BotAllocWeaponState());
	case BOTLIB_AI_FREE_WEAPON_STATE:
		botlib_export->ai.BotFreeWeaponState( args[0] );
		return SysCallArg();
	case BOTLIB_AI_RESET_WEAPON_STATE:
		botlib_export->ai.BotResetWeaponState( args[0] );
		return SysCallArg();

	case BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION:
		return SysCallArg::Int(botlib_export->ai.GeneticParentsAndChildSelection(args[0], args[1], args[2], args[3], args[4]));

	case TRAP_MEMSET:
		Com_Memset( args[0], (int)args[1], (int)args[2] );
		return SysCallArg();

	case TRAP_MEMCPY:
		Com_Memcpy( args[0], args[1], (int)args[2] );
		return SysCallArg();

	case TRAP_STRNCPY:
		return SysCallArg::Ptr(strncpy( args[0], args[1], (int)args[2] ));

	case TRAP_SIN:
		return SysCallArg::Fixed( FIXED_SIN( (gfixed)args[0] ) );

	case TRAP_COS:
		return SysCallArg::Fixed( FIXED_COS( (gfixed)args[0] ) );

	case TRAP_ATAN2:
		return SysCallArg::Fixed( FIXED_ATAN2( (gfixed)args[0], (gfixed)args[1] ) );

	case TRAP_SQRT:
		return SysCallArg::Fixed( FIXED_SQRT( (gfixed)args[0] ) );

	case TRAP_MATRIXMULTIPLY:
		MatrixMultiply( *(bmat_33 *)(void *)args[0], *(bmat_33 *)(void *)args[1], *(bmat_33 *)(void *)args[2] );
		return SysCallArg();

	case TRAP_ANGLEVECTORS:
		AngleVectors( args[0], args[1], args[2], args[3] );
		return SysCallArg();

	case TRAP_PERPENDICULARVECTOR_A:
		PerpendicularVector( (afixed *)(void *)args[0], (afixed *)(void *)args[1]);
		return SysCallArg();

	case TRAP_PERPENDICULARVECTOR_B:
		PerpendicularVector( (bfixed *)(void *)args[0], (bfixed *)(void *)args[1] );
		return SysCallArg();

	case TRAP_FLOOR:
		return SysCallArg::Fixed( FIXED_FLOOR( (gfixed)args[0] ) );

	case TRAP_CEIL:
		return SysCallArg::Fixed( FIXED_CEIL( (gfixed)args[0] ) );

	default:
		Com_Error( ERR_DROP, "Bad game system trap: %i", callnum);
	}

	SysCallArg ret;
	ret=-1;

	return ret;
}

/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs( void ) {
	if ( !gvm ) {
		return;
	}
	SysCallArgs args(1);
	args[0]=qfalse;
	VM_Call( gvm, GAME_SHUTDOWN, args);

	VM_Free( gvm );
	gvm = NULL;
}

/*
==================
SV_InitGameVM

Called for both a full init and a restart
==================
*/
static void SV_InitGameVM( qboolean restart ) {
	int		i;

	// start the entity parsing at the beginning
	sv.entityParsePoint = CM_EntityString();

	// clear all gentity pointers that might still be set from
	// a previous level
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=522
	//   now done before GAME_INIT call
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		svs.clients[i].gentity = NULL;
	}
	
	// use the current msec count for a random seed
	// init for this gamestate
	SysCallArgs args(3);
	args[0]=svs.time;
	args[1]=Com_Milliseconds();
	args[2]=restart;
	VM_Call( gvm, GAME_INIT, args );
}



/*
===================
SV_RestartGameProgs

Called on a map_restart, but not on a normal map change
===================
*/
void SV_RestartGameProgs( void ) {
	if ( !gvm ) {
		return;
	}

	SysCallArgs args(1);
	args[0]=qtrue;
	VM_Call( gvm, GAME_SHUTDOWN, args);

	// do a restart instead of a free
	gvm = VM_Restart( gvm );
	if ( !gvm ) { // bk001212 - as done below
		Com_Error( ERR_FATAL, "VM_Restart on game failed" );
	}

	SV_InitGameVM( qtrue );
}


/*
===============
SV_InitGameProgs

Called on a normal map change, not on a map_restart
===============
*/
void SV_InitGameProgs( void ) {
	cvar_t	*var;
	//FIXME these are temp while I make bots run in vm
	extern int	bot_enable;

	var = Cvar_Get( "bot_enable", "1", CVAR_LATCH );
	if ( var ) {
		bot_enable = var->integer;
	}
	else {
		bot_enable = 0;
	}

	// load the dll or bytecode
	gvm = VM_Create( "qagame", SV_GameSystemCalls, (vmInterpret_t )Cvar_VariableIntegerValue( "vm_game" ) );
	if ( !gvm ) {
		Com_Error( ERR_FATAL, "VM_Create on game failed" );
	}

	SV_InitGameVM( qfalse );
}


/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
qboolean SV_GameCommand( void ) {
	if ( sv.state != SS_GAME ) {
		return qfalse;
	}

	return VM_Call( gvm, GAME_CONSOLE_COMMAND, SysCallArgs(0) );
}

