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
#include"game_pch.h"

// this file is only included when building a dll
// g__G_syscalls.asm is included instead when building a qvm
#ifdef Q3_VM
#error "Do not use in VM build"
#endif

static SysCallArg (QDECL *_G_syscall)(int id, const SysCallArgs &args) = (SysCallArg (QDECL *)(int, const SysCallArgs &args))-1;

EXTERN_C DLLEXPORT void dllEntry( SysCallArg (QDECL *syscallptr)(int id, const SysCallArgs &args) )
{
	_G_syscall = syscallptr;
}


void	_G_trap_Printf( const char *fmt ) {
	SysCallArgs args(1);
	args[0]=fmt;
	_G_syscall( G_PRINT, args );
}

void	_G_trap_Error( const char *fmt ) {
	SysCallArgs args(1);
	args[0]=fmt;
	_G_syscall( G_ERROR, args );
}

int		_G_trap_Milliseconds( void ) {
	return (int)_G_syscall( G_MILLISECONDS,SysCallArgs(0) ); 
}
int		_G_trap_Argc( void ) {
	return (int)_G_syscall( G_ARGC,SysCallArgs(0) );
}

void	_G_trap_Argv( int n, char *buffer, int bufferLength ) {
	SysCallArgs args(3);
	args[0]=n;
	args[1]=buffer;
	args[2]=bufferLength;
	_G_syscall( G_ARGV, args);
}

int		_G_trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	SysCallArgs args(3);
	args[0]=qpath;
	args[1]=f;
	args[2]=(int)mode;
	return (int)_G_syscall( G_FS_FOPEN_FILE, args);
}

void	_G_trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	SysCallArgs args(3);
	args[0]=buffer;
	args[1]=len;
	args[2]=(int)f;
	_G_syscall( G_FS_READ, args);
}

void	_G_trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	SysCallArgs args(3);
	args[0]=buffer;
	args[1]=len;
	args[2]=(int)f;
	_G_syscall( G_FS_WRITE, args);
}

void	_G_trap_FS_FCloseFile( fileHandle_t f ) {
	SysCallArgs args(1);
	args[0]=(int)f;
	_G_syscall( G_FS_FCLOSE_FILE, args);
}

int _G_trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	SysCallArgs args(4);
	args[0]=path;
	args[1]=extension;
	args[2]=listbuf;
	args[3]=bufsize;
	return (int)_G_syscall( G_FS_GETFILELIST, args);
}

int _G_trap_FS_Seek( fileHandle_t f, long offset, int origin ) {
	SysCallArgs args(3);
	args[0]=(int)f;
	args[1]=offset;
	args[2]=origin;
	return (int)_G_syscall( G_FS_SEEK, args );
}

void	_G_trap_SendConsoleCommand( int exec_when, const char *text ) {
	SysCallArgs args(2);
	args[0]=exec_when;
	args[1]=text;
	_G_syscall( G_SEND_CONSOLE_COMMAND, args );
}

void	_G_trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	SysCallArgs args(4);
	args[0]=cvar;
	args[1]=var_name;
	args[2]=value;
	args[3]=flags;
	_G_syscall( G_CVAR_REGISTER, args );
}

void	_G_trap_Cvar_Update( vmCvar_t *cvar ) {
	SysCallArgs args(1);
	args[0]=cvar;
	_G_syscall( G_CVAR_UPDATE, args );
}

void _G_trap_Cvar_Set( const char *var_name, const char *value ) {
	SysCallArgs args(2);
	args[0]=var_name;
	args[1]=value;
	_G_syscall( G_CVAR_SET, args );
}

int _G_trap_Cvar_VariableIntegerValue( const char *var_name ) {
	SysCallArgs args(1);
	args[0]=var_name;
	return (int)_G_syscall( G_CVAR_VARIABLE_INTEGER_VALUE, args );
}

void _G_trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	SysCallArgs args(3);
	args[0]=var_name;
	args[1]=buffer;
	args[2]=bufsize;
	_G_syscall( G_CVAR_VARIABLE_STRING_BUFFER, args);
}


void _G_trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						 playerState_t *clients, int sizeofGClient ) {
	SysCallArgs args(5);
	args[0]=gEnts;
	args[1]=numGEntities;
	args[2]=sizeofGEntity_t;
	args[3]=clients;
	args[4]=sizeofGClient;
	_G_syscall( G_LOCATE_GAME_DATA, args);
}

void _G_trap_DropClient( int clientNum, const char *reason ) {
	SysCallArgs args(2);
	args[0]=clientNum;
	args[1]=reason;
	_G_syscall( G_DROP_CLIENT, args);
}

void _G_trap_SendServerCommand( int clientNum, const char *text ) {
	SysCallArgs args(2);
	args[0]=clientNum;
	args[1]=text;
	_G_syscall( G_SEND_SERVER_COMMAND, args);
}

void _G_trap_SetConfigstring( int num, const char *string ) {
	SysCallArgs args(2);
	args[0]=num;
	args[1]=string;
	_G_syscall( G_SET_CONFIGSTRING, args);
}

void _G_trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	SysCallArgs args(3);
	args[0]=num;
	args[1]=buffer;
	args[2]=bufferSize;
	_G_syscall( G_GET_CONFIGSTRING, args );
}

void _G_trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	SysCallArgs args(3);
	args[0]=num;
	args[1]=buffer;
	args[2]=bufferSize;
	_G_syscall( G_GET_USERINFO, args);
}

void _G_trap_SetUserinfo( int num, const char *buffer ) {
	SysCallArgs args(2);
	args[0]=num;
	args[1]=buffer;
	_G_syscall( G_SET_USERINFO, args );
}

void _G_trap_GetServerinfo( char *buffer, int bufferSize ) {
	SysCallArgs args(2);
	args[0]=buffer;
	args[1]=bufferSize;
	_G_syscall( G_GET_SERVERINFO, args);
}

void _G_trap_SetBrushModel( gentity_t *ent, const char *name ) {
	SysCallArgs args(2);
	args[0]=ent;
	args[1]=name;
	_G_syscall( G_SET_BRUSH_MODEL, args);
}

void _G_trap_Trace( trace_t *results, const bvec3_t start, const bvec3_t mins, const bvec3_t maxs, 
				   const bvec3_t end, int passEntityNum, int contentmask ) {
	SysCallArgs args(7);
	args[0]=results;
	args[1]=start;
	args[2]=mins;
	args[3]=maxs;
	args[4]=end;
	args[5]=passEntityNum;
	args[6]=contentmask;
	_G_syscall( G_TRACE, args );
}

void _G_trap_TraceCapsule( trace_t *results, const bvec3_t start, const bvec3_t mins, const bvec3_t maxs, 
						  const bvec3_t end, int passEntityNum, int contentmask ) {
	SysCallArgs args(7);
	args[0]=results;
	args[1]=start;
	args[2]=mins;
	args[3]=maxs;
	args[4]=end;
	args[5]=passEntityNum;
	args[6]=contentmask;
	_G_syscall( G_TRACECAPSULE, args );
}

int _G_trap_PointContents( const bvec3_t point, int passEntityNum ) {
	SysCallArgs args(2);
	args[0]=point;
	args[1]=passEntityNum;
	return (int)_G_syscall( G_POINT_CONTENTS, args );
}


qboolean _G_trap_InPVS( const bvec3_t p1, const bvec3_t p2 ) {
	SysCallArgs args(2);
	args[0]=p1;
	args[1]=p2;
	return (int)(qboolean)_G_syscall( G_IN_PVS, args );
}

qboolean _G_trap_InPVSIgnorePortals( const bvec3_t p1, const bvec3_t p2 ) {
	SysCallArgs args(2);
	args[0]=p1;
	args[1]=p2;
	return (int)(qboolean)_G_syscall( G_IN_PVS_IGNORE_PORTALS, args);
}

void _G_trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	SysCallArgs args(2);
	args[0]=ent;
	args[1]=open;
	_G_syscall( G_ADJUST_AREA_PORTAL_STATE, args);
}

qboolean _G_trap_AreasConnected( int area1, int area2 ) {
	SysCallArgs args(2);
	args[0]=area1;
	args[1]=area2;
	return (int)(qboolean)_G_syscall( G_AREAS_CONNECTED, args );
}

void _G_trap_LinkEntity( gentity_t *ent ) {
	SysCallArgs args(1);
	args[0]=ent;
	_G_syscall( G_LINKENTITY, args );
}

void _G_trap_UnlinkEntity( gentity_t *ent ) {
	SysCallArgs args(1);
	args[0]=ent;
	_G_syscall( G_UNLINKENTITY, args );
}

int _G_trap_EntitiesInBox( const bvec3_t mins, const bvec3_t maxs, int *list, int maxcount ) {
	SysCallArgs args(4);
	args[0]=mins;
	args[1]=maxs;
	args[2]=list;
	args[3]=maxcount;
	return (int)_G_syscall( G_ENTITIES_IN_BOX, args );
}

qboolean _G_trap_EntityContact( const bvec3_t mins, const bvec3_t maxs, const gentity_t *ent ) {
	SysCallArgs args(3);
	args[0]=mins;
	args[1]=maxs;
	args[2]=ent;
	return (int)(qboolean)_G_syscall( G_ENTITY_CONTACT, args );
}

qboolean _G_trap_EntityContactCapsule( const bvec3_t mins, const bvec3_t maxs, const gentity_t *ent ) {
	SysCallArgs args(3);
	args[0]=mins;
	args[1]=maxs;
	args[2]=ent;
	return (int)(qboolean)_G_syscall( G_ENTITY_CONTACTCAPSULE, args);
}

int _G_trap_BotAllocateClient( void ) {
	return (int)_G_syscall( G_BOT_ALLOCATE_CLIENT,SysCallArgs(0) );
}

void _G_trap_BotFreeClient( int clientNum ) {
	SysCallArgs args(1);
	args[0]=clientNum;
	_G_syscall( G_BOT_FREE_CLIENT, args );
}

void _G_trap_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	SysCallArgs args(2);
	args[0]=clientNum;
	args[1]=cmd;
	_G_syscall( G_GET_USERCMD, args );
}

qboolean _G_trap_GetEntityToken( char *buffer, int bufferSize ) {
	SysCallArgs args(2);
	args[0]=buffer;
	args[1]=bufferSize;
	return (int)(qboolean)_G_syscall( G_GET_ENTITY_TOKEN, args);
}

int _G_trap_DebugPolygonCreate(int color, int numPoints, bvec3_t *points) {
	SysCallArgs args(3);
	args[0]=color;
	args[1]=numPoints;
	args[2]=points;
	return (int)_G_syscall( G_DEBUG_POLYGON_CREATE, args );
}

void _G_trap_DebugPolygonDelete(int id) {
	SysCallArgs args(1);
	args[0]=id;
	_G_syscall( G_DEBUG_POLYGON_DELETE, args);
}

int _G_trap_RealTime( qtime_t *qtime ) {
	SysCallArgs args(1);
	args[0]=qtime;
	return (int)_G_syscall( G_REAL_TIME, args);
}

// BotLib traps start here
int _G_trap_BotLibSetup( void ) {
	return (int)_G_syscall( BOTLIB_SETUP,SysCallArgs(0) );
}

int _G_trap_BotLibShutdown( void ) {
	return (int)_G_syscall( BOTLIB_SHUTDOWN,SysCallArgs(0) );
}

int _G_trap_BotLibVarSet(const char *var_name, const char *value) {
	SysCallArgs args(2);
	args[0]=var_name;
	args[1]=value;
	return (int)_G_syscall( BOTLIB_LIBVAR_SET, args);
}

int _G_trap_BotLibVarGet(const char *var_name, char *value, int size) {
	SysCallArgs args(3);
	args[0]=var_name;
	args[1]=value;
	args[2]=size;
	return (int)_G_syscall( BOTLIB_LIBVAR_GET, args);
}

int _G_trap_BotLibDefine(const char *string) {
	SysCallArgs args(1);
	args[0]=string;
	return (int)_G_syscall( BOTLIB_PC_ADD_GLOBAL_DEFINE, args );
}

int _G_trap_BotLibStartFrame(gfixed time) {
	SysCallArgs args(1);
	args[0]=time;
	return (int)_G_syscall( BOTLIB_START_FRAME, args );
}

int _G_trap_BotLibLoadMap(const char *mapname) {
	SysCallArgs args(1);
	args[0]=mapname;
	return (int)_G_syscall( BOTLIB_LOAD_MAP, args );
}

int _G_trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue) {
	SysCallArgs args(2);
	args[0]=ent;
	args[1]=bue;
	return (int)_G_syscall( BOTLIB_UPDATENTITY, args );
}

int _G_trap_BotLibTest(int parm0, char *parm1, bvec3_t parm2, bvec3_t parm3) {
	SysCallArgs args(4);
	args[0]=parm0;
	args[1]=parm1;
	args[2]=parm2;
	args[3]=parm3;
	return (int)_G_syscall( BOTLIB_TEST, args );
}

int _G_trap_BotGetSnapshotEntity( int clientNum, int sequence ) {
	SysCallArgs args(2);
	args[0]=clientNum;
	args[1]=sequence;
	return (int)_G_syscall( BOTLIB_GET_SNAPSHOT_ENTITY, args);
}

int _G_trap_BotGetServerCommand(int clientNum, char *message, int size) {
	SysCallArgs args(3);
	args[0]=clientNum;
	args[1]=message;
	args[2]=size;
	return (int)_G_syscall( BOTLIB_GET_CONSOLE_MESSAGE, args);
}

void _G_trap_BotUserCommand(int clientNum, usercmd_t *ucmd) {
	SysCallArgs args(2);
	args[0]=clientNum;
	args[1]=ucmd;
	_G_syscall( BOTLIB_USER_COMMAND, args);
}

void _G_trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info) {
	SysCallArgs args(2);
	args[0]=entnum;
	args[1]=info;
	_G_syscall( BOTLIB_AAS_ENTITY_INFO, args);
}

int _G_trap_AAS_Initialized(void) {
	return (int)_G_syscall( BOTLIB_AAS_INITIALIZED,SysCallArgs(0) );
}

void _G_trap_AAS_PresenceTypeBoundingBox(int presencetype, bvec3_t mins, bvec3_t maxs) {
	SysCallArgs args(3);
	args[0]=presencetype;
	args[1]=mins;
	args[2]=maxs;
	_G_syscall( BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX, args);
}

gfixed _G_trap_AAS_Time(void) {
	return (gfixed)_G_syscall( BOTLIB_AAS_TIME,SysCallArgs(0) );
}

int _G_trap_AAS_PointAreaNum(bvec3_t point) {
	SysCallArgs args(1);
	args[0]=point;
	return (int)_G_syscall( BOTLIB_AAS_POINT_AREA_NUM, args );
}

int _G_trap_AAS_PointReachabilityAreaIndex(bvec3_t point) {
	SysCallArgs args(1);
	args[0]=point;
	return (int)_G_syscall( BOTLIB_AAS_POINT_REACHABILITY_AREA_INDEX, args );
}

int _G_trap_AAS_TraceAreas(bvec3_t start, bvec3_t end, int *areas, bvec3_t *points, int maxareas) {
	SysCallArgs args(5);
	args[0]=start;
	args[1]=end;
	args[2]=areas;
	args[3]=points;
	args[4]=maxareas;

	return (int)_G_syscall( BOTLIB_AAS_TRACE_AREAS, args);
}

int _G_trap_AAS_BBoxAreas(bvec3_t absmins, bvec3_t absmaxs, int *areas, int maxareas) {
	SysCallArgs args(4);
	args[0]=absmins;
	args[1]=absmaxs;
	args[2]=areas;
	args[3]=maxareas;
	return (int)_G_syscall( BOTLIB_AAS_BBOX_AREAS, args);
}

int _G_trap_AAS_AreaInfo( int areanum, void /* struct aas_areainfo_s */ *info ) {
	SysCallArgs args(2);
	args[0]=areanum;
	args[1]=info;
	return (int)_G_syscall( BOTLIB_AAS_AREA_INFO, args );
}

int _G_trap_AAS_PointContents(bvec3_t point) {
	SysCallArgs args(1);
	args[0]=point;
	return (int)_G_syscall( BOTLIB_AAS_POINT_CONTENTS, args );
}

int _G_trap_AAS_NextBSPEntity(int ent) {
	SysCallArgs args(1);
	args[0]=ent;
	return (int)_G_syscall( BOTLIB_AAS_NEXT_BSP_ENTITY, args );
}

int _G_trap_AAS_ValueForBSPEpairKey(int ent, const char *key, char *value, int size) {
	SysCallArgs args(4);
	args[0]=ent;
	args[1]=key;
	args[2]=value;
	args[3]=size;
	return (int)_G_syscall( BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY, args);
}

int _G_trap_AAS_VectorForBSPEpairKey(int ent, const char *key, bvec3_t v) {
	SysCallArgs args(3);
	args[0]=ent;
	args[1]=key;
	args[2]=v;
	return (int)_G_syscall( BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY, args );
}

int _G_trap_AAS_FloatForBSPEpairKey(int ent, const char *key, gfixed *value) {
	SysCallArgs args(3);
	args[0]=ent;
	args[1]=key;
	args[2]=value;
	return (int)_G_syscall( BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY, args);
}

int _G_trap_AAS_IntForBSPEpairKey(int ent, const char *key, int *value) {
	SysCallArgs args(3);
	args[0]=ent;
	args[1]=key;
	args[2]=value;
	return (int)_G_syscall( BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY, args);
}

int _G_trap_AAS_AreaReachability(int areanum) {
	SysCallArgs args(1);
	args[0]=areanum;
	return (int)_G_syscall( BOTLIB_AAS_AREA_REACHABILITY, args );
}

int _G_trap_AAS_AreaTravelTimeToGoalArea(int areanum, bvec3_t origin, int goalareanum, int travelflags) {
	SysCallArgs args(4);
	args[0]=areanum;
	args[1]=origin;
	args[2]=goalareanum;
	args[3]=travelflags;
	return (int)_G_syscall( BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA, args );
}

int _G_trap_AAS_EnableRoutingArea( int areanum, int enable ) {
	SysCallArgs args(2);
	args[0]=areanum;
	args[1]=enable;
	return (int)_G_syscall( BOTLIB_AAS_ENABLE_ROUTING_AREA, args );
}

int _G_trap_AAS_PredictRoute(void /*struct aas_predictroute_s*/ *route, int areanum, bvec3_t origin,
							int goalareanum, int travelflags, int maxareas, int maxtime,
							int stopevent, int stopcontents, int stoptfl, int stopareanum) {
	SysCallArgs args(11);
	args[0]=route;
	args[1]=areanum;
	args[2]=origin;
	args[3]=goalareanum;
	args[4]=travelflags;
	args[5]=maxareas;
	args[6]=maxtime;
	args[7]=stopevent;
	args[8]=stopcontents;
	args[9]=stoptfl;
	args[10]=stopareanum;
	return (int)_G_syscall( BOTLIB_AAS_PREDICT_ROUTE, args );
}

int _G_trap_AAS_AlternativeRouteGoals(bvec3_t start, int startareanum, bvec3_t goal, int goalareanum, int travelflags,
										void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals,
										int type) {
	SysCallArgs args(8);
	args[0]=start;
	args[1]=startareanum;
	args[2]=goal;
	args[3]=goalareanum;
	args[4]=travelflags;
	args[5]=altroutegoals;
	args[6]=maxaltroutegoals;
	args[7]=type;
	return (int)_G_syscall( BOTLIB_AAS_ALTERNATIVE_ROUTE_GOAL, args );
}

int _G_trap_AAS_Swimming(bvec3_t origin) {
	SysCallArgs args(1);
	args[0]=origin;
	return (int)_G_syscall( BOTLIB_AAS_SWIMMING, args);
}

int _G_trap_AAS_PredictClientMovement(void /* struct aas_clientmove_s */ *move, int entnum, bvec3_t origin, int presencetype, 
									  int onground, bvec3_t velocity, bvec3_t cmdmove, int cmdframes, int maxframes, 
									  gfixed frametime, int stopevent, int stopareanum, int visualize) {
	SysCallArgs args(13);
	args[0]=move;
	args[1]=entnum;
	args[2]=origin;
	args[3]=presencetype;
	args[4]=onground;
	args[5]=velocity;
	args[6]=cmdmove;
	args[7]=cmdframes;
	args[8]=maxframes;
	args[9]=frametime;
	args[10]=stopevent;
	args[11]=stopareanum;
	args[12]=visualize;
	return (int)_G_syscall( BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT, args);
}

void _G_trap_EA_Say(int client, const char *str) {
	SysCallArgs args(2);
	args[0]=client;
	args[1]=str;
	_G_syscall( BOTLIB_EA_SAY, args);
}

void _G_trap_EA_SayTeam(int client, const char *str) {
	SysCallArgs args(2);
	args[0]=client;
	args[1]=str;
	_G_syscall( BOTLIB_EA_SAY_TEAM, args);
}

void _G_trap_EA_Command(int client, const char *command) {
	SysCallArgs args(2);
	args[0]=client;
	args[1]=command;
	_G_syscall( BOTLIB_EA_COMMAND, args);
}

void _G_trap_EA_Action(int client, int action) {
	SysCallArgs args(2);
	args[0]=client;
	args[1]=action;
	_G_syscall( BOTLIB_EA_ACTION, args );
}

void _G_trap_EA_Gesture(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_GESTURE, args );
}

void _G_trap_EA_Talk(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_TALK, args );
}

void _G_trap_EA_Attack(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_ATTACK, args );
}

void _G_trap_EA_Use(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_USE, args );
}

void _G_trap_EA_Respawn(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_RESPAWN, args );
}

void _G_trap_EA_Crouch(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_CROUCH, args );
}

void _G_trap_EA_MoveUp(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_MOVE_UP, args );
}

void _G_trap_EA_MoveDown(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_MOVE_DOWN, args );
}

void _G_trap_EA_MoveForward(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_MOVE_FORWARD, args );
}

void _G_trap_EA_MoveBack(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_MOVE_BACK, args );
}

void _G_trap_EA_MoveLeft(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_MOVE_LEFT, args );
}

void _G_trap_EA_MoveRight(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_MOVE_RIGHT, args );
}

void _G_trap_EA_SelectWeapon(int client, int weapon) {
	SysCallArgs args(2);
	args[0]=client;
	args[1]=weapon;
	_G_syscall( BOTLIB_EA_SELECT_WEAPON, args );
}

void _G_trap_EA_Jump(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_JUMP, args );
}

void _G_trap_EA_DelayedJump(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_DELAYED_JUMP, args );
}

void _G_trap_EA_Move(int client, avec3_t dir, bfixed speed) {
	SysCallArgs args(3);
	args[0]=client;
	args[1]=dir;
	args[2]=speed;
	_G_syscall( BOTLIB_EA_MOVE, args );
}

void _G_trap_EA_View(int client, avec3_t viewangles) {
	SysCallArgs args(2);
	args[0]=client;
	args[1]=viewangles;
	_G_syscall( BOTLIB_EA_VIEW, args );
}

void _G_trap_EA_EndRegular(int client, gfixed thinktime) {
	SysCallArgs args(2);
	args[0]=client;
	args[1]=thinktime;
	_G_syscall( BOTLIB_EA_END_REGULAR, args );
}

void _G_trap_EA_GetInput(int client, gfixed thinktime, void /* struct bot_input_s */ *input) {
	SysCallArgs args(3);
	args[0]=client;
	args[1]=thinktime;
	args[2]=input;
	_G_syscall( BOTLIB_EA_GET_INPUT, args);
}

void _G_trap_EA_ResetInput(int client) {
	SysCallArgs args(1);
	args[0]=client;
	_G_syscall( BOTLIB_EA_RESET_INPUT, args );
}

int _G_trap_BotLoadCharacter(const char *charfile, gfixed skill) {
	SysCallArgs args(2);
	args[0]=charfile;
	args[1]=skill;
	return (int)_G_syscall( BOTLIB_AI_LOAD_CHARACTER, args);
}

void _G_trap_BotFreeCharacter(int character) {
	SysCallArgs args(1);
	args[0]=character;
	_G_syscall( BOTLIB_AI_FREE_CHARACTER, args );
}

gfixed _G_trap_Characteristic_Float(int character, int index) {
	SysCallArgs args(2);
	args[0]=character;
	args[1]=index;
	return (gfixed)_G_syscall( BOTLIB_AI_CHARACTERISTIC_FLOAT, args );
}

gfixed _G_trap_Characteristic_BFloat(int character, int index, gfixed min, gfixed max) {
	SysCallArgs args(4);
	args[0]=character;
	args[1]=index;
	args[2]=min;
	args[3]=max;
	return (gfixed)_G_syscall( BOTLIB_AI_CHARACTERISTIC_BFLOAT, args );
}

int _G_trap_Characteristic_Integer(int character, int index) {
	SysCallArgs args(2);
	args[0]=character;
	args[1]=index;
	return (int)_G_syscall( BOTLIB_AI_CHARACTERISTIC_INTEGER, args );
}

int _G_trap_Characteristic_BInteger(int character, int index, int min, int max) {
	SysCallArgs args(4);
	args[0]=character;
	args[1]=index;
	args[2]=min;
	args[3]=max;
	return (int)_G_syscall( BOTLIB_AI_CHARACTERISTIC_BINTEGER, args );
}

void _G_trap_Characteristic_String(int character, int index, char *buf, int size) {
	SysCallArgs args(4);
	args[0]=character;
	args[1]=index;
	args[2]=buf;
	args[3]=size;
	_G_syscall( BOTLIB_AI_CHARACTERISTIC_STRING, args);
}

int _G_trap_BotAllocChatState(void) {
	return (int)_G_syscall( BOTLIB_AI_ALLOC_CHAT_STATE,SysCallArgs(0) );
}

void _G_trap_BotFreeChatState(int handle) {
	SysCallArgs args(1);
	args[0]=handle;
	_G_syscall( BOTLIB_AI_FREE_CHAT_STATE, args);
}

void _G_trap_BotQueueConsoleMessage(int chatstate, int type, const char *message) {
	SysCallArgs args(3);
	args[0]=chatstate;
	args[1]=type;
	args[2]=message;
	_G_syscall( BOTLIB_AI_QUEUE_CONSOLE_MESSAGE, args);
}

void _G_trap_BotRemoveConsoleMessage(int chatstate, int handle) {
	SysCallArgs args(2);
	args[0]=chatstate;
	args[1]=handle;
	_G_syscall( BOTLIB_AI_REMOVE_CONSOLE_MESSAGE, args);
}

int _G_trap_BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm) {
	SysCallArgs args(2);
	args[0]=chatstate;
	args[1]=cm;
	return (int)_G_syscall( BOTLIB_AI_NEXT_CONSOLE_MESSAGE, args);
}

int _G_trap_BotNumConsoleMessages(int chatstate) {
	SysCallArgs args(1);
	args[0]=chatstate;
	return (int)_G_syscall( BOTLIB_AI_NUM_CONSOLE_MESSAGE, args);
}

void _G_trap_BotInitialChat(int chatstate, const char *type, int mcontext, const char *var0, const char *var1, const char *var2,
							const char *var3, const char *var4, const char *var5, const char *var6, const char *var7 ) {
	SysCallArgs args(11);
	args[0]=chatstate;
	args[1]=type;
	args[2]=mcontext;
	args[3]=var0;
	args[4]=var1;
	args[5]=var2;
	args[6]=var3;
	args[7]=var4;
	args[8]=var5;
	args[9]=var6;
	args[10]=var7;
	_G_syscall( BOTLIB_AI_INITIAL_CHAT, args );
}

int	_G_trap_BotNumInitialChats(int chatstate, const char *type) {
	SysCallArgs args(2);
	args[0]=chatstate;
	args[1]=type;
	return (int)_G_syscall( BOTLIB_AI_NUM_INITIAL_CHATS, args);
}

int _G_trap_BotReplyChat(int chatstate, const char *message, int mcontext, int vcontext, const char *var0, const char *var1, 
						 const char *var2, const char *var3, const char *var4, const char *var5, const char *var6, const char *var7 ) {
	SysCallArgs args(12);
	args[0]=chatstate;
	args[1]=message;
	args[2]=mcontext;
	args[3]=vcontext;
	args[4]=var0;
	args[5]=var1;
	args[6]=var2;
	args[7]=var3;
	args[8]=var4;
	args[9]=var5;
	args[10]=var6;
	args[11]=var7;
	return (int)_G_syscall( BOTLIB_AI_REPLY_CHAT, args );
}

int _G_trap_BotChatLength(int chatstate) {
	SysCallArgs args(1);
	args[0]=chatstate;
	return (int)_G_syscall( BOTLIB_AI_CHAT_LENGTH, args);
}

void _G_trap_BotEnterChat(int chatstate, int client, int sendto) {
	SysCallArgs args(3);
	args[0]=chatstate;
	args[1]=client;
	args[2]=sendto;
	_G_syscall( BOTLIB_AI_ENTER_CHAT, args );
}

void _G_trap_BotGetChatMessage(int chatstate, char *buf, int size) {
	SysCallArgs args(3);
	args[0]=chatstate;
	args[1]=buf;
	args[2]=size;
	_G_syscall( BOTLIB_AI_GET_CHAT_MESSAGE, args);
}

int _G_trap_StringContains(const char *str1, const char *str2, int casesensitive) {
	SysCallArgs args(3);
	args[0]=str1;
	args[1]=str2;
	args[2]=casesensitive;
	return (int)_G_syscall( BOTLIB_AI_STRING_CONTAINS, args);
}

int _G_trap_BotFindMatch(const char *str, void /* struct bot_match_s */ *match, unsigned long int context) {
	SysCallArgs args(3);
	args[0]=str;
	args[1]=match;
	args[2]=(int)context;
	return (int)_G_syscall( BOTLIB_AI_FIND_MATCH, args);
}

void _G_trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size) {
	SysCallArgs args(4);
	args[0]=match;
	args[1]=variable;
	args[2]=buf;
	args[3]=size;
	_G_syscall( BOTLIB_AI_MATCH_VARIABLE, args);
}

void _G_trap_UnifyWhiteSpaces(char *string) {
	SysCallArgs args(1);
	args[0]=string;
	_G_syscall( BOTLIB_AI_UNIFY_WHITE_SPACES, args);
}

void _G_trap_BotReplaceSynonyms(char *string, unsigned long int context) {
	SysCallArgs args(2);
	args[0]=string;
	args[1]=(int)context;
	_G_syscall( BOTLIB_AI_REPLACE_SYNONYMS, args);
}

int _G_trap_BotLoadChatFile(int chatstate, const char *chatfile, const char *chatname) {
	SysCallArgs args(3);
	args[0]=chatstate;
	args[1]=chatfile;
	args[2]=chatname;
	return (int)_G_syscall( BOTLIB_AI_LOAD_CHAT_FILE, args);
}

void _G_trap_BotSetChatGender(int chatstate, int gender) {
	SysCallArgs args(2);
	args[0]=chatstate;
	args[1]=gender;
	_G_syscall( BOTLIB_AI_SET_CHAT_GENDER, args);
}

void _G_trap_BotSetChatName(int chatstate, const char *name, int client) {
	SysCallArgs args(3);
	args[0]=chatstate;
	args[1]=name;
	args[2]=client;
	_G_syscall( BOTLIB_AI_SET_CHAT_NAME, args);
}

void _G_trap_BotResetGoalState(int goalstate) {
	SysCallArgs args(1);
	args[0]=goalstate;
	_G_syscall( BOTLIB_AI_RESET_GOAL_STATE, args);
}

void _G_trap_BotResetAvoidGoals(int goalstate) {
	SysCallArgs args(1);
	args[0]=goalstate;
	_G_syscall( BOTLIB_AI_RESET_AVOID_GOALS, args);
}

void _G_trap_BotRemoveFromAvoidGoals(int goalstate, int number) {
	SysCallArgs args(2);
	args[0]=goalstate;
	args[1]=number;
	_G_syscall( BOTLIB_AI_REMOVE_FROM_AVOID_GOALS, args);
}

void _G_trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	SysCallArgs args(2);
	args[0]=goalstate;
	args[1]=goal;
	_G_syscall( BOTLIB_AI_PUSH_GOAL, args );
}

void _G_trap_BotPopGoal(int goalstate) {
	SysCallArgs args(1);
	args[0]=goalstate;
	_G_syscall( BOTLIB_AI_POP_GOAL, args );
}

void _G_trap_BotEmptyGoalStack(int goalstate) {
	SysCallArgs args(1);
	args[0]=goalstate;
	_G_syscall( BOTLIB_AI_EMPTY_GOAL_STACK, args);
}

void _G_trap_BotDumpAvoidGoals(int goalstate) {
	SysCallArgs args(1);
	args[0]=goalstate;
	_G_syscall( BOTLIB_AI_DUMP_AVOID_GOALS, args);
}

void _G_trap_BotDumpGoalStack(int goalstate) {
	SysCallArgs args(1);
	args[0]=goalstate;
	_G_syscall( BOTLIB_AI_DUMP_GOAL_STACK, args);
}

void _G_trap_BotGoalName(int number, char *name, int size) {
	SysCallArgs args(3);
	args[0]=number;
	args[1]=name;
	args[2]=size;
	_G_syscall( BOTLIB_AI_GOAL_NAME, args );
}

int _G_trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	SysCallArgs args(2);
	args[0]=goalstate;
	args[1]=goal;
	return (int)_G_syscall( BOTLIB_AI_GET_TOP_GOAL, args );
}

int _G_trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	SysCallArgs args(2);
	args[0]=goalstate;
	args[1]=goal;
	return (int)_G_syscall( BOTLIB_AI_GET_SECOND_GOAL, args );
}

int _G_trap_BotChooseLTGItem(int goalstate, bvec3_t origin, int *inventory, int travelflags) {
	SysCallArgs args(4);
	args[0]=goalstate;
	args[1]=origin;
	args[2]=inventory;
	args[3]=travelflags;
	return (int)_G_syscall( BOTLIB_AI_CHOOSE_LTG_ITEM, args);
}

int _G_trap_BotChooseNBGItem(int goalstate, bvec3_t origin, int *inventory, int travelflags, 
							 void /* struct bot_goal_s */ *ltg, gfixed maxtime) {
	SysCallArgs args(6);
	args[0]=goalstate;
	args[1]=origin;
	args[2]=inventory;
	args[3]=travelflags;
	args[4]=ltg;
	args[5]=maxtime;
	return (int)_G_syscall( BOTLIB_AI_CHOOSE_NBG_ITEM, args );
}

int _G_trap_BotTouchingGoal(bvec3_t origin, void /* struct bot_goal_s */ *goal) {
	SysCallArgs args(2);
	args[0]=origin;
	args[1]=goal;
	return (int)_G_syscall( BOTLIB_AI_TOUCHING_GOAL, args);
}

int _G_trap_BotItemGoalInVisButNotVisible(int viewer, bvec3_t eye, avec3_t viewangles, void /* struct bot_goal_s */ *goal) {
	SysCallArgs args(4);
	args[0]=viewer;
	args[1]=eye;
	args[2]=viewangles;
	args[3]=goal;
	return (int)_G_syscall( BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE, args );
}

int _G_trap_BotGetLevelItemGoal(int index, const char *classname, void /* struct bot_goal_s */ *goal) {
	SysCallArgs args(3);
	args[0]=index;
	args[1]=classname;
	args[2]=goal;
	return (int)_G_syscall( BOTLIB_AI_GET_LEVEL_ITEM_GOAL, args);
}

int _G_trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal) {
	SysCallArgs args(2);
	args[0]=num;
	args[1]=goal;
	return (int)_G_syscall( BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL, args);
}

int _G_trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal) {
	SysCallArgs args(2);
	args[0]=name;
	args[1]=goal;
	return (int)_G_syscall( BOTLIB_AI_GET_MAP_LOCATION_GOAL, args);
}

gfixed _G_trap_BotAvoidGoalTime(int goalstate, int number) {
	SysCallArgs args(2);
	args[0]=goalstate;
	args[1]=number;
	return (gfixed)_G_syscall( BOTLIB_AI_AVOID_GOAL_TIME, args);
}

void _G_trap_BotSetAvoidGoalTime(int goalstate, int number, gfixed avoidtime) {
	SysCallArgs args(3);
	args[0]=goalstate;
	args[1]=number;
	args[2]=avoidtime;

	_G_syscall( BOTLIB_AI_SET_AVOID_GOAL_TIME, args);
}

void _G_trap_BotInitLevelItems(void) {
	_G_syscall( BOTLIB_AI_INIT_LEVEL_ITEMS,SysCallArgs(0) );
}

void _G_trap_BotUpdateEntityItems(void) {
	_G_syscall( BOTLIB_AI_UPDATE_ENTITY_ITEMS,SysCallArgs(0) );
}

int _G_trap_BotLoadItemWeights(int goalstate, const char *filename) {
	SysCallArgs args(2);
	args[0]=goalstate;
	args[1]=filename;
	return (int)_G_syscall( BOTLIB_AI_LOAD_ITEM_WEIGHTS,args);
}

void _G_trap_BotFreeItemWeights(int goalstate) {
	SysCallArgs args(1);
	args[0]=goalstate;
	_G_syscall( BOTLIB_AI_FREE_ITEM_WEIGHTS, args);
}

void _G_trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child) {
	SysCallArgs args(3);
	args[0]=parent1;
	args[1]=parent2;
	args[2]=child;
	_G_syscall( BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC, args);
}

void _G_trap_BotSaveGoalFuzzyLogic(int goalstate, const char *filename) {
	SysCallArgs args(2);
	args[0]=goalstate;
	args[1]=filename;
	_G_syscall( BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC, args);
}

void _G_trap_BotMutateGoalFuzzyLogic(int goalstate, gfixed range) {
	SysCallArgs args(2);
	args[0]=goalstate;
	args[1]=range;
	_G_syscall( BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC, args);
}

int _G_trap_BotAllocGoalState(int state) {
	SysCallArgs args(1);
	args[0]=state;
	return (int)_G_syscall( BOTLIB_AI_ALLOC_GOAL_STATE, args);
}

void _G_trap_BotFreeGoalState(int handle) {
	SysCallArgs args(1);
	args[0]=handle;
	_G_syscall( BOTLIB_AI_FREE_GOAL_STATE, args);
}

void _G_trap_BotResetMoveState(int movestate) {
	SysCallArgs args(1);
	args[0]=movestate;
	_G_syscall( BOTLIB_AI_RESET_MOVE_STATE, args);
}

void _G_trap_BotAddAvoidSpot(int movestate, bvec3_t origin, bfixed radius, int type) {
	SysCallArgs args(4);
	args[0]=movestate;
	args[1]=origin;
	args[2]=radius;
	args[3]=type;
	_G_syscall( BOTLIB_AI_ADD_AVOID_SPOT, args);
}

void _G_trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal,
						   int travelflags) {
	SysCallArgs args(4);
	args[0]=result;
	args[1]=movestate;
	args[2]=goal;
	args[3]=travelflags;
	_G_syscall( BOTLIB_AI_MOVE_TO_GOAL, args);
}

int _G_trap_BotMoveInDirection(int movestate, avec3_t dir, bfixed speed, int type) {
	SysCallArgs args(4);
	args[0]=movestate;
	args[1]=dir;
	args[2]=speed;
	args[3]=type;
	return (int)_G_syscall( BOTLIB_AI_MOVE_IN_DIRECTION, args);
}

void _G_trap_BotResetAvoidReach(int movestate) {
	SysCallArgs args(1);
	args[0]=movestate;
	_G_syscall( BOTLIB_AI_RESET_AVOID_REACH, args);
}

void _G_trap_BotResetLastAvoidReach(int movestate) {
	SysCallArgs args(1);
	args[0]=movestate;
	_G_syscall( BOTLIB_AI_RESET_LAST_AVOID_REACH, args);
}

int _G_trap_BotReachabilityArea(bvec3_t origin, int testground) {
	SysCallArgs args(2);
	args[0]=origin;
	args[1]=testground;
	return (int)_G_syscall( BOTLIB_AI_REACHABILITY_AREA, args );
}

int _G_trap_BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, bfixed lookahead, 
								  bvec3_t target) {
	SysCallArgs args(5);
	args[0]=movestate;
	args[1]=goal;
	args[2]=travelflags;
	args[3]=lookahead;
	args[4]=target;
	return (int)_G_syscall( BOTLIB_AI_MOVEMENT_VIEW_TARGET, args);
}

int _G_trap_BotPredictVisiblePosition(bvec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags,
									  bvec3_t target) {
	SysCallArgs args(5);
	args[0]=origin;
	args[1]=areanum;
	args[2]=goal;
	args[3]=travelflags;
	args[4]=target;
	return (int)_G_syscall( BOTLIB_AI_PREDICT_VISIBLE_POSITION, args);
}

int _G_trap_BotAllocMoveState(void) {

	return (int)_G_syscall( BOTLIB_AI_ALLOC_MOVE_STATE,SysCallArgs(0) );
}

void _G_trap_BotFreeMoveState(int handle) {
	SysCallArgs args(1);
	args[0]=handle;
	_G_syscall( BOTLIB_AI_FREE_MOVE_STATE, args );
}

void _G_trap_BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove) {
	SysCallArgs args(2);
	args[0]=handle;
	args[1]=initmove;
	_G_syscall( BOTLIB_AI_INIT_MOVE_STATE, args);
}

int _G_trap_BotChooseBestFightWeapon(int weaponstate, int *inventory) {
	SysCallArgs args(2);
	args[0]=weaponstate;
	args[1]=inventory;
	return (int)_G_syscall( BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON, args);
}

void _G_trap_BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo) {
	SysCallArgs args(3);
	args[0]=weaponstate;
	args[1]=weapon;
	args[2]=weaponinfo;
	_G_syscall( BOTLIB_AI_GET_WEAPON_INFO, args);
}

int _G_trap_BotLoadWeaponWeights(int weaponstate, const char *filename) {
	SysCallArgs args(2);
	args[0]=weaponstate;
	args[1]=filename;
	return (int)_G_syscall( BOTLIB_AI_LOAD_WEAPON_WEIGHTS, args);
}

int _G_trap_BotAllocWeaponState(void) {
	return (int)_G_syscall( BOTLIB_AI_ALLOC_WEAPON_STATE,SysCallArgs(0) );
}

void _G_trap_BotFreeWeaponState(int weaponstate) {
	SysCallArgs args(1);
	args[0]=weaponstate;
	_G_syscall( BOTLIB_AI_FREE_WEAPON_STATE, args );
}

void _G_trap_BotResetWeaponState(int weaponstate) {
	SysCallArgs args(1);
	args[0]=weaponstate;
	_G_syscall( BOTLIB_AI_RESET_WEAPON_STATE, args );
}

int _G_trap_GeneticParentsAndChildSelection(int numranks, gfixed *ranks, int *parent1, int *parent2, int *child) {
	SysCallArgs args(5);
	args[0]=numranks;
	args[1]=ranks;
	args[2]=parent1;
	args[3]=parent2;
	args[4]=child;
	return (int)_G_syscall( BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION, args );
}

int _G_trap_PC_LoadSource( const char *filename ) {
	SysCallArgs args(1);
	args[0]=filename;
	return (int)_G_syscall( BOTLIB_PC_LOAD_SOURCE, args);
}

int _G_trap_PC_FreeSource( int handle ) {
	SysCallArgs args(1);
	args[0]=handle;
	return (int)_G_syscall( BOTLIB_PC_FREE_SOURCE, args);
}

int _G_trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	SysCallArgs args(2);
	args[0]=handle;
	args[1]=pc_token;
	return (int)_G_syscall( BOTLIB_PC_READ_TOKEN, args);
}

int _G_trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	SysCallArgs args(3);
	args[0]=handle;
	args[1]=filename;
	args[2]=line;
	return (int)_G_syscall( BOTLIB_PC_SOURCE_FILE_AND_LINE, args);
}
