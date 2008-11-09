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
#include"ui_pch.h"
#include"qcommon.h"
#include"server.h"
#include"cm_public.h"
#include"client.h"
#include"botlib.h"


extern botlib_export_t	*botlib_export;

void GetClientState( uiClientState_t *state );
void LAN_LoadCachedServers( );
void LAN_SaveServersToCache( );
void LAN_ResetPings(int source);
int LAN_AddServer(int source, const char *name, const char *address);
void LAN_RemoveServer(int source, const char *addr);
int LAN_GetServerCount( int source );
void LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int LAN_GetServerPing( int source, int n );
serverInfo_t *LAN_GetServerPtr( int source, int n );
int LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 );
int LAN_GetPingQueueCount( void );
void LAN_ClearPing( int n );
void LAN_GetPing( int n, char *buf, int buflen, int *pingtime );
void LAN_GetPingInfo( int n, char *buf, int buflen );
void LAN_MarkServerVisible(int source, int n, qboolean visible );
int LAN_ServerIsVisible(int source, int n );
qboolean LAN_UpdateVisiblePings(int source );
int LAN_GetServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
void CLUI_GetGlconfig( glconfig_t *config );
void CL_GetClipboardData( char *buf, int buflen );
void Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
void Key_GetBindingBuf( int keynum, char *buf, int buflen );
int Key_GetCatcher( void );
void Key_SetCatcher( int catcher );
void CLUI_GetCDKey( char *buf, int buflen );
void CLUI_SetCDKey( char *buf );
int GetConfigString(int index, char *buf, int size);



void _UI_trap_Print( const char *string ) {
	Com_Printf( "%s", string );
}

void _UI_trap_Error( const char *string ) {
	Com_Error( ERR_DROP, "%s", string);
}

int _UI_trap_Milliseconds( void ) {
	return Sys_Milliseconds();
}

void _UI_trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	Cvar_Register( cvar,var_name,value,flags ); 
}

void _UI_trap_Cvar_Update( vmCvar_t *cvar ) {
	Cvar_Update( cvar );
}

void _UI_trap_Cvar_Set( const char *var_name, const char *value ) {
	Cvar_Set( var_name,value );
}

lfixed _UI_trap_Cvar_VariableValue( const char *var_name ) {
	return Cvar_VariableValue(var_name);
}

void _UI_trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	Cvar_VariableStringBuffer( var_name, buffer, bufsize  );
}

void _UI_trap_Cvar_SetValue( const char *var_name, lfixed value ) {
	Cvar_SetValue( var_name, value);
}

void _UI_trap_Cvar_Reset( const char *name ) {
	Cvar_Reset( name );
}

void _UI_trap_Cvar_Create( const char *var_name, const char *var_value, int flags ) {
	Cvar_Get( var_name, var_value, flags );
}

void _UI_trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize ) {
	Cvar_InfoStringBuffer( bit, buffer, bufsize );
}

int _UI_trap_Argc( void ) {
	return Cmd_Argc();
}

void _UI_trap_Argv( int n, char *buffer, int bufferLength ) {
	Cmd_ArgvBuffer( n, buffer, bufferLength );
}

void _UI_trap_Cmd_ExecuteText( int exec_when, const char *text ) {
	Cbuf_ExecuteText( exec_when, text );
}

int _UI_trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return FS_FOpenFileByMode( qpath, f, mode );
}

void _UI_trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	FS_Read2( buffer, len, f );
}

void _UI_trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	FS_Write( buffer, len, f  );
}

void _UI_trap_FS_FCloseFile( fileHandle_t f ) {
	FS_FCloseFile( f );
}

int _UI_trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return FS_GetFileList(path, extension, listbuf, bufsize  );
}

int _UI_trap_FS_Seek( fileHandle_t f, long offset, int origin ) {
	return FS_Seek( f, offset, origin );
}

qhandle_t _UI_trap_R_RegisterModel( const char *name ) {
	return re.RegisterModel( name );
}

qhandle_t _UI_trap_R_RegisterSkin( const char *name ) {
	return re.RegisterSkin( name );
}

void _UI_trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font) {
	re.RegisterFont(fontName, pointSize, font );
}

qhandle_t _UI_trap_R_RegisterShaderNoMip( const char *name ) {
	return re.RegisterShaderNoMip( name  );
}

void _UI_trap_R_ClearScene( void ) {
	re.ClearScene();
}

void _UI_trap_R_AddRefEntityToScene( const refEntity_t *pre ) {
	re.AddRefEntityToScene( pre );
}

void _UI_trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts ) {
	re.AddPolyToScene( hShader, numVerts, verts , 1 );
}

void _UI_trap_R_AddLightToScene( const bvec3_t org, gfixed intensity, gfixed r, gfixed g, gfixed b ) {
	re.AddLightToScene( org,intensity,r,g,b);
}

void _UI_trap_R_RenderScene( const refdef_t *fd ) {
	re.RenderScene( fd );
}

void _UI_trap_R_SetColor( const gfixed *rgba ) {
	re.SetColor( rgba );
}

void _UI_trap_R_DrawStretchPic( gfixed x, gfixed y, gfixed w, gfixed h, gfixed s1, gfixed t1, gfixed s2, gfixed t2, qhandle_t hShader ) {
	re.DrawStretchPic( x,y,w,h,s1,t1,s2,t2,hShader );
}

void	_UI_trap_R_ModelBounds( clipHandle_t model, bvec3_t mins, bvec3_t maxs ) {
	re.ModelBounds( (qhandle_t)model,mins,maxs);
}

void _UI_trap_UpdateScreen( void ) {
	SCR_UpdateScreen();
}

int _UI_trap_CM_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, gfixed frac, const char *tagName ) {
	return re.LerpTag( tag, mod, startFrame, endFrame, frac, tagName  );
}

void _UI_trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	S_StartLocalSound( sfx,channelNum);
}

sfxHandle_t	_UI_trap_S_RegisterSound( const char *sample, qboolean compressed ) {
	return S_RegisterSound( sample,compressed );
}

void _UI_trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	Key_KeynumToStringBuf(keynum,buf,buflen);
}

void _UI_trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	Key_GetBindingBuf( keynum, buf, buflen );
}

void _UI_trap_Key_SetBinding( int keynum, const char *binding ) {
	Key_SetBinding( keynum, binding);
}

qboolean _UI_trap_Key_IsDown( int keynum ) {
	return Key_IsDown( keynum  );
}

qboolean _UI_trap_Key_GetOverstrikeMode( void ) {
	return Key_GetOverstrikeMode();
}

void _UI_trap_Key_SetOverstrikeMode( qboolean state ) {
	Key_SetOverstrikeMode( state );
}

void _UI_trap_Key_ClearStates( void ) {
	Key_ClearStates();
}

int _UI_trap_Key_GetCatcher( void ) {
	return Key_GetCatcher();
}

void _UI_trap_Key_SetCatcher( int catcher ) {
	Key_SetCatcher( catcher );
}

void _UI_trap_GetClipboardData( char *buf, int bufsize ) {
	CL_GetClipboardData( buf, bufsize );
}

void _UI_trap_GetClientState( uiClientState_t *state ) {
	GetClientState( state  );
}

void _UI_trap_GetGlconfig( glconfig_t *glconfig ) {
	CLUI_GetGlconfig( glconfig );
}

int _UI_trap_GetConfigString( int index, char* buff, int buffsize ) {
	return GetConfigString( index, buff, buffsize );
}

int	_UI_trap_LAN_GetServerCount( int source ) {
	return LAN_GetServerCount(source);
}

void _UI_trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	LAN_GetServerAddressString( source, n, buf, buflen );
}

void _UI_trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	LAN_GetServerInfo( source, n, buf, buflen );
}

int _UI_trap_LAN_GetServerPing( int source, int n ) {
	return LAN_GetServerPing( source, n  );
}

int _UI_trap_LAN_GetPingQueueCount( void ) {
	return LAN_GetPingQueueCount();
}

int _UI_trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	return LAN_GetServerStatus( serverAddress, serverStatus, maxLen  );
}

void _UI_trap_LAN_SaveCachedServers() {
	LAN_SaveServersToCache();
}

void _UI_trap_LAN_LoadCachedServers() {
	LAN_LoadCachedServers();
}

void _UI_trap_LAN_ResetPings(int n) {
	LAN_ResetPings( n );
}

void _UI_trap_LAN_ClearPing( int n ) {
	LAN_ClearPing( n );
}

void _UI_trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	LAN_GetPing( n, buf, buflen, pingtime );
}

void _UI_trap_LAN_GetPingInfo( int n, char *buf, int buflen ) {
	LAN_GetPingInfo( n, buf, buflen  );
}

void _UI_trap_LAN_MarkServerVisible( int source, int n, qboolean visible ) {
	LAN_MarkServerVisible( source, n, visible  );
}

int _UI_trap_LAN_ServerIsVisible( int source, int n) {
	return LAN_ServerIsVisible( source, n );
}

qboolean _UI_trap_LAN_UpdateVisiblePings( int source ) {
	return LAN_UpdateVisiblePings( source );
}

int _UI_trap_LAN_AddServer(int source, const char *name, const char *addr) {
	return LAN_AddServer(source, name, addr );
}

void _UI_trap_LAN_RemoveServer(int source, const char *addr) {
	LAN_RemoveServer(source, addr );
}

int _UI_trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	return LAN_CompareServers( source,sortKey,sortDir,s1,s2 );
}

int _UI_trap_MemoryRemaining( void ) {
	return Hunk_MemoryRemaining();
}

void _UI_trap_GetCDKey( char *buf, int buflen ) {
	CLUI_GetCDKey( buf, buflen );
}

void _UI_trap_SetCDKey( char *buf ) {
	CLUI_SetCDKey( buf );
}

int _UI_trap_PC_AddGlobalDefine(const char *define ) {
	return botlib_export->PC_AddGlobalDefine( define );
}

int _UI_trap_PC_LoadSource( const char *filename ) {
	return botlib_export->PC_LoadSourceHandle( filename);
}

int _UI_trap_PC_FreeSource( int handle ) {
	return botlib_export->PC_FreeSourceHandle( handle );
}

int _UI_trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return botlib_export->PC_ReadTokenHandle( handle,pc_token);
}

int _UI_trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return botlib_export->PC_SourceFileAndLine( handle, filename, line );
}

void _UI_trap_S_StopBackgroundTrack( void ) {
	S_StopBackgroundTrack();
}

void _UI_trap_S_StartBackgroundTrack( const char *intro, const char *loop) {
	S_StartBackgroundTrack( intro,loop);
}

int _UI_trap_RealTime(qtime_t *qtime) {
	return Com_RealTime( qtime );
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int _UI_trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits) {
	return CIN_PlayCinematic(arg0, xpos, ypos, width, height, bits);
}
 
// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status _UI_trap_CIN_StopCinematic(int handle) {
	return CIN_StopCinematic(handle);
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status _UI_trap_CIN_RunCinematic (int handle) {
	return CIN_RunCinematic(handle);
}
 

// draws the current frame
void _UI_trap_CIN_DrawCinematic (int handle) {
	CIN_DrawCinematic(handle);
}
 

// allows you to resize the animation dynamically
void _UI_trap_CIN_SetExtents (int handle, int x, int y, int w, int h) {
	CIN_SetExtents(handle, x, y, w, h);
}


void	_UI_trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	re.RemapShader( oldShader, newShader, timeOffset );
}

qboolean _UI_trap_VerifyCDKey( const char *key, const char *chksum) {
	return CL_CDKeyValidate(key, chksum);
}

void _UI_trap_SetPbClStatus( int status ) {
}
