#include "q3pvr.h"

#include<PVRShell.h>
#include<PVRShellOS.h>
#include"vidplay.h"
#include"xmalloc.h"
#ifndef _WIN32
#include"unixdefs.h"
#endif
#include"q_shared.h"
#include"qcommon.h"
#include"pvr_local.h"

Q3PVR *g_pQ3PVR;

bool g_no_cinematics=false;

static DWORD s_SND_SIZE=((int)(2*1024*1024));
static DWORD s_ZONE_SIZE=0;
static DWORD s_HUNK_SIZE=0;

static bool s_bVidPlay=false;

CXMalloc g_mem_snd("q3pvr_snd");
CXMalloc g_mem_zone("q3pvr_zone");
CXMalloc g_mem_hunk("q3pvr_hunk");

CXMalloc g_mem_all("q3pvr_mem");

int	totalMsec, countMsec;
bool	g_bQuit=false;

#ifndef _WIN32_WCE
static DWORD GetFreeProgramMemory()
{
	return 128*1024*1024;
}
#endif

int g_pvr_window_height=0;
int g_pvr_window_width=0;

bool Q3PVR_InitMemory(void)
{
	// Figure out how much memory we can allocate to lg and hunk
	DWORD dwBaseReserve,dwAvailable;
	
	dwBaseReserve=s_SND_SIZE+65536;
	
	dwAvailable=(GetFreeProgramMemory()-LOW_WATER_MARK-65536);
	
	if(dwAvailable<(dwBaseReserve+(8*1024*1024)+(16*1024*1024)))
	{
		return false;
	}
	
	s_ZONE_SIZE=((int)((dwAvailable-dwBaseReserve)/3))/65536*65536;
	s_HUNK_SIZE=(dwAvailable-dwBaseReserve-s_ZONE_SIZE)/65536*65536;
	
	if(!g_mem_hunk.InitMalloc(s_HUNK_SIZE))
		return false;

	if(!g_mem_zone.InitMalloc(s_ZONE_SIZE))
		return false;
	
	if(!g_mem_snd.InitMalloc(s_SND_SIZE))
		return false;

	return true;
}

bool Q3PVR_TerminateMemory(void)
{
	g_mem_snd.Terminate();
	g_mem_zone.Terminate();
	g_mem_hunk.Terminate();

	return true;
}

PVRShell* NewDemo()
{
	return new Q3PVR();
}


class Q3PVREventHandler:public PVREventHandler
{
	virtual void KeyUp(int keycode,int time)
	{
		Q3Event evt;
		evt.type=Q3EVT_KEYUP;
		evt.time=time;
		evt.args[0]=keycode;
		g_EventQueue.push(evt);		
	}

	virtual void KeyDown(int keycode,int time)
	{
		Q3Event evt;
		evt.type=Q3EVT_KEYDOWN;
		evt.time=time;
		evt.args[0]=keycode;
		g_EventQueue.push(evt);
	}

	virtual void KeyChar(int character,int time)
	{
		Q3Event evt;
		evt.type=Q3EVT_KEYCHAR;
		evt.time=time;
		evt.args[0]=character;
		g_EventQueue.push(evt);
	}

	virtual void MouseMove(int x, int y, int time)
	{
	}

	virtual void MouseDown(int button, int time)
	{
	}

	virtual void MouseUp(int button, int time)
	{
	}
};


Q3PVREventHandler g_Q3PVREventHandler;


bool Q3PVR::InitApplication()
{	
	g_pQ3PVR=this;

	if(!Q3PVR_InitMemory())
	{
		fprintf(stderr,"Insufficient memory to start Q3PVR.\nTry freeing up more program or storage memory.\n");
		return 0;
	}

	if(!g_no_cinematics)
	{
#ifdef USE_VIDPLAY
		if(!VidPlay_Init())
		{
			s_bVidPlay=false;
		}
		else 
		{
			s_bVidPlay=true;
		}
#else
	s_bVidPlay=false;
#endif
	}

}

int main_init( int argc, char **argv );
char *g_args[4];
int g_arg_count;

bool Q3PVR::InitView()
{
	g_pvr_window_height=PVRShellGet(prefHeight);
	g_pvr_window_width=PVRShellGet(prefWidth);
	
	int argc=PVRShellGet(prefCommandLineOptNum);
	SCmdLineOpt *argv=(SCmdLineOpt *)PVRShellGet(prefCommandLineOpts);

	int i;
	for(i=0;i<argc;i++)
	{
		if(strcmp(argv[i].pArg,"-start")==0)
		{
			break;
		}
	}
	if(i<argc)
	{
		g_args[0]=strdup("q3pvr");
		g_args[1]=strdup("spmap q3dm1");
		g_args[2]=NULL;
		g_arg_count=2;
	}
	else
	{
		
		g_args[0]=strdup("q3pvr");
		g_args[1]=NULL;
		g_arg_count=1;
	}

	main_init( g_arg_count, g_args );

	SetEventHandler(&g_Q3PVREventHandler);

	return true;
}

bool Q3PVR::ReleaseView()
{
	for(int i=0;i<g_arg_count;i++)
	{
		free(g_args[i]);
	}

	return true;
}

bool Q3PVR::QuitApplication()
{
	if(!g_no_cinematics)
	{
		if(s_bVidPlay)
		{
#ifdef USE_VIDPLAY
			VidPlay_Kill();
#endif
		}
	}

	Q3PVR_TerminateMemory();	

	return true;
}

bool Q3PVR::RenderScene()
{

	int startTime, endTime;
	
	// if not running as a game client, sleep a bit
	//if ( g_wv.isMinimized || ( com_dedicated && com_dedicated->integer ) ) {
	//	usleep( 5000 );
	//}

		// set low precision every frame, because some system calls
		// reset it arbitrarily
//		_controlfp( _PC_24, _MCW_PC );
//    _controlfp( -1, _MCW_EM  ); // no exceptions, even if some crappy
				// syscall turns them back on!

	startTime = Sys_Milliseconds();

	// make sure mouse and joystick are only called once a frame
	IN_Frame();

	// run the game
	Com_Frame();
	
	endTime = Sys_Milliseconds();
	totalMsec += endTime - startTime;
	countMsec++;

	return !g_bQuit;
}

void Quit(void)
{
	g_bQuit=true;
}

