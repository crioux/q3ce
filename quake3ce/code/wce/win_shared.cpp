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

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#include "win_local.h"
#include <stdio.h>
#include "dll_load.h"

extern bool g_use_dllload;

/*
================
Sys_Milliseconds
================
*/
int			sys_timeBase;
int Sys_Milliseconds (void)
{
	int			sys_curtime;
	static qboolean	initialized = qfalse;

	if (!initialized) {
		sys_timeBase = timeGetTime();
		initialized = qtrue;
	}
	sys_curtime = timeGetTime() - sys_timeBase;

	return sys_curtime;
}


void Sys_SnapVector( gfixed *v )
{
	v[0]=FIXED_SNAP(v[0]);
	v[1]=FIXED_SNAP(v[1]);
	v[2]=FIXED_SNAP(v[2]);
}


/*
**
** Disable all optimizations temporarily so this code works correctly!
**
*/
#pragma optimize( "", off )


int Sys_GetProcessorId( void )
{
#if defined _M_ALPHA
	return CPUID_AXP;
#elif defined ARM
	return CPUID_ARM;
#elif !defined _M_IX86
	return CPUID_GENERIC;
#else

	return CPUID_INTEL_MMX;

#endif
}

/*
**
** Re-enable optimizations back to what they were
**
*/
#pragma optimize( "", on )

//============================================

char *Sys_GetCurrentUser( void )
{
	static char s_userName[1024];
	unsigned long size = sizeof( s_userName );


	strcpy( s_userName, "player" );

	return s_userName;
}

char	*Sys_DefaultHomePath(void) {
	return NULL;
}

char *Sys_DefaultInstallPath(void)
{
	return Sys_Cwd();
}


/*
========================================================================

LOAD/UNLOAD DLL

========================================================================
*/

/*
=================
Sys_UnloadDll

=================
*/
void Sys_UnloadDll( void *dllHandle ) {
	if ( !dllHandle ) {
		return;
	}

	if(g_use_dllload)
	{
		if ( !FreeDLL( (HMODULE)dllHandle ) ) 
		{
			Com_Error (ERR_FATAL, "Sys_UnloadDll FreeDLL failed");
		}
	} 
	else 
	{
		if ( !FreeLibrary( (HMODULE)dllHandle ) ) 
		{
			Com_Error (ERR_FATAL, "Sys_UnloadDll FreeLibrary failed");
		}
	}
}

/*
=================
Sys_LoadDll

Used to load a development dll instead of a virtual machine

TTimo: added some verbosity in debug
=================
*/
extern char		*FS_BuildOSPath( const char *base, const char *game, const char *qpath );

// fqpath param added 7/20/02 by T.Ray - Sys_LoadDll is only called in vm.c at this time
// fqpath will be empty if dll not loaded, otherwise will hold fully qualified path of dll module loaded
// fqpath buffersize must be at least MAX_QPATH+1 bytes long
void * QDECL Sys_LoadDll( const char *name, char *fqpath , SysCallArg (QDECL **entryPoint)(int, const SysCallArgs &args ),
				  SysCallArg (QDECL *systemcalls)(int, const SysCallArgs &args) ) {
	static int	lastWarning = 0;
	HINSTANCE	libHandle;
	void (QDECL *dllEntry)( SysCallArg (QDECL *syscallptr)(int id, const SysCallArgs &args) );

	char	*basepath;
	char	*cdpath;
	char	*gamedir;
	char	*fn;
/*
#ifdef NDEBUG
	int		timestamp;
	int   ret;
#endif
*/
	char	filename[MAX_QPATH];
	wchar_t wname[256];

	*fqpath = 0 ;		// added 7/20/02 by T.Ray

#ifdef ARM
	Com_sprintf( filename, sizeof( filename ), "%sarm.dll", name );
#else
	Com_sprintf( filename, sizeof( filename ), "%sx86.dll", name );
#endif

	/*
#ifdef NDEBUG
	timestamp = Sys_Milliseconds();
	if( ((timestamp - lastWarning) > (5 * 60000)) && !Cvar_VariableIntegerValue( "dedicated" )
		&& !Cvar_VariableIntegerValue( "com_blindlyLoadDLLs" ) ) {
		if (FS_FileExists(filename)) {
			lastWarning = timestamp;
			ret = MessageBoxEx( NULL, "You are about to load a .DLL executable that\n"
				  "has not been verified for use with Quake III Arena.\n"
				  "This type of file can compromise the security of\n"
				  "your computer.\n\n"
				  "Select 'OK' if you choose to load it anyway.",
				  "Security Warning", MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND,
				  MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ) );
			if( ret != IDOK ) {
				return NULL;
			}
		}
	}
#endif
*/
#ifndef NDEBUG
	_snwprintf(wname,256,L"%S",filename);
	if(g_use_dllload)
	{
		libHandle = LoadDLL( wname );
	}
	else
	{
		libHandle = LoadLibrary( wname );
	}
  if (libHandle)
    Com_Printf("LoadLibrary '%s' ok\n", filename);
  else
    Com_Printf("LoadLibrary '%s' failed\n", filename);
	if ( !libHandle ) {
#endif
	basepath = Cvar_VariableString( "fs_basepath" );
	cdpath = Cvar_VariableString( "fs_cdpath" );
	gamedir = Cvar_VariableString( "fs_game" );

	fn = FS_BuildOSPath( basepath, gamedir, filename );
	_snwprintf(wname,256,L"%S",fn);
	if(g_use_dllload)
	{
		libHandle = LoadDLL( wname );
	}
	else
	{
		libHandle = LoadLibrary( wname );
	}
#ifndef NDEBUG
  if (libHandle)
    Com_Printf("LoadLibrary '%s' ok\n", fn);
  else
    Com_Printf("LoadLibrary '%s' failed\n", fn);
#endif

	if ( !libHandle ) {
		if( cdpath[0] ) {
			fn = FS_BuildOSPath( cdpath, gamedir, filename );
			_snwprintf(wname,256,L"%S",fn);
		if(g_use_dllload)
		{
			libHandle = LoadDLL( wname );
		}
		else
		{
			libHandle = LoadLibrary( wname );
		}
			
#ifndef NDEBUG
      if (libHandle)
        Com_Printf("LoadLibrary '%s' ok\n", fn);
      else
        Com_Printf("LoadLibrary '%s' failed\n", fn);
#endif
		}

		if ( !libHandle ) {
			return NULL;
		}
	}
#ifndef NDEBUG
	}
#endif

	if(g_use_dllload)
	{
		dllEntry = (void (QDECL *)( SysCallArg (QDECL *syscallptr)(int, SysCallArgs &) ))GetDLLProcAddress( libHandle, L"dllEntry" ); 
		*entryPoint = (SysCallArg (QDECL *)(int, SysCallArgs &))GetDLLProcAddress( libHandle, L"vmMain" );
	}
	else
	{
		dllEntry = (void (QDECL *)( SysCallArg (QDECL *syscallptr)(int, SysCallArgs &) ))GetProcAddress( libHandle, L"dllEntry" ); 
		*entryPoint = (SysCallArg (QDECL *)(int, SysCallArgs &))GetProcAddress( libHandle, L"vmMain" );
	}
	if ( !*entryPoint || !dllEntry ) {
		if(g_use_dllload)
		{
			FreeDLL( libHandle );
		}
		else
		{
			FreeLibrary( libHandle );
		}
		return NULL;
	}
	dllEntry( systemcalls );

	if ( libHandle ) Q_strncpyz ( fqpath , filename , MAX_QPATH ) ;		// added 7/20/02 by T.Ray
	return libHandle;
}

