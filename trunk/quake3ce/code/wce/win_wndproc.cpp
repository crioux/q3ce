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

#include "../client/client.h"
#include "win_local.h"

WinVars_t	g_wv;

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported by the OS 
#endif

static UINT MSH_MOUSEWHEEL;


#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

LONG WINAPI MainWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

extern bool g_bShuttingDown;


/*
==================
VID_AppActivate
==================
*/
static void VID_AppActivate(BOOL fActive, BOOL minimize)
{
	g_wv.isMinimized = minimize;

	Com_DPrintf("VID_AppActivate: %i\n", fActive );

	Key_ClearStates();	// FIXME!!!

	// we don't want to act like we're active if we're minimized
	if (fActive && !g_wv.isMinimized )
	{
		g_wv.activeApp = qtrue;
	}
	else
	{
		g_wv.activeApp = qfalse;
	}

	// minimize/restore mouse-capture on demand
	if (!g_wv.activeApp )
	{
		IN_Activate (qfalse);
	}
	else
	{
		IN_Activate (qtrue);
	}
}


/*
=======
MapKey

Map from windows to quake keynums
=======
*/
static int MapKey (int key)
{
	if(key==0x10 || key==0x5B || key==0x84 || key==0x86)
	{
		return -1;
	}

	switch ( key )
	{
	case VK_UP:
		return K_UPARROW;
	case VK_LEFT:
		return K_LEFTARROW;
	case VK_RIGHT:
		return K_RIGHTARROW;
	case VK_DOWN:
		return K_DOWNARROW;
	case VK_HOME:
		return K_HOME;
	case VK_END:
		return K_END;
	case VK_PRIOR:
		return K_PGUP;
	case VK_NEXT:
		return K_PGDN;
	case VK_INSERT:
		return K_INS;
	case VK_DELETE:
		return K_DEL;
	case VK_F1:
		return K_F1;
	case VK_F2:
		return K_F2;
	case VK_F3:
		return K_F3;
	case VK_F4:
		return K_F4;
	case VK_F5:
		return K_F5;
	case VK_F6:
		return K_F6;
	case VK_F7:
		return K_F7;
	case VK_F8:
		return K_F8;
	case VK_F9:
		return K_F9;
	case VK_F10:
		return K_F10;

#ifdef TARGET_AXIMX50V
	case 0xC1:
		return K_AUX1;
	case 0xC2:
		return K_AUX2;
	case 0xC3:
		return K_AUX3;
	case 0xC4:
		return K_AUX4;
	case 0xC5:
		return K_AUX5;
	case 0xC6:
		return K_ESCAPE;
	case 0xC7:
		return K_AUX7;
	case 0xC8:
		return K_AUX8;
	case 0xC9:
		return K_AUX9;
#else
	case 0xC1:
		return K_ESCAPE;
	case 0xC2:
		return K_AUX2;
	case 0xC3:
		return K_AUX3;
	case 0xC4:
		return K_AUX4;
	case 0xC5:
		return K_AUX5;
	case 0xC6:
		return K_AUX6;
	case 0xC7:
		return K_AUX7;
	case 0xC8:
		return K_AUX8;
	case 0xC9:
		return K_AUX9;
#endif

	}

	if(key>=0x80)
	{
		return -1;
	}

	return key;
}


/*
====================
MainWndProc

main window procedure
====================
*/
LONG WINAPI MainWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
	int reskey;

	switch (uMsg)
	{
	case WM_CREATE:

		g_wv.hWnd = hWnd;
		break;
	case WM_DESTROY:
		// let sound and input know about this?
		g_wv.hWnd = NULL;
		break;
	
	case WM_HIBERNATE:
		break;

	case WM_CLOSE:
		//Cbuf_ExecuteText( EXEC_APPEND, "quit" );
		//Sys_Quit();
		break;

	case WM_ACTIVATE:
		{
			int	fActive, fMinimized;

			fActive = LOWORD(wParam);
			fMinimized = (BOOL) HIWORD(wParam);

			if(!fActive || fMinimized )
			{
				if(!g_bShuttingDown)
				{
					Sys_Quit();
				}
			}
		}
		break;

	case WM_MOVE:
		{
		}
		break;

// this is complicated because Win32 seems to pack multiple mouse events into
// one update sometimes, so we always check all states and look for events
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		{
			int	temp, xPos, yPos;

			temp = 0;

			if (wParam & MK_LBUTTON)
				temp |= 1;

			if (wParam & MK_RBUTTON)
				temp |= 2;

			if (wParam & MK_MBUTTON)
				temp |= 4;

			xPos = LOWORD(lParam); 
			yPos = HIWORD(lParam);

			IN_MouseEvent (temp, xPos, yPos);
		}
		break;

	case WM_SYSCOMMAND:
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		reskey=MapKey( wParam );
		if(reskey>0)
		{
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, reskey, qtrue, 0, NULL );
		}
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP:
		reskey=MapKey( wParam );
		if(reskey>0)
		{
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, reskey, qfalse, 0, NULL );
		}
		break;

	case WM_CHAR:
		Sys_QueEvent( g_wv.sysMsgTime, SE_CHAR, wParam, 0, 0, NULL );
		break;
   }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

