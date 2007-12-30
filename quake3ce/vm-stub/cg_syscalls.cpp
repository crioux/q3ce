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

#include"cgame_pch.h"
#include"..\qcommon\qcommon.h"
#include"..\client\client.h"
#include"..\qcommon\cm_public.h"
#include"..\client\snd_public.h"
#include"..\game\botlib.h"

extern botlib_export_t	*botlib_export;


void CL_GetGameState( gameState_t *gs );
void CL_GetGlconfig( glconfig_t *glconfig );
qboolean CL_GetUserCmd( int cmdNumber, usercmd_t *ucmd );
int CL_GetCurrentCmdNumber( void );
qboolean	CL_GetParseEntityState( int parseEntityNumber, entityState_t *state );
void	CL_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );
qboolean	CL_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );
void CL_SetUserCmdValue( int userCmdValue, gfixed sensitivityScale );
void CL_AddCgameCommand( const char *cmdName );
void CL_CgameError( const char *string );
void CL_ConfigstringModified( void );
qboolean CL_GetServerCommand( int serverCommandNumber );
void CL_ShutdownCGame( void );



void	_CG_trap_Print( const char *fmt ) {
	Com_Printf( "%s", fmt);
}

void	_CG_trap_Error( const char *fmt ) {
	Com_Error( ERR_DROP, "%s", fmt );
}

int		_CG_trap_Milliseconds( void ) {
	return Sys_Milliseconds();
}

void	_CG_trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags ) {
	Cvar_Register( vmCvar, varName, defaultValue, flags ); 
}

void	_CG_trap_Cvar_Update( vmCvar_t *vmCvar ) {
	Cvar_Update( vmCvar );
}

void	_CG_trap_Cvar_Set( const char *var_name, const char *value ) {
	Cvar_Set( var_name, value );
}

void _CG_trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

int		_CG_trap_Argc( void ) {
	return Cmd_Argc();
}

void	_CG_trap_Argv( int n, char *buffer, int bufferLength ) {
	Cmd_ArgvBuffer( n, buffer, bufferLength );
}

void	_CG_trap_Args( char *buffer, int bufferLength ) {
	Cmd_ArgsBuffer( buffer, bufferLength );
}

int		_CG_trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return FS_FOpenFileByMode( qpath, f,  mode );
}

void	_CG_trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	FS_Read2( buffer,len,f );
}

void	_CG_trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	FS_Write( buffer,len,f);
}

void	_CG_trap_FS_FCloseFile( fileHandle_t f ) {
	FS_FCloseFile( f );
}

int _CG_trap_FS_Seek( fileHandle_t f, long offset, int origin ) {
	return FS_Seek( f,offset,origin );
}

void	_CG_trap_SendConsoleCommand( const char *text ) {
	Cbuf_AddText( text );
}

void	_CG_trap_AddCommand( const char *cmdName ) {
	Cmd_AddCommand( cmdName, NULL );
}

void	_CG_trap_RemoveCommand( const char *cmdName ) {
	Cmd_RemoveCommand( cmdName);
}

void	_CG_trap_SendClientCommand( const char *s ) {
	CL_AddReliableCommand( s );
}

void	_CG_trap_UpdateScreen( void ) {
	SCR_UpdateScreen();
}

void	_CG_trap_CM_LoadMap( const char *mapname ) {
	int		checksum;
	CM_LoadMap( mapname, qtrue, &checksum );
}

int		_CG_trap_CM_NumInlineModels( void ) {
	return CM_NumInlineModels();
}

clipHandle_t _CG_trap_CM_InlineModel( int index ) {
	return CM_InlineModel( index );
}

clipHandle_t _CG_trap_CM_TempBoxModel( const bvec3_t mins, const bvec3_t maxs ) {
	return CM_TempBoxModel( mins, maxs, qfalse );
}

clipHandle_t _CG_trap_CM_TempCapsuleModel( const bvec3_t mins, const bvec3_t maxs ) {
	return CM_TempBoxModel( mins, maxs, qtrue );
}

int		_CG_trap_CM_PointContents( const bvec3_t p, clipHandle_t model ) {
	return CM_PointContents( p, model );
}

int		_CG_trap_CM_TransformedPointContents( const bvec3_t p, clipHandle_t model, const bvec3_t origin, const avec3_t angles ) {
	return CM_TransformedPointContents( p, model, origin, angles );
}

void	_CG_trap_CM_BoxTrace( trace_t *results, const bvec3_t start, const bvec3_t end,
						  const bvec3_t mins, const bvec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	CM_BoxTrace( results, start, end, mins, maxs, model, brushmask, qfalse );
}

void	_CG_trap_CM_CapsuleTrace( trace_t *results, const bvec3_t start, const bvec3_t end,
						  const bvec3_t mins, const bvec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	CM_BoxTrace( results, start, end, mins, maxs, model, brushmask, qtrue );
}

void	_CG_trap_CM_TransformedBoxTrace( trace_t *results, const bvec3_t start, const bvec3_t end,
						  const bvec3_t mins, const bvec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const bvec3_t origin, const avec3_t angles ) {
	CM_TransformedBoxTrace(results, start, end, mins, maxs, model, brushmask, origin, angles, qfalse );
}

void	_CG_trap_CM_TransformedCapsuleTrace( trace_t *results, const bvec3_t start, const bvec3_t end,
						  const bvec3_t mins, const bvec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const bvec3_t origin, const avec3_t angles ) {
	CM_TransformedBoxTrace( results, start, end, mins, maxs, model, brushmask, origin, angles, qtrue );
}

int		_CG_trap_CM_MarkFragments( int numPoints, const bvec3_t *points, 
				const bvec3_t projection,
				int maxPoints, bvec3_t pointBuffer,
				int maxFragments, markFragment_t *fragmentBuffer ) 
{
	return re.MarkFragments( numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer );
}

void	_CG_trap_S_StartSound( bvec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx ) {
	S_StartSound( origin, entityNum, entchannel, sfx );
}

void	_CG_trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	S_StartLocalSound( sfx, channelNum);
}

void	_CG_trap_S_ClearLoopingSounds( qboolean killall ) {
	S_ClearLoopingSounds(killall);
}

void	_CG_trap_S_AddLoopingSound( int entityNum, const bvec3_t origin, const bvec3_t velocity, sfxHandle_t sfx ) {
	S_AddLoopingSound( entityNum,origin, velocity, sfx );
}

void	_CG_trap_S_AddRealLoopingSound( int entityNum, const bvec3_t origin, const bvec3_t velocity, sfxHandle_t sfx ) {
	S_AddRealLoopingSound(  entityNum, origin, velocity, sfx );
}

void	_CG_trap_S_StopLoopingSound( int entityNum ) {
	S_StopLoopingSound( entityNum );
}

void	_CG_trap_S_UpdateEntityPosition( int entityNum, const bvec3_t origin ) {
	S_UpdateEntityPosition( entityNum, origin );
}

void	_CG_trap_S_Respatialize( int entityNum, const bvec3_t origin, avec3_t axis[3], int inwater ) {
	S_Respatialize( entityNum, origin, axis, inwater );
}

sfxHandle_t	_CG_trap_S_RegisterSound( const char *sample, qboolean compressed ) {
	return S_RegisterSound( sample, compressed );
}

void	_CG_trap_S_StartBackgroundTrack( const char *intro, const char *loop ) {
	S_StartBackgroundTrack( intro, loop );
}

void	_CG_trap_R_LoadWorldMap( const char *mapname ) {
	re.LoadWorld( mapname );
}

qhandle_t _CG_trap_R_RegisterModel( const char *name ) {
	return re.RegisterModel( name );
}

qhandle_t _CG_trap_R_RegisterSkin( const char *name ) {
	return re.RegisterSkin( name );
}

qhandle_t _CG_trap_R_RegisterShader( const char *name ) {
	return re.RegisterShader( name );
}

qhandle_t _CG_trap_R_RegisterShaderNoMip( const char *name ) {
	return re.RegisterShaderNoMip( name );
}

void _CG_trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font) {
	re.RegisterFont( fontName, pointSize, font );
}

void	_CG_trap_R_ClearScene( void ) {
	re.ClearScene();
}

void	_CG_trap_R_AddRefEntityToScene( const refEntity_t *refe ) {
	re.AddRefEntityToScene( refe );
}

void	_CG_trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts ) {
	re.AddPolyToScene( hShader, numVerts, verts, 1 );
}

void	_CG_trap_R_AddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int num ) {
	re.AddPolyToScene( hShader, numVerts, verts, num );
}

int		_CG_trap_R_LightForPoint( bvec3_t point, vec3_t ambientLight, vec3_t directedLight, avec3_t lightDir ) {
	return re.LightForPoint( point, ambientLight, directedLight, lightDir );
}

void	_CG_trap_R_AddLightToScene( const bvec3_t org, gfixed intensity, gfixed r, gfixed g, gfixed b ) {
	re.AddLightToScene( org, intensity, r, g, b );
}

void	_CG_trap_R_AddAdditiveLightToScene( const bvec3_t org, gfixed intensity, gfixed r, gfixed g, gfixed b ) {
	re.AddAdditiveLightToScene( org,intensity,r,g,b);
}

void	_CG_trap_R_RenderScene( const refdef_t *fd ) {
	re.RenderScene( fd );
}

void	_CG_trap_R_SetColor( const gfixed *rgba ) {
	re.SetColor( rgba );
}

void	_CG_trap_R_DrawStretchPic( gfixed x, gfixed y, gfixed w, gfixed h, 
							   gfixed s1, gfixed t1, gfixed s2, gfixed t2, qhandle_t hShader ) {
	re.DrawStretchPic( x,y,w,h,s1,t1,s2,t2,hShader );
}

void	_CG_trap_R_ModelBounds( clipHandle_t model, bvec3_t mins, bvec3_t maxs ) {
	re.ModelBounds( model, mins, maxs );
}

int		_CG_trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
					   gfixed frac, const char *tagName ) {
	return re.LerpTag( tag,mod,startFrame,endFrame,frac,tagName);
}

void	_CG_trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	re.RemapShader( oldShader,newShader,timeOffset);
}

void		_CG_trap_GetGlconfig( glconfig_t *glconfig ) {
	CL_GetGlconfig( glconfig );
}

void		_CG_trap_GetGameState( gameState_t *gamestate ) {
	CL_GetGameState( gamestate );
}

void		_CG_trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	CL_GetCurrentSnapshotNumber( snapshotNumber,serverTime );
}

qboolean	_CG_trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot ) {
	return CL_GetSnapshot( snapshotNumber,snapshot );
}

qboolean	_CG_trap_GetServerCommand( int serverCommandNumber ) {
	return CL_GetServerCommand( serverCommandNumber);
}

int			_CG_trap_GetCurrentCmdNumber( void ) {
	return CL_GetCurrentCmdNumber();
}

qboolean	_CG_trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd ) {
	return CL_GetUserCmd( cmdNumber, ucmd );
}

void		_CG_trap_SetUserCmdValue( int stateValue, gfixed sensitivityScale ) {
	CL_SetUserCmdValue( stateValue,sensitivityScale );
}

int _CG_trap_MemoryRemaining( void ) {
	return Hunk_MemoryRemaining();
}

qboolean _CG_trap_Key_IsDown( int keynum ) {
	return Key_IsDown( keynum );
}

int _CG_trap_Key_GetCatcher( void ) {
	return Key_GetCatcher();
}

void _CG_trap_Key_SetCatcher( int catcher ) {
	Key_SetCatcher( catcher );
}

int _CG_trap_Key_GetKey( const char *binding ) {
	return Key_GetKey( binding );
}

int _CG_trap_PC_AddGlobalDefine( char *define ) {
	return botlib_export->PC_AddGlobalDefine( define );
}

int _CG_trap_PC_LoadSource( const char *filename ) {
	return botlib_export->PC_LoadSourceHandle( filename );
}

int _CG_trap_PC_FreeSource( int handle ) {
	return botlib_export->PC_FreeSourceHandle( handle );
}

int _CG_trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return botlib_export->PC_ReadTokenHandle( handle, pc_token );
}

int _CG_trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return botlib_export->PC_SourceFileAndLine( handle, filename, line);
}

void	_CG_trap_S_StopBackgroundTrack( void ) {
	S_StopBackgroundTrack();
}

int _CG_trap_RealTime(qtime_t *qtime) {
	return Com_RealTime( qtime );
}

void _CG_trap_SnapVector( gfixed *v ) {
	Sys_SnapVector( v );
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int _CG_trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits) {
	return CIN_PlayCinematic(arg0,xpos,ypos,width,height,bits);
}
 
// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status _CG_trap_CIN_StopCinematic(int handle) {
 	return CIN_StopCinematic(handle);
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status _CG_trap_CIN_RunCinematic (int handle) {
	return CIN_RunCinematic(handle);
}
 

// draws the current frame
void _CG_trap_CIN_DrawCinematic (int handle) {
	CIN_DrawCinematic(handle);
}
 

// allows you to resize the animation dynamically
void _CG_trap_CIN_SetExtents (int handle, int x, int y, int w, int h) {
	CIN_SetExtents(handle,x,y,w,h);
}

qboolean _CG_trap_GetEntityToken( char *buffer, int bufferSize ) {
	return re.GetEntityToken( buffer, bufferSize);
}

qboolean _CG_trap_R_inPVS( const bvec3_t p1, const bvec3_t p2 ) {
	return re.inPVS( p1, p2);
}
