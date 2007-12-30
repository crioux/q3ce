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
// cg__CG_syscalls.c -- this file is only included when building a dll
// cg__CG_syscalls.asm is included instead when building a qvm
#include"cgame_pch.h"

#ifdef Q3_VM
#error "Do not use in VM build"
#endif


static SysCallArg (QDECL *_CG_syscall)(int id, SysCallArgs &args) = (SysCallArg (QDECL *)(int, SysCallArgs &))-1;

void dllEntry( SysCallArg (QDECL *syscallptr)(int id, SysCallArgs &args) )
{
	_CG_syscall = syscallptr;
}



void	_CG_trap_Print( const char *fmt ) {
	SysCallArgs args(1);
	args[0]=fmt;
	_CG_syscall( CG_PRINT, args );
}

void	_CG_trap_Error( const char *fmt ) {
	SysCallArgs args(1);
	args[0]=fmt;
	_CG_syscall( CG_ERROR, args );
}

int		_CG_trap_Milliseconds( void ) {
	return (int)_CG_syscall( CG_MILLISECONDS,SysCallArgs(0) ); 
}

void	_CG_trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags ) {
	SysCallArgs args(4);
	args[0]=vmCvar;
	args[1]=varName;
	args[2]=defaultValue;
	args[3]=flags;
	_CG_syscall( CG_CVAR_REGISTER, args );
}

void	_CG_trap_Cvar_Update( vmCvar_t *vmCvar ) {
	SysCallArgs args(1);
	args[0]=vmCvar;
	_CG_syscall( CG_CVAR_UPDATE, args );
}

void	_CG_trap_Cvar_Set( const char *var_name, const char *value ) {
	SysCallArgs args(2);
	args[0]=var_name;
	args[1]=value;
	_CG_syscall( CG_CVAR_SET, args );
}

void _CG_trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	SysCallArgs args(3);
	args[0]=var_name;
	args[1]=buffer;
	args[2]=bufsize;
	_CG_syscall( CG_CVAR_VARIABLESTRINGBUFFER, args);
}

int		_CG_trap_Argc( void ) {
	return (int)_CG_syscall( CG_ARGC,SysCallArgs(0) );
}

void	_CG_trap_Argv( int n, char *buffer, int bufferLength ) {
	SysCallArgs args(3);
	args[0]=n;
	args[1]=buffer;
	args[2]=bufferLength;
	_CG_syscall( CG_ARGV, args );
}

void	_CG_trap_Args( char *buffer, int bufferLength ) {
	SysCallArgs args(2);
	args[0]=buffer;
	args[1]=bufferLength;
	_CG_syscall( CG_ARGS, args );
}

int		_CG_trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	SysCallArgs args(3);
	args[0]=qpath;
	args[1]=f;
	args[2]=(int)mode;
	return (int)_CG_syscall( CG_FS_FOPENFILE, args);
}

void	_CG_trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	SysCallArgs args(3);
	args[0]=buffer;
	args[1]=len;
	args[2]=(int)f;
	_CG_syscall( CG_FS_READ, args );
}

void	_CG_trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	SysCallArgs args(3);
	args[0]=buffer;
	args[1]=len;
	args[2]=(int)f;
	_CG_syscall( CG_FS_WRITE, args);
}

void	_CG_trap_FS_FCloseFile( fileHandle_t f ) {
	SysCallArgs args(1);
	args[0]=(int)f;
	_CG_syscall( CG_FS_FCLOSEFILE, args );
}

int _CG_trap_FS_Seek( fileHandle_t f, long offset, int origin ) {
	SysCallArgs args(3);
	args[0]=(int)f;
	args[1]=offset;
	args[2]=origin;
	return (int)_CG_syscall( CG_FS_SEEK, args );
}

void	_CG_trap_SendConsoleCommand( const char *text ) {
	SysCallArgs args(1);
	args[0]=text;
	_CG_syscall( CG_SENDCONSOLECOMMAND, args );
}

void	_CG_trap_AddCommand( const char *cmdName ) {
	SysCallArgs args(1);
	args[0]=cmdName;
	_CG_syscall( CG_ADDCOMMAND, args );
}

void	_CG_trap_RemoveCommand( const char *cmdName ) {
	SysCallArgs args(1);
	args[0]=cmdName;
	_CG_syscall( CG_REMOVECOMMAND, args);
}

void	_CG_trap_SendClientCommand( const char *s ) {
	SysCallArgs args(1);
	args[0]=s;
	_CG_syscall( CG_SENDCLIENTCOMMAND, args );
}

void	_CG_trap_UpdateScreen( void ) {
	_CG_syscall( CG_UPDATESCREEN,SysCallArgs(0) );
}

void	_CG_trap_CM_LoadMap( const char *mapname ) {
	SysCallArgs args(1);
	args[0]=mapname;
	_CG_syscall( CG_CM_LOADMAP, args );
}

int		_CG_trap_CM_NumInlineModels( void ) {
	return (int)_CG_syscall( CG_CM_NUMINLINEMODELS,SysCallArgs(0) );
}

clipHandle_t _CG_trap_CM_InlineModel( int index ) {
	SysCallArgs args(1);
	args[0]=index;
	return (int)(clipHandle_t)_CG_syscall( CG_CM_INLINEMODEL, args );
}

clipHandle_t _CG_trap_CM_TempBoxModel( const bvec3_t mins, const bvec3_t maxs ) {
	SysCallArgs args(2);
	args[0]=mins;
	args[1]=maxs;
	return (int)(clipHandle_t)_CG_syscall( CG_CM_TEMPBOXMODEL, args);
}

clipHandle_t _CG_trap_CM_TempCapsuleModel( const bvec3_t mins, const bvec3_t maxs ) {
	SysCallArgs args(2);
	args[0]=mins;
	args[1]=maxs;
	return (int)(clipHandle_t)_CG_syscall( CG_CM_TEMPCAPSULEMODEL, args );
}

int		_CG_trap_CM_PointContents( const bvec3_t p, clipHandle_t model ) {
	SysCallArgs args(2);
	args[0]=p;
	args[1]=(int)model;
	return (int)_CG_syscall( CG_CM_POINTCONTENTS, args );
}

int		_CG_trap_CM_TransformedPointContents( const bvec3_t p, clipHandle_t model, const bvec3_t origin, const avec3_t angles ) {
	SysCallArgs args(4);
	args[0]=p;
	args[1]=(int)model;
	args[2]=origin;
	args[3]=angles;
	return (int)_CG_syscall( CG_CM_TRANSFORMEDPOINTCONTENTS, args );
}

void	_CG_trap_CM_BoxTrace( trace_t *results, const bvec3_t start, const bvec3_t end,
						  const bvec3_t mins, const bvec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	SysCallArgs args(7);
	args[0]=results;
	args[1]=start;
	args[2]=end;
	args[3]=mins;
	args[4]=maxs;
	args[5]=model;
	args[6]=brushmask;
	_CG_syscall( CG_CM_BOXTRACE, args );
}

void	_CG_trap_CM_CapsuleTrace( trace_t *results, const bvec3_t start, const bvec3_t end,
						  const bvec3_t mins, const bvec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	SysCallArgs args(7);
	args[0]=results;
	args[1]=start;
	args[2]=end;
	args[3]=mins;
	args[4]=maxs;
	args[5]=(int)model;
	args[6]=brushmask;
	_CG_syscall( CG_CM_CAPSULETRACE, args );
}

void	_CG_trap_CM_TransformedBoxTrace( trace_t *results, const bvec3_t start, const bvec3_t end,
						  const bvec3_t mins, const bvec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const bvec3_t origin, const avec3_t angles ) {
	SysCallArgs args(9);
	args[0]=results;
	args[1]=start;
	args[2]=end;
	args[3]=mins;
	args[4]=maxs;
	args[5]=(int)model;
	args[6]=brushmask;
	args[7]=origin;
	args[8]=angles;
	_CG_syscall( CG_CM_TRANSFORMEDBOXTRACE, args );
}

void	_CG_trap_CM_TransformedCapsuleTrace( trace_t *results, const bvec3_t start, const bvec3_t end,
						  const bvec3_t mins, const bvec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const bvec3_t origin, const bvec3_t angles ) {
	SysCallArgs args(9);
	args[0]=results;
	args[1]=start;
	args[2]=end;
	args[3]=mins;
	args[4]=maxs;
	args[5]=(int)model;
	args[6]=brushmask;
	args[7]=origin;
	args[8]=angles;
	_CG_syscall( CG_CM_TRANSFORMEDCAPSULETRACE, args );
}

int		_CG_trap_CM_MarkFragments( int numPoints, const bvec3_t *points, 
				const bvec3_t projection,
				int maxPoints, bvec3_t pointBuffer,
				int maxFragments, markFragment_t *fragmentBuffer ) {
	SysCallArgs args(7);
	args[0]=numPoints;
	args[1]=points;
	args[2]=projection;
	args[3]=maxPoints;
	args[4]=pointBuffer;
	args[5]=maxFragments;
	args[6]=fragmentBuffer;
	return (int)_CG_syscall( CG_CM_MARKFRAGMENTS, args );
}

void	_CG_trap_S_StartSound( bvec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx ) {
	SysCallArgs args(4);
	args[0]=origin;
	args[1]=entityNum;
	args[2]=entchannel;
	args[3]=(int)sfx;
	_CG_syscall( CG_S_STARTSOUND, args );
}

void	_CG_trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	SysCallArgs args(2);
	args[0]=(int)sfx;
	args[1]=channelNum;
	_CG_syscall( CG_S_STARTLOCALSOUND, args);
}

void	_CG_trap_S_ClearLoopingSounds( qboolean killall ) {
	SysCallArgs args(1);
	args[0]=(int)killall;
	_CG_syscall( CG_S_CLEARLOOPINGSOUNDS, args );
}

void	_CG_trap_S_AddLoopingSound( int entityNum, const bvec3_t origin, const bvec3_t velocity, sfxHandle_t sfx ) {
	SysCallArgs args(4);
	args[0]=entityNum;
	args[1]=origin;
	args[2]=velocity;
	args[3]=(int)sfx;

	_CG_syscall( CG_S_ADDLOOPINGSOUND, args );
}

void	_CG_trap_S_AddRealLoopingSound( int entityNum, const bvec3_t origin, const bvec3_t velocity, sfxHandle_t sfx ) {
	SysCallArgs args(4);
	args[0]=entityNum;
	args[1]=origin;
	args[2]=velocity;
	args[3]=(int)sfx;
	_CG_syscall( CG_S_ADDREALLOOPINGSOUND, args );
}

void	_CG_trap_S_StopLoopingSound( int entityNum ) {
	SysCallArgs args(1);
	args[0]=entityNum;
	_CG_syscall( CG_S_STOPLOOPINGSOUND, args );
}

void	_CG_trap_S_UpdateEntityPosition( int entityNum, const bvec3_t origin ) {
	SysCallArgs args(2);
	args[0]=entityNum;
	args[1]=origin;
	_CG_syscall( CG_S_UPDATEENTITYPOSITION, args );
}

void	_CG_trap_S_Respatialize( int entityNum, const bvec3_t origin, avec3_t axis[3], int inwater ) {
	SysCallArgs args(4);
	args[0]=entityNum;
	args[1]=origin;
	args[2]=axis;
	args[3]=inwater;
	_CG_syscall( CG_S_RESPATIALIZE, args );
}

sfxHandle_t	_CG_trap_S_RegisterSound( const char *sample, qboolean compressed ) {
	SysCallArgs args(2);
	args[0]=sample;
	args[1]=(int)compressed;
	return (int)(sfxHandle_t)_CG_syscall( CG_S_REGISTERSOUND, args);
}

void	_CG_trap_S_StartBackgroundTrack( const char *intro, const char *loop ) {
	SysCallArgs args(2);
	args[0]=intro;
	args[1]=loop;
	_CG_syscall( CG_S_STARTBACKGROUNDTRACK, args);
}

void	_CG_trap_R_LoadWorldMap( const char *mapname ) {
	SysCallArgs args(1);
	args[0]=mapname;
	_CG_syscall( CG_R_LOADWORLDMAP, args);
}

qhandle_t _CG_trap_R_RegisterModel( const char *name ) {
	SysCallArgs args(1);
	args[0]=name;
	return (int)(qhandle_t)_CG_syscall( CG_R_REGISTERMODEL, args);
}

qhandle_t _CG_trap_R_RegisterSkin( const char *name ) {
	SysCallArgs args(1);
	args[0]=name;
	return (int)(qhandle_t)_CG_syscall( CG_R_REGISTERSKIN, args);
}

qhandle_t _CG_trap_R_RegisterShader( const char *name ) {
	SysCallArgs args(1);
	args[0]=name;
	return (int)(qhandle_t)_CG_syscall( CG_R_REGISTERSHADER, args );
}

qhandle_t _CG_trap_R_RegisterShaderNoMip( const char *name ) {
	SysCallArgs args(1);
	args[0]=name;
	return (int)(qhandle_t)_CG_syscall( CG_R_REGISTERSHADERNOMIP, args );
}

void _CG_trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font) {
	SysCallArgs args(3);
	args[0]=fontName;
	args[1]=pointSize;
	args[2]=font;
	_CG_syscall(CG_R_REGISTERFONT, args);
}

void	_CG_trap_R_ClearScene( void ) {
	_CG_syscall( CG_R_CLEARSCENE,SysCallArgs(0) );
}

void	_CG_trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	SysCallArgs args(1);
	args[0]=re;
	_CG_syscall( CG_R_ADDREFENTITYTOSCENE, args );
}

void	_CG_trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts ) {
	SysCallArgs args(3);
	args[0]=(int)hShader;
	args[1]=numVerts;
	args[2]=verts;
	_CG_syscall( CG_R_ADDPOLYTOSCENE, args);
}

void	_CG_trap_R_AddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int num ) {
	SysCallArgs args(4);
	args[0]=(qhandle_t)hShader;
	args[1]=numVerts;
	args[2]=verts;
	args[3]=num;
	_CG_syscall( CG_R_ADDPOLYSTOSCENE, args );
}

int		_CG_trap_R_LightForPoint( bvec3_t point, vec3_t ambientLight, bvec3_t directedLight, avec3_t lightDir ) {
	SysCallArgs args(4);
	args[0]=point;
	args[1]=ambientLight;
	args[2]=directedLight;
	args[3]=lightDir;
	return (int)_CG_syscall( CG_R_LIGHTFORPOINT, args );
}

void	_CG_trap_R_AddLightToScene( const bvec3_t org, gfixed intensity, gfixed r, gfixed g, gfixed b ) {
	SysCallArgs args(5);
	args[0]=org;
	args[1]=intensity;
	args[2]=r;
	args[3]=g;
	args[4]=b;
	_CG_syscall( CG_R_ADDLIGHTTOSCENE, args );
}

void	_CG_trap_R_AddAdditiveLightToScene( const bvec3_t org, gfixed intensity, gfixed r, gfixed g, gfixed b ) {
	SysCallArgs args(5);
	args[0]=org;
	args[1]=intensity;
	args[2]=r;
	args[3]=g;
	args[4]=b;
	_CG_syscall( CG_R_ADDADDITIVELIGHTTOSCENE, args);
}

void	_CG_trap_R_RenderScene( const refdef_t *fd ) {
	SysCallArgs args(1);
	args[0]=fd;
	_CG_syscall( CG_R_RENDERSCENE, args );
}

void	_CG_trap_R_SetColor( const gfixed *rgba ) {
	SysCallArgs args(1);
	args[0]=rgba;
	_CG_syscall( CG_R_SETCOLOR, args );
}

void	_CG_trap_R_DrawStretchPic( gfixed x, gfixed y, gfixed w, gfixed h, 
							   gfixed s1, gfixed t1, gfixed s2, gfixed t2, qhandle_t hShader ) {
	SysCallArgs args(9);
	args[0]=x;
	args[1]=y;
	args[2]=w;
	args[3]=h;
	args[4]=s1;
	args[5]=t1;
	args[6]=s2;
	args[7]=t2;
	args[8]=(int)hShader;
	_CG_syscall( CG_R_DRAWSTRETCHPIC,args);
}

void	_CG_trap_R_ModelBounds( clipHandle_t model, bvec3_t mins, bvec3_t maxs ) {
	SysCallArgs args(3);
	args[0]=(int)model;
	args[1]=mins;
	args[2]=maxs;
	_CG_syscall( CG_R_MODELBOUNDS, args );
}

int		_CG_trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
					   gfixed frac, const char *tagName ) {
	SysCallArgs args(6);
	args[0]=tag;
	args[1]=(int)mod;
	args[2]=startFrame;
	args[3]=endFrame;
	args[4]=frac;
	args[5]=tagName;
	return (int)_CG_syscall( CG_R_LERPTAG, args );
}

void	_CG_trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	SysCallArgs args(3);
	args[0]=oldShader;
	args[1]=newShader;
	args[2]=timeOffset;
	_CG_syscall( CG_R_REMAP_SHADER, args );
}

void		_CG_trap_GetGlconfig( glconfig_t *glconfig ) {
	SysCallArgs args(1);
	args[0]=glconfig;
	_CG_syscall( CG_GETGLCONFIG, args );
}

void		_CG_trap_GetGameState( gameState_t *gamestate ) {
	SysCallArgs args(1);
	args[0]=gamestate;
	_CG_syscall( CG_GETGAMESTATE, args );
}

void		_CG_trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	SysCallArgs args(2);
	args[0]=snapshotNumber;
	args[1]=serverTime;
	_CG_syscall( CG_GETCURRENTSNAPSHOTNUMBER, args );
}

qboolean	_CG_trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot ) {
	SysCallArgs args(2);
	args[0]=snapshotNumber;
	args[1]=snapshot;
	return (int)(qboolean) _CG_syscall( CG_GETSNAPSHOT, args );
}

qboolean	_CG_trap_GetServerCommand( int serverCommandNumber ) {
	SysCallArgs args(1);
	args[0]=serverCommandNumber;
	return (int)(qboolean)_CG_syscall( CG_GETSERVERCOMMAND, args);
}

int			_CG_trap_GetCurrentCmdNumber( void ) {
	return (int)_CG_syscall( CG_GETCURRENTCMDNUMBER,SysCallArgs(0));
}

qboolean	_CG_trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd ) {
	SysCallArgs args(2);
	args[0]=cmdNumber;
	args[1]=ucmd;
	return (int)(qboolean) _CG_syscall( CG_GETUSERCMD, args );
}

void		_CG_trap_SetUserCmdValue( int stateValue, gfixed sensitivityScale ) {
	SysCallArgs args(2);
	args[0]=stateValue;
	args[1]=sensitivityScale;
	_CG_syscall( CG_SETUSERCMDVALUE, args );
}

void		testPrintInt( char *string, int i ) {
	SysCallArgs args(2);
	args[0]=string;
	args[1]=i;
	_CG_syscall( CG_TESTPRINTINT, args);
}

void		testPrintFloat( char *string, gfixed f ) {
	SysCallArgs args(2);
	args[0]=string;
	args[1]=f;
	_CG_syscall( CG_TESTPRINTFLOAT, args );
}

int _CG_trap_MemoryRemaining( void ) {
	return (int)_CG_syscall( CG_MEMORY_REMAINING,SysCallArgs(0) );
}

qboolean _CG_trap_Key_IsDown( int keynum ) {
	SysCallArgs args(1);
	args[0]=keynum;
	return (int)(qboolean)_CG_syscall( CG_KEY_ISDOWN, args);
}

int _CG_trap_Key_GetCatcher( void ) {
	return (int)_CG_syscall( CG_KEY_GETCATCHER,SysCallArgs(0) );
}

void _CG_trap_Key_SetCatcher( int catcher ) {
	SysCallArgs args(1);
	args[0]=catcher;
	_CG_syscall( CG_KEY_SETCATCHER, args );
}

int _CG_trap_Key_GetKey( const char *binding ) {
	SysCallArgs args(1);
	args[0]=binding;
	return (int)_CG_syscall( CG_KEY_GETKEY, args );
}

int _CG_trap_PC_AddGlobalDefine( char *define ) {
	SysCallArgs args(1);
	args[0]=define;
	return (int)_CG_syscall( CG_PC_ADD_GLOBAL_DEFINE, args);
}

int _CG_trap_PC_LoadSource( const char *filename ) {
	SysCallArgs args(1);
	args[0]=filename;
	return (int)_CG_syscall( CG_PC_LOAD_SOURCE, args );
}

int _CG_trap_PC_FreeSource( int handle ) {
	SysCallArgs args(1);
	args[0]=handle;
	return (int)_CG_syscall( CG_PC_FREE_SOURCE, args );
}

int _CG_trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	SysCallArgs args(2);
	args[0]=handle;
	args[1]=pc_token;
	return (int)_CG_syscall( CG_PC_READ_TOKEN, args );
}

int _CG_trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	SysCallArgs args(3);
	args[0]=handle;
	args[1]=filename;
	args[2]=line;
	return (int)_CG_syscall( CG_PC_SOURCE_FILE_AND_LINE, args );
}

void	_CG_trap_S_StopBackgroundTrack( void ) {
	_CG_syscall( CG_S_STOPBACKGROUNDTRACK,SysCallArgs(0) );
}

int _CG_trap_RealTime(qtime_t *qtime) {
	SysCallArgs args(1);
	args[0]=qtime;
	return (int)_CG_syscall( CG_REAL_TIME, args );
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int _CG_trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits) {
	SysCallArgs args(6);
	args[0]=arg0;
	args[1]=xpos;
	args[2]=ypos;
	args[3]=width;
	args[4]=height;
	args[5]=bits;
	return (int)_CG_syscall(CG_CIN_PLAYCINEMATIC, args);
}
 
// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status _CG_trap_CIN_StopCinematic(int handle) {
	SysCallArgs args(1);
	args[0]=handle;
	return (e_status)(int)_CG_syscall(CG_CIN_STOPCINEMATIC, args);
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status _CG_trap_CIN_RunCinematic (int handle) {
	SysCallArgs args(1);
	args[0]=handle;
	return (e_status)(int)_CG_syscall(CG_CIN_RUNCINEMATIC, args);
}
 

// draws the current frame
void _CG_trap_CIN_DrawCinematic (int handle) {
	SysCallArgs args(1);
	args[0]=handle;
	_CG_syscall(CG_CIN_DRAWCINEMATIC, args);
}
 

// allows you to resize the animation dynamically
void _CG_trap_CIN_SetExtents (int handle, int x, int y, int w, int h) {
	SysCallArgs args(5);
	args[0]=handle;
	args[1]=x;
	args[2]=y;
	args[3]=w;
	args[4]=h;
	_CG_syscall(CG_CIN_SETEXTENTS, args);
}

qboolean _CG_trap_GetEntityToken( char *buffer, int bufferSize ) {
	SysCallArgs args(2);
	args[0]=buffer;
	args[1]=bufferSize;
	return (int)(qboolean)_CG_syscall( CG_GET_ENTITY_TOKEN, args);
}

qboolean _CG_trap_R_inPVS( const bvec3_t p1, const bvec3_t p2 ) {
	SysCallArgs args(2);
	args[0]=p1;
	args[1]=p2;
	return (int)(qboolean)_CG_syscall( CG_R_INPVS, args);
}
