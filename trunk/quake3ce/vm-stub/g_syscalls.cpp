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
#include"qcommon.h"
#include"server.h"
#include"cm_public.h"
#include"botlib.h"


extern botlib_export_t	*botlib_export;


void SV_GameError( const char *string );
void SV_GamePrint( const char *string );
int	SV_NumForGentity( sharedEntity_t *ent );
sharedEntity_t *SV_GentityNum( int num );
playerState_t *SV_GameClientNum( int num );
svEntity_t	*SV_SvEntityForGentity( sharedEntity_t *gEnt );
sharedEntity_t *SV_GEntityForSvEntity( svEntity_t *svEnt );
void SV_GameSendServerCommand( int clientNum, const char *text );
void SV_GameDropClient( int clientNum, const char *reason );
void SV_SetBrushModel( sharedEntity_t *ent, const char *name );
qboolean SV_inPVS (const bvec3_t p1, const bvec3_t p2);
qboolean SV_inPVSIgnorePortals( const bvec3_t p1, const bvec3_t p2);
void SV_AdjustAreaPortalState( sharedEntity_t *ent, qboolean open );
qboolean	SV_EntityContact( const bvec3_t mins, const bvec3_t maxs, const sharedEntity_t *gEnt, int capsule );
void SV_GetServerinfo( char *buffer, int bufferSize );
void SV_LocateGameData( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *clients, int sizeofGameClient );
void SV_GetUsercmd( int clientNum, usercmd_t *cmd );




void	_G_trap_Printf( const char *fmt ) {
	Com_Printf( "%s", fmt);
}

void	_G_trap_Error( const char *fmt ) {
	Com_Error( ERR_DROP, "%s", fmt );
}

int		_G_trap_Milliseconds( void ) {
	return Sys_Milliseconds();
}
int		_G_trap_Argc( void ) {
	return Cmd_Argc();
}

void	_G_trap_Argv( int n, char *buffer, int bufferLength ) {
	Cmd_ArgvBuffer( n,buffer,bufferLength);
}

int		_G_trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return FS_FOpenFileByMode(	qpath,f,mode);
}

void	_G_trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	FS_Read2(buffer,len,f);
}

void	_G_trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	FS_Write(buffer,len,f);
}

void	_G_trap_FS_FCloseFile( fileHandle_t f ) {
	FS_FCloseFile(f);
}

int _G_trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return FS_GetFileList(path,extension,listbuf,bufsize);
}

int _G_trap_FS_Seek( fileHandle_t f, long offset, int origin ) {
	return FS_Seek(f,offset,origin);
}

void	_G_trap_SendConsoleCommand( int exec_when, const char *text ) {
	Cbuf_ExecuteText( exec_when,text );
}

void	_G_trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	Cvar_Register( cvar, var_name, value, flags ); 
}

void	_G_trap_Cvar_Update( vmCvar_t *cvar ) {
	Cvar_Update( cvar );
}

void _G_trap_Cvar_Set( const char *var_name, const char *value ) {
	Cvar_Set( var_name,value);
}

int _G_trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return Cvar_VariableIntegerValue( var_name );
}

void _G_trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	Cvar_VariableStringBuffer( var_name,buffer,bufsize);
}


void _G_trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						 playerState_t *clients, int sizeofGClient ) 
{
	SV_LocateGameData( (sharedEntity_t *)gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );	
}

void _G_trap_DropClient( int clientNum, const char *reason ) {
	SV_GameDropClient( clientNum,reason);
}

void _G_trap_SendServerCommand( int clientNum, const char *text ) {
	SV_GameSendServerCommand( clientNum,text );
	
}

void _G_trap_SetConfigstring( int num, const char *string ) {
	SV_SetConfigstring( num,string);
}

void _G_trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	SV_GetConfigstring(num,buffer,bufferSize);
}

void _G_trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	SV_GetUserinfo( num,buffer,bufferSize);
}

void _G_trap_SetUserinfo( int num, const char *buffer ) {
	SV_SetUserinfo( num, buffer );
}

void _G_trap_GetServerinfo( char *buffer, int bufferSize ) {
	SV_GetServerinfo( buffer,bufferSize);
}

void _G_trap_SetBrushModel( gentity_t *ent, const char *name ) {
	SV_SetBrushModel( (sharedEntity_t *)ent, name );
}

void _G_trap_Trace( trace_t *results, const bvec3_t start, const bvec3_t mins, const bvec3_t maxs, const bvec3_t end, int passEntityNum, int contentmask ) {
	SV_Trace( results,start,mins,maxs,end,passEntityNum,contentmask,qfalse);
}

void _G_trap_TraceCapsule( trace_t *results, const bvec3_t start, const bvec3_t mins, const bvec3_t maxs, const bvec3_t end, int passEntityNum, int contentmask ) {
	SV_Trace( results, start, mins, maxs, end, passEntityNum, contentmask, qtrue );
}

int _G_trap_PointContents( const bvec3_t point, int passEntityNum ) {
	return SV_PointContents( point,passEntityNum );
}


qboolean _G_trap_InPVS( const bvec3_t p1, const bvec3_t p2 ) {
	return SV_inPVS( p1,p2 );
}

qboolean _G_trap_InPVSIgnorePortals( const bvec3_t p1, const bvec3_t p2 ) {
	return SV_inPVSIgnorePortals( p1, p2);
}

void _G_trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	SV_AdjustAreaPortalState( (sharedEntity_t *)ent,open);
}

qboolean _G_trap_AreasConnected( int area1, int area2 ) {
	return CM_AreasConnected( area1,area2);
}

void _G_trap_LinkEntity( gentity_t *ent ) {
	SV_LinkEntity( (sharedEntity_t *)ent );
}

void _G_trap_UnlinkEntity( gentity_t *ent ) {
	SV_UnlinkEntity( (sharedEntity_t *)ent );
}

int _G_trap_EntitiesInBox( const bvec3_t mins, const bvec3_t maxs, int *list, int maxcount ) {
	return SV_AreaEntities( mins, maxs, list, maxcount );
}

qboolean _G_trap_EntityContact( const bvec3_t mins, const bvec3_t maxs, const gentity_t *ent ) {
	return SV_EntityContact( mins, maxs, (const sharedEntity_t *)ent, qfalse );
}

qboolean _G_trap_EntityContactCapsule( const bvec3_t mins, const bvec3_t maxs, const gentity_t *ent ) {
	return SV_EntityContact( mins, maxs, (const sharedEntity_t *)ent, qtrue );
}

int _G_trap_BotAllocateClient( void ) {
	return SV_BotAllocateClient();
}

void _G_trap_BotFreeClient( int clientNum ) {
	SV_BotFreeClient( clientNum );
}

void _G_trap_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	SV_GetUsercmd( clientNum,cmd );
}

qboolean _G_trap_GetEntityToken( char *buffer, int bufferSize ) {
	const char	*s;

	s = COM_Parse( (const char **)&sv.entityParsePoint );
	Q_strncpyz( buffer, s, bufferSize);
	if ( !sv.entityParsePoint && !s[0] ) {
		return qfalse;
	}
	return qtrue;
}

int _G_trap_DebugPolygonCreate(int color, int numPoints, bvec3_t *points) {
	return BotImport_DebugPolygonCreate(  color, numPoints, points  );
}

void _G_trap_DebugPolygonDelete(int id) {
	BotImport_DebugPolygonDelete( id);
}

int _G_trap_RealTime( qtime_t *qtime ) {
	return Com_RealTime( qtime );
}

void _G_trap_SnapVector( gfixed *v ) {
	Sys_SnapVector( v );
	return;
}

// BotLib traps start here
int _G_trap_BotLibSetup( void ) {
	return SV_BotLibSetup();
}

int _G_trap_BotLibShutdown( void ) {
	return SV_BotLibShutdown();
}

int _G_trap_BotLibVarSet(char *var_name, char *value) {
	return botlib_export->BotLibVarSet( var_name, value );
}

int _G_trap_BotLibVarGet(char *var_name, char *value, int size) {
	return botlib_export->BotLibVarGet(var_name, value, size );
}

int _G_trap_BotLibDefine(const char *string) {
	return botlib_export->PC_AddGlobalDefine( string);
}

int _G_trap_BotLibStartFrame(gfixed time) {
	return botlib_export->BotLibStartFrame( time );
}

int _G_trap_BotLibLoadMap(const char *mapname) {
	return botlib_export->BotLibLoadMap( mapname );
}

int _G_trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue) {
	return botlib_export->BotLibUpdateEntity( ent, (bot_entitystate_t *) bue);
}

int _G_trap_BotLibTest(int parm0, char *parm1, bvec3_t parm2, avec3_t parm3) {
	return botlib_export->Test( parm0,parm1,parm2,parm3);
}

int _G_trap_BotGetSnapshotEntity( int clientNum, int sequence ) {
	return SV_BotGetSnapshotEntity( clientNum, sequence );
}

int _G_trap_BotGetServerCommand(int clientNum, char *message, int size) {
	return SV_BotGetConsoleMessage( clientNum, message, size );
}

void _G_trap_BotUserCommand(int clientNum, usercmd_t *ucmd) {
	SV_ClientThink( &svs.clients[clientNum], (usercmd_t *)ucmd );
}

void _G_trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info) {
	botlib_export->aas.AAS_EntityInfo( entnum, (struct aas_entityinfo_s *) info);
}

int _G_trap_AAS_Initialized(void) {
	return botlib_export->aas.AAS_Initialized();
}

void _G_trap_AAS_PresenceTypeBoundingBox(int presencetype, bvec3_t mins, bvec3_t maxs) {
	botlib_export->aas.AAS_PresenceTypeBoundingBox( presencetype,mins,maxs );
}

gfixed _G_trap_AAS_Time(void) {
	return botlib_export->aas.AAS_Time();
}

int _G_trap_AAS_PointAreaNum(bvec3_t point) {
	return botlib_export->aas.AAS_PointAreaNum( point );
}

int _G_trap_AAS_PointReachabilityAreaIndex(bvec3_t point) {
	return botlib_export->aas.AAS_PointReachabilityAreaIndex( point );
}

int _G_trap_AAS_TraceAreas(bvec3_t start, bvec3_t end, int *areas, bvec3_t *points, int maxareas) {
	return botlib_export->aas.AAS_TraceAreas( start, end, areas, points, maxareas );
}

int _G_trap_AAS_BBoxAreas(bvec3_t absmins, bvec3_t absmaxs, int *areas, int maxareas) {
	return botlib_export->aas.AAS_BBoxAreas( absmins, absmaxs, areas, maxareas  );
}

int _G_trap_AAS_AreaInfo( int areanum, void /* struct aas_areainfo_s */ *info ) {
	return botlib_export->aas.AAS_AreaInfo( areanum, (aas_areainfo_s *)info  );
}

int _G_trap_AAS_PointContents(bvec3_t point) {
	return botlib_export->aas.AAS_PointContents( point );
}

int _G_trap_AAS_NextBSPEntity(int ent) {
	return botlib_export->aas.AAS_NextBSPEntity( ent );
}

int _G_trap_AAS_ValueForBSPEpairKey(int ent, const char *key, char *value, int size) {
	return botlib_export->aas.AAS_ValueForBSPEpairKey( ent, key, value, size );
}

int _G_trap_AAS_VectorForBSPEpairKey(int ent, const char *key, bvec3_t v) {
	return botlib_export->aas.AAS_VectorForBSPEpairKey( ent, key, v  );
}

int _G_trap_AAS_FloatForBSPEpairKey(int ent, const char *key, gfixed *value) {
	return botlib_export->aas.AAS_FloatForBSPEpairKey( ent, key, value );
}

int _G_trap_AAS_IntForBSPEpairKey(int ent, const char *key, int *value) {
	return botlib_export->aas.AAS_IntForBSPEpairKey( ent, key, value  );
}

int _G_trap_AAS_AreaReachability(int areanum) {
	return botlib_export->aas.AAS_AreaReachability( areanum );
}

int _G_trap_AAS_AreaTravelTimeToGoalArea(int areanum, bvec3_t origin, int goalareanum, int travelflags) {
	return botlib_export->aas.AAS_AreaTravelTimeToGoalArea( areanum, origin, goalareanum, travelflags );
}

int _G_trap_AAS_EnableRoutingArea( int areanum, int enable ) {
	return botlib_export->aas.AAS_EnableRoutingArea( areanum, enable  );
}

int _G_trap_AAS_PredictRoute(void /*struct aas_predictroute_s*/ *route, int areanum, bvec3_t origin,
							int goalareanum, int travelflags, int maxareas, int maxtime,
							int stopevent, int stopcontents, int stoptfl, int stopareanum) {
	return botlib_export->aas.AAS_PredictRoute( (aas_predictroute_s *)route, areanum, origin, goalareanum, travelflags, maxareas, maxtime, stopevent, stopcontents, stoptfl, stopareanum  );
}

int _G_trap_AAS_AlternativeRouteGoals(bvec3_t start, int startareanum, bvec3_t goal, int goalareanum, int travelflags,
										void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals,
										int type) {
	return botlib_export->aas.AAS_AlternativeRouteGoals( start, startareanum, goal, goalareanum, travelflags,(aas_altroutegoal_s *) altroutegoals, maxaltroutegoals, type  );
}

int _G_trap_AAS_Swimming(bvec3_t origin) {
	return botlib_export->aas.AAS_Swimming( origin );
}

int _G_trap_AAS_PredictClientMovement(void /* struct aas_clientmove_s */ *move, int entnum, bvec3_t origin, int presencetype, int onground, bvec3_t velocity, bvec3_t cmdmove, int cmdframes, int maxframes, gfixed frametime, int stopevent, int stopareanum, int visualize) {
	return botlib_export->aas.AAS_PredictClientMovement( (aas_clientmove_s*)move, entnum, origin, presencetype, onground, velocity, cmdmove, cmdframes, maxframes, frametime, stopevent, stopareanum, visualize );
}

void _G_trap_EA_Say(int client, const char *str) {
	botlib_export->ea.EA_Say( client, str  );
}

void _G_trap_EA_SayTeam(int client, const char *str) {
	botlib_export->ea.EA_SayTeam( client,str);
}

void _G_trap_EA_Command(int client, const char *command) {
	botlib_export->ea.EA_Command( client,command );
}


void _G_trap_EA_Action(int client, int action) {
	botlib_export->ea.EA_Action( client,action );
}

void _G_trap_EA_Gesture(int client) {
	botlib_export->ea.EA_Gesture( client);
}

void _G_trap_EA_Talk(int client) {
	botlib_export->ea.EA_Talk( client );
}

void _G_trap_EA_Attack(int client) {
	botlib_export->ea.EA_Attack( client);
}

void _G_trap_EA_Use(int client) {
	botlib_export->ea.EA_Use( client);
}

void _G_trap_EA_Respawn(int client) {
	botlib_export->ea.EA_Respawn( client );
}

void _G_trap_EA_Crouch(int client) {
	botlib_export->ea.EA_Crouch( client );
}

void _G_trap_EA_MoveUp(int client) {
	botlib_export->ea.EA_MoveUp( client );
}

void _G_trap_EA_MoveDown(int client) {
	botlib_export->ea.EA_MoveDown( client );
}

void _G_trap_EA_MoveForward(int client) {
	botlib_export->ea.EA_MoveForward( client );
}

void _G_trap_EA_MoveBack(int client) {
	botlib_export->ea.EA_MoveBack( client );
}

void _G_trap_EA_MoveLeft(int client) {
	botlib_export->ea.EA_MoveLeft( client );
}

void _G_trap_EA_MoveRight(int client) {
	botlib_export->ea.EA_MoveRight( client );
}

void _G_trap_EA_SelectWeapon(int client, int weapon) {
	botlib_export->ea.EA_SelectWeapon( client, weapon );
}

void _G_trap_EA_Jump(int client) {
	botlib_export->ea.EA_Jump(client);
}

void _G_trap_EA_DelayedJump(int client) {
	botlib_export->ea.EA_DelayedJump( client );
}

void _G_trap_EA_Move(int client, avec3_t dir, bfixed speed) {
	botlib_export->ea.EA_Move( client, dir, speed );
}

void _G_trap_EA_View(int client, avec3_t viewangles) {
	botlib_export->ea.EA_View( client, viewangles );
}

void _G_trap_EA_EndRegular(int client, gfixed thinktime) {
	botlib_export->ea.EA_EndRegular( client, thinktime );
}

void _G_trap_EA_GetInput(int client, gfixed thinktime, void /* struct bot_input_s */ *input) {
	botlib_export->ea.EA_GetInput( client, thinktime, (bot_input_t *) input );
}

void _G_trap_EA_ResetInput(int client) {
	botlib_export->ea.EA_ResetInput( client );
}

int _G_trap_BotLoadCharacter(const char *charfile, gfixed skill) {
	return botlib_export->ai.BotLoadCharacter( charfile, skill );
}

void _G_trap_BotFreeCharacter(int character) {
	botlib_export->ai.BotFreeCharacter( character );
}

gfixed _G_trap_Characteristic_Float(int character, int index) {
	return botlib_export->ai.Characteristic_Float( character, index );
}

gfixed _G_trap_Characteristic_BFloat(int character, int index, gfixed min, gfixed max) {
	return botlib_export->ai.Characteristic_BFloat( character, index, min, max );
}

int _G_trap_Characteristic_Integer(int character, int index) {
	return botlib_export->ai.Characteristic_Integer(  character, index );
}

int _G_trap_Characteristic_BInteger(int character, int index, int min, int max) {
	return botlib_export->ai.Characteristic_BInteger( character, index, min, max );
}

void _G_trap_Characteristic_String(int character, int index, char *buf, int size) {
	botlib_export->ai.Characteristic_String( character, index, buf, size );
}

int _G_trap_BotAllocChatState(void) {
	return botlib_export->ai.BotAllocChatState();
}

void _G_trap_BotFreeChatState(int handle) {
	botlib_export->ai.BotFreeChatState( handle );
}

void _G_trap_BotQueueConsoleMessage(int chatstate, int type, const char *message) {
	botlib_export->ai.BotQueueConsoleMessage( chatstate, type, message );
}

void _G_trap_BotRemoveConsoleMessage(int chatstate, int handle) {
	botlib_export->ai.BotRemoveConsoleMessage( chatstate, handle );
}

int _G_trap_BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm) {
	return botlib_export->ai.BotNextConsoleMessage( chatstate, (bot_consolemessage_s *)cm );
}

int _G_trap_BotNumConsoleMessages(int chatstate) {
	return botlib_export->ai.BotNumConsoleMessages( chatstate );
}

void _G_trap_BotInitialChat(int chatstate, const char *type, int mcontext, const char *var0, const char *var1, const char *var2, const char *var3, const char *var4, const char *var5, const char *var6, const char *var7 ) {
	botlib_export->ai.BotInitialChat( chatstate, type, mcontext, var0, var1, var2, var3, var4, var5, var6, var7);
}

int	_G_trap_BotNumInitialChats(int chatstate, char *type) {
	return botlib_export->ai.BotNumInitialChats( chatstate, type);
}

int _G_trap_BotReplyChat(int chatstate, const char *message, int mcontext, int vcontext, const char *var0, const char *var1, const char *var2, const char *var3, const char *var4, const char *var5, const char *var6, const char *var7 ) {
	return botlib_export->ai.BotReplyChat( chatstate, message, mcontext, vcontext, var0, var1, var2, var3, var4, var5, var6, var7);
}

int _G_trap_BotChatLength(int chatstate) {
	return botlib_export->ai.BotChatLength( chatstate );
}

void _G_trap_BotEnterChat(int chatstate, int client, int sendto) {
	botlib_export->ai.BotEnterChat( chatstate, client, sendto );
}

void _G_trap_BotGetChatMessage(int chatstate, char *buf, int size) {
	botlib_export->ai.BotGetChatMessage( chatstate, buf, size );
}

int _G_trap_StringContains(const char *str1, const char *str2, int casesensitive) {
	return botlib_export->ai.StringContains( str1, str2, casesensitive  );
}

int _G_trap_BotFindMatch(const char *str, void /* struct bot_match_s */ *match, unsigned long int context) {
	return botlib_export->ai.BotFindMatch( str, (struct bot_match_s *)match,context );
}

void _G_trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size) {
	botlib_export->ai.BotMatchVariable( (bot_match_s *)match, variable, buf, size );
}

void _G_trap_UnifyWhiteSpaces(char *string) {
	botlib_export->ai.UnifyWhiteSpaces( string );
}

void _G_trap_BotReplaceSynonyms(char *string, unsigned long int context) {
	botlib_export->ai.BotReplaceSynonyms( string, context );
}

int _G_trap_BotLoadChatFile(int chatstate, const char *chatfile, const char *chatname) {
	return botlib_export->ai.BotLoadChatFile( chatstate, chatfile, chatname );
}

void _G_trap_BotSetChatGender(int chatstate, int gender) {
	botlib_export->ai.BotSetChatGender( chatstate, gender );
}

void _G_trap_BotSetChatName(int chatstate, const char *name, int client) {
	botlib_export->ai.BotSetChatName( chatstate, name, client );
}

void _G_trap_BotResetGoalState(int goalstate) {
	botlib_export->ai.BotResetGoalState( goalstate );
}

void _G_trap_BotResetAvoidGoals(int goalstate) {
	botlib_export->ai.BotResetAvoidGoals( goalstate );
}

void _G_trap_BotRemoveFromAvoidGoals(int goalstate, int number) {
	botlib_export->ai.BotRemoveFromAvoidGoals( goalstate, number );
}

void _G_trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	botlib_export->ai.BotPushGoal( goalstate, (bot_goal_s *)goal );
}

void _G_trap_BotPopGoal(int goalstate) {
	botlib_export->ai.BotPopGoal(goalstate);
}

void _G_trap_BotEmptyGoalStack(int goalstate) {
	botlib_export->ai.BotEmptyGoalStack( goalstate );
}

void _G_trap_BotDumpAvoidGoals(int goalstate) {
	botlib_export->ai.BotDumpAvoidGoals( goalstate );
}

void _G_trap_BotDumpGoalStack(int goalstate) {
	botlib_export->ai.BotDumpGoalStack( goalstate);
}

void _G_trap_BotGoalName(int number, char *name, int size) {
	botlib_export->ai.BotGoalName(number, name, size );
}

int _G_trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	return botlib_export->ai.BotGetTopGoal( goalstate, (bot_goal_s *)goal );
}

int _G_trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	return botlib_export->ai.BotGetSecondGoal( goalstate, (bot_goal_s *)goal );
}

int _G_trap_BotChooseLTGItem(int goalstate, bvec3_t origin, int *inventory, int travelflags) {
	return botlib_export->ai.BotChooseLTGItem( goalstate, origin, inventory, travelflags  );
}

int _G_trap_BotChooseNBGItem(int goalstate, bvec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, gfixed maxtime) {
	return botlib_export->ai.BotChooseNBGItem( goalstate, origin, inventory, travelflags, (bot_goal_s *)ltg, maxtime );
}

int _G_trap_BotTouchingGoal(bvec3_t origin, void /* struct bot_goal_s */ *goal) {
	return botlib_export->ai.BotTouchingGoal( origin, (bot_goal_s *)goal );
}

int _G_trap_BotItemGoalInVisButNotVisible(int viewer, bvec3_t eye, avec3_t viewangles, void /* struct bot_goal_s */ *goal) {
	return botlib_export->ai.BotItemGoalInVisButNotVisible( viewer, eye, viewangles, (bot_goal_s *)goal );
}

int _G_trap_BotGetLevelItemGoal(int index, const char *classname, void /* struct bot_goal_s */ *goal) {
	return botlib_export->ai.BotGetLevelItemGoal( index, classname,(bot_goal_s*)goal );
}

int _G_trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal) {
	return botlib_export->ai.BotGetNextCampSpotGoal( num, (bot_goal_s*)goal );
}

int _G_trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal) {
	return botlib_export->ai.BotGetMapLocationGoal( name, (bot_goal_s*)goal );
}

gfixed _G_trap_BotAvoidGoalTime(int goalstate, int number) {
	return botlib_export->ai.BotAvoidGoalTime( goalstate,number );
}

void _G_trap_BotSetAvoidGoalTime(int goalstate, int number, gfixed avoidtime) {
	botlib_export->ai.BotSetAvoidGoalTime(  goalstate, number, avoidtime);
}

void _G_trap_BotInitLevelItems(void) {
	botlib_export->ai.BotInitLevelItems();
}

void _G_trap_BotUpdateEntityItems(void) {
	botlib_export->ai.BotUpdateEntityItems();
}

int _G_trap_BotLoadItemWeights(int goalstate, const char *filename) {
	return botlib_export->ai.BotLoadItemWeights( goalstate, filename  );
}

void _G_trap_BotFreeItemWeights(int goalstate) {
	botlib_export->ai.BotFreeItemWeights( goalstate );
}

void _G_trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child) {
	botlib_export->ai.BotInterbreedGoalFuzzyLogic( parent1, parent2, child);
}

void _G_trap_BotSaveGoalFuzzyLogic(int goalstate, const char *filename) {
	botlib_export->ai.BotSaveGoalFuzzyLogic( goalstate, filename );
}

void _G_trap_BotMutateGoalFuzzyLogic(int goalstate, gfixed range) {
	botlib_export->ai.BotMutateGoalFuzzyLogic ( goalstate, range );
}

int _G_trap_BotAllocGoalState(int state) {
	return botlib_export->ai.BotAllocGoalState( state );
}

void _G_trap_BotFreeGoalState(int handle) {
	botlib_export->ai.BotFreeGoalState( handle);
}

void _G_trap_BotResetMoveState(int movestate) {
	botlib_export->ai.BotResetMoveState(movestate  );
}

void _G_trap_BotAddAvoidSpot(int movestate, bvec3_t origin, bfixed radius, int type) {
	botlib_export->ai.BotAddAvoidSpot( movestate, origin, radius, type);
}

void _G_trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags) {
	botlib_export->ai.BotMoveToGoal( (bot_moveresult_s *)result, movestate, (bot_goal_s*)goal, travelflags );
}

int _G_trap_BotMoveInDirection(int movestate, avec3_t dir, bfixed speed, int type) {
	return botlib_export->ai.BotMoveInDirection( movestate,dir, speed,type );
}

void _G_trap_BotResetAvoidReach(int movestate) {
	botlib_export->ai.BotResetAvoidReach( movestate);
}

void _G_trap_BotResetLastAvoidReach(int movestate) {
	botlib_export->ai.BotResetLastAvoidReach( movestate );
}

int _G_trap_BotReachabilityArea(bvec3_t origin, int testground) {
	return botlib_export->ai.BotReachabilityArea( origin, testground );
}

int _G_trap_BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, bfixed lookahead, bvec3_t target) {
	return botlib_export->ai.BotMovementViewTarget( movestate, (bot_goal_s *)goal, travelflags, lookahead, target  );
}

int _G_trap_BotPredictVisiblePosition(bvec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, bvec3_t target) {
	return botlib_export->ai.BotPredictVisiblePosition( origin, areanum, (bot_goal_s *)goal, travelflags, target);
}

int _G_trap_BotAllocMoveState(void) {
	return botlib_export->ai.BotAllocMoveState();
}

void _G_trap_BotFreeMoveState(int handle) {
	botlib_export->ai.BotFreeMoveState( handle);
}

void _G_trap_BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove) {
	botlib_export->ai.BotInitMoveState( handle, (bot_initmove_s *)initmove );
}

int _G_trap_BotChooseBestFightWeapon(int weaponstate, int *inventory) {
	return botlib_export->ai.BotChooseBestFightWeapon( weaponstate, inventory );
}

void _G_trap_BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo) {
	botlib_export->ai.BotGetWeaponInfo( weaponstate, weapon, (weaponinfo_s *)weaponinfo );
}

int _G_trap_BotLoadWeaponWeights(int weaponstate, const char *filename) {
	return botlib_export->ai.BotLoadWeaponWeights( weaponstate, filename );
}

int _G_trap_BotAllocWeaponState(void) {
	return botlib_export->ai.BotAllocWeaponState();
}

void _G_trap_BotFreeWeaponState(int weaponstate) {
	botlib_export->ai.BotFreeWeaponState( weaponstate );
}

void _G_trap_BotResetWeaponState(int weaponstate) {
	botlib_export->ai.BotResetWeaponState( weaponstate );
}

int _G_trap_GeneticParentsAndChildSelection(int numranks, gfixed *ranks, int *parent1, int *parent2, int *child) {
	return botlib_export->ai.GeneticParentsAndChildSelection(numranks, ranks, parent1, parent2, child);
}

int _G_trap_PC_LoadSource( const char *filename ) {
	return botlib_export->PC_LoadSourceHandle( filename );
}

int _G_trap_PC_FreeSource( int handle ) {
	return botlib_export->PC_FreeSourceHandle( handle );
}

int _G_trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return botlib_export->PC_ReadTokenHandle( handle, pc_token  );
}

int _G_trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return botlib_export->PC_SourceFileAndLine( handle, filename, line );
}
