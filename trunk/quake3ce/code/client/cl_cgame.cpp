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
// cl_cgame.c  -- client system interaction with client game

#include"client_pch.h"
#include"cg_public.h"

extern	botlib_export_t	*botlib_export;


/*
====================
CL_GetGameState
====================
*/
void CL_GetGameState( gameState_t *gs ) {
	*gs = cl.gameState;
}

/*
====================
CL_GetGlconfig
====================
*/
void CL_GetGlconfig( glconfig_t *glconfig ) {
	*glconfig = cls.glconfig;
}


/*
====================
CL_GetUserCmd
====================
*/
qboolean CL_GetUserCmd( int cmdNumber, usercmd_t *ucmd ) {
	// cmds[cmdNumber] is the last properly generated command

	// can't return anything that we haven't created yet
	if ( cmdNumber > cl.cmdNumber ) {
		Com_Error( ERR_DROP, "CL_GetUserCmd: %i >= %i", cmdNumber, cl.cmdNumber );
	}

	// the usercmd has been overwritten in the wrapping
	// buffer because it is too far out of date
	if ( cmdNumber <= cl.cmdNumber - CMD_BACKUP ) {
		return qfalse;
	}

	*ucmd = cl.cmds[ cmdNumber & CMD_MASK ];

	return qtrue;
}

int CL_GetCurrentCmdNumber( void ) {
	return cl.cmdNumber;
}


/*
====================
CL_GetParseEntityState
====================
*/
qboolean	CL_GetParseEntityState( int parseEntityNumber, entityState_t *state ) {
	// can't return anything that hasn't been parsed yet
	if ( parseEntityNumber >= cl.parseEntitiesNum ) {
		Com_Error( ERR_DROP, "CL_GetParseEntityState: %i >= %i",
			parseEntityNumber, cl.parseEntitiesNum );
	}

	// can't return anything that has been overwritten in the circular buffer
	if ( parseEntityNumber <= cl.parseEntitiesNum - MAX_PARSE_ENTITIES ) {
		return qfalse;
	}

	*state = cl.parseEntities[ parseEntityNumber & ( MAX_PARSE_ENTITIES - 1 ) ];
	return qtrue;
}

/*
====================
CL_GetCurrentSnapshotNumber
====================
*/
void	CL_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	*snapshotNumber = cl.snap.messageNum;
	*serverTime = cl.snap.serverTime;
}

/*
====================
CL_GetSnapshot
====================
*/
qboolean	CL_GetSnapshot( int snapshotNumber, snapshot_t *snapshot ) {
	clSnapshot_t	*clSnap;
	int				i, count;

	if ( snapshotNumber > cl.snap.messageNum ) {
		Com_Error( ERR_DROP, "CL_GetSnapshot: snapshotNumber > cl.snapshot.messageNum" );
	}

	// if the frame has fallen out of the circular buffer, we can't return it
	if ( cl.snap.messageNum - snapshotNumber >= PACKET_BACKUP ) {
		return qfalse;
	}

	// if the frame is not valid, we can't return it
	clSnap = &cl.snapshots[snapshotNumber & PACKET_MASK];
	if ( !clSnap->valid ) {
		return qfalse;
	}

	// if the entities in the frame have fallen out of their
	// circular buffer, we can't return it
	if ( cl.parseEntitiesNum - clSnap->parseEntitiesNum >= MAX_PARSE_ENTITIES ) {
		return qfalse;
	}

	// write the snapshot
	snapshot->snapFlags = clSnap->snapFlags;
	snapshot->serverCommandSequence = clSnap->serverCommandNum;
	snapshot->ping = clSnap->ping;
	snapshot->serverTime = clSnap->serverTime;
	Com_Memcpy( snapshot->areamask, clSnap->areamask, sizeof( snapshot->areamask ) );
	snapshot->ps = clSnap->ps;
	count = clSnap->numEntities;
	if ( count > MAX_ENTITIES_IN_SNAPSHOT ) {
		Com_DPrintf( "CL_GetSnapshot: truncated %i entities to %i\n", count, MAX_ENTITIES_IN_SNAPSHOT );
		count = MAX_ENTITIES_IN_SNAPSHOT;
	}
	snapshot->numEntities = count;
	for ( i = 0 ; i < count ; i++ ) {
		snapshot->entities[i] = 
			cl.parseEntities[ ( clSnap->parseEntitiesNum + i ) & (MAX_PARSE_ENTITIES-1) ];
	}

	// FIXME: configstring changes and server commands!!!

	return qtrue;
}

/*
=====================
CL_SetUserCmdValue
=====================
*/
void CL_SetUserCmdValue( int userCmdValue, gfixed sensitivityScale ) {
	cl.cgameUserCmdValue = userCmdValue;
	cl.cgameSensitivity = sensitivityScale;
}

/*
=====================
CL_AddCgameCommand
=====================
*/
void CL_AddCgameCommand( const char *cmdName ) {
	Cmd_AddCommand( cmdName, NULL );
}

/*
=====================
CL_CgameError
=====================
*/
void CL_CgameError( const char *string ) {
	Com_Error( ERR_DROP, "%s", string );
}


/*
=====================
CL_ConfigstringModified
=====================
*/
void CL_ConfigstringModified( void ) {
	const char		*old, *s;
	int			i, index;
	const char		*dup;
	gameState_t	oldGs;
	int			len;

	index = atoi( Cmd_Argv(1) );
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error( ERR_DROP, "configstring > MAX_CONFIGSTRINGS" );
	}
	// get everything after "cs <num>"
	s = Cmd_ArgsFrom(2);

	old = cl.gameState.stringData + cl.gameState.stringOffsets[ index ];
	if ( !strcmp( old, s ) ) {
		return;		// unchanged
	}

	// build the new gameState_t
	oldGs = cl.gameState;

	Com_Memset( &cl.gameState, 0, sizeof( cl.gameState ) );

	// leave the first 0 for uninitialized strings
	cl.gameState.dataCount = 1;
		
	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		if ( i == index ) {
			dup = s;
		} else {
			dup = oldGs.stringData + oldGs.stringOffsets[ i ];
		}
		if ( !dup[0] ) {
			continue;		// leave with the default empty string
		}

		len = strlen( dup );

		if ( len + 1 + cl.gameState.dataCount > MAX_GAMESTATE_CHARS ) {
			Com_Error( ERR_DROP, "MAX_GAMESTATE_CHARS exceeded" );
		}

		// append it to the gameState string buffer
		cl.gameState.stringOffsets[ i ] = cl.gameState.dataCount;
		Com_Memcpy( cl.gameState.stringData + cl.gameState.dataCount, dup, len + 1 );
		cl.gameState.dataCount += len + 1;
	}

	if ( index == CS_SYSTEMINFO ) {
		// parse serverId and other cvars
		CL_SystemInfoChanged();
	}

}


/*
===================
CL_GetServerCommand

Set up argc/argv for the given command
===================
*/
qboolean CL_GetServerCommand( int serverCommandNumber ) {
	const char	*s;
	const char	*cmd;
	static char bigConfigString[BIG_INFO_STRING];
	int argc;

	// if we have irretrievably lost a reliable command, drop the connection
	if ( serverCommandNumber <= clc.serverCommandSequence - MAX_RELIABLE_COMMANDS ) {
		// when a demo record was started after the client got a whole bunch of
		// reliable commands then the client never got those first reliable commands
		if ( clc.demoplaying )
			return qfalse;
		Com_Error( ERR_DROP, "CL_GetServerCommand: a reliable command was cycled out" );
		return qfalse;
	}

	if ( serverCommandNumber > clc.serverCommandSequence ) {
		Com_Error( ERR_DROP, "CL_GetServerCommand: requested a command not received" );
		return qfalse;
	}

	s = clc.serverCommands[ serverCommandNumber & ( MAX_RELIABLE_COMMANDS - 1 ) ];
	clc.lastExecutedServerCommand = serverCommandNumber;

	Com_DPrintf( "serverCommand: %i : %s\n", serverCommandNumber, s );

rescan:
	Cmd_TokenizeString( s );
	cmd = Cmd_Argv(0);
	argc = Cmd_Argc();

	if ( !strcmp( cmd, "disconnect" ) ) {
		// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=552
		// allow server to indicate why they were disconnected
		if ( argc >= 2 )
			Com_Error (ERR_SERVERDISCONNECT, va( "Server Disconnected - %s", Cmd_Argv( 1 ) ) );
		else
			Com_Error (ERR_SERVERDISCONNECT,"Server disconnected\n");
	}

	if ( !strcmp( cmd, "bcs0" ) ) {
		Com_sprintf( bigConfigString, BIG_INFO_STRING, "cs %s \"%s", Cmd_Argv(1), Cmd_Argv(2) );
		return qfalse;
	}

	if ( !strcmp( cmd, "bcs1" ) ) {
		s = Cmd_Argv(2);
		if( strlen(bigConfigString) + strlen(s) >= BIG_INFO_STRING ) {
			Com_Error( ERR_DROP, "bcs exceeded BIG_INFO_STRING" );
		}
		strcat( bigConfigString, s );
		return qfalse;
	}

	if ( !strcmp( cmd, "bcs2" ) ) {
		s = Cmd_Argv(2);
		if( strlen(bigConfigString) + strlen(s) + 1 >= BIG_INFO_STRING ) {
			Com_Error( ERR_DROP, "bcs exceeded BIG_INFO_STRING" );
		}
		strcat( bigConfigString, s );
		strcat( bigConfigString, "\"" );
		s = bigConfigString;
		goto rescan;
	}

	if ( !strcmp( cmd, "cs" ) ) {
		CL_ConfigstringModified();
		// reparse the string, because CL_ConfigstringModified may have done another Cmd_TokenizeString()
		Cmd_TokenizeString( s );
		return qtrue;
	}

	if ( !strcmp( cmd, "map_restart" ) ) {
		// clear notify lines and outgoing commands before passing
		// the restart to the cgame
		Con_ClearNotify();
		Com_Memset( cl.cmds, 0, sizeof( cl.cmds ) );
		return qtrue;
	}

	// the clientLevelShot command is used during development
	// to generate 128*128 screenshots from the intermission
	// point of levels for the menu system to use
	// we pass it along to the cgame to make apropriate adjustments,
	// but we also clear the console and notify lines here
	if ( !strcmp( cmd, "clientLevelShot" ) ) {
		// don't do it if we aren't running the server locally,
		// otherwise malicious remote servers could overwrite
		// the existing thumbnails
		if ( !com_sv_running->integer ) {
			return qfalse;
		}
		// close the console
		Con_Close();
		// take a special screenshot next frame
		Cbuf_AddText( "wait ; wait ; wait ; wait ; screenshot levelshot\n" );
		return qtrue;
	}

	// we may want to put a "connect to other server" command here

	// cgame can now act on the command
	return qtrue;
}


/*
====================
CL_CM_LoadMap

Just adds default parameters that cgame doesn't need to know about
====================
*/
void CL_CM_LoadMap( const char *mapname ) {
	int		checksum;

	CM_LoadMap( mapname, qtrue, &checksum );
}

/*
====================
CL_ShutdonwCGame

====================
*/
void CL_ShutdownCGame( void ) {
	cls.keyCatchers &= ~KEYCATCH_CGAME;
	cls.cgameStarted = qfalse;
	if ( !cgvm ) {
		return;
	}
	VM_Call( cgvm, CG_SHUTDOWN ,SysCallArgs(0));
	VM_Free( cgvm );
	cgvm = NULL;
}


/*
====================
CL_CgameSystemCalls

The cgame module is making a system call
====================
*/

SysCallArg CL_CgameSystemCalls( int callnum, const SysCallArgs &args ) {
	switch( callnum ) {
	case CG_PRINT:
		Com_Printf( "%s", (const char *)args[0]);
		return SysCallArg();
	case CG_ERROR:
		Com_Error( ERR_DROP, "%s", (const char *)args[0] );
		return SysCallArg();
	case CG_MILLISECONDS:
		return SysCallArg::Int(Sys_Milliseconds());
	case CG_CVAR_REGISTER:
		Cvar_Register( args[0], args[1], args[2], args[3] ); 
		return SysCallArg();
	case CG_CVAR_UPDATE:
		Cvar_Update( args[0] );
		return SysCallArg();
	case CG_CVAR_SET:
		Cvar_Set( args[0], args[1] );
		return SysCallArg();
	case CG_CVAR_VARIABLESTRINGBUFFER:
		Cvar_VariableStringBuffer( args[0], args[1], args[2] );
		return SysCallArg();
	case CG_ARGC:
		return SysCallArg::Int(Cmd_Argc());
	case CG_ARGV:
		Cmd_ArgvBuffer( args[0], args[1], args[2] );
		return SysCallArg();
	case CG_ARGS:
		Cmd_ArgsBuffer( args[0], args[1] );
		return SysCallArg();
	case CG_FS_FOPENFILE:
		return SysCallArg::Int(FS_FOpenFileByMode( args[0], args[1], (fsMode_t)(int)args[2] ));
	case CG_FS_READ:
		FS_Read2( args[0], args[1], args[2] );
		return SysCallArg();
	case CG_FS_WRITE:
		FS_Write( args[0], args[1], args[2] );
		return SysCallArg();
	case CG_FS_FCLOSEFILE:
		FS_FCloseFile( args[0] );
		return SysCallArg();
	case CG_FS_SEEK:
		return SysCallArg::Int(FS_Seek( args[0], (int)args[1], args[2] ));
	case CG_SENDCONSOLECOMMAND:
		Cbuf_AddText( args[0] );
		return SysCallArg();
	case CG_ADDCOMMAND:
		CL_AddCgameCommand( args[0] );
		return SysCallArg();
	case CG_REMOVECOMMAND:
		Cmd_RemoveCommand( args[0] );
		return SysCallArg();
	case CG_SENDCLIENTCOMMAND:
		CL_AddReliableCommand( args[0] );
		return SysCallArg();
	case CG_UPDATESCREEN:
		// this is used during lengthy level loading, so pump message loop
//		Com_EventLoop();	// FIXME: if a server restarts here, BAD THINGS HAPPEN!
// We can't call Com_EventLoop here, a restart will crash and this _does_ happen
// if there is a map change while we are downloading at pk3.
// ZOID
		SCR_UpdateScreen();
		return SysCallArg();
	case CG_CM_LOADMAP:
		CL_CM_LoadMap( args[0] );
		return SysCallArg();
	case CG_CM_NUMINLINEMODELS:
		return SysCallArg::Int(CM_NumInlineModels());
	case CG_CM_INLINEMODEL:
		return SysCallArg::Int(CM_InlineModel( args[0] ));
	case CG_CM_TEMPBOXMODEL:
		return SysCallArg::Int(CM_TempBoxModel( args[0], args[1], /*int capsule*/ qfalse ));
	case CG_CM_TEMPCAPSULEMODEL:
		return SysCallArg::Int(CM_TempBoxModel( args[0], args[1], /*int capsule*/ qtrue ));
	case CG_CM_POINTCONTENTS:
		return SysCallArg::Int(CM_PointContents( args[0], args[1] ));
	case CG_CM_TRANSFORMEDPOINTCONTENTS:
		return SysCallArg::Int(CM_TransformedPointContents( args[0], args[1], args[2], args[3] ));
	case CG_CM_BOXTRACE:
		CM_BoxTrace( args[0], args[1], args[2], args[3], args[4], args[5], args[6], /*int capsule*/ qfalse );
		return SysCallArg();
	case CG_CM_CAPSULETRACE:
		CM_BoxTrace( args[0], args[1], args[2], args[3], args[4], args[5], args[6], /*int capsule*/ qtrue );
		return SysCallArg();
	case CG_CM_TRANSFORMEDBOXTRACE:
		CM_TransformedBoxTrace( args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], /*int capsule*/ qfalse );
		return SysCallArg();
	case CG_CM_TRANSFORMEDCAPSULETRACE:
		CM_TransformedBoxTrace( args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], /*int capsule*/ qtrue );
		return SysCallArg();
	case CG_CM_MARKFRAGMENTS:
		return SysCallArg::Int(re.MarkFragments( args[0], args[1], args[2], args[3], args[4], args[5], args[6] ));
	case CG_S_STARTSOUND:
		S_StartSound( args[0], args[1], args[2], args[3] );
		return SysCallArg();
	case CG_S_STARTLOCALSOUND:
		S_StartLocalSound( args[0], args[1] );
		return SysCallArg();
	case CG_S_CLEARLOOPINGSOUNDS:
		S_ClearLoopingSounds(args[0]);
		return SysCallArg();
	case CG_S_ADDLOOPINGSOUND:
		S_AddLoopingSound( args[0], args[1], args[2], args[3] );
		return SysCallArg();
	case CG_S_ADDREALLOOPINGSOUND:
		S_AddRealLoopingSound( args[0], args[1], args[2], args[3] );
		return SysCallArg();
	case CG_S_STOPLOOPINGSOUND:
		S_StopLoopingSound( args[0] );
		return SysCallArg();
	case CG_S_UPDATEENTITYPOSITION:
		S_UpdateEntityPosition( args[0], args[1] );
		return SysCallArg();
	case CG_S_RESPATIALIZE:
		S_Respatialize( args[0], args[1], args[2], args[3] );
		return SysCallArg();
	case CG_S_REGISTERSOUND:
		return SysCallArg::Int(S_RegisterSound( args[0], args[1] ));
	case CG_S_STARTBACKGROUNDTRACK:
		S_StartBackgroundTrack( args[0], args[1] );
		return SysCallArg();
	case CG_R_LOADWORLDMAP:
		re.LoadWorld( args[0] );
		return SysCallArg(); 
	case CG_R_REGISTERMODEL:
		return SysCallArg::Int(re.RegisterModel( args[0] ));
	case CG_R_REGISTERSKIN:
		return SysCallArg::Int(re.RegisterSkin( args[0] ));
	case CG_R_REGISTERSHADER:
		return SysCallArg::Int(re.RegisterShader( args[0] ));
	case CG_R_REGISTERSHADERNOMIP:
		return SysCallArg::Int(re.RegisterShaderNoMip( args[0] ));
	case CG_R_REGISTERFONT:
		re.RegisterFont( args[0], args[1], args[2]);
	case CG_R_CLEARSCENE:
		re.ClearScene();
		return SysCallArg();
	case CG_R_ADDREFENTITYTOSCENE:
		re.AddRefEntityToScene( args[0] );
		return SysCallArg();
	case CG_R_ADDPOLYTOSCENE:
		re.AddPolyToScene( args[0], args[1], args[2], 1 );
		return SysCallArg();
	case CG_R_ADDPOLYSTOSCENE:
		re.AddPolyToScene( args[0], args[1], args[2], args[3] );
		return SysCallArg();
	case CG_R_LIGHTFORPOINT:
		return SysCallArg::Int(re.LightForPoint( args[0], args[1], args[2], args[3] ));
	case CG_R_ADDLIGHTTOSCENE:
		re.AddLightToScene( args[0], args[1], args[2], args[3], args[4] );
		return SysCallArg();
	case CG_R_ADDADDITIVELIGHTTOSCENE:
		re.AddAdditiveLightToScene( args[0], args[1], args[2], args[3], args[4] );
		return SysCallArg();
	case CG_R_RENDERSCENE:
		re.RenderScene( args[0] );
		return SysCallArg();
	case CG_R_SETCOLOR:
		re.SetColor( args[0] );
		return SysCallArg();
	case CG_R_DRAWSTRETCHPIC:
		re.DrawStretchPic( args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8] );
		return SysCallArg();
	case CG_R_MODELBOUNDS:
		re.ModelBounds( args[0], args[1], args[2] );
		return SysCallArg();
	case CG_R_LERPTAG:
		return SysCallArg::Int(re.LerpTag( args[0], args[1], args[2], args[3], args[4], args[5] ));
	case CG_GETGLCONFIG:
		CL_GetGlconfig( args[0] );
		return SysCallArg();
	case CG_GETGAMESTATE:
		CL_GetGameState( args[0] );
		return SysCallArg();
	case CG_GETCURRENTSNAPSHOTNUMBER:
		CL_GetCurrentSnapshotNumber( args[0], args[1] );
		return SysCallArg();
	case CG_GETSNAPSHOT:
		return SysCallArg::Int(CL_GetSnapshot( args[0], args[1] ));
	case CG_GETSERVERCOMMAND:
		return SysCallArg::Int(CL_GetServerCommand( args[0] ));
	case CG_GETCURRENTCMDNUMBER:
		return SysCallArg::Int(CL_GetCurrentCmdNumber());
	case CG_GETUSERCMD:
		return SysCallArg::Int(CL_GetUserCmd( args[0], args[1] ));
	case CG_SETUSERCMDVALUE:
		CL_SetUserCmdValue( args[0], args[1] );
		return SysCallArg();
	case CG_MEMORY_REMAINING:
		return SysCallArg::Int(Hunk_MemoryRemaining());
	case CG_KEY_ISDOWN:
	  return SysCallArg::Int(Key_IsDown( args[0] ));
	case CG_KEY_GETCATCHER:
	  return SysCallArg::Int(Key_GetCatcher());
	case CG_KEY_SETCATCHER:
		Key_SetCatcher( args[0] );
	    return SysCallArg();
	case CG_KEY_GETKEY:
		return SysCallArg::Int(Key_GetKey( args[0] ));


	case CG_MEMSET:
		Com_Memset( args[0], args[1], (int) args[2] );
		return SysCallArg();
	case CG_MEMCPY:
		Com_Memcpy( args[0], args[1], (int) args[2] );
		return SysCallArg();
	case CG_STRNCPY:
		return SysCallArg::Int((int)strncpy( args[0], args[1], (int)args[2] ));
	case CG_SIN:
		return SysCallArg::Fixed( FIXED_SIN( (gfixed)args[0] ) );
	case CG_COS:
		return SysCallArg::Fixed( FIXED_COS( (gfixed)args[0] ) );
	case CG_ATAN2:
		return SysCallArg::Fixed( FIXED_ATAN2( (gfixed)args[0], (gfixed)args[1] ) );
	case CG_SQRT:
		return SysCallArg::Fixed( FIXED_SQRT( (gfixed)args[0] ) );
	case CG_FLOOR:
		return SysCallArg::Fixed( FIXED_FLOOR( (gfixed)args[0] ) );
	case CG_CEIL:
		return SysCallArg::Fixed( FIXED_CEIL( (gfixed)args[0] ) );
	case CG_ACOS:
		return SysCallArg::Fixed( Q_acos( (gfixed)args[0] ) );

	case CG_PC_ADD_GLOBAL_DEFINE:
		return SysCallArg::Int(botlib_export->PC_AddGlobalDefine( args[0] ));
	case CG_PC_LOAD_SOURCE:
		return SysCallArg::Int(botlib_export->PC_LoadSourceHandle( args[0] ));
	case CG_PC_FREE_SOURCE:
		return SysCallArg::Int(botlib_export->PC_FreeSourceHandle( args[0] ));
	case CG_PC_READ_TOKEN:
		return SysCallArg::Int(botlib_export->PC_ReadTokenHandle( args[0], args[1] ));
	case CG_PC_SOURCE_FILE_AND_LINE:
		return SysCallArg::Int(botlib_export->PC_SourceFileAndLine( args[0], args[1], args[2] ));

	case CG_S_STOPBACKGROUNDTRACK:
		S_StopBackgroundTrack();
		return SysCallArg();

	case CG_REAL_TIME:
		return SysCallArg::Int(Com_RealTime( args[0] ));
	case CG_SNAPVECTOR:
		Sys_SnapVector( args[0] );
		return SysCallArg();

	case CG_CIN_PLAYCINEMATIC:
		return SysCallArg::Int(CIN_PlayCinematic(args[0], args[1], args[2], args[3], args[4], args[5]));

	case CG_CIN_STOPCINEMATIC:
		return SysCallArg::Int(CIN_StopCinematic(args[0]));

	case CG_CIN_RUNCINEMATIC:
		return SysCallArg::Int(CIN_RunCinematic(args[0]));

	case CG_CIN_DRAWCINEMATIC:
	  CIN_DrawCinematic(args[0]);
	  return SysCallArg();

	case CG_CIN_SETEXTENTS:
	  CIN_SetExtents(args[0], args[1], args[2], args[3], args[4]);
	  return SysCallArg();

	case CG_R_REMAP_SHADER:
		re.RemapShader( args[0], args[1], args[2] );
		return SysCallArg();

/*
	case CG_LOADCAMERA:
		return loadCamera(args[0]);

	case CG_STARTCAMERA:
		startCamera(args[0]);
		return SysCallArg();

	case CG_GETCAMERAINFO:
		return getCameraInfo(args[0], args[1], args[2]);
*/
	case CG_GET_ENTITY_TOKEN:
		return SysCallArg::Int(re.GetEntityToken( args[0], args[1] ));
	case CG_R_INPVS:
		return SysCallArg::Int(re.inPVS( args[0], args[1] ));

	default:
//	        assert(0); // bk010102
		Com_Error( ERR_DROP, "Bad cgame system trap: %i", callnum );
	}
	return SysCallArg();
}


/*
====================
CL_InitCGame

Should only be called by CL_StartHunkUsers
====================
*/
void CL_InitCGame( void ) {
	const char			*info;
	const char			*mapname;
	int					t1, t2;
	vmInterpret_t		interpret;

	t1 = Sys_Milliseconds();

	// put away the console
	Con_Close();

	// find the current mapname
	info = cl.gameState.stringData + cl.gameState.stringOffsets[ CS_SERVERINFO ];
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf( cl.mapname, sizeof( cl.mapname ), "maps/%s.bsp", mapname );

	// load the dll or bytecode
	if ( cl_connectedToPureServer != 0 ) {
		// if sv_pure is set we only allow qvms to be loaded
		interpret = VMI_COMPILED;
	}
	else {
		interpret = (vmInterpret_t) Cvar_VariableIntegerValue( "vm_cgame" );
	}
	cgvm = VM_Create( "cgame", CL_CgameSystemCalls, interpret );
	if ( !cgvm ) {
		Com_Error( ERR_DROP, "VM_Create on cgame failed" );
	}
	cls.state = CA_LOADING;

	// init for this gamestate
	// use the lastExecutedServerCommand instead of the serverCommandSequence
	// otherwise server commands sent just before a gamestate are dropped
	SysCallArgs args(3);
	args[0]=clc.serverMessageSequence;
	args[1]=clc.lastExecutedServerCommand;
	args[2]=clc.clientNum;

	VM_Call( cgvm, CG_INIT, args);

	// we will send a usercmd this frame, which
	// will cause the server to send us the first snapshot
	cls.state = CA_PRIMED;

	t2 = Sys_Milliseconds();

	Com_Printf( "CL_InitCGame: %5.2f seconds\n", (t2-t1)/1000.0 );

	// have the renderer touch all its images, so they are present
	// on the card even if the driver does deferred loading
	re.EndRegistration();

	// clear anything that got printed
	Con_ClearNotify ();
}


/*
====================
CL_GameCommand

See if the current console command is claimed by the cgame
====================
*/
qboolean CL_GameCommand( void ) {
	if ( !cgvm ) {
		return qfalse;
	}

	return VM_Call( cgvm, CG_CONSOLE_COMMAND, SysCallArgs(0));
}



/*
=====================
CL_CGameRendering
=====================
*/
void CL_CGameRendering( stereoFrame_t stereo ) {
	SysCallArgs args(3);
	args[0]=cl.serverTime;
	args[1]=stereo;
	args[2]=clc.demoplaying;
	VM_Call( cgvm, CG_DRAW_ACTIVE_FRAME, args);
	VM_Debug( 0 );
}


/*
=================
CL_AdjustTimeDelta

Adjust the clients view of server time.

We attempt to have cl.serverTime exactly equal the server's view
of time plus the timeNudge, but with variable latencies over
the internet it will often need to drift a bit to match conditions.

Our ideal time would be to have the adjusted time approach, but not pass,
the very latest snapshot.

Adjustments are only made when a new snapshot arrives with a rational
latency, which keeps the adjustment process framerate independent and
prevents massive overadjustment during times of significant packet loss
or bursted delayed packets.
=================
*/

#define	RESET_TIME	500

void CL_AdjustTimeDelta( void ) {
	int		resetTime;
	int		newDelta;
	int		deltaDelta;

	cl.newSnapshots = qfalse;

	// the delta never drifts when replaying a demo
	if ( clc.demoplaying ) {
		return;
	}

	// if the current time is WAY off, just correct to the current value
	if ( com_sv_running->integer ) {
		resetTime = 100;
	} else {
		resetTime = RESET_TIME;
	}

	newDelta = cl.snap.serverTime - cls.realtime;
	deltaDelta = abs( newDelta - cl.serverTimeDelta );

	if ( deltaDelta > RESET_TIME ) {
		cl.serverTimeDelta = newDelta;
		cl.oldServerTime = cl.snap.serverTime;	// FIXME: is this a problem for cgame?
		cl.serverTime = cl.snap.serverTime;
		if ( cl_showTimeDelta->integer ) {
			Com_Printf( "<RESET> " );
		}
	} else if ( deltaDelta > 100 ) {
		// fast adjust, cut the difference in half
		if ( cl_showTimeDelta->integer ) {
			Com_Printf( "<FAST> " );
		}
		cl.serverTimeDelta = ( cl.serverTimeDelta + newDelta ) >> 1;
	} else {
		// slow drift adjust, only move 1 or 2 msec

		// if any of the frames between this and the previous snapshot
		// had to be extrapolated, nudge our sense of time back a little
		// the granularity of +1 / -2 is too high for timescale modified frametimes
		if ( FIXED_IS_ZERO(com_timescale->value) || FIXED_IS_ONE(com_timescale->value)) {
			if ( cl.extrapolatedSnapshot ) {
				cl.extrapolatedSnapshot = qfalse;
				cl.serverTimeDelta -= 2;
			} else {
				// otherwise, move our sense of time forward to minimize total latency
				cl.serverTimeDelta++;
			}
		}
	}

	if ( cl_showTimeDelta->integer ) {
		Com_Printf( "%i ", cl.serverTimeDelta );
	}
}


/*
==================
CL_FirstSnapshot
==================
*/
void CL_FirstSnapshot( void ) {
	// ignore snapshots that don't have entities
	if ( cl.snap.snapFlags & SNAPFLAG_NOT_ACTIVE ) {
		return;
	}
	cls.state = CA_ACTIVE;

	// set the timedelta so we are exactly on this first frame
	cl.serverTimeDelta = cl.snap.serverTime - cls.realtime;
	cl.oldServerTime = cl.snap.serverTime;

	clc.timeDemoBaseTime = cl.snap.serverTime;

	// if this is the first frame of active play,
	// execute the contents of activeAction now
	// this is to allow scripting a timedemo to start right
	// after loading
	if ( cl_activeAction->string[0] ) {
		Cbuf_AddText( cl_activeAction->string );
		Cvar_Set( "activeAction", "" );
	}
	
	Sys_BeginProfiling();
}

/*
==================
CL_SetCGameTime
==================
*/
void CL_SetCGameTime( void ) {
	// getting a valid frame message ends the connection process
	if ( cls.state != CA_ACTIVE ) {
		if ( cls.state != CA_PRIMED ) {
			return;
		}
		if ( clc.demoplaying ) {
			// we shouldn't get the first snapshot on the same frame
			// as the gamestate, because it causes a bad time skip
			if ( !clc.firstDemoFrameSkipped ) {
				clc.firstDemoFrameSkipped = qtrue;
				return;
			}
			CL_ReadDemoMessage();
		}
		if ( cl.newSnapshots ) {
			cl.newSnapshots = qfalse;
			CL_FirstSnapshot();
		}
		if ( cls.state != CA_ACTIVE ) {
			return;
		}
	}	

	// if we have gotten to this point, cl.snap is guaranteed to be valid
	if ( !cl.snap.valid ) {
		Com_Error( ERR_DROP, "CL_SetCGameTime: !cl.snap.valid" );
	}

	// allow pause in single player
	if ( sv_paused->integer && cl_paused->integer && com_sv_running->integer ) {
		// paused
		return;
	}

	if ( cl.snap.serverTime < cl.oldFrameServerTime ) {
		Com_Error( ERR_DROP, "cl.snap.serverTime < cl.oldFrameServerTime" );
	}
	cl.oldFrameServerTime = cl.snap.serverTime;


	// get our current view of time

	if ( clc.demoplaying && cl_freezeDemo->integer ) {
		// cl_freezeDemo is used to lock a demo in place for single frame advances

	} else {
		// cl_timeNudge is a user adjustable cvar that allows more
		// or less latency to be added in the interest of better 
		// smoothness or better responsiveness.
		int tn;
		
		tn = cl_timeNudge->integer;
		if (tn<-30) {
			tn = -30;
		} else if (tn>30) {
			tn = 30;
		}

		cl.serverTime = cls.realtime + cl.serverTimeDelta - tn;

		// guarantee that time will never flow backwards, even if
		// serverTimeDelta made an adjustment or cl_timeNudge was changed
		if ( cl.serverTime < cl.oldServerTime ) {
			cl.serverTime = cl.oldServerTime;
		}
		cl.oldServerTime = cl.serverTime;

		// note if we are almost past the latest frame (without timeNudge),
		// so we will try and adjust back a bit when the next snapshot arrives
		if ( cls.realtime + cl.serverTimeDelta >= cl.snap.serverTime - 5 ) {
			cl.extrapolatedSnapshot = qtrue;
		}
	}

	// if we have gotten new snapshots, drift serverTimeDelta
	// don't do this every frame, or a period of packet loss would
	// make a huge adjustment
	if ( cl.newSnapshots ) {
		CL_AdjustTimeDelta();
	}

	if ( !clc.demoplaying ) {
		return;
	}

	// if we are playing a demo back, we can just keep reading
	// messages from the demo file until the cgame definately
	// has valid snapshots to interpolate between

	// a timedemo will always use a deterministic set of time samples
	// no matter what speed machine it is run on,
	// while a normal demo may have different time samples
	// each time it is played back
	if ( cl_timedemo->integer ) {
		if (!clc.timeDemoStart) {
			clc.timeDemoStart = Sys_Milliseconds();
		}
		clc.timeDemoFrames++;
		cl.serverTime = clc.timeDemoBaseTime + clc.timeDemoFrames * 50;
	}

	while ( cl.serverTime >= cl.snap.serverTime ) {
		// feed another messag, which should change
		// the contents of cl.snap
		CL_ReadDemoMessage();
		if ( cls.state != CA_ACTIVE ) {
			return;		// end of demo
		}
	}

}



