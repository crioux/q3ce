/*
======================================
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
======================================
*/
//
#include"ui_pch.h"

// this file is only included when building a dll
// _UI_syscalls.asm is included instead when building a qvm
#ifdef Q3_VM
#error "Do not use in VM build"
#endif

static SysCallArg (QDECL *_UI_syscall)(int id, SysCallArgs &args)= (SysCallArg (QDECL *)(int, SysCallArgs &))-1;

void dllEntry( SysCallArg (QDECL *syscallptr)(int id, SysCallArgs &args) )
{
//	DebugBreak();

	_UI_syscall= syscallptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void _UI_trap_Print( const char *string ) {
	SysCallArgs args(1);
	args[0]=string;
	_UI_syscall( UI_PRINT, args );
}

void _UI_trap_Error( const char *string ) {
	SysCallArgs args(1);
	args[0]=string;
	_UI_syscall( UI_ERROR, args );
}

int _UI_trap_Milliseconds( void ) {
	return (int)_UI_syscall( UI_MILLISECONDS,SysCallArgs(0) ); 
}

void _UI_trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	SysCallArgs args(4);
	args[0]=cvar;
	args[1]=var_name;
	args[2]=value;
	args[3]=flags;
	
	_UI_syscall( UI_CVAR_REGISTER, args);
}

void _UI_trap_Cvar_Update( vmCvar_t *cvar ) {
	SysCallArgs args(1);
	args[0]=cvar;
	_UI_syscall( UI_CVAR_UPDATE, args );
}

void _UI_trap_Cvar_Set( const char *var_name, const char *value ) {
	SysCallArgs args(2);
	args[0]=var_name;
	args[1]=value;
	_UI_syscall( UI_CVAR_SET, args );
}

lfixed _UI_trap_Cvar_VariableValue( const char *var_name ) {
	SysCallArgs args(1);
	args[0]=var_name;
	return (lfixed)_UI_syscall( UI_CVAR_VARIABLEVALUE, args );
}

void _UI_trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	SysCallArgs args(3);
	args[0]=var_name;
	args[1]=buffer;
	args[2]=bufsize;
	_UI_syscall( UI_CVAR_VARIABLESTRINGBUFFER, args);
}

void _UI_trap_Cvar_SetValue( const char *var_name, lfixed value ) {
	SysCallArgs args(2);
	args[0]=var_name;
	args[1]=value;
	_UI_syscall( UI_CVAR_SETVALUE_L, args );
}

void _UI_trap_Cvar_SetValue( const char *var_name, gfixed value ) {
	SysCallArgs args(2);
	args[0]=var_name;
	args[1]=value;
	_UI_syscall( UI_CVAR_SETVALUE_G, args );
}

void _UI_trap_Cvar_Reset( const char *name ) {
	SysCallArgs args(1);
	args[0]=name;
	_UI_syscall( UI_CVAR_RESET, args); 
}

void _UI_trap_Cvar_Create( const char *var_name, const char *var_value, int flags ) {
	SysCallArgs args(3);
	args[0]=var_name;
	args[1]=var_value;
	args[2]=flags;
	_UI_syscall( UI_CVAR_CREATE, args);
}

void _UI_trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize ) {
	SysCallArgs args(3);
	args[0]=bit;
	args[1]=buffer;
	args[2]=bufsize;
	_UI_syscall( UI_CVAR_INFOSTRINGBUFFER, args );
}

int _UI_trap_Argc( void ) {
	return (int)_UI_syscall( UI_ARGC,SysCallArgs(0) );
}

void _UI_trap_Argv( int n, char *buffer, int bufferLength ) {
	SysCallArgs args(3);
	args[0]=n;
	args[1]=buffer;
	args[2]=bufferLength;
	_UI_syscall( UI_ARGV, args);
}

void _UI_trap_Cmd_ExecuteText( int exec_when, const char *text ) {
	SysCallArgs args(2);
	args[0]=exec_when;
	args[1]=text;
	_UI_syscall( UI_CMD_EXECUTETEXT, args );
}

int _UI_trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	SysCallArgs args(3);
	args[0]=qpath;
	args[1]=f;
	args[2]=(int)mode;
	return (int)_UI_syscall( UI_FS_FOPENFILE, args);
}

void _UI_trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	SysCallArgs args(3);
	args[0]=buffer;
	args[1]=len;
	args[2]=(int)f;
	_UI_syscall( UI_FS_READ, args );
}

void _UI_trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	SysCallArgs args(3);
	args[0]=buffer;
	args[1]=len;
	args[2]=(int)f;
	_UI_syscall( UI_FS_WRITE, args);
}

void _UI_trap_FS_FCloseFile( fileHandle_t f ) {
	SysCallArgs args(1);
	args[0]=(int)f;
	_UI_syscall( UI_FS_FCLOSEFILE, args );
}

int _UI_trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	SysCallArgs args(4);
	args[0]=path;
	args[1]=extension;
	args[2]=listbuf;
	args[3]=bufsize;
	return (int)_UI_syscall( UI_FS_GETFILELIST, args );
}

int _UI_trap_FS_Seek( fileHandle_t f, long offset, int origin ) {
	SysCallArgs args(3);
	args[0]=(int)f;
	args[1]=offset;
	args[2]=origin;
	return (int)_UI_syscall( UI_FS_SEEK, args);
}

qhandle_t _UI_trap_R_RegisterModel( const char *name ) {
	SysCallArgs args(1);
	args[0]=name;
	return (int)(qhandle_t)_UI_syscall( UI_R_REGISTERMODEL, args);
}

qhandle_t _UI_trap_R_RegisterSkin( const char *name ) {
	SysCallArgs args(1);
	args[0]=name;
	return (int)(qhandle_t)_UI_syscall( UI_R_REGISTERSKIN, args );
}

void _UI_trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font) {
	SysCallArgs args(3);
	args[0]=fontName;
	args[1]=pointSize;
	args[2]=font;
	_UI_syscall( UI_R_REGISTERFONT, args );
}

qhandle_t _UI_trap_R_RegisterShaderNoMip( const char *name ) {
	SysCallArgs args(1);
	args[0]=name;
	return (int)(qhandle_t)_UI_syscall( UI_R_REGISTERSHADERNOMIP, args );
}

void _UI_trap_R_ClearScene( void ) {
	_UI_syscall( UI_R_CLEARSCENE,SysCallArgs(0));
}

void _UI_trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	SysCallArgs args(1);
	args[0]=re;
	_UI_syscall( UI_R_ADDREFENTITYTOSCENE, args);
}

void _UI_trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts ) {
	SysCallArgs args(3);
	args[0]=(int)hShader;
	args[1]=numVerts;
	args[2]=verts;
	_UI_syscall( UI_R_ADDPOLYTOSCENE, args );
}

void _UI_trap_R_AddLightToScene( const bvec3_t org, gfixed intensity, gfixed r, gfixed g, gfixed b ) {
	SysCallArgs args(5);
	args[0]=org;
	args[1]=intensity;
	args[2]=r;
	args[3]=g;
	args[4]=b;

	_UI_syscall( UI_R_ADDLIGHTTOSCENE, args);
}

void _UI_trap_R_RenderScene( const refdef_t *fd ) {
	SysCallArgs args(1);
	args[0]=fd;
	_UI_syscall( UI_R_RENDERSCENE, args);
}

void _UI_trap_R_SetColor( const gfixed *rgba ) {
	SysCallArgs args(1);
	args[0]=rgba;
	_UI_syscall( UI_R_SETCOLOR, args );
}

void _UI_trap_R_DrawStretchPic( gfixed x, gfixed y, gfixed w, gfixed h, gfixed s1, gfixed t1, gfixed s2, gfixed t2, qhandle_t hShader ) {
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
	_UI_syscall( UI_R_DRAWSTRETCHPIC, args);
}

void	_UI_trap_R_ModelBounds( clipHandle_t model, bvec3_t mins, bvec3_t maxs ) {
	SysCallArgs args(3);
	args[0]=(int)model;
	args[1]=mins;
	args[2]=maxs;
	_UI_syscall( UI_R_MODELBOUNDS, args);
}

void _UI_trap_UpdateScreen( void ) {
	_UI_syscall( UI_UPDATESCREEN,SysCallArgs(0));
}

int _UI_trap_CM_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, gfixed frac, const char *tagName ) {
	SysCallArgs args(6);
	args[0]=tag;
	args[1]=(int)mod;
	args[2]=startFrame;
	args[3]=endFrame;
	args[4]=frac;
	args[5]=tagName;
	return (int)_UI_syscall( UI_CM_LERPTAG, args );
}

void _UI_trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	SysCallArgs args(2);
	args[0]=(int)sfx;
	args[1]=channelNum;
	_UI_syscall( UI_S_STARTLOCALSOUND, args);
}

sfxHandle_t	_UI_trap_S_RegisterSound( const char *sample, qboolean compressed ) {
	SysCallArgs args(2);
	args[0]=sample;
	args[1]=compressed;
	return (sfxHandle_t)(int)_UI_syscall( UI_S_REGISTERSOUND, args);
}

void _UI_trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	SysCallArgs args(3);
	args[0]=keynum;
	args[1]=buf;
	args[2]=buflen;
	_UI_syscall( UI_KEY_KEYNUMTOSTRINGBUF, args );
}

void _UI_trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	SysCallArgs args(3);
	args[0]=keynum;
	args[1]=buf;
	args[2]=buflen;
	_UI_syscall( UI_KEY_GETBINDINGBUF, args );
}

void _UI_trap_Key_SetBinding( int keynum, const char *binding ) {
	SysCallArgs args(2);
	args[0]=keynum;
	args[1]=binding;
	_UI_syscall( UI_KEY_SETBINDING, args );
}

qboolean _UI_trap_Key_IsDown( int keynum ) {
	SysCallArgs args(1);
	args[0]=keynum;
	return (int)(qboolean)_UI_syscall( UI_KEY_ISDOWN, args);
}

qboolean _UI_trap_Key_GetOverstrikeMode( void ) {
	return (int)(qboolean)_UI_syscall( UI_KEY_GETOVERSTRIKEMODE,SysCallArgs(0) );
}

void _UI_trap_Key_SetOverstrikeMode( qboolean state ) {
	SysCallArgs args(1);
	args[0]=(int)state;
	_UI_syscall( UI_KEY_SETOVERSTRIKEMODE, args );
}

void _UI_trap_Key_ClearStates( void ) {
	_UI_syscall( UI_KEY_CLEARSTATES,SysCallArgs(0));
}

int _UI_trap_Key_GetCatcher( void ) {
	return (int)_UI_syscall( UI_KEY_GETCATCHER,SysCallArgs(0) );
}

void _UI_trap_Key_SetCatcher( int catcher ) {
	SysCallArgs args(1);
	args[0]=catcher;
	_UI_syscall( UI_KEY_SETCATCHER, args );
}

void _UI_trap_GetClipboardData( char *buf, int bufsize ) {
	SysCallArgs args(2);
	args[0]=buf;
	args[1]=bufsize;
	_UI_syscall( UI_GETCLIPBOARDDATA, args);
}

void _UI_trap_GetClientState( uiClientState_t *state ) {
	SysCallArgs args(1);
	args[0]=state;
	_UI_syscall( UI_GETCLIENTSTATE, args );
}

void _UI_trap_GetGlconfig( glconfig_t *glconfig ) {
	SysCallArgs args(1);
	args[0]=glconfig;
	_UI_syscall( UI_GETGLCONFIG, args );
}

int _UI_trap_GetConfigString( int index, char* buff, int buffsize ) {
	SysCallArgs args(3);
	args[0]=index;
	args[1]=buff;
	args[2]=buffsize;
	return (int)_UI_syscall( UI_GETCONFIGSTRING, args);
}

int	_UI_trap_LAN_GetServerCount( int source ) {
	SysCallArgs args(1);
	args[0]=source;
	return (int)_UI_syscall( UI_LAN_GETSERVERCOUNT, args );
}

void _UI_trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	SysCallArgs args(4);
	args[0]=source;
	args[1]=n;
	args[2]=buf;
	args[3]=buflen;
	_UI_syscall( UI_LAN_GETSERVERADDRESSSTRING, args);
}

void _UI_trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	SysCallArgs args(4);
	args[0]=source;
	args[1]=n;
	args[2]=buf;
	args[3]=buflen;
	_UI_syscall( UI_LAN_GETSERVERINFO, args );
}

int _UI_trap_LAN_GetServerPing( int source, int n ) {
	SysCallArgs args(2);
	args[0]=source;
	args[1]=n;
	return (int)_UI_syscall( UI_LAN_GETSERVERPING, args );
}

int _UI_trap_LAN_GetPingQueueCount( void ) {
	return (int)_UI_syscall( UI_LAN_GETPINGQUEUECOUNT,SysCallArgs(0) );
}

int _UI_trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	SysCallArgs args(3);
	args[0]=serverAddress;
	args[1]=serverStatus;
	args[2]=maxLen;
	return (int)_UI_syscall( UI_LAN_SERVERSTATUS, args);
}

void _UI_trap_LAN_SaveCachedServers() {
	_UI_syscall( UI_LAN_SAVECACHEDSERVERS,SysCallArgs(0) );
}

void _UI_trap_LAN_LoadCachedServers() {
	_UI_syscall( UI_LAN_LOADCACHEDSERVERS,SysCallArgs(0) );
}

void _UI_trap_LAN_ResetPings(int n) {
	SysCallArgs args(1);
	args[0]=n;
	_UI_syscall( UI_LAN_RESETPINGS, args );
}

void _UI_trap_LAN_ClearPing( int n ) {
	SysCallArgs args(1);
	args[0]=n;
	_UI_syscall( UI_LAN_CLEARPING, args );
}

void _UI_trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	SysCallArgs args(4);
	args[0]=n;
	args[1]=buf;
	args[2]=buflen;
	args[3]=pingtime;
	_UI_syscall( UI_LAN_GETPING, args );
}

void _UI_trap_LAN_GetPingInfo( int n, char *buf, int buflen ) {
	SysCallArgs args(3);
	args[0]=n;
	args[1]=buf;
	args[2]=buflen;
	_UI_syscall( UI_LAN_GETPINGINFO, args);
}

void _UI_trap_LAN_MarkServerVisible( int source, int n, qboolean visible ) {
	SysCallArgs args(3);
	args[0]=source;
	args[1]=n;
	args[2]=(int)visible;
	_UI_syscall( UI_LAN_MARKSERVERVISIBLE, args);
}

int _UI_trap_LAN_ServerIsVisible( int source, int n) {
	SysCallArgs args(2);
	args[0]=source;
	args[1]=n;
	return (int)_UI_syscall( UI_LAN_SERVERISVISIBLE, args);
}

qboolean _UI_trap_LAN_UpdateVisiblePings( int source ) {
	SysCallArgs args(1);
	args[0]=source;
	return (int)(qboolean)_UI_syscall( UI_LAN_UPDATEVISIBLEPINGS, args);
}

int _UI_trap_LAN_AddServer(int source, const char *name, const char *addr) {
	SysCallArgs args(3);
	args[0]=source;
	args[1]=name;
	args[2]=addr;
	return (int)_UI_syscall( UI_LAN_ADDSERVER, args);
}

void _UI_trap_LAN_RemoveServer(int source, const char *addr) {
	SysCallArgs args(2);
	args[0]=source;
	args[1]=addr;
	_UI_syscall( UI_LAN_REMOVESERVER, args);
}

int _UI_trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	SysCallArgs args(5);
	args[0]=source;
	args[1]=sortKey;
	args[2]=sortDir;
	args[3]=s1;
	args[4]=s2;
	return (int)_UI_syscall( UI_LAN_COMPARESERVERS, args);
}

int _UI_trap_MemoryRemaining( void ) {
	return (int)_UI_syscall( UI_MEMORY_REMAINING,SysCallArgs(0) );
}

void _UI_trap_GetCDKey( char *buf, int buflen ) {
	SysCallArgs args(2);
	args[0]=buf;
	args[1]=buflen;
	_UI_syscall( UI_GET_CDKEY, args );
}

void _UI_trap_SetCDKey( char *buf ) {
	SysCallArgs args(1);
	args[0]=buf;
	_UI_syscall( UI_SET_CDKEY, args );
}

int _UI_trap_PC_AddGlobalDefine( char *define ) {
	SysCallArgs args(1);
	args[0]=define;
	return (int)_UI_syscall( UI_PC_ADD_GLOBAL_DEFINE, args);
}

int _UI_trap_PC_LoadSource( const char *filename ) {
	SysCallArgs args(1);
	args[0]=filename;
	return (int)_UI_syscall( UI_PC_LOAD_SOURCE, args );
}

int _UI_trap_PC_FreeSource( int handle ) {
	SysCallArgs args(1);
	args[0]=handle;
	return (int)_UI_syscall( UI_PC_FREE_SOURCE, args );
}

int _UI_trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	SysCallArgs args(2);
	args[0]=handle;
	args[1]=pc_token;
	return (int)_UI_syscall( UI_PC_READ_TOKEN, args);
}

int _UI_trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	SysCallArgs args(3);
	args[0]=handle;
	args[1]=filename;
	args[2]=line;
	return (int)_UI_syscall( UI_PC_SOURCE_FILE_AND_LINE, args);
}

void _UI_trap_S_StopBackgroundTrack( void ) {
	_UI_syscall( UI_S_STOPBACKGROUNDTRACK,SysCallArgs(0) );
}

void _UI_trap_S_StartBackgroundTrack( const char *intro, const char *loop) {
	SysCallArgs args(2);
	args[0]=intro;
	args[1]=loop;
	_UI_syscall( UI_S_STARTBACKGROUNDTRACK, args);
}

int _UI_trap_RealTime(qtime_t *qtime) {
	SysCallArgs args(1);
	args[0]=qtime;
	return (int)_UI_syscall( UI_REAL_TIME, args);
}

int _UI_trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits) {
	SysCallArgs args(6);
	args[0]=arg0;
	args[1]=xpos;
	args[2]=ypos;
	args[3]=width;
	args[4]=height;
	args[5]=bits;
	return (int)_UI_syscall(UI_CIN_PLAYCINEMATIC, args);
}
 
e_status _UI_trap_CIN_StopCinematic(int handle) {
	SysCallArgs args(1);
	args[0]=handle;
	return (e_status)(int)_UI_syscall(UI_CIN_STOPCINEMATIC, args);
}

e_status _UI_trap_CIN_RunCinematic (int handle) {
	SysCallArgs args(1);
	args[0]=handle;
	return (e_status)(int)_UI_syscall(UI_CIN_RUNCINEMATIC, args);
}

void _UI_trap_CIN_DrawCinematic (int handle) {
	SysCallArgs args(1);
	args[0]=handle;
	_UI_syscall(UI_CIN_DRAWCINEMATIC, args);
}

void _UI_trap_CIN_SetExtents (int handle, int x, int y, int w, int h) {
	SysCallArgs args(5);
	args[0]=handle;
	args[1]=x;
	args[2]=y;
	args[3]=w;
	args[4]=h;
	_UI_syscall(UI_CIN_SETEXTENTS, args);
}


void	_UI_trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	SysCallArgs args(3);
	args[0]=oldShader;
	args[1]=newShader;
	args[2]=timeOffset;
	_UI_syscall( UI_R_REMAP_SHADER, args);
}

qboolean _UI_trap_VerifyCDKey( const char *key, const char *chksum) {
	SysCallArgs args(2);
	args[0]=key;
	args[1]=chksum;
	return (int)(qboolean)_UI_syscall( UI_VERIFY_CDKEY, args);
}

void _UI_trap_SetPbClStatus( int status ) {
	SysCallArgs args(1);
	args[0]=status;
	_UI_syscall( UI_SET_PBCLSTATUS, args );
}
