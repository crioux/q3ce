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
// cl_scrn.c -- master for refresh, status bar, console, chat, notify, etc

#include"client_pch.h"

qboolean	scr_initialized;		// ready to draw

cvar_t		*cl_timegraph;
cvar_t		*cl_debuggraph;
cvar_t		*cl_graphheight;
cvar_t		*cl_graphscale;
cvar_t		*cl_graphshift;

/*
================
SCR_DrawNamedPic

Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawNamedPic( gfixed x, gfixed y, gfixed width, gfixed height, const char *picname ) {
	qhandle_t	hShader;

	assert( FIXED_NOT_ZERO(width) );

	hShader = re.RegisterShader( picname );
	SCR_AdjustFrom640( &x, &y, &width, &height );
	re.DrawStretchPic( x, y, width, height, GFIXED_0, GFIXED_0, GFIXED_1, GFIXED_1, hShader );
}


/*
================
SCR_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void SCR_AdjustFrom640( gfixed *x, gfixed *y, gfixed *w, gfixed *h ) {
	gfixed	xscale;
	gfixed	yscale;
	int vx,vy;

	vx=cls.glconfig.vidWidth;
	vy=cls.glconfig.vidHeight;

	// adjust for wide screens
	if ( vx * 480 > vy * 640 ) {
		if(x)
		{
			*x += GFIXED(0,5) * MAKE_GFIXED( vx - ( vy * 640 / 480 ) );
		}
	}
	else if ( vx * 480 < vy * 640 ) {
		if(y)
		{
			*y += GFIXED(0,5) * MAKE_GFIXED( vy - ( vx * 480 / 640 ) );
		}
	}

	// scale for screen sizes
	
	xscale = FIXED_INT32RATIO_G(vx,640);
	yscale = FIXED_INT32RATIO_G(vy,480);
	
	if(xscale>yscale)
	{
		if(x) *x *= yscale;
		if(y) *y *= yscale;
		if(w) *w *= yscale;
		if(h) *h *= yscale;
	}
	else
	{
		if(x) *x *= xscale;
		if(y) *y *= xscale;
		if(w) *w *= xscale;
		if(h) *h *= xscale;
	}

}

/*
================
SCR_FillRect

Coordinates are 640*480 virtual values
=================
*/
void SCR_FillRect( gfixed x, gfixed y, gfixed width, gfixed height, const gfixed *color ) {
	re.SetColor( color );

	SCR_AdjustFrom640( &x, &y, &width, &height );
	re.DrawStretchPic( x, y, width, height, GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_0, cls.whiteShader );

	re.SetColor( NULL );
}


/*
================
SCR_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawPic( gfixed x, gfixed y, gfixed width, gfixed height, qhandle_t hShader ) {
	SCR_AdjustFrom640( &x, &y, &width, &height );
	re.DrawStretchPic( x, y, width, height, GFIXED_0, GFIXED_0, GFIXED_1, GFIXED_1, hShader );
}



/*
** SCR_DrawChar
** chars are drawn at 640*480 virtual screen size
*/
void SCR_DrawChar( int x, int y, gfixed size, int ch ) {
	int row, col;
	gfixed frow, fcol;
	gfixed	ax, ay, aw, ah;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	if ( MAKE_GFIXED(y) < -size ) {
		return;
	}

	ax = MAKE_GFIXED(x);
	ay = MAKE_GFIXED(y);
	aw = size;
	ah = size;
	SCR_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = ch>>4;
	col = ch&15;

	frow = MAKE_GFIXED(row)*GFIXED(0,0625);
	fcol = MAKE_GFIXED(col)*GFIXED(0,0625);
	size = GFIXED(0,0625);

	re.DrawStretchPic( ax, ay, aw, ah,
					   fcol, frow, 
					   fcol + size, frow + size, 
					   cls.charSetShader );
}

/*
** SCR_DrawSmallChar
** small chars are drawn at native screen resolution
*/
void SCR_DrawSmallChar( int x, int y, int ch ) {
	int row, col;
	gfixed frow, fcol;
	gfixed size;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	if ( y < -SMALLCHAR_HEIGHT ) {
		return;
	}

	row = ch>>4;
	col = ch&15;

	frow = MAKE_GFIXED(row)*GFIXED(0,0625);
	fcol = MAKE_GFIXED(col)*GFIXED(0,0625);
	size = GFIXED(0,0625);

	re.DrawStretchPic( MAKE_GFIXED(x), MAKE_GFIXED(y), GFIXED(8,0), GFIXED(12,0),
					   fcol, frow, 
					   fcol + size, frow + size, 
					   cls.charSetShader );
}


/*
==================
SCR_DrawBigString[Color]

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void SCR_DrawStringExt( int x, int y, gfixed size, const char *string, gfixed *setColor, qboolean forceColor ) {
	vec4_t		color;
	const char	*s;
	int			xx;

	// draw the drop shadow
	color[0] = color[1] = color[2] = GFIXED_0;
	color[3] = setColor[3];
	re.SetColor( color );
	s = string;
	xx = x;
	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s += 2;
			continue;
		}
		SCR_DrawChar( xx+2, y+2, size, *s );
		xx += FIXED_TO_INT(size);
		s++;
	}


	// draw the colored text
	s = string;
	xx = x;
	re.SetColor( setColor );
	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				Com_Memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
				color[3] = setColor[3];
				re.SetColor( color );
			}
			s += 2;
			continue;
		}
		SCR_DrawChar( xx, y, size, *s );
		xx += FIXED_TO_INT(size);
		s++;
	}
	re.SetColor( NULL );
}


void SCR_DrawBigString( int x, int y, const char *s, gfixed alpha ) {
	gfixed	color[4];

	color[0] = color[1] = color[2] = GFIXED_1;
	color[3] = alpha;
	SCR_DrawStringExt( x, y, MAKE_GFIXED(BIGCHAR_WIDTH), s, color, qfalse );
}

void SCR_DrawBigStringColor( int x, int y, const char *s, vec4_t color ) {
	SCR_DrawStringExt( x, y, MAKE_GFIXED(BIGCHAR_WIDTH), s, color, qtrue );
}


/*
==================
SCR_DrawSmallString[Color]

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void SCR_DrawSmallStringExt( int x, int y, const char *string, gfixed *setColor, qboolean forceColor ) {
	vec4_t		color;
	const char	*s;
	int			xx;

	// draw the colored text
	s = string;
	xx = x;
	re.SetColor( setColor );
	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				Com_Memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
				color[3] = setColor[3];
				re.SetColor( color );
			}
			s += 2;
			continue;
		}
		SCR_DrawSmallChar( xx, y, *s );
		xx += SMALLCHAR_WIDTH;
		s++;
	}
	re.SetColor( NULL );
}



/*
** SCR_Strlen -- skips color escape codes
*/
static int SCR_Strlen( const char *str ) {
	const char *s = str;
	int count = 0;

	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s += 2;
		} else {
			count++;
			s++;
		}
	}

	return count;
}

/*
** SCR_GetBigStringWidth
*/ 
int	SCR_GetBigStringWidth( const char *str ) {
	return SCR_Strlen( str ) * 16;
}


//===============================================================================

/*
=================
SCR_DrawDemoRecording
=================
*/
void SCR_DrawDemoRecording( void ) {
	char	string[1024];
	int		pos;

	if ( !clc.demorecording ) {
		return;
	}
	if ( clc.spDemoRecording ) {
		return;
	}

	pos = FS_FTell( clc.demofile );
	sprintf( string, "RECORDING %s: %ik", clc.demoName, pos / 1024 );

	SCR_DrawStringExt( 320 - strlen( string ) * 4, 20, GFIXED(8,0), string, g_color_table[7], qtrue );
}


/*
===============================================================================

DEBUG GRAPH

===============================================================================
*/

typedef struct
{
	gfixed	value;
	int		color;
} graphsamp_t;

static	int			current;
static	graphsamp_t	values[1024];

/*
==============
SCR_DebugGraph
==============
*/
void SCR_DebugGraph (gfixed value, int color)
{
	values[current&1023].value = value;
	values[current&1023].color = color;
	current++;
}

/*
==============
SCR_DrawDebugGraph
==============
*/
void SCR_DrawDebugGraph (void)
{
	int		a, x, y, w, i, h;
	gfixed	v;
	int		color;

	//
	// draw the graph
	//
	w = cls.glconfig.vidWidth;
	x = 0;
	y = cls.glconfig.vidHeight;
	re.SetColor( g_color_table[0] );
	re.DrawStretchPic(MAKE_GFIXED(x), MAKE_GFIXED(y - cl_graphheight->integer), 
		MAKE_GFIXED(w), MAKE_GFIXED(cl_graphheight->integer), GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_0, cls.whiteShader );
	re.SetColor( NULL );

	for (a=0 ; a<w ; a++)
	{
		i = (current-1-a+1024) & 1023;
		v = values[i].value;
		color = values[i].color;
		v = v * MAKE_GFIXED(cl_graphscale->integer + cl_graphshift->integer);
		
		if (v < GFIXED_0)
			v += MAKE_GFIXED(cl_graphheight->integer) * MAKE_GFIXED(1+FIXED_TO_INT(-v / MAKE_GFIXED(cl_graphheight->integer)));
		h = FIXED_TO_INT(v) % cl_graphheight->integer;
		re.DrawStretchPic( MAKE_GFIXED(x+w-1-a), MAKE_GFIXED(y - h), GFIXED_1, MAKE_GFIXED(h), GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_0, cls.whiteShader );
	}
}

//=============================================================================

/*
==================
SCR_Init
==================
*/
void SCR_Init( void ) {
	cl_timegraph = Cvar_Get ("timegraph", "0", CVAR_CHEAT);
	cl_debuggraph = Cvar_Get ("debuggraph", "0", CVAR_CHEAT);
	cl_graphheight = Cvar_Get ("graphheight", "32", CVAR_CHEAT);
	cl_graphscale = Cvar_Get ("graphscale", "1", CVAR_CHEAT);
	cl_graphshift = Cvar_Get ("graphshift", "0", CVAR_CHEAT);

	scr_initialized = qtrue;
}


//=======================================================

/*
==================
SCR_DrawScreenField

This will be called twice if rendering in stereo mode
==================
*/
void SCR_DrawScreenField( stereoFrame_t stereoFrame ) {
	re.BeginFrame( stereoFrame );

	// wide aspect ratio screens need to have the sides cleared
	// unless they are displaying game renderings
	if ( cls.state != CA_ACTIVE ) {
		if ( cls.glconfig.vidWidth * 480 > cls.glconfig.vidHeight * 640 ) {
			re.SetColor( g_color_table[0] );
			re.DrawStretchPic( GFIXED_0, GFIXED_0, MAKE_GFIXED(cls.glconfig.vidWidth), MAKE_GFIXED(cls.glconfig.vidHeight), GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_0, cls.whiteShader );
			re.SetColor( NULL );
		}
		else if ( cls.glconfig.vidWidth * 480 < cls.glconfig.vidHeight * 640 ) {
			re.SetColor( g_color_table[0] );
			re.DrawStretchPic( GFIXED_0, GFIXED_0, MAKE_GFIXED(cls.glconfig.vidWidth), MAKE_GFIXED(cls.glconfig.vidHeight), GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_0, cls.whiteShader );
			re.SetColor( NULL );
		}
	}

	if ( !uivm ) {
		Com_DPrintf("draw screen without UI loaded\n");
		return;
	}

	// if the menu is going to cover the entire screen, we
	// don't need to render anything under it
	
	if ( !(int)VM_Call( uivm, UI_IS_FULLSCREEN, SysCallArgs(0) )) {
		switch( cls.state ) {
		default:
			Com_Error( ERR_FATAL, "SCR_DrawScreenField: bad cls.state" );
			break;
		case CA_CINEMATIC:
			SCR_DrawCinematic();
			break;
		case CA_DISCONNECTED:
			// force menu up
			{
				S_StopAllSounds();
				SysCallArgs args(1);
				args[0]=UIMENU_MAIN;
				VM_Call( uivm, UI_SET_ACTIVE_MENU, args );
			}
			break;
		case CA_CONNECTING:
		case CA_CHALLENGING:
		case CA_CONNECTED:
			// connecting clients will only show the connection dialog
			// refresh to update the time
			{
				SysCallArgs args(1);
				args[0]=cls.realtime;
				VM_Call( uivm, UI_REFRESH, args);
				args[0]=qfalse;
				VM_Call( uivm, UI_DRAW_CONNECT_SCREEN, args);
			}
			break;
		case CA_LOADING:
		case CA_PRIMED:
			// draw the game information screen and loading progress
			CL_CGameRendering( stereoFrame );

			// also draw the connection information, so it doesn't
			// flash away too briefly on local or lan games
			// refresh to update the time
			{
				SysCallArgs args(1);
				args[0]=cls.realtime;
				VM_Call( uivm, UI_REFRESH, args );
				args[0]=qtrue;
				VM_Call( uivm, UI_DRAW_CONNECT_SCREEN, args );
			}
			break;
		case CA_ACTIVE:
			CL_CGameRendering( stereoFrame );
			SCR_DrawDemoRecording();
			break;
		}
	}

	// the menu draws next
	if ( cls.keyCatchers & KEYCATCH_UI && uivm ) {
		SysCallArgs args(1);
		args[0]=cls.realtime;
		VM_Call( uivm, UI_REFRESH, args);
	}

	// console draws next
	Con_DrawConsole ();

	// debug graph can be drawn on top of anything
	if ( cl_debuggraph->integer || cl_timegraph->integer || cl_debugMove->integer ) {
		SCR_DrawDebugGraph ();
	}
}

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
void SCR_UpdateScreen( void ) {
	static int	recursive;

	if ( !scr_initialized ) {
		return;				// not initialized yet
	}

	if ( ++recursive > 2 ) {
		Com_Error( ERR_FATAL, "SCR_UpdateScreen: recursively called" );
	}
	recursive = 1;

	// if running in stereo, we need to draw the frame twice
	if ( cls.glconfig.stereoEnabled ) {
		SCR_DrawScreenField( STEREO_LEFT );
		SCR_DrawScreenField( STEREO_RIGHT );
	} else {
		SCR_DrawScreenField( STEREO_CENTER );
	}

	if ( com_speeds->integer ) {
		re.EndFrame( &time_frontend, &time_backend );
	} else {
		re.EndFrame( NULL, NULL );
	}

	recursive = 0;
}

