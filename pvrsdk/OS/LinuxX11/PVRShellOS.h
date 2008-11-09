/******************************************************************************

 @File         PVRShellOS.h

 @Title        

 @Copyright    Copyright (C) 2004 - 2008 by Imagination Technologies Limited.

 @Platform     X11

 @Description  Makes programming for 3D APIs easier by wrapping surface
               initialization, Texture allocation and other functions for use by a demo.

******************************************************************************/
#ifndef _PVRSHELLOS_
#define _PVRSHELLOS_

#include "X11/Xlib.h"
#include "X11/Xutil.h"

#define PVRSHELL_DIR_SYM	'/'
#define _stricmp strcasecmp

/*!***************************************************************************
 PVRShellInitOS
 @Brief Class. Interface with specific Operative System.
*****************************************************************************/
class PVRShellInitOS
{
public:
	char* szTitle;

	Display*     x11display;
	long         x11screen;
	XVisualInfo* x11visual;
	Colormap     x11colormap;
	Window       x11window;

	// Pixmap support: variables for the pixmap
	Pixmap		x11pixmap;
	GC			x11gc;

	PVREventHandler *m_pEventHandler;

public:
	PVRShellInitOS();

	int OpenX11Window(const PVRShell &shell);
	void CloseX11Window();

	void SetEventHandler(PVREventHandler *pkh);
	PVREventHandler *GetEventHandler();
};

#endif /* _PVRSHELLOS_ */
/*****************************************************************************
 End of file (PVRShellOS.h)
*****************************************************************************/
