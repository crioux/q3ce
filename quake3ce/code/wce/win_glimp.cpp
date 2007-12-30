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
/*
** WIN_GLIMP.C
**
** This file contains ALL Win32 specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_LogComment
** GLimp_Shutdown
**
** Note that the GLW_xxx functions are Windows specific GL-subsystem
** related functions that are relevant ONLY to win_glimp.c
*/
#include <assert.h>
#include "../renderer/tr_local.h"
#include "../qcommon/qcommon.h"
#include "res/resource.h"

#include "gles/gl.h"
//#include "gles/egltypes.h"
#include "gles/egl.h"
#include "glw_win.h"
#include "win_local.h"


typedef enum {
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;

#define TRY_PFD_SUCCESS		0
#define TRY_PFD_FAIL_SOFT	1
#define TRY_PFD_FAIL_HARD	2

#define	WINDOW_CLASS_NAME	L"Quake 3: Arena"

static void		GLW_InitExtensions( void );
static qboolean s_classRegistered = qfalse;

extern int Q3CE_OpenDisplay(HWND hWnd);
extern int Q3CE_CloseDisplay();


//
// variable declarations
//
glwstate_t glw_state;

cvar_t	*r_allowSoftwareGL;		// don't abort out if the pixelformat claims software
cvar_t	*r_maskMinidriver;		// allow a different dll name to be treated as if it were opengl32.dll



#ifdef TARGET_AXIMX50V
extern void (*pClipPlanexIMG)( GLenum p, const GLfixed *eqn );
#endif

/*
** GLW_InitDriver
**
** - get a DC if one doesn't exist
** - create an HGLRC if one doesn't exist
*/
static qboolean GLW_InitDriver()
{
//	int rb,gb,bb;
	
	EGLConfig configs[10];
	EGLint matchingConfigs; 
	EGLint attribList12[] = { EGL_RED_SIZE,4,EGL_GREEN_SIZE,4,EGL_BLUE_SIZE,4, EGL_DEPTH_SIZE,16, EGL_NONE};
	EGLint attribList15[] = { EGL_RED_SIZE,5,EGL_GREEN_SIZE,5,EGL_BLUE_SIZE,5, EGL_DEPTH_SIZE,16, EGL_NONE};
	EGLint attribList16[] = { EGL_RED_SIZE,5,EGL_GREEN_SIZE,6,EGL_BLUE_SIZE,5, EGL_DEPTH_SIZE,16, EGL_NONE};
	EGLint attribList24[] = { EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8, EGL_DEPTH_SIZE,16, EGL_NONE};
	EGLint attribList32[] = { EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8, EGL_DEPTH_SIZE,16, EGL_NONE};
	EGLint noAttribList[] = { EGL_NONE };
	EGLint *pAttribList;
	EGLConfig config;

	ri.Printf( PRINT_ALL, "Initializing OpenGL ES driver\n" );

	glw_state.hDisp=eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(glw_state.hDisp,NULL,NULL);

	if(glw_state.desktopBitsPixel==12)
	{
		pAttribList=attribList12;
	}
	if(glw_state.desktopBitsPixel==15)
	{
		pAttribList=attribList15;
	}
	else if(glw_state.desktopBitsPixel==16)
	{
		pAttribList=attribList16;
	}
	else if(glw_state.desktopBitsPixel==24)
	{
		pAttribList=attribList24;
	}
	else if(glw_state.desktopBitsPixel==32)
	{
		pAttribList=attribList32;
	}
	else
	{
		ri.Printf(PRINT_ALL,"Couldn't find appropriate bit depth\n");
		return qfalse;
	}

    
	if(eglChooseConfig(glw_state.hDisp,pAttribList,&(configs[0]),10,&matchingConfigs)==EGL_FALSE)
	{
		return qfalse;
	}
	if (matchingConfigs < 1) {
		return qfalse;
	}

	config = configs[0];	// pick any

	glw_state.hSurface=eglCreateWindowSurface(glw_state.hDisp,config,g_wv.hWnd,NULL);
	if(glw_state.hSurface==NULL)
	{
		return qfalse;
	}

	glw_state.hContext=eglCreateContext(glw_state.hDisp,config,EGL_NO_CONTEXT,&(noAttribList[0]));
	if(glw_state.hContext==NULL)
	{
		return qfalse;
	}

	
	if(eglMakeCurrent(glw_state.hDisp,glw_state.hSurface,glw_state.hSurface,glw_state.hContext)!=EGL_TRUE)
	{	
		return qfalse;
	}

    /*
	** store PFD specifics 
	*/

//	glGetIntegerv(GL_RED_BITS,&rb);
//	glGetIntegerv(GL_GREEN_BITS,&gb);
//	glGetIntegerv(GL_BLUE_BITS,&bb);
	glConfig.colorBits = glw_state.desktopBitsPixel;

	glGetIntegerv(GL_DEPTH_BITS,&glConfig.depthBits);

	glGetIntegerv(GL_STENCIL_BITS,&glConfig.stencilBits);
	glConfig.displayFrequency = 60;
	glConfig.deviceSupportsGamma=qfalse;
	glConfig.isFullscreen = qtrue;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS,&glConfig.maxActiveTextures);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&glConfig.maxTextureSize);
	glConfig.smpActive=qfalse;
	glConfig.stereoEnabled=qfalse;
	glConfig.vidHeight=glw_state.desktopHeight;
	glConfig.vidWidth=glw_state.desktopWidth;
	glConfig.windowAspect=GFIXED_1;
	
	return qtrue;
}

/*
** GLW_CreateWindow
**
** Responsible for creating the Win32 window and initializing the OpenGL driver.
*/
#define	WINDOW_STYLE	(WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE)
static qboolean GLW_CreateWindow( int width, int height, int colorbits)
{
	int				stylebits;
	int				x, y, w, h;
	int				exstyle;

	//
	// register the window class if necessary
	//
	if ( !s_classRegistered )
	{
		WNDCLASS wc;

		memset( &wc, 0, sizeof( wc ) );

		wc.style         = 0;
		wc.lpfnWndProc   = (WNDPROC) glw_state.wndproc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = g_wv.hInstance;
		wc.hIcon         = LoadIcon( g_wv.hInstance, MAKEINTRESOURCE(IDI_Q3CE));
		wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
		wc.hbrBackground = CreateSolidBrush(RGB(0,0,0));
		wc.lpszMenuName  = 0;
		wc.lpszClassName = WINDOW_CLASS_NAME;

		if ( !RegisterClass( &wc ) )
		{
			ri.Error( ERR_FATAL, "GLW_CreateWindow: could not register window class" );
		}
		s_classRegistered = qtrue;
		ri.Printf( PRINT_ALL, "...registered window class\n" );
	}

	//
	// create the HWND if one does not already exist
	//
	if ( !g_wv.hWnd )
	{
		//
		// compute width and height
		//
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP|WS_VISIBLE|WS_SYSMENU;
	
		w = width;
		h = height;
		x = 0;
		y = 0;
		
		g_wv.hWnd = CreateWindowEx (
			 exstyle, 
			 WINDOW_CLASS_NAME,
			 L"Quake 3: Arena",
			 stylebits,
			 x, y, w, h,
			 NULL,
			 NULL,
			 g_wv.hInstance,
			 NULL);

		if ( !g_wv.hWnd )
		{
			ri.Error (ERR_FATAL, "GLW_CreateWindow() - Couldn't create window");
		}
	
		ShowWindow( g_wv.hWnd, SW_SHOW );
		UpdateWindow( g_wv.hWnd );
		ri.Printf( PRINT_ALL, "...created window@%d,%d (%dx%d)\n", x, y, w, h );
	}
	else
	{
		ri.Printf( PRINT_ALL, "...window already present, CreateWindowEx skipped\n" );
	}

	if ( !GLW_InitDriver() )
	{
		ShowWindow( g_wv.hWnd, SW_HIDE );
		DestroyWindow( g_wv.hWnd );
		g_wv.hWnd = NULL;

		return qfalse;
	}

	SetForegroundWindow( g_wv.hWnd );
	SetFocus( g_wv.hWnd );

	Q3CE_OpenDisplay(g_wv.hWnd);

	return qtrue;
}

/*
** GLW_InitExtensions
*/
static void GLW_InitExtensions( void )
{
	if ( !r_allowExtensions->integer )
	{
		ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

	glConfig.textureCompression = TC_NONE;
	glConfig.textureEnvAddAvailable = qfalse;
/*
#ifdef TARGET_AXIMX50V
	const GLubyte 		 *pszGLExtensions;

	// Retrieve GL extension string 
	pszGLExtensions = glGetString(GL_EXTENSIONS);

	// Test for presence of the extension
	if (strstr((const char *)pszGLExtensions, "GL_IMG_user_clip_plane"))
	{
		// Retrieve function address pointer
		pClipPlanexIMG = (TYPEOF_pClipPlanexIMG)eglGetProcAddress("glClipPlanexIMG");
	}
	
#endif
*/
}

/*
** GLimp_EndFrame
*/
void GLimp_EndFrame (void)
{
	//
	// swapinterval stuff
	//
	if ( r_swapInterval->modified ) {
		r_swapInterval->modified = qfalse;

	}

	// don't flip if drawing to front buffer
	if ( Q_stricmp( r_drawBuffer->string, "GL_FRONT" ) != 0 )
	{
		if ( glConfig.driverType > GLDRV_ICD )
		{
			if ( !eglSwapBuffers( glw_state.hDisp,glw_state.hSurface) )
			{
				ri.Error( ERR_FATAL, "GLimp_EndFrame() - SwapBuffers() failed!\n" );
			}
		}
		else
		{
			eglSwapBuffers( glw_state.hDisp,glw_state.hSurface) ;
		}
	}

}

static void GLW_StartOpenGLES( void )
{
	GLW_CreateWindow(glw_state.desktopWidth,glw_state.desktopHeight,glw_state.desktopBitsPixel);
	
	ri.Cvar_Set( "r_glDriver", OPENGL_DRIVER_NAME );
	r_glDriver->modified = qfalse;
}

/*
** GLimp_Init
**
** This is the platform specific OpenGL initialization function.  It
** is responsible for loading OpenGL, initializing it, setting
** extensions, creating a window of the appropriate size, doing
** fullscreen manipulations, etc.  Its overall responsibility is
** to make sure that a functional OpenGL subsystem is operating
** when it returns to the ref.
*/
void GLimp_Init( void )
{
	HDC hDC;

	cvar_t *lastValidRenderer = ri.Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );
	cvar_t	*cv;

	ri.Printf( PRINT_ALL, "Initializing OpenGL subsystem\n" );

	// save off hInstance and wndproc
	cv = ri.Cvar_Get( "win_hinstance", "", 0 );
	sscanf( cv->string, "%i", (int *)&g_wv.hInstance );

	cv = ri.Cvar_Get( "win_wndproc", "", 0 );
	sscanf( cv->string, "%i", (int *)&glw_state.wndproc );

	r_allowSoftwareGL = ri.Cvar_Get( "r_allowSoftwareGL", "1", CVAR_LATCH );
	r_maskMinidriver = ri.Cvar_Get( "r_maskMinidriver", "0", CVAR_LATCH );

	// Get default sizes
	hDC = GetDC( GetDesktopWindow() );
	glw_state.desktopBitsPixel = GetDeviceCaps( hDC, BITSPIXEL );
	glw_state.desktopWidth = GetDeviceCaps( hDC, HORZRES );
	glw_state.desktopHeight = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( GetDesktopWindow(), hDC );

	// load appropriate DLL and initialize subsystem
	GLW_StartOpenGLES();

	// get our config strings
	Q_strncpyz( glConfig.vendor_string, (const char *)glGetString(GL_VENDOR), sizeof( glConfig.vendor_string ) );
	Q_strncpyz( glConfig.renderer_string, (const char *)glGetString(GL_RENDERER), sizeof( glConfig.renderer_string ) );
	Q_strncpyz( glConfig.version_string, (const char *)glGetString(GL_VERSION), sizeof( glConfig.version_string ) );
	Q_strncpyz( glConfig.extensions_string, (const char *)glGetString(GL_EXTENSIONS), sizeof( glConfig.extensions_string ) );

	//
	// chipset specific configuration
	//
	glConfig.hardwareType = GLHW_GENERIC;
				
	ri.Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );

	GLW_InitExtensions();
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.
*/
void GLimp_Shutdown( void )
{
//	const char *strings[] = { "soft", "hard" };
	const char *success[] = { "failed", "success" };
	int retVal;

	ri.Printf( PRINT_ALL, "Shutting down OpenGL subsystem\n" );

	// set current context to NULL
	retVal = eglMakeCurrent(glw_state.hDisp,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT) != 0;
	ri.Printf( PRINT_ALL, "...eglMakeCurrent(glw_state.hDisp,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NOCONTEXT) : %s\n", success[retVal] );

	eglDestroySurface(glw_state.hDisp,glw_state.hSurface);

	// delete HGLRC
	if ( glw_state.hContext)
	{
		retVal = eglDestroyContext(  glw_state.hDisp, glw_state.hContext ) != 0;
		ri.Printf( PRINT_ALL, "...deleting GL context: %s\n", success[retVal] );
		glw_state.hContext= NULL;
	}


	// destroy window
	if ( g_wv.hWnd )
	{
		Q3CE_CloseDisplay();

		ri.Printf( PRINT_ALL, "...destroying window\n" );
		ShowWindow( g_wv.hWnd, SW_HIDE );
		DestroyWindow( g_wv.hWnd );
		g_wv.hWnd = NULL;
		glw_state.pixelFormatSet = qfalse;
	}

	// close the r_logFile
	if ( glw_state.log_fp )
	{
		fclose( glw_state.log_fp );
		glw_state.log_fp = 0;
	}

	// reset display settings
	if ( glw_state.cdsFullscreen )
	{
		ri.Printf( PRINT_ALL, "...resetting display\n" );
		glw_state.cdsFullscreen = qfalse;
	}

	
	memset( &glConfig, 0, sizeof( glConfig ) );
	memset( &glState, 0, sizeof( glState ) );
}

void GLimp_PauseDisplay(void)
{
	if(eglMakeCurrent(glw_state.hDisp,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT)!=EGL_TRUE)
	{	
		return;
	}
	ShowWindow(g_wv.hWnd,SW_HIDE);
}

void GLimp_ResumeDisplay(void)
{
	ShowWindow(g_wv.hWnd,SW_SHOW);
	if(eglMakeCurrent(glw_state.hDisp,glw_state.hSurface,glw_state.hSurface,glw_state.hContext)!=EGL_TRUE)
	{	
		return;
	}
}


/*
** GLimp_LogComment
*/
void GLimp_LogComment( char *comment ) 
{
	if ( glw_state.log_fp ) {
		fprintf( glw_state.log_fp, "%s", comment );
	}
}


