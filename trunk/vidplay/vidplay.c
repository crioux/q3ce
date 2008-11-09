#include"stdafx.h"
#include"common/common.h"
#include"vidplay.h"

static bool_t Play=0;
static node* Player;

//////////////////////////////////////////////////////////////////////////////////////////////////


int SilentError(void* p,int Param,int Param2)
{
//	DebugMessage((const tchar_t*)Param2);
	return ERR_NONE;
}

int PlayerNotify(node* Player,int Param,int Param2)
{
	if (Param == PLAYER_PLAY)
		Play = Param2;

	return ERR_NONE;
}



void Static_Init(void)
{
//	ATI3200_Init();
//	Intel2700G_Init();
	LibMad_Init();
	mpeg4_init();
	AVI_Init();
//	ATI4200_Init();

//	StringAdd(1,FILE_ID,NODE_CONTENTTYPE,TEXT("FILE"));
	StringAdd(1,AVI_ID,NODE_EXTS,TEXT("avi:a"));
	StringAdd(1,LIBMAD_ID,NODE_CONTENTTYPE,TEXT("acodec/0x0050,acodec/0x0051,acodec/0x0052,acodec/0x0053,acodec/0x0054,acodec/0x0055"));
	StringAdd(1,MPEG4_ID,NODE_CONTENTTYPE,TEXT("vcodec/DIVX"));
	
}

void Static_Done(void)
{
//	ATI3200_Done();
//	Intel2700G_Done();
	LibMad_Done();
	mpeg4_done();
	AVI_Done();
//	ATI4200_Done();
}

int VidPlay_Init(void) 
{
	int h;

	if(!Context_Init(TEXT("vidplay"),TEXT("vidplay"),3,TEXT("vidplay")))
	{
		return 0;
	}

	h=VidPlay_Open("",0,0,10,10,0);
	VidPlay_Close(h);

	Context()->Error.Func = SilentError;

	return 1;
}

int VidPlay_Kill(void)
{
	Context_Done();

	return 1;
}
	
int VidPlay_Open(const char *svName, int x, int y, int w, int h, int loop)
{
	notify Notify;
	int Int;
	int vphandle=1;
	tchar_t Loc[256];

/*
	int i,cnt;
	TCHAR wcwd[256];
	TCHAR *last;

	GetModuleFileName(NULL,wcwd,256);
	last=_tcsrchr(wcwd,TEXT('\\'));
	last[0]=TEXT('\0');

	_snwprintf(Loc,256,TEXT("%s\\%S"),wcwd,svName);
	cnt=_tcslen(Loc);
	for(i=0;i<cnt;i++)
	{
		if(Loc[i]==TEXT('/'))
		{
			Loc[i]=TEXT('\\');
		}
	}
*/
	_snwprintf(Loc,256,TEXT("%S"),svName);
	
	Player = Context()->Player;

	Notify.Func = (notifyfunc) PlayerNotify;
	Notify.This = Player;
	Player->Set(Player,PLAYER_NOTIFY,&Notify,sizeof(Notify));

	// empty saved playlist
	Int = 0;
	Player->Set(Player,PLAYER_LIST_COUNT,&Int,sizeof(Int));

	Player->Set(Player,PLAYER_LIST_URL+0,Loc,_tcslen(Loc)+1);

	VidPlay_SetExtents(vphandle, x, y, w, h);
	VidPlay_SetLoop(vphandle, loop);

	Context_Wnd((void*)1); //fake window handle	

	return vphandle;
}

int VidPlay_Play(int handle)
{
	bool_t Bool = 1;
	Player->Set(Player,PLAYER_PLAY,&Bool,sizeof(Bool));
	((player*)Player)->Paint(Player,NULL,0,0);
	return 1;
}

int VidPlay_Stop(int handle)
{
	bool_t Bool = 0;
	Player->Set(Player,PLAYER_PLAY,&Bool,sizeof(Bool));
	return 1;
}

int VidPlay_Close(int handle)
{
	VidPlay_Stop(handle);
	Context_Wnd(NULL);
	return 1;
}

int VidPlay_Process(int handle)
{
#ifndef MULTITHREAD
	if(((player*)Player)->Process(Player)==ERR_BUFFER_FULL)
	{
		Sleep(4);
	}
#endif
	if(!Play)
	{
		return 0;
	}
	return 1;
}

int VidPlay_SetExtents(int handle, int x, int y, int w, int h)
{
	rect Rect;
	Rect.x = x;
	Rect.y = y;
	Rect.Width = w;
	Rect.Height = h;
	Player->Set(Player,PLAYER_SKIN_VIEWPORT,&Rect,sizeof(Rect));
	return 1;
}

int VidPlay_SetLoop(int handle, int loop)
{
	// turn off repeat
	bool_t Bool = loop;
	Player->Set(Player,PLAYER_REPEAT,&Bool,sizeof(Bool));
	return 1;
}





