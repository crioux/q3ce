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
// win_input.c -- win32 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.

#include "../client/client.h"
#include "win_local.h"

qboolean	in_appactive;

extern int Q3CE_OpenInput();
extern int Q3CE_CloseInput();

int oldButtonState;
int	width, height;
int s_biasx;
int s_biasy;
int lastsmx=0;
int lastsmy=0;
gfixed s_scale;
	
/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent (int mstate, int mx, int my)
{
	int i;
	int smx,smy,dx,dy;
	int v1,v2;

	smx=FIXED_TO_INT(MAKE_GFIXED(mx-s_biasx)/s_scale);
	smy=FIXED_TO_INT(MAKE_GFIXED(my-s_biasy)/s_scale);

	if(smx>639) smx=639;
	if(smy>479) smy=479;
	if(smx<0) smx=0;
	if(smy<0) smy=0;

	// Allow stylus to pick up and put down somewhere else
	if((mstate & 1) && !(oldButtonState & 1))
	{
		lastsmx=smx;
		lastsmy=smy;
	}
	
	dx=smx-lastsmx;
	dy=smy-lastsmy;
	lastsmx=smx;
	lastsmy=smy;
 
	v1=(smy<<16)|(smx&0xFFFF);
	v2=(dy<<16)|(dx&0xFFFF);
	
	// perform mouse movement
	Sys_QueEvent( 0, SE_MOUSE, v1,v2, 0, NULL );

	// perform button actions
	for  (i = 0 ; i < 3 ; i++ )
	{
		if ( (mstate & (1<<i)) &&
			!(oldButtonState & (1<<i)) )
		{
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MOUSE1 + i, qtrue, 0, NULL );
		}

		if ( !(mstate & (1<<i)) &&
			(oldButtonState & (1<<i)) )
		{
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MOUSE1 + i, qfalse, 0, NULL );
		}
	}	

	oldButtonState = mstate;
}


/*
===========
IN_MouseMove
===========
*/
void IN_MouseMove ( void ) 
{

/*
	int		mx, my;

	IN_Win32Mouse( &mx, &my );
	
	if ( !mx && !my ) {
		return;
	}

	Sys_QueEvent( 0, SE_MOUSE, mx, my, 0, NULL );
*/
}


/*
=========================================================================

=========================================================================
*/

/*
===========
IN_Startup
===========
*/
void IN_Startup( void ) {
	Com_Printf ("\n-- Input Initialization --\n");
	
	if(Q3CE_OpenInput())
	{
		Com_Printf("Q3CE_OpenInput() succeeded.\n");
	}
	else
	{
		Com_Printf("Q3CE_OpenInput() failed!n");
	}
	
	Com_Printf ("--------------------------\n");
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown( void ) {
	Q3CE_CloseInput();
}


/*
===========
IN_Init
===========
*/
void IN_Init( void ) {
	IN_Startup();

	width = GetSystemMetrics (SM_CXSCREEN);
	height = GetSystemMetrics (SM_CYSCREEN);

	// for 640x480 virtualized screen
	if(height<width)
	{
		s_scale = MAKE_GFIXED(height) * (GFIXED_1/GFIXED(480,0));
	}
	else
	{
		s_scale = MAKE_GFIXED(width) * (GFIXED_1/GFIXED(640,0));
	}

	// no wide screen
	s_biasx = 0;
	s_biasy = 0;
	if ( width * 480 > height * 640 ) {
		// wide screen
		s_biasx = FIXED_TO_INT(GFIXED(0,5) * ( MAKE_GFIXED(width) - ( MAKE_GFIXED(height) * (GFIXED(640,0)/GFIXED(480,0)) ) ));
	}
	else if ( width * 480 < height * 640 ) {
		// long screen
		s_biasy = FIXED_TO_INT(GFIXED(0,5) * ( MAKE_GFIXED(height) - ( MAKE_GFIXED(width) * (GFIXED(480,0)/GFIXED(640,0)) ) ));
	}
	

}


/*
===========
IN_Activate

Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void IN_Activate (qboolean active) {
	in_appactive = active;
}


/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
void IN_Frame (void) {
}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void) 
{
	oldButtonState = 0;
}


/*
=========================================================================

JOYSTICK

=========================================================================
*/
