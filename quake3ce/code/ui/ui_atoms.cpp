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
/**********************************************************************
	UI_ATOMS.C

	User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"

qboolean		m_entersound;		// after a frame, so caching won't disrupt the sound

// these are here so the functions in q_shared.c can link
#ifndef UI_HARD_LINKED

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	_UI_trap_Error( va("%s", text) );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	_UI_trap_Print( va("%s", text) );
}

#endif

qboolean newUI = qfalse;


/*
=================
UI_ClampCvar
=================
*/
gfixed UI_ClampCvar( gfixed min, gfixed max, gfixed value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

/*
=================
UI_StartDemoLoop
=================
*/
void UI_StartDemoLoop( void ) {
	_UI_trap_Cmd_ExecuteText( EXEC_APPEND, "d1\n" );
}


#ifndef MISSIONPACK // bk001206
static void NeedCDAction( qboolean result ) {
	if ( !result ) {
		_UI_trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}
#endif // MISSIONPACK

#ifndef MISSIONPACK // bk001206
static void NeedCDKeyAction( qboolean result ) {
	if ( !result ) {
		_UI_trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}
#endif // MISSIONPACK

char *UI_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	_UI_trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


char *UI_Cvar_VariableString( const char *var_name ) {
	static char	buffer[MAX_STRING_CHARS];

	_UI_trap_Cvar_VariableStringBuffer( var_name, buffer, sizeof( buffer ) );

	return buffer;
}



void UI_SetBestScores(postGameInfo_t *newInfo, qboolean postGame) {
	_UI_trap_Cvar_Set("ui_scoreAccuracy",     va("%i%%", newInfo->accuracy));
	_UI_trap_Cvar_Set("ui_scoreImpressives",	va("%i", newInfo->impressives));
	_UI_trap_Cvar_Set("ui_scoreExcellents", 	va("%i", newInfo->excellents));
	_UI_trap_Cvar_Set("ui_scoreDefends", 			va("%i", newInfo->defends));
	_UI_trap_Cvar_Set("ui_scoreAssists", 			va("%i", newInfo->assists));
	_UI_trap_Cvar_Set("ui_scoreGauntlets", 		va("%i", newInfo->gauntlets));
	_UI_trap_Cvar_Set("ui_scoreScore", 				va("%i", newInfo->score));
	_UI_trap_Cvar_Set("ui_scorePerfect",	 		va("%i", newInfo->perfects));
	_UI_trap_Cvar_Set("ui_scoreTeam",					va("%i to %i", newInfo->redScore, newInfo->blueScore));
	_UI_trap_Cvar_Set("ui_scoreBase",					va("%i", newInfo->baseScore));
	_UI_trap_Cvar_Set("ui_scoreTimeBonus",		va("%i", newInfo->timeBonus));
	_UI_trap_Cvar_Set("ui_scoreSkillBonus",		va("%i", newInfo->skillBonus));
	_UI_trap_Cvar_Set("ui_scoreShutoutBonus",	va("%i", newInfo->shutoutBonus));
	_UI_trap_Cvar_Set("ui_scoreTime",					va("%02i:%02i", newInfo->time / 60, newInfo->time % 60));
	_UI_trap_Cvar_Set("ui_scoreCaptures",		va("%i", newInfo->captures));
  if (postGame) {
		_UI_trap_Cvar_Set("ui_scoreAccuracy2",     va("%i%%", newInfo->accuracy));
		_UI_trap_Cvar_Set("ui_scoreImpressives2",	va("%i", newInfo->impressives));
		_UI_trap_Cvar_Set("ui_scoreExcellents2", 	va("%i", newInfo->excellents));
		_UI_trap_Cvar_Set("ui_scoreDefends2", 			va("%i", newInfo->defends));
		_UI_trap_Cvar_Set("ui_scoreAssists2", 			va("%i", newInfo->assists));
		_UI_trap_Cvar_Set("ui_scoreGauntlets2", 		va("%i", newInfo->gauntlets));
		_UI_trap_Cvar_Set("ui_scoreScore2", 				va("%i", newInfo->score));
		_UI_trap_Cvar_Set("ui_scorePerfect2",	 		va("%i", newInfo->perfects));
		_UI_trap_Cvar_Set("ui_scoreTeam2",					va("%i to %i", newInfo->redScore, newInfo->blueScore));
		_UI_trap_Cvar_Set("ui_scoreBase2",					va("%i", newInfo->baseScore));
		_UI_trap_Cvar_Set("ui_scoreTimeBonus2",		va("%i", newInfo->timeBonus));
		_UI_trap_Cvar_Set("ui_scoreSkillBonus2",		va("%i", newInfo->skillBonus));
		_UI_trap_Cvar_Set("ui_scoreShutoutBonus2",	va("%i", newInfo->shutoutBonus));
		_UI_trap_Cvar_Set("ui_scoreTime2",					va("%02i:%02i", newInfo->time / 60, newInfo->time % 60));
		_UI_trap_Cvar_Set("ui_scoreCaptures2",		va("%i", newInfo->captures));
	}
}

void UI_LoadBestScores(const char *map, int game) {
	char		fileName[MAX_QPATH];
	fileHandle_t f;
	postGameInfo_t newInfo;
	memset(&newInfo, 0, sizeof(postGameInfo_t));
	Com_sprintf(fileName, MAX_QPATH, "games/%s_%i.game", map, game);
	if (_UI_trap_FS_FOpenFile(fileName, &f, FS_READ) >= 0) {
		int size = 0;
		_UI_trap_FS_Read(&size, sizeof(int), f);
		if (size == sizeof(postGameInfo_t)) {
			_UI_trap_FS_Read(&newInfo, sizeof(postGameInfo_t), f);
		}
		_UI_trap_FS_FCloseFile(f);
	}
	UI_SetBestScores(&newInfo, qfalse);

	Com_sprintf(fileName, MAX_QPATH, "demos/%s_%d.dm_%d", map, game, FIXED_TO_INT(_UI_trap_Cvar_VariableValue("protocol")));
	uiInfo.demoAvailable = qfalse;
	if (_UI_trap_FS_FOpenFile(fileName, &f, FS_READ) >= 0) {
		uiInfo.demoAvailable = qtrue;
		_UI_trap_FS_FCloseFile(f);
	} 
}

/*
===============
UI_ClearScores
===============
*/
void UI_ClearScores() {
	char	gameList[4096];
	char *gameFile;
	int		i, len, count, size;
	fileHandle_t f;
	postGameInfo_t newInfo;

	count = _UI_trap_FS_GetFileList( "games", "game", gameList, sizeof(gameList) );

	size = sizeof(postGameInfo_t);
	memset(&newInfo, 0, size);

	if (count > 0) {
		gameFile = gameList;
		for ( i = 0; i < count; i++ ) {
			len = strlen(gameFile);
			if (_UI_trap_FS_FOpenFile(va("games/%s",gameFile), &f, FS_WRITE) >= 0) {
				_UI_trap_FS_Write(&size, sizeof(int), f);
				_UI_trap_FS_Write(&newInfo, size, f);
				_UI_trap_FS_FCloseFile(f);
			}
			gameFile += len + 1;
		}
	}
	
	UI_SetBestScores(&newInfo, qfalse);

}



static void	UI_Cache_f() {
	Display_CacheAll();
}

/*
=======================
UI_CalcPostGameStats
=======================
*/
static void UI_CalcPostGameStats() {
	char		map[MAX_QPATH];
	char		fileName[MAX_QPATH];
	char		info[MAX_INFO_STRING];
	fileHandle_t f;
	int size, game, time, adjustedTime;
	postGameInfo_t oldInfo;
	postGameInfo_t newInfo;
	qboolean newHigh = qfalse;

	_UI_trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) );
	Q_strncpyz( map, Info_ValueForKey( info, "mapname" ), sizeof(map) );
	game = atoi(Info_ValueForKey(info, "g_gametype"));

	// compose file name
	Com_sprintf(fileName, MAX_QPATH, "games/%s_%i.game", map, game);
	// see if we have one already
	memset(&oldInfo, 0, sizeof(postGameInfo_t));
	if (_UI_trap_FS_FOpenFile(fileName, &f, FS_READ) >= 0) {
	// if so load it
		size = 0;
		_UI_trap_FS_Read(&size, sizeof(int), f);
		if (size == sizeof(postGameInfo_t)) {
			_UI_trap_FS_Read(&oldInfo, sizeof(postGameInfo_t), f);
		}
		_UI_trap_FS_FCloseFile(f);
	}					 

	newInfo.accuracy = atoi(UI_Argv(3));
	newInfo.impressives = atoi(UI_Argv(4));
	newInfo.excellents = atoi(UI_Argv(5));
	newInfo.defends = atoi(UI_Argv(6));
	newInfo.assists = atoi(UI_Argv(7));
	newInfo.gauntlets = atoi(UI_Argv(8));
	newInfo.baseScore = atoi(UI_Argv(9));
	newInfo.perfects = atoi(UI_Argv(10));
	newInfo.redScore = atoi(UI_Argv(11));
	newInfo.blueScore = atoi(UI_Argv(12));
	time = atoi(UI_Argv(13));
	newInfo.captures = atoi(UI_Argv(14));

	newInfo.time = (time - FIXED_TO_INT(_UI_trap_Cvar_VariableValue("ui_matchStartTime"))) / 1000;
	adjustedTime = uiInfo.mapList[ui_currentMap.integer].timeToBeat[game];
	if (newInfo.time < adjustedTime) { 
		newInfo.timeBonus = (adjustedTime - newInfo.time) * 10;
	} else {
		newInfo.timeBonus = 0;
	}

	if (newInfo.redScore > newInfo.blueScore && newInfo.blueScore <= 0) {
		newInfo.shutoutBonus = 100;
	} else {
		newInfo.shutoutBonus = 0;
	}

	newInfo.skillBonus = FIXED_TO_INT(_UI_trap_Cvar_VariableValue("g_spSkill"));
	if (newInfo.skillBonus <= 0) {
		newInfo.skillBonus = 1;
	}
	newInfo.score = newInfo.baseScore + newInfo.shutoutBonus + newInfo.timeBonus;
	newInfo.score *= newInfo.skillBonus;

	// see if the score is higher for this one
	newHigh = (newInfo.redScore > newInfo.blueScore && newInfo.score > oldInfo.score);

	if  (newHigh) {
		// if so write out the new one
		uiInfo.newHighScoreTime = uiInfo.uiDC.realTime + 20000;
		if (_UI_trap_FS_FOpenFile(fileName, &f, FS_WRITE) >= 0) {
			size = sizeof(postGameInfo_t);
			_UI_trap_FS_Write(&size, sizeof(int), f);
			_UI_trap_FS_Write(&newInfo, sizeof(postGameInfo_t), f);
			_UI_trap_FS_FCloseFile(f);
		}
	}

	if (newInfo.time < oldInfo.time) {
		uiInfo.newBestTime = uiInfo.uiDC.realTime + 20000;
	}
 
	// put back all the ui overrides
	_UI_trap_Cvar_Set("capturelimit", UI_Cvar_VariableString("ui_saveCaptureLimit"));
	_UI_trap_Cvar_Set("fraglimit", UI_Cvar_VariableString("ui_saveFragLimit"));
	_UI_trap_Cvar_Set("cg_drawTimer", UI_Cvar_VariableString("ui_drawTimer"));
	_UI_trap_Cvar_Set("g_doWarmup", UI_Cvar_VariableString("ui_doWarmup"));
	_UI_trap_Cvar_Set("g_Warmup", UI_Cvar_VariableString("ui_Warmup"));
	_UI_trap_Cvar_Set("sv_pure", UI_Cvar_VariableString("ui_pure"));
	_UI_trap_Cvar_Set("g_friendlyFire", UI_Cvar_VariableString("ui_friendlyFire"));

	UI_SetBestScores(&newInfo, qtrue);
	UI_ShowPostGame(newHigh);


}


/*
=================
UI_ConsoleCommand
=================
*/
qboolean UI_ConsoleCommand( int realTime ) {
	char	*cmd;

	uiInfo.uiDC.frameTime = realTime - uiInfo.uiDC.realTime;
	uiInfo.uiDC.realTime = realTime;

	cmd = UI_Argv( 0 );

	// ensure minimum menu data is available
	//Menu_Cache();

	if ( Q_stricmp (cmd, "ui_test") == 0 ) {
		UI_ShowPostGame(qtrue);
	}

	if ( Q_stricmp (cmd, "ui_report") == 0 ) {
		UI_Report();
		return qtrue;
	}
	
	if ( Q_stricmp (cmd, "ui_load") == 0 ) {
		UI_Load();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "remapShader") == 0 ) {
		if (_UI_trap_Argc() == 4) {
			char shader1[MAX_QPATH];
			char shader2[MAX_QPATH];
			Q_strncpyz(shader1, UI_Argv(1), sizeof(shader1));
			Q_strncpyz(shader2, UI_Argv(2), sizeof(shader2));
			_UI_trap_R_RemapShader(shader1, shader2, UI_Argv(3));
			return qtrue;
		}
	}

	if ( Q_stricmp (cmd, "postgame") == 0 ) {
		UI_CalcPostGameStats();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_cache") == 0 ) {
		UI_Cache_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_teamOrders") == 0 ) {
		//UI_TeamOrdersMenu_f();
		return qtrue;
	}


	if ( Q_stricmp (cmd, "ui_cdkey") == 0 ) {
		//UI_CDKeyMenu_f();
		return qtrue;
	}

	return qfalse;
}

/*
=================
UI_Shutdown
=================
*/
void UI_Shutdown( void ) {
}

/*
================
UI_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom640( gfixed *x, gfixed *y, gfixed *w, gfixed *h ) {
	// expect valid pointers
#if 0
	*x = *x * uiInfo.uiDC.scale + uiInfo.uiDC.bias;
	*y *= uiInfo.uiDC.scale;
	*w *= uiInfo.uiDC.scale;
	*h *= uiInfo.uiDC.scale;
#endif

	*x *= uiInfo.uiDC.xscale;
	*y *= uiInfo.uiDC.yscale;
	*w *= uiInfo.uiDC.xscale;
	*h *= uiInfo.uiDC.yscale;

}

void UI_DrawNamedPic( gfixed x, gfixed y, gfixed width, gfixed height, const char *picname ) {
	qhandle_t	hShader;

	hShader = _UI_trap_R_RegisterShaderNoMip( picname );
	UI_AdjustFrom640( &x, &y, &width, &height );
	_UI_trap_R_DrawStretchPic( x, y, width, height, GFIXED_0, GFIXED_0, GFIXED_1, GFIXED_1, hShader );
}

void UI_DrawHandlePic( gfixed x, gfixed y, gfixed w, gfixed h, qhandle_t hShader ) {
	gfixed	s0;
	gfixed	s1;
	gfixed	t0;
	gfixed	t1;

	if( w < GFIXED_0 ) {	// flip about vertical
		w  = -w;
		s0 = GFIXED_1;
		s1 = GFIXED_0;
	}
	else {
		s0 = GFIXED_0;
		s1 = GFIXED_1;
	}

	if( h < GFIXED_0 ) {	// flip about horizontal
		h  = -h;
		t0 = GFIXED_1;
		t1 = GFIXED_0;
	}
	else {
		t0 = GFIXED_0;
		t1 = GFIXED_1;
	}
	
	UI_AdjustFrom640( &x, &y, &w, &h );
	_UI_trap_R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
================
UI_FillRect

Coordinates are 640*480 virtual values
=================
*/
void UI_FillRect( gfixed x, gfixed y, gfixed width, gfixed height, const gfixed *color ) {
	_UI_trap_R_SetColor( color );

	UI_AdjustFrom640( &x, &y, &width, &height );
	_UI_trap_R_DrawStretchPic( x, y, width, height, GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_0, uiInfo.uiDC.whiteShader );

	_UI_trap_R_SetColor( NULL );
}

void UI_DrawSides(gfixed x, gfixed y, gfixed w, gfixed h) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	_UI_trap_R_DrawStretchPic( x, y, GFIXED_1, h, GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_0, uiInfo.uiDC.whiteShader );
	_UI_trap_R_DrawStretchPic( x + w - GFIXED_1, y, GFIXED_1, h, GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_0, uiInfo.uiDC.whiteShader );
}

void UI_DrawTopBottom(gfixed x, gfixed y, gfixed w, gfixed h) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	_UI_trap_R_DrawStretchPic( x, y, w, GFIXED_1, GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_0, uiInfo.uiDC.whiteShader );
	_UI_trap_R_DrawStretchPic( x, y + h - GFIXED_1, w, GFIXED_1, GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_0, uiInfo.uiDC.whiteShader );
}
/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRect( gfixed x, gfixed y, gfixed width, gfixed height, const gfixed *color ) {
	_UI_trap_R_SetColor( color );

  UI_DrawTopBottom(x, y, width, height);
  UI_DrawSides(x, y, width, height);

	_UI_trap_R_SetColor( NULL );
}

void UI_SetColor( const gfixed *rgba ) {
	_UI_trap_R_SetColor( rgba );
}

void UI_UpdateScreen( void ) {
	_UI_trap_UpdateScreen();
}


void UI_DrawTextBox (int x, int y, int width, int lines)
{
	UI_FillRect( MAKE_GFIXED(x + BIGCHAR_WIDTH/2), 
		     MAKE_GFIXED(y + BIGCHAR_HEIGHT/2),
		     MAKE_GFIXED(( width + 1 ) * BIGCHAR_WIDTH),
		     MAKE_GFIXED(( lines + 1 ) * BIGCHAR_HEIGHT),
	             colorBlack );
	UI_DrawRect( MAKE_GFIXED(x + BIGCHAR_WIDTH/2), 
		     MAKE_GFIXED(y + BIGCHAR_HEIGHT/2),
                     MAKE_GFIXED(( width + 1 ) * BIGCHAR_WIDTH),
                     MAKE_GFIXED(( lines + 1 ) * BIGCHAR_HEIGHT),
                     colorWhite );
}

qboolean UI_CursorInRect (int x, int y, int width, int height)
{
	if (uiInfo.uiDC.cursorx < x ||
		uiInfo.uiDC.cursory < y ||
		uiInfo.uiDC.cursorx > x+width ||
		uiInfo.uiDC.cursory > y+height)
		return qfalse;

	return qtrue;
}
