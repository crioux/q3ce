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
// win_syscon.h
#include "../client/client.h"
#include "win_local.h"
#include "../wce/res/resource.h"
#include <wingdi.h>
#include "../qcommon/fixed.h"
#include <stdio.h>

#define COPY_ID			1
#define QUIT_ID			2
#define CLEAR_ID		3

#define ERRORBOX_ID		10
#define ERRORTEXT_ID	11

#define EDIT_ID			100
#define INPUT_ID		101

typedef struct
{
	HWND		hWnd;
	HWND		hwndBuffer;

	HWND		hwndButtonClear;
	HWND		hwndButtonCopy;
	HWND		hwndButtonQuit;

	HWND		hwndErrorBox;
	HWND		hwndErrorText;

	HBITMAP		hbmLogo;
	HBITMAP		hbmClearBitmap;

	HBRUSH		hbrEditBackground;
	HBRUSH		hbrErrorBackground;

	HFONT		hfBufferFont;
	HFONT		hfButtonFont;

	HWND		hwndInputLine;

	char		errorString[80];

	char consoleText[512], returnedText[512];
	int			visLevel;
	qboolean	quitOnClose;
	int			windowWidth, windowHeight;
	
	WNDPROC		SysInputLineWndProc;

} WinConData;

static WinConData s_wcd;

static LONG WINAPI ConWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char *cmdString;
	static qboolean s_timePolarity;

	switch (uMsg)
	{
	case WM_ACTIVATE:
		if ( LOWORD( wParam ) != WA_INACTIVE )
		{
			SetFocus( s_wcd.hwndInputLine );
		}

		if ( com_viewlog && ( com_dedicated && !com_dedicated->integer ) )
		{
			// if the viewlog is open, check to see if it's being minimized
			if ( com_viewlog->integer == 1 )
			{
				if ( HIWORD( wParam ) )		// minimized flag
				{
					Cvar_Set( "viewlog", "2" );
				}
			}
			else if ( com_viewlog->integer == 2 )
			{
				if ( !HIWORD( wParam ) )		// minimized flag
				{
					Cvar_Set( "viewlog", "1" );
				}
			}
		}
		break;

	case WM_CLOSE:
		if ( ( com_dedicated && com_dedicated->integer ) )
		{
			cmdString = CopyString( "quit" );
			Sys_QueEvent( 0, SE_CONSOLE, 0, 0, strlen( cmdString ) + 1, cmdString );
		}
		else if ( s_wcd.quitOnClose )
		{
			PostQuitMessage( 0 );
		}
		else
		{
			Sys_ShowConsole( 0, qfalse );
			Cvar_Set( "viewlog", "0" );
		}
		return 0;
	case WM_CTLCOLORSTATIC:
		if ( ( HWND ) lParam == s_wcd.hwndBuffer )
		{
			SetBkColor( ( HDC ) wParam, RGB( 0x00, 0x00, 0xB0 ) );
			SetTextColor( ( HDC ) wParam, RGB( 0xff, 0xff, 0x00 ) );
			return ( long ) s_wcd.hbrEditBackground;
		}
		else if ( ( HWND ) lParam == s_wcd.hwndErrorBox )
		{
			SetBkColor( ( HDC ) wParam, RGB( 0x80, 0x80, 0x80 ) );
			SetTextColor( ( HDC ) wParam, RGB( 0xff, 0x0, 0x00 ) );
			return ( long ) s_wcd.hbrErrorBackground;
		}
		break;

	case WM_COMMAND:
		if ( wParam == COPY_ID )
		{
			SendMessage( s_wcd.hwndBuffer, EM_SETSEL, 0, -1 );
			SendMessage( s_wcd.hwndBuffer, WM_COPY, 0, 0 );
		}
		else if ( wParam == QUIT_ID )
		{
			if ( s_wcd.quitOnClose )
			{
				PostQuitMessage( 0 );
			}
			else
			{
				cmdString = CopyString( "quit" );
				Sys_QueEvent( 0, SE_CONSOLE, 0, 0, strlen( cmdString ) + 1, cmdString );
			}
		}
		else if ( wParam == CLEAR_ID )
		{
			SendMessage( s_wcd.hwndBuffer, EM_SETSEL, 0, -1 );
			SendMessage( s_wcd.hwndBuffer, EM_REPLACESEL, FALSE, ( LPARAM ) "" );
			UpdateWindow( s_wcd.hwndBuffer );
		}
		break;
	case WM_CREATE:
		s_wcd.hbrEditBackground = CreateSolidBrush( RGB( 0x00, 0x00, 0xB0 ) );
		s_wcd.hbrErrorBackground = CreateSolidBrush( RGB( 0x80, 0x80, 0x80 ) );
		break;
	case WM_ERASEBKGND:
	    return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

LONG WINAPI InputLineWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR inputBuffer[1024];
	char inputBuffer2[1024];

	switch ( uMsg )
	{
	case WM_KILLFOCUS:
		if ( ( HWND ) wParam == s_wcd.hWnd ||
			 ( HWND ) wParam == s_wcd.hwndErrorBox )
		{
			SetFocus( hWnd );
			return 0;
		}
		break;

	case WM_CHAR:
		if ( wParam == 13 )
		{
			GetWindowText( s_wcd.hwndInputLine, inputBuffer, sizeof( inputBuffer ) );
			_snprintf(inputBuffer2,1024,"%S",inputBuffer);

			strncpy( s_wcd.consoleText, inputBuffer2, sizeof( s_wcd.consoleText ) - strlen( s_wcd.consoleText ) - 5 );
			strcat( s_wcd.consoleText, "\n" );

			SetWindowText( s_wcd.hwndInputLine, L"" );

			Sys_Print( va( "]%S\n", inputBuffer2 ) );

			return 0;
		}
	}

	return CallWindowProc( s_wcd.SysInputLineWndProc, hWnd, uMsg, wParam, lParam );
}

DWORD ManageConsole(LPVOID pv)
{
	HWND hw=s_wcd.hWnd;
	MSG msg;
	while(GetMessage(&msg,hw,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}



/*
** Sys_CreateConsole
*/
void Sys_CreateConsole( void )
{
	HDC hDC;
	WNDCLASS wc;
	const TCHAR *DEDCLASS = L"Q3 CEConsole";
	int nHeight;
	int swidth, sheight, cwidth, cheight;
	RECT cr;
	LOGFONT lf;
	int DEDSTYLE = WS_CAPTION | WS_MINIMIZEBOX;
	
	hDC = GetDC( GetDesktopWindow() );
	swidth = GetDeviceCaps( hDC, HORZRES );
	sheight = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( GetDesktopWindow(), hDC );

	int lh1=(int)(sheight*0.075); //24 on 320
	int lh2=(int)(sheight*0.0625); //20 on 320

	
	memset( &wc, 0, sizeof( wc ) );

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC) ConWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = g_wv.hInstance;
	wc.hIcon         = LoadIcon( g_wv.hInstance, MAKEINTRESOURCE(IDI_Q3CE));
	wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = DEDCLASS;

	if ( !RegisterClass (&wc) )
		return;

	s_wcd.windowWidth = swidth;
	s_wcd.windowHeight = sheight;

	s_wcd.hWnd = CreateWindowEx( 0,
							   DEDCLASS,
							   L"Quake 3 Console",
							   DEDSTYLE,
							   0, 0, swidth, sheight,
							   NULL,
							   NULL,
							   (HINSTANCE)GetModuleHandle(NULL),
							   NULL );

	if ( s_wcd.hWnd == NULL )
	{
		return;
	}

	GetClientRect(s_wcd.hWnd,&cr);
	cwidth=cr.right-cr.left+1;
	cheight=cr.bottom-cr.top+1;

	//
	// create fonts
	//
	hDC = GetDC( s_wcd.hWnd );
	nHeight = -8 * GetDeviceCaps( hDC, LOGPIXELSY) / 72;

	
	memset(&lf,0,sizeof(LOGFONT));
	lf.lfHeight=nHeight;
	lf.lfPitchAndFamily=FF_MODERN | FIXED_PITCH;
	lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
	lf.lfQuality=DEFAULT_QUALITY;
	lf.lfWeight=FW_LIGHT;
	lf.lfCharSet=DEFAULT_CHARSET;
	lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
	_tcscpy(lf.lfFaceName,L"Courier New");
	s_wcd.hfBufferFont = CreateFontIndirect(&lf);


	ReleaseDC( s_wcd.hWnd, hDC );

	//
	// create the input line
	//
	s_wcd.hwndInputLine = CreateWindow( L"edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | 
												ES_LEFT | ES_AUTOHSCROLL,
												0, cheight-lh2, cwidth, lh2,
												s_wcd.hWnd, 
												( HMENU ) INPUT_ID,	// child window ID
												g_wv.hInstance, NULL );

	//
	// create the buttons
	//
	s_wcd.hwndButtonCopy = CreateWindow( L"button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
												0, cheight-lh2-lh1, cwidth/3, lh1,
												s_wcd.hWnd, 
												( HMENU ) COPY_ID,	// child window ID
												g_wv.hInstance, NULL );
	SendMessage( s_wcd.hwndButtonCopy, WM_SETTEXT, 0, ( LPARAM ) L"copy" );

	s_wcd.hwndButtonClear = CreateWindow( L"button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
												cwidth/3, cheight-lh2-lh1, cwidth/3, lh1,
												s_wcd.hWnd, 
												( HMENU ) CLEAR_ID,	// child window ID
												g_wv.hInstance, NULL );
	SendMessage( s_wcd.hwndButtonClear, WM_SETTEXT, 0, ( LPARAM ) L"clear" );

	s_wcd.hwndButtonQuit = CreateWindow( L"button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
												(cwidth/3)*2, cheight-lh2-lh1, cwidth/3+1, lh1,
												s_wcd.hWnd, 
												( HMENU ) QUIT_ID,	// child window ID
												g_wv.hInstance, NULL );
	SendMessage( s_wcd.hwndButtonQuit, WM_SETTEXT, 0, ( LPARAM ) L"quit" );


	//
	// create the scrollbuffer
	//
	s_wcd.hwndBuffer = CreateWindow( L"edit", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | 
												ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
												0, 0, cwidth, cheight-lh2-lh1,
												s_wcd.hWnd, 
												( HMENU ) EDIT_ID,	// child window ID
												g_wv.hInstance, NULL );
	SendMessage( s_wcd.hwndBuffer, WM_SETFONT, ( WPARAM ) s_wcd.hfBufferFont, 0 );

	s_wcd.SysInputLineWndProc = ( WNDPROC ) SetWindowLong( s_wcd.hwndInputLine, GWL_WNDPROC, ( long ) InputLineWndProc );
	//SendMessage( s_wcd.hwndInputLine, WM_SETFONT, ( WPARAM ) s_wcd.hfBufferFont, 0 );

	ShowWindow( s_wcd.hWnd, SW_SHOW);
	UpdateWindow( s_wcd.hWnd );
	SetForegroundWindow( s_wcd.hWnd );
	SetFocus( s_wcd.hwndInputLine );

	DWORD dwId;
	HANDLE ht=CreateThread(NULL,0,ManageConsole,NULL,0,&dwId);
	CloseHandle(ht);


	s_wcd.visLevel = 1;
}

/*
** Sys_DestroyConsole
*/
void Sys_DestroyConsole( void ) {
	if ( s_wcd.hWnd ) {
		ShowWindow( s_wcd.hWnd, SW_HIDE );
	//	CloseWindow( s_wcd.hWnd );
		DestroyWindow( s_wcd.hWnd );
		s_wcd.hWnd = 0;
	}
}

/*
** Sys_ShowConsole
*/
void Sys_ShowConsole( int visLevel, qboolean quitOnClose )
{
	s_wcd.quitOnClose = quitOnClose;

	if ( visLevel == s_wcd.visLevel )
	{
		return;
	}

	s_wcd.visLevel = visLevel;

	if ( !s_wcd.hWnd )
		return;

	switch ( visLevel )
	{
	case 0:
		ShowWindow( s_wcd.hWnd, SW_HIDE );
		break;
	case 1:
		ShowWindow( s_wcd.hWnd, SW_SHOWNORMAL );
		SendMessage( s_wcd.hwndBuffer, EM_LINESCROLL, 0, 0xffff );
		break;
	case 2:
		ShowWindow( s_wcd.hWnd, SW_MINIMIZE );
		break;
	default:
		Sys_Error( "Invalid visLevel %d sent to Sys_ShowConsole\n", visLevel );
		break;
	}
}

/*
** Sys_ConsoleInput
*/
char *Sys_ConsoleInput( void )
{
	static char buf[2048];

	if ( s_wcd.consoleText[0] == 0 )
	{
		return NULL;
	}
		
	strcpy( s_wcd.returnedText, s_wcd.consoleText );
	s_wcd.consoleText[0] = 0;
	
	
	return buf;
}

/*
** Conbuf_AppendText
*/
void Conbuf_AppendText( const char *pMsg )
{
#define CONSOLE_BUFFER_SIZE 2048
	TCHAR buffer[CONSOLE_BUFFER_SIZE*2];

	TCHAR *b = buffer;
	const char *msg;
	int bufLen;
	int i = 0;
	static unsigned long s_totalChars;

	//
	// if the message is REALLY long, use just the last portion of it
	//
	if ( strlen( pMsg ) > CONSOLE_BUFFER_SIZE - 1 )
	{
		msg = pMsg + strlen( pMsg ) - CONSOLE_BUFFER_SIZE + 1;
	}
	else
	{
		msg = pMsg;
	}

	//
	// copy into an intermediate buffer
	//
	while ( msg[i] && ( ( b - buffer ) < sizeof( buffer ) - 1 ) )
	{
		if ( msg[i] == '\n' && msg[i+1] == '\r' )
		{
			b[0] = L'\r';
			b[1] = L'\n';
			b += 2;
			i++;
		}
		else if ( msg[i] == '\r' )
		{
			b[0] = L'\r';
			b[1] = L'\n';
			b += 2;
		}
		else if ( msg[i] == '\n' )
		{
			b[0] = L'\r';
			b[1] = L'\n';
			b += 2;
		}
		else if ( Q_IsColorString( &msg[i] ) )
		{
			i++;
		}
		else
		{
			*b= (TCHAR)(msg[i]);
			b++;
		}
		i++;
	}
	*b = 0;
	bufLen = b - buffer;

	s_totalChars += bufLen;

	//
	// replace selection instead of appending if we're overflowing
	//
	if ( s_totalChars > 0x7fff )
	{
		SendMessage( s_wcd.hwndBuffer, EM_SETSEL, 0, -1 );
		s_totalChars = bufLen;
	}

	//
	// put this text into the windows console
	//
	SendMessage( s_wcd.hwndBuffer, EM_LINESCROLL, 0, 0xffff );
	SendMessage( s_wcd.hwndBuffer, EM_SCROLLCARET, 0, 0 );
	SendMessage( s_wcd.hwndBuffer, EM_REPLACESEL, 0, (LPARAM) buffer );
}

/*
** Sys_SetErrorText
*/
void Sys_SetErrorText( const char *buf )
{
	TCHAR blah[1024];
	_snprintf(s_wcd.errorString,sizeof( s_wcd.errorString ),"%s",buf);
	
	if ( !s_wcd.hwndErrorBox )
	{
		s_wcd.hwndErrorBox = CreateWindow( L"static", NULL, WS_CHILD | WS_VISIBLE,
													6, 5, 526, 30,
													s_wcd.hWnd, 
													( HMENU ) ERRORBOX_ID,	// child window ID
													g_wv.hInstance, NULL );
		SendMessage( s_wcd.hwndErrorBox, WM_SETFONT, ( WPARAM ) s_wcd.hfBufferFont, 0 );

		_snwprintf(blah,1024,L"%S",s_wcd.errorString);
		SetWindowText( s_wcd.hwndErrorBox, blah);

		DestroyWindow( s_wcd.hwndInputLine );
		s_wcd.hwndInputLine = NULL;
	}
}
