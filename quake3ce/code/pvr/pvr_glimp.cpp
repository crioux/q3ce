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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "tr_local.h"
#include "client.h"
#include "pvr_local.h"
#include "fixed.h"
#include "unix_glw.h"
#include<PVRShell.h>
#include<PVRShellOS.h>
#include<PVRTools.h>
#include<queue>
#include"q3pvr.h"

#define	WINDOW_CLASS_NAME	"Quake III: Arena"

typedef enum
{
  RSERR_OK,
  RSERR_INVALID_FULLSCREEN,
  RSERR_INVALID_MODE,
  RSERR_UNKNOWN
} rserr_t;

glwstate_t glw_state;


#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | \
		    PointerMotionMask | ButtonMotionMask )
#define X_MASK (KEY_MASK | MOUSE_MASK | VisibilityChangeMask | StructureNotifyMask )

static qboolean mouse_avail;
static qboolean mouse_active = qfalse;
static int mwx, mwy;
static int mx = 0, my = 0;

// Time mouse was reset, we ignore the first 50ms of the mouse to allow settling of events
static int mouseResetTime = 0;
#define MOUSE_RESET_DELAY 50

static cvar_t *in_mouse;
cvar_t *in_subframe;
cvar_t *in_nograb; // this is strictly for developers

// bk001130 - from cvs1.17 (mkv), but not static
cvar_t   *in_joystick      = NULL;
cvar_t   *in_joystickDebug = NULL;
cvar_t   *joy_threshold    = NULL;

cvar_t  *r_allowSoftwareGL;   // don't abort out if the pixelformat claims software
cvar_t  *r_previousglDriver;

qboolean vidmode_ext = qfalse;
static int vidmode_MajorVersion = 0, vidmode_MinorVersion = 0; // major and minor of XF86VidExtensions

// gamma value of the X display before we start playing with it
//static XF86VidModeGamma vidmode_InitialGamma;

static int win_x, win_y;

//static XF86VidModeModeInfo **vidmodes;
//static int default_dotclock_vidmode; // bk001204 - unused
static int num_vidmodes;
static qboolean vidmode_active = qfalse;

static int mouse_accel_numerator;
static int mouse_accel_denominator;
static int mouse_threshold;

// Keyboard mappings for pvrshell

/*
char g_LastKeyMap[16];    
const int KeyMapTable[16]={
	K_UPARROW,
	K_DOWNARROW,
	K_LEFTARROW,
	K_RIGHTARROW,
	K_ENTER,
	K_SPACE,
	K_PGDN,
	K_PGUP,
	K_ESCAPE,
	0,0,0,0,0,0,0 };
*/

std::queue<Q3Event> g_EventQueue;


/*
* Find the first occurrence of find in s.
*/
// bk001130 - from cvs1.17 (mkv), const
// bk001130 - made first argument const
static const char *Q_stristr( const char *s, const char *find)
{
  register char c, sc;
  register size_t len;

  if ((c = *find++) != 0)
  {
    if (c >= 'a' && c <= 'z')
    {
      c -= ('a' - 'A');
    }
    len = strlen(find);
    do
    {
      do
      {
        if ((sc = *s++) == 0)
          return NULL;
        if (sc >= 'a' && sc <= 'z')
        {
          sc -= ('a' - 'A');
        }
      } while (sc != c);
    } while (Q_stricmpn(s, find, len) != 0);
    s--;
  }
  return s;
}

/*
void ScanKeyMap(char keymap[])
{
	keymap[0]=g_pQ3PVR->PVRShellIsKeyPressed(PVRShellKeyNameUP)?1:0;
	keymap[1]=g_pQ3PVR->PVRShellIsKeyPressed(PVRShellKeyNameDOWN)?1:0;
	keymap[2]=g_pQ3PVR->PVRShellIsKeyPressed(PVRShellKeyNameLEFT)?1:0;
	keymap[3]=g_pQ3PVR->PVRShellIsKeyPressed(PVRShellKeyNameRIGHT)?1:0;
	keymap[4]=g_pQ3PVR->PVRShellIsKeyPressed(PVRShellKeyNameSELECT)?1:0;	// enter/fire
	keymap[5]=g_pQ3PVR->PVRShellIsKeyPressed(PVRShellKeyNameACTION1)?1:0;	// space/jump
	keymap[6]=g_pQ3PVR->PVRShellIsKeyPressed(PVRShellKeyNameACTION2)?1:0;	// pgdn/switch weapon
	keymap[8]=g_pQ3PVR->PVRShellIsKeyPressed(PVRShellKeyNameQUIT)?1:0;	// esc/cancel
}	
*/



extern int Sys_XTimeToSysTime (unsigned long xtime);



static void XLateKey(KeySym keysym, int *key)

{
  *key = 0;
  switch (keysym)
  {
  case XK_KP_Page_Up: 
  case XK_KP_9:  *key = K_KP_PGUP; break;
  case XK_Page_Up:   *key = K_PGUP; break;
  case XK_KP_Page_Down: 
  case XK_KP_3: *key = K_KP_PGDN; break;
  case XK_Page_Down:   *key = K_PGDN; break;
  case XK_KP_Home: *key = K_KP_HOME; break;
  case XK_KP_7: *key = K_KP_HOME; break;
  case XK_Home:  *key = K_HOME; break;
  case XK_KP_End:
  case XK_KP_1:   *key = K_KP_END; break;
  case XK_End:   *key = K_END; break;
  case XK_KP_Left: *key = K_KP_LEFTARROW; break;
  case XK_KP_4: *key = K_KP_LEFTARROW; break;
  case XK_Left:  *key = K_LEFTARROW; break;
  case XK_KP_Right: *key = K_KP_RIGHTARROW; break;
  case XK_KP_6: *key = K_KP_RIGHTARROW; break;
  case XK_Right:  *key = K_RIGHTARROW;    break;
  case XK_KP_Down:
  case XK_KP_2:    *key = K_KP_DOWNARROW; break;
  case XK_Down:  *key = K_DOWNARROW; break;
  case XK_KP_Up:   
  case XK_KP_8:    *key = K_KP_UPARROW; break;
  case XK_Up:    *key = K_UPARROW;   break;
  case XK_Escape: *key = K_ESCAPE;    break;
  case XK_KP_Enter: *key = K_KP_ENTER;  break;
  case XK_Return: *key = K_ENTER;    break;
  case XK_Tab:    *key = K_TAB;      break;
  case XK_F1:    *key = K_F1;       break;
  case XK_F2:    *key = K_F2;       break;
  case XK_F3:    *key = K_F3;       break;
  case XK_F4:    *key = K_F4;       break;
  case XK_F5:    *key = K_F5;       break;
  case XK_F6:    *key = K_F6;       break;
  case XK_F7:    *key = K_F7;       break;
  case XK_F8:    *key = K_F8;       break;
  case XK_F9:    *key = K_F9;       break;
  case XK_F10:    *key = K_F10;      break;
  case XK_F11:    *key = K_F11;      break;
  case XK_F12:    *key = K_F12;      break;
    // bk001206 - from Ryan's Fakk2 
    //case XK_BackSpace: *key = 8; break; // ctrl-h
  case XK_BackSpace: *key = K_BACKSPACE; break; // ctrl-h
  case XK_KP_Delete:
  case XK_KP_Decimal: *key = K_KP_DEL; break;
  case XK_Delete: *key = K_DEL; break;
  case XK_Pause:  *key = K_PAUSE;    break;
  case XK_Shift_L:
  case XK_Shift_R:  *key = K_SHIFT;   break;
  case XK_Execute: 
  case XK_Control_L: 
  case XK_Control_R:  *key = K_CTRL;  break;
  case XK_Alt_L:  
  case XK_Meta_L: 
  case XK_Alt_R:  
  case XK_Meta_R: *key = K_ALT;     break;
  case XK_KP_Begin: *key = K_KP_5;  break;
  case XK_Insert:   *key = K_INS; break;
  case XK_KP_Insert:
  case XK_KP_0: *key = K_KP_INS; break;
  case XK_KP_Multiply: *key = '*'; break;
  case XK_KP_Add:  *key = K_KP_PLUS; break;
  case XK_KP_Subtract: *key = K_KP_MINUS; break;
  case XK_KP_Divide: *key = K_KP_SLASH; break;
  // bk001130 - from cvs1.17 (mkv)
  case XK_exclam: *key = '1'; break;
  case XK_at: *key = '2'; break;
  case XK_numbersign: *key = '3'; break;
  case XK_dollar: *key = '4'; break;
  case XK_percent: *key = '5'; break;
  case XK_asciicircum: *key = '6'; break;
  case XK_ampersand: *key = '7'; break;
  case XK_asterisk: *key = '8'; break;
  case XK_parenleft: *key = '9'; break;
  case XK_parenright: *key = '0'; break;

  // weird french keyboards ..
  // NOTE: console toggle is hardcoded in cl_keys.c, can't be unbound
  //   cleaner would be .. using hardware key codes instead of the key syms
  //   could also add a new K_KP_CONSOLE
  case XK_twosuperior: *key = '~'; break;
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=472
	case XK_space:
	case XK_KP_Space: *key = K_SPACE; break;

  default:

      *key = keysym;
      if (*key >= 'A' && *key <= 'Z')
        *key = *key - 'A' + 'a';
      // if ctrl is pressed, the keys are not between 'A' and 'Z', for instance ctrl-z == 26 ^Z ^C etc.
      // see https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=19
      else if (*key >= 1 && *key <= 26)
     	  *key = *key + 'a' - 1;
    break;

  } 


}





static void HandleEvents(void)
{
  int b;
  int key;
  qboolean dowarp = qfalse;
  char *p;
  int dx, dy;
  int t = 0; // default to 0 in case we don't set

	while(g_EventQueue.size()!=0)
	{
		Q3Event evt=g_EventQueue.front();
		g_EventQueue.pop();
		int t=Sys_XTimeToSysTime(evt.time);

		switch(evt.type)
		{
		case Q3EVT_KEYUP:
			XLateKey(evt.args[0], &key);
      			if (key)
      			{
				Sys_QueEvent( t, SE_KEY, key, qfalse, 0, NULL );
			}
			break;
		case Q3EVT_KEYDOWN:
			XLateKey(evt.args[0], &key);
      			if (key)
      			{
				Sys_QueEvent( t, SE_KEY, key, qtrue, 0, NULL );
			}
			break;
		case Q3EVT_KEYCHAR:
			Sys_QueEvent( t, SE_CHAR, evt.args[0], 0, 0, NULL );
			break;
		case Q3EVT_MOUSEMOVE:
			break;
		case Q3EVT_MOUSEUP:
			break;
		case Q3EVT_MOUSEDOWN:
			break;
		default:
			break;
		}
	}
/*	
  char newkeymap[16];
  ScanKeyMap(newkeymap);
  for(int i=0;i<16;i++)
  {
	if(((g_LastKeyMap[i]&1)==0) && ((newkeymap[i]&1)==1))
	{
        	Sys_QueEvent( t, SE_KEY, KeyMapTable[i], qtrue, 0, NULL );
	}
	else if(((g_LastKeyMap[i]&1)==1) && ((newkeymap[i]&1)==0))
	{
        	Sys_QueEvent( t, SE_KEY, KeyMapTable[i], qfalse, 0, NULL );
	}
  }
  memcpy(g_LastKeyMap,newkeymap,16);
*/

/*
  while (XPending(dpy))
  {
    XNextEvent(dpy, &event);
    switch (event.type)
    {
    case KeyPress:
			t = Sys_XTimeToSysTime(event.xkey.time);
      p = XLateKey(&event.xkey, &key);
      if (key)
      {
        Sys_QueEvent( t, SE_KEY, key, qtrue, 0, NULL );
      }
      if (p)
      {
        while (*p)
        {
          Sys_QueEvent( t, SE_CHAR, *p++, 0, 0, NULL );
        }
      }
      break;

    case KeyRelease:
			t = Sys_XTimeToSysTime(event.xkey.time);
      // bk001206 - handle key repeat w/o XAutRepatOn/Off
      //            also: not done if console/menu is active.
      // From Ryan's Fakk2.
      // see game/q_shared.h, KEYCATCH_* . 0 == in 3d game.  
      if (cls.keyCatchers == 0)
      {   // FIXME: KEYCATCH_NONE
        if (repeated_press(&event) == qtrue)
          continue;
      } // if
      XLateKey(&event.xkey, &key);

      Sys_QueEvent( t, SE_KEY, key, qfalse, 0, NULL );
      break;

    case MotionNotify:
			t = Sys_XTimeToSysTime(event.xkey.time);
      if (mouse_active)
      {
          // If it's a center motion, we've just returned from our warp
          if (event.xmotion.x == glConfig.vidWidth/2 &&
              event.xmotion.y == glConfig.vidHeight/2)
          {
            mwx = glConfig.vidWidth/2;
            mwy = glConfig.vidHeight/2;
            if (t - mouseResetTime > MOUSE_RESET_DELAY )
            {
              Sys_QueEvent( t, SE_MOUSE, mx, my, 0, NULL );
            }
            mx = my = 0;
            break;
          }

          dx = ((int)event.xmotion.x - mwx);
          dy = ((int)event.xmotion.y - mwy);
          if (abs(dx) > 1)
            mx += dx * 2;
          else
            mx += dx;
          if (abs(dy) > 1)
            my += dy * 2;
          else
            my += dy;

          mwx = event.xmotion.x;
          mwy = event.xmotion.y;
          dowarp = qtrue;
      }
      break;

    case ButtonPress:
		  t = Sys_XTimeToSysTime(event.xkey.time);
      if (event.xbutton.button == 4)
      {
        Sys_QueEvent( t, SE_KEY, K_MWHEELUP, qtrue, 0, NULL );
      } else if (event.xbutton.button == 5)
      {
        Sys_QueEvent( t, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL );
      } else
      {
        // NOTE TTimo there seems to be a weird mapping for K_MOUSE1 K_MOUSE2 K_MOUSE3 ..
        b=-1;
        if (event.xbutton.button == 1)
        {
          b = 0; // K_MOUSE1
        } else if (event.xbutton.button == 2)
        {
          b = 2; // K_MOUSE3
        } else if (event.xbutton.button == 3)
        {
          b = 1; // K_MOUSE2
        } else if (event.xbutton.button == 6)
        {
          b = 3; // K_MOUSE4
        } else if (event.xbutton.button == 7)
        {
          b = 4; // K_MOUSE5
        };

        Sys_QueEvent( t, SE_KEY, K_MOUSE1 + b, qtrue, 0, NULL );
      }
      break;

    case ButtonRelease:
		  t = Sys_XTimeToSysTime(event.xkey.time);
      if (event.xbutton.button == 4)
      {
        Sys_QueEvent( t, SE_KEY, K_MWHEELUP, qfalse, 0, NULL );
      } else if (event.xbutton.button == 5)
      {
        Sys_QueEvent( t, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL );
      } else
      {
        b=-1;
        if (event.xbutton.button == 1)
        {
          b = 0;
        } else if (event.xbutton.button == 2)
        {
          b = 2;
        } else if (event.xbutton.button == 3)
        {
          b = 1;
        } else if (event.xbutton.button == 6)
        {
          b = 3; // K_MOUSE4
        } else if (event.xbutton.button == 7)
        {
          b = 4; // K_MOUSE5
        };
        Sys_QueEvent( t, SE_KEY, K_MOUSE1 + b, qfalse, 0, NULL );
      }
      break;

    case CreateNotify :
      win_x = event.xcreatewindow.x;
      win_y = event.xcreatewindow.y;
      break;

    case ConfigureNotify :
      win_x = event.xconfigure.x;
      win_y = event.xconfigure.y;
      break;
    }
  }

  if (dowarp)
  {
    XWarpPointer(dpy,None,win,0,0,0,0, 
                 (glConfig.vidWidth/2),(glConfig.vidHeight/2));
  }
*/
}

// NOTE TTimo for the tty console input, we didn't rely on those .. 
//   it's not very surprising actually cause they are not used otherwise
void KBD_Init(void)
{
//	memset(g_LastKeyMap,0,256);
}

void KBD_Close(void)
{
}

void IN_ActivateMouse( void ) 
{
  if (!mouse_avail)
    return;

  if (!mouse_active)
  {
	mouse_active = qtrue;
  }
}

void IN_DeactivateMouse( void ) 
{
  if (!mouse_avail)
    return;

  if (mouse_active)
  {
	mouse_active = qfalse;
  }
}
/*****************************************************************************/

/*
** GLimp_SetGamma
**
** This routine should only be called if glConfig.deviceSupportsGamma is TRUE
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] )
{
  red;green;blue;
  return;
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void GLimp_Shutdown( void )
{
  IN_DeactivateMouse();

  vidmode_active = qfalse;
  
  memset( &glConfig, 0, sizeof( glConfig ) );
  memset( &glState, 0, sizeof( glState ) );
}

/*
** GLimp_LogComment
*/
void GLimp_LogComment( const char *comment ) 
{
  if ( glw_state.log_fp )
  {
    fprintf( glw_state.log_fp, "%s", comment );
  }
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

  // GL_S3_s3tc
  if ( Q_stristr( glConfig.extensions_string, "GL_S3_s3tc" ) )
  {
    if ( r_ext_compressed_textures->value!=LFIXED_0 )
    {
      glConfig.textureCompression = TC_S3TC;
      ri.Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
    } else
    {
      glConfig.textureCompression = TC_NONE;
      ri.Printf( PRINT_ALL, "...ignoring GL_S3_s3tc\n" );
    }
  } else
  {
    glConfig.textureCompression = TC_NONE;
    ri.Printf( PRINT_ALL, "...GL_S3_s3tc not found\n" );
  }

  // GL_EXT_texture_env_add
  glConfig.textureEnvAddAvailable = qfalse;
  if ( Q_stristr( glConfig.extensions_string, "EXT_texture_env_add" ) )
  {
    if ( r_ext_texture_env_add->integer )
    {
      glConfig.textureEnvAddAvailable = qtrue;
      ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
    } else
    {
      glConfig.textureEnvAddAvailable = qfalse;
      ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
    }
  } else
  {
    ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
  }  
}

/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  
*/
void GLimp_Init( void )
{
  qboolean success = qfalse;
  //char  buf[1024];
  cvar_t *lastValidRenderer = ri.Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );

  // guarded, as this is only relevant to SMP renderer thread
#ifdef SMP
  if (!XInitThreads())
  {
    Com_Printf("GLimp_Init() - XInitThreads() failed, disabling r_smp\n");
    ri.Cvar_Set( "r_smp", "0" );
  }
#endif

  r_allowSoftwareGL = ri.Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );

  r_previousglDriver = ri.Cvar_Get( "r_previousglDriver", "", CVAR_ROM );

  InitSig();

  // Hack here so that if the UI 
  if ( *r_previousglDriver->string )
  {
    // The UI changed it on us, hack it back
    // This means the renderer can't be changed on the fly
    ri.Cvar_Set( "r_glDriver", r_previousglDriver->string );
  }
  
	int rbits,gbits,bbits;
	glGetIntegerv(GL_RED_BITS,&rbits);
	glGetIntegerv(GL_GREEN_BITS,&gbits);
	glGetIntegerv(GL_BLUE_BITS,&bbits);

	glConfig.colorBits = rbits+gbits+bbits;
	glGetIntegerv(GL_DEPTH_BITS,&glConfig.depthBits);
	glGetIntegerv(GL_STENCIL_BITS,&glConfig.stencilBits);
	glConfig.displayFrequency = 60;
	glConfig.deviceSupportsGamma=qfalse;
	glConfig.isFullscreen = qfalse;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS,&glConfig.maxActiveTextures);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&glConfig.maxTextureSize);
	glConfig.smpActive=qfalse;
	glConfig.stereoEnabled=qfalse;
	glConfig.vidHeight=g_pvr_window_height;
	glConfig.vidWidth=g_pvr_window_width;
	glConfig.windowAspect=GFIXED_1;
	  
  // Save it in case the UI stomps it
  ri.Cvar_Set( "r_previousglDriver", r_glDriver->string );

  // This values force the UI to disable driver selection
  glConfig.driverType = GLDRV_ICD;
  glConfig.hardwareType = GLHW_GENERIC;

  // get our config strings
  Q_strncpyz( glConfig.vendor_string, (const char *)glGetString (GL_VENDOR), sizeof( glConfig.vendor_string ) );
  Q_strncpyz( glConfig.renderer_string, (const char *)glGetString (GL_RENDERER), sizeof( glConfig.renderer_string ) );
  if (*glConfig.renderer_string && glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] == '\n')
    glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] = 0;
  Q_strncpyz( glConfig.version_string, (const char *)glGetString (GL_VERSION), sizeof( glConfig.version_string ) );
  Q_strncpyz( glConfig.extensions_string, (const char *) glGetString (GL_EXTENSIONS), sizeof( glConfig.extensions_string ) );

  //
  // chipset specific configuration
  //
  //strcpy( buf, glConfig.renderer_string );
  //strlwr( buf );

  //
  // NOTE: if changing cvars, do it within this block.  This allows them
  // to be overridden when testing driver fixes, etc. but only sets
  // them to their default state when the hardware is first installed/run.
  //
  if ( Q_stricmp( lastValidRenderer->string, glConfig.renderer_string ) )
  {
    glConfig.hardwareType = GLHW_GENERIC;

    ri.Cvar_Set( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST" );
  }

  ri.Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );

  // initialize extensions
  GLW_InitExtensions();
  //GLW_InitGamma();

  InitSig(); // not clear why this is at begin & end of function

  return;
}


/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame (void)
{
}

#ifdef SMP
/*
===========================================================

SMP acceleration

===========================================================
*/

static pthread_mutex_t	smpMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t		renderCommandsEvent = PTHREAD_COND_INITIALIZER;
static pthread_cond_t		renderCompletedEvent = PTHREAD_COND_INITIALIZER;

static void (*glimpRenderThread)( void );

static void *GLimp_RenderThreadWrapper( void *arg )
{
	Com_Printf( "Render thread starting\n" );

	glimpRenderThread();

	qglXMakeCurrent( dpy, None, NULL );

	Com_Printf( "Render thread terminating\n" );

	return arg;
}

qboolean GLimp_SpawnRenderThread( void (*function)( void ) )
{
	pthread_t renderThread;
	int ret;

	pthread_mutex_init( &smpMutex, NULL );

	pthread_cond_init( &renderCommandsEvent, NULL );
	pthread_cond_init( &renderCompletedEvent, NULL );

  glimpRenderThread = function;

	ret = pthread_create( &renderThread,
						  NULL,			// attributes
						  GLimp_RenderThreadWrapper,
						  NULL );		// argument
	if ( ret ) {
		ri.Printf( PRINT_ALL, "pthread_create returned %d: %s", ret, strerror( ret ) );
    return qfalse;
	} else {
		ret = pthread_detach( renderThread );
		if ( ret ) {
			ri.Printf( PRINT_ALL, "pthread_detach returned %d: %s", ret, strerror( ret ) );
		}
  }

  return qtrue;
}

static volatile void    *smpData = NULL;
static volatile qboolean smpDataReady;

void *GLimp_RendererSleep( void )
{
	void  *data;

	qglXMakeCurrent( dpy, None, NULL );

	pthread_mutex_lock( &smpMutex );
	{
		smpData = NULL;
		smpDataReady = qfalse;

		// after this, the front end can exit GLimp_FrontEndSleep
		pthread_cond_signal( &renderCompletedEvent );

		while ( !smpDataReady ) {
			pthread_cond_wait( &renderCommandsEvent, &smpMutex );
		}

		data = (void *)smpData;
	}
	pthread_mutex_unlock( &smpMutex );

	qglXMakeCurrent( dpy, win, ctx );

  return data;
}

void GLimp_FrontEndSleep( void )
{
	pthread_mutex_lock( &smpMutex );
	{
		while ( smpData ) {
			pthread_cond_wait( &renderCompletedEvent, &smpMutex );
		}
	}
	pthread_mutex_unlock( &smpMutex );

	qglXMakeCurrent( dpy, win, ctx );
}

void GLimp_WakeRenderer( void *data )
{
	qglXMakeCurrent( dpy, None, NULL );

	pthread_mutex_lock( &smpMutex );
	{
		assert( smpData == NULL );
		smpData = data;
		smpDataReady = qtrue;

		// after this, the renderer can continue through GLimp_RendererSleep
		pthread_cond_signal( &renderCommandsEvent );
	}
	pthread_mutex_unlock( &smpMutex );
}

#else

void GLimp_RenderThreadWrapper( void *stub ) {}
qboolean GLimp_SpawnRenderThread( void (*function)( void ) ) {
	ri.Printf( PRINT_WARNING, "ERROR: SMP support was disabled at compile time\n");
  return qfalse;
}
void *GLimp_RendererSleep( void ) {
  return NULL;
}
void GLimp_FrontEndSleep( void ) {}
void GLimp_WakeRenderer( void *data ) {}

#endif

/*****************************************************************************/
/* MOUSE                                                                     */
/*****************************************************************************/

void IN_Init(void) {
	Com_Printf ("\n------- Input Initialization -------\n");
  // mouse variables
  in_mouse = Cvar_Get ("in_mouse", "1", CVAR_ARCHIVE);
  	
  // turn on-off sub-frame timing of X events
  in_subframe = Cvar_Get ("in_subframe", "1", CVAR_ARCHIVE);
	
// developer feature, allows to break without loosing mouse pointer
  in_nograb = Cvar_Get ("in_nograb", "0", 0);

  // bk001130 - from cvs.17 (mkv), joystick variables
  in_joystick = Cvar_Get ("in_joystick", "0", CVAR_ARCHIVE|CVAR_LATCH);
  // bk001130 - changed this to match win32
  in_joystickDebug = Cvar_Get ("in_debugjoystick", "0", CVAR_TEMP);
  joy_threshold = Cvar_Get ("joy_threshold", "0.15", CVAR_ARCHIVE); // FIXME: in_joythreshold

  if (in_mouse->value!=LFIXED_0)
    mouse_avail = qtrue;
  else
    mouse_avail = qfalse;

//  IN_StartupJoystick( ); // bk001130 - from cvs1.17 (mkv)
	Com_Printf ("------------------------------------\n");
}

void IN_Shutdown(void)
{
  mouse_avail = qfalse;
}

void IN_Frame (void) {

  // bk001130 - from cvs 1.17 (mkv)
//  IN_JoyMove(); // FIXME: disable if on desktop?

  if ( cls.keyCatchers & KEYCATCH_CONSOLE )
  {
    // temporarily deactivate if not in the game and
    // running on the desktop
    // voodoo always counts as full screen
    if (Cvar_VariableValue ("r_fullscreen") == LFIXED_0
        && strcmp( Cvar_VariableString("r_glDriver"), _3DFX_DRIVER_NAME ) )
    {
      IN_DeactivateMouse ();
      return;
    }
  }

  IN_ActivateMouse();
}

void IN_Activate(void)
{
}

// bk001130 - cvs1.17 joystick code (mkv) was here, no linux_joystick.c

void Sys_SendKeyEvents (void) {
  // XEvent event; // bk001204 - unused

  HandleEvents();
}


// bk010216 - added stubs for non-Linux UNIXes here
// FIXME - use NO_JOYSTICK or something else generic

#if defined( __FreeBSD__ ) // rb010123
void IN_StartupJoystick( void ) {}
void IN_JoyMove( void ) {}
#endif
