/******************************************************************************

 @File         PVRShellOS.cpp

 @Title        

 @Copyright    Copyright (C) 2004 - 2008 by Imagination Technologies Limited.

 @Platform     X11

 @Description  Makes programming for 3D APIs easier by wrapping window creation
               and other functions for use by a demo.

******************************************************************************/
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "PVRShell.h"
#include "PVRShellAPI.h"
#include "PVRShellOS.h"
#include "PVRShellImpl.h"

/*!***************************************************************************
	Defines
*****************************************************************************/

/*****************************************************************************
	Declarations
*****************************************************************************/
static Bool WaitForMapNotify( Display *d, XEvent *e, char *arg );

/*!***************************************************************************
	Class: PVRShellInit
*****************************************************************************/

/*!***********************************************************************
@Function		PVRShellOutputDebug
@Input			format			printf style format followed by arguments it requires
@Description	Writes the resultant string to the debug output (e.g. using
				printf(), OutputDebugString(), ...). Check the SDK release notes for
				details on how the string is output.
*************************************************************************/
void PVRShell::PVRShellOutputDebug(char const * const format, ...) const
{
	va_list arg;
	char	buf[1024];

	va_start(arg, format);
	vsnprintf(buf, 1024, format, arg);
	va_end(arg);

	// Passes the data to a platform dependant function
	m_pShellInit->OsDisplayDebugString(buf);
}

/*!***********************************************************************
 @Function		OsInit
 @description	Initialisation for OS-specific code.
*************************************************************************/
void PVRShellInit::OsInit()
{
	m_pShell->m_pShellData->bFullScreen=true;	// linux overrides default to use fullscreen

	m_pShell->m_pShellData->nShellDimX = 240;
	m_pShell->m_pShellData->nShellDimY = 320;

	x11display = NULL;
	x11visual = NULL;

	// Pixmap support: init variables to 0
	x11pixmap = BadValue;

	/*
		Construct the binary path for GetReadPath() and GetWritePath()
	*/
	// Get PID
	pid_t ourPid = getpid();
	char *pszExePath, pszSrcLink[64];
	int len = 64;
	int res;

	sprintf(pszSrcLink, "/proc/%d/exe", ourPid);
	pszExePath = 0;
	do
	{
		len *= 2;
		delete[] pszExePath;
		pszExePath = new char[len];
		res = readlink(pszSrcLink, pszExePath, len);
	} while((res < 0) || (res >= len));
	pszExePath[res] = '\0'; // Null-terminate readlink's result
	SetReadPath(pszExePath);
	SetWritePath(pszExePath);
	SetAppName(pszExePath);
	delete[] pszExePath;
}

/*!***********************************************************************
 @Function		OsInitOS
 @description	Saves instance handle and creates main window
				In this function, we save the instance handle in a global variable and
				create and display the main program window.
*************************************************************************/
bool PVRShellInit::OsInitOS()
{
	// Initialize global strings
	szTitle = "PVRShell";

	x11display = XOpenDisplay( NULL );

	if( !x11display )
	{
		m_pShell->PVRShellOutputDebug( "Unable to open X display" );
		return false;
	}

	x11screen = XDefaultScreen( x11display );

	if(m_pShell->m_pShellData->bShellPosWasDefault)
	{
		m_pShell->m_pShellData->nShellDimX = XDisplayWidth(x11display,x11screen);
		m_pShell->m_pShellData->nShellDimY = XDisplayHeight(x11display,x11screen);
		if ( (m_pShell->m_pShellData->nShellDimX>640) || (m_pShell->m_pShellData->nShellDimY>480))
		{
			m_pShell->m_pShellData->nShellDimX = 640;
			m_pShell->m_pShellData->nShellDimY = 480;
		}

		if( !m_pShell->m_pShellData->bFullScreen ){
			m_pShell->m_pShellData->nShellDimX /= 2;
			m_pShell->m_pShellData->nShellDimY /= 2;
			m_pShell->m_pShellData->nShellPosX += m_pShell->m_pShellData->nShellDimX/2;
			m_pShell->m_pShellData->nShellPosY += m_pShell->m_pShellData->nShellDimY/2;
		}
	}

	// Create the window
	if( !OpenX11Window(*m_pShell))
	{
		m_pShell->PVRShellOutputDebug( "Unable to open X11 window" );
		return false;
	}

	// Pixmap support: create the pixmap
	if(m_pShell->m_pShellData->bNeedPixmap)
	{
		int depth = DefaultDepth(x11display, x11screen);
		x11pixmap = XCreatePixmap(x11display,x11window,m_pShell->m_pShellData->nShellDimX,m_pShell->m_pShellData->nShellDimY,depth);
		x11gc	  = XCreateGC(x11display,x11window,0,0);
	}

	return true;
}

/*!***********************************************************************
 @Function		OsReleaseOS
 @description	Destroys main window
*************************************************************************/
void PVRShellInit::OsReleaseOS()
{
	XCloseDisplay( x11display );
}

/*!***********************************************************************
 @Function		OsExit
 @description	Destroys main window
*************************************************************************/
void PVRShellInit::OsExit()
{
	// Show the exit message to the user
	m_pShell->PVRShellOutputDebug((const char*)m_pShell->PVRShellGet(prefExitMessage));
}

/*!***********************************************************************
 @Function		OsDoInitAPI
 @Return		true on success
 @description	Perform API initialisation and bring up window / fullscreen
*************************************************************************/
bool PVRShellInit::OsDoInitAPI()
{

	if(!ApiInitAPI())
	{
		return false;
	}

	// No problem occured
	return true;
}

/*!***********************************************************************
 @Function		OsDoReleaseAPI
 @description	Clean up after we're done
*************************************************************************/
void PVRShellInit::OsDoReleaseAPI()
{
	ApiReleaseAPI();

	if(m_pShell->m_pShellData->bNeedPixmap)
	{
		// Pixmap support: free the pixmap
		XFreePixmap(x11display,x11pixmap);
		XFreeGC(x11display,x11gc);
	}

	CloseX11Window();
}

/*!***********************************************************************
 @Function		OsRenderComplete
 @Returns		false when the app should quit
 @description	Main message loop / render loop
*************************************************************************/
void PVRShellInit::OsRenderComplete()
{
	int		numMessages;
	XEvent	event;

	// Are there messages waiting, maybe this should be a while loop
	numMessages = XPending( x11display );
	for( int i = 0; i < numMessages; i++ )
	{
		XNextEvent( x11display, &event );

		switch( event.type )
		{

			case ClientMessage:
				if (*XGetAtomName(x11display, event.xclient.message_type) == *"WM_PROTOCOLS")
				{
					gShellDone = true;
				}
				break;

			// exit on mouse click
			case ButtonRelease:
        	case ButtonPress:
        		//gShellDone = true;
        		break;

			// should SDK handle these?
			case MapNotify:
        	case UnmapNotify:
        		break;

			case KeyPress:
			{
				XKeyEvent *key_event = ((XKeyEvent *) &event);

				nLastKeyPressed = (PVRShellKeyName) key_event->keycode;

				switch(nLastKeyPressed)
				{
					case 9:	 	nLastKeyPressed = PVRShellKeyNameQUIT;	break; 			// Esc
					case 95:	nLastKeyPressed = PVRShellKeyNameScreenshot;	break; 	// F11
					case 36:	nLastKeyPressed = PVRShellKeyNameSELECT;	break; 		// Enter
					case 10:	nLastKeyPressed = PVRShellKeyNameACTION1;	break; 		// number 1
					case 11:	nLastKeyPressed = PVRShellKeyNameACTION2;	break; 		// number 2
					case 98:
					case 111:	nLastKeyPressed = PVRShellKeyNameUP;	break;
					case 104:
					case 116:	nLastKeyPressed = PVRShellKeyNameDOWN;	break;
					case 100:
					case 113:	nLastKeyPressed = PVRShellKeyNameLEFT;	break;
					case 102:
					case 114: 	nLastKeyPressed = PVRShellKeyNameRIGHT;	break;
					default:
						break;
				}
				
				if(m_pEventHandler)
				{
					static char buf[64];

					KeySym keysym;

					int XLookupRet;

					
					XLookupRet = XLookupString(key_event, buf, sizeof buf, &keysym, 0);

					
					m_pEventHandler->KeyDown(keysym,key_event->time);
					for(int i=0;i<XLookupRet;i++)
					{
						m_pEventHandler->KeyChar(buf[i],key_event->time);
					}
					
				}
			}
			break;

			case KeyRelease:
			{
				XKeyEvent *key_event = ((XKeyEvent *) &event);

//				char buf[10];
//				XLookupString(&event.xkey,buf,10,NULL,NULL);
//				charsPressed[ (int) *buf ] = 0;

				if(m_pEventHandler)
				{
					static char buf[64];
					KeySym keysym;
					int XLookupRet;
					
					XLookupRet = XLookupString(key_event, buf, sizeof buf, &keysym, 0);
				
					m_pEventHandler->KeyUp(keysym,key_event->time);
				
				}
			}
			break;

			default:
				break;
		}
	}
}

/*!***********************************************************************
 @Function		OsPixmapCopy
 @Return		true if the copy succeeded
 @description	When using pixmaps, copy the render to the display
*************************************************************************/
bool PVRShellInit::OsPixmapCopy()
{
	return (XCopyArea(x11display,x11pixmap,x11window,x11gc,0,0,m_pShell->m_pShellData->nShellDimX,m_pShell->m_pShellData->nShellDimY,0,0) == 0);
}

/*!***********************************************************************
 @Function		OsGetNativeDisplayType
 @Return		The 'NativeDisplayType' for EGL
 @description	Called from InitAPI() to get the NativeDisplayType
*************************************************************************/
void *PVRShellInit::OsGetNativeDisplayType()
{
	return x11display;
}

/*!***********************************************************************
 @Function		OsGetNativePixmapType
 @Return		The 'NativePixmapType' for EGL
 @description	Called from InitAPI() to get the NativePixmapType
*************************************************************************/
void *PVRShellInit::OsGetNativePixmapType()
{
	// Pixmap support: return the pixmap
	return (void*)x11pixmap;
}

/*!***********************************************************************
 @Function		OsGetNativeWindowType
 @Return		The 'NativeWindowType' for EGL
 @description	Called from InitAPI() to get the NativeWindowType
*************************************************************************/
void *PVRShellInit::OsGetNativeWindowType()
{
	return (void*)x11window;
}

/*!***********************************************************************
 @Function		OsGet
 @Input			prefName	Name of value to get
 @Modified		pn A pointer set to the value asked for
 @Returns		true on success
 @Description	Retrieves OS-specific data
*************************************************************************/
bool PVRShellInit::OsGet(const prefNameIntEnum prefName, int *pn)
{
	return false;
}

/*!***********************************************************************
 @Function		OsGet
 @Input			prefName	Name of value to get
 @Modified		pp A pointer set to the value asked for
 @Returns		true on success
 @Description	Retrieves OS-specific data
*************************************************************************/
bool PVRShellInit::OsGet(const prefNamePtrEnum prefName, void **pp)
{
	return false;
}

/*!***********************************************************************
 @Function		OsDisplayDebugString
 @Input			str		string to output
 @Description	Prints a debug string
*************************************************************************/
void PVRShellInit::OsDisplayDebugString(char const * const str)
{
	fprintf(stderr, str);
}

/*!***********************************************************************
 @Function		OsGetTime
 @Return		Time in milliseconds since the beginning of the application
 @Description	Gets the time in milliseconds since the beginning of the application
*************************************************************************/
unsigned long PVRShellInit::OsGetTime()
{
	//return ((unsigned long)(clock()*(1000.0f/CLOCKS_PER_SEC)));

	//timeb tt;
	//ftime(&tt);
	//return tt.millitm;

	timeval tv;
	gettimeofday(&tv,NULL);
	return (unsigned long)((tv.tv_sec*1000) + (tv.tv_usec/1000.0));
}

/*****************************************************************************
 Class: PVRShellInitOS
*****************************************************************************/

PVRShellInitOS::PVRShellInitOS() 
{ 
	m_pEventHandler=NULL; 
}


// needs to be run after SelectEGLConfiguration()
// for gEglConfig to be valid
int PVRShellInitOS::OpenX11Window(const PVRShell &shell)
{
    XSetWindowAttributes	wa;
    XSizeHints				sh;
    XEvent					event;
    unsigned long			mask;

	int depth = DefaultDepth(x11display, x11screen);
	x11visual = new XVisualInfo;
	XMatchVisualInfo( x11display, x11screen, depth, TrueColor, x11visual);

    if( !x11visual )
    {
    	shell.PVRShellOutputDebug( "Unable to acquire visual" );
    	return false;
    }

    x11colormap = XCreateColormap( x11display, RootWindow(x11display, x11screen), x11visual->visual, AllocNone );

    wa.colormap = x11colormap;
    wa.background_pixel = 0xFFFFFFFF;
    wa.border_pixel = 0;

    // add to these for handling other events
    wa.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;

    mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

    x11window = XCreateWindow( x11display, RootWindow(x11display, x11screen), shell.m_pShellData->nShellPosX, shell.m_pShellData->nShellPosY,
			                    shell.m_pShellData->nShellDimX, shell.m_pShellData->nShellDimY, 0,
			                    CopyFromParent, InputOutput, CopyFromParent, mask, &wa);

//	m_pShell->m_pShellData->nShellDimX = DisplayWidth( x11display, x11screen );
//	m_pShell->m_pShellData->nShellDimY = DisplayHeight( x11display, x11screen );

	// todo:meh remove this?
    sh.flags = USPosition;
    sh.x = shell.m_pShellData->nShellPosX;
    sh.y = shell.m_pShellData->nShellPosY;
    XSetStandardProperties( x11display, x11window, szTitle, szTitle, None, 0, 0, &sh );

    // Map and then wait till mapped
    XMapWindow( x11display, x11window );
    XIfEvent( x11display, &event, WaitForMapNotify, (char*)x11window );

	Atom wmDelete = XInternAtom(x11display, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(x11display, x11window, &wmDelete, 1);

    XSetWMColormapWindows( x11display, x11window, &x11window, 1 );

    XFlush( x11display );

    return true;
}

void PVRShellInitOS::CloseX11Window()
{
	XDestroyWindow( x11display, x11window );
    XFreeColormap( x11display, x11colormap );
	if( x11visual )
		XFree( (char*)x11visual );
}

/*****************************************************************************
 Global code
*****************************************************************************/

static Bool WaitForMapNotify( Display *d, XEvent *e, char *arg )
{
	return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}


void PVRShellInitOS::SetEventHandler(PVREventHandler *pkh) 
{ 
	m_pEventHandler=pkh;
}

PVREventHandler *PVRShellInitOS::GetEventHandler() 
{ 
	return m_pEventHandler;
}

/*!***************************************************************************
@function		main
@input			argc	count of args from OS
@input			argv	array of args from OS
@returns		result code to OS
@description	Main function of the program
*****************************************************************************/
int main(int argc, char **argv)
{
	PVRShellInit init;

	/*
		Create the demo, process the command line, create the OS initialiser.
	*/
	PVRShell *pDemo = NewDemo();
	if(!pDemo)
		return EXIT_ERR_CODE;

	init.Init(*pDemo);
	init.CommandLine((argc-1),&argv[1]);

	/*
		Initialise/run/shutdown
	*/
	while(init.Run());

	delete pDemo;
	return EXIT_NOERR_CODE;
}

/*****************************************************************************
 End of file (PVRShellOS.cpp)
*****************************************************************************/
