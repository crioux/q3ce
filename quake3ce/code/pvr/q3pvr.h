#ifndef __INC_Q3PVR_H
#define __INC_Q3PVR_H

#include<PVRTools.h>
#include<PVRShell.h>
#include<PVRShellOS.h>
#include<queue>


enum Q3EventType {
	Q3EVT_KEYUP,
	Q3EVT_KEYDOWN,
	Q3EVT_KEYCHAR,
	Q3EVT_MOUSEMOVE,
	Q3EVT_MOUSEUP,
	Q3EVT_MOUSEDOWN
};

struct Q3Event
{
	Q3EventType type;
	int time;
	int args[2];
};

extern std::queue<Q3Event> g_EventQueue;


class Q3PVR: public PVRShell
{
public:
    virtual bool InitApplication();
    virtual bool InitView();
    virtual bool ReleaseView();
    virtual bool QuitApplication();
    virtual bool RenderScene();
};

extern Q3PVR *g_pQ3PVR;
extern int g_pvr_window_height;
extern int g_pvr_window_width;

#endif

