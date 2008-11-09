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

/*****************************************************************************
 * name:		cl_cin.c
 *
 * desc:		video and cinematic playback
 *
 * $Archive: /MissionPack/code/client/cl_cin.c $
 *
 * cl_glconfig.hwtype trtypes 3dfx/ragepro need 256x256
 *
 *****************************************************************************/

#include"client_pch.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "vidplay.h"
#include "common/common.h"
#include "common/node.h"
}

extern bool g_no_cinematics;

#ifdef USE_VIDPLAY
	
#define PK3FILE_ID			FOURCC('P','K','3','F')


typedef struct pk3stream
{
	stream Stream;
	fileHandle_t Handle;
	tchar_t URL[MAXPATH];
	filepos_t Length;
	filepos_t Pos;
	bool_t Silent;
	bool_t Create;

} pk3stream;

static int PK3Get(pk3stream* p, int No, void* Data, int Size);
static int PK3Open(pk3stream* p, const tchar_t* URL, bool_t ReOpen);
static int PK3Set(pk3stream* p, int No, const void* Data, int Size);
static int PK3Read(void* p,void* Data,int Size);
static int PK3ReadBlock(void* p,block* Block,int Ofs,int Size);
static int PK3Seek(void* p,int Pos,int SeekMode);
static int PK3Write(void* p,const void* Data,int Size);
static int PK3EnumDir(void* p,const tchar_t* URL,const tchar_t* Exts,bool_t ExtFilter,streamdir* Item);
static int PK3Create(pk3stream* p);
static void PK3Delete(pk3stream* p);
void PK3FileInit();
void PK3FileDone();


#define MAXSIZE				8
#define MINSIZE				4
#define DEFAULT_CIN_WIDTH	240
#define DEFAULT_CIN_HEIGHT	120
#define MAX_VIDEO_HANDLES	1

typedef struct 
{
	int vidplay_handle;
	int x,y,w,h;
	int loop;
	int altergamestate;
} cin_t;

static cin_t cinTable[MAX_VIDEO_HANDLES];
static int currentHandle = -1;
static int CL_handle = -1;
static int cin_init=0;

static const nodedef PK3File =
{
	sizeof(pk3stream),
	PK3FILE_ID,
	STREAM_CLASS,
	PRI_MINIMUM,
	(nodecreate)PK3Create,
	(nodedelete)PK3Delete,
};


void CIN_Init(void)
{
	if(g_no_cinematics)
	{
		return;
	}

	int i;

	for(i=0;i<MAX_VIDEO_HANDLES;i++)
	{
		cinTable[i].vidplay_handle=-1;
		cinTable[i].x=0;
		cinTable[i].y=0;
		cinTable[i].w=0;
		cinTable[i].h=0;
		cinTable[i].loop=0;
		cinTable[i].altergamestate=0;
	}

	cin_init=1;

	NodeRegisterClass(&PK3File);

	StringAdd(1,PK3FILE_ID,NODE_CONTENTTYPE,(tchar_t*)L"FILE");

}

void CIN_CloseAllVideos(void) 
{
	if(g_no_cinematics)
	{
		return;
	}

	int i;
	for(i=0;i<MAX_VIDEO_HANDLES;i++) 
	{
		if (cinTable[i].vidplay_handle!=-1) 
		{
			CIN_StopCinematic(i);
		}
	}
}

static int CIN_HandleForVideo(void) 
{
	if(g_no_cinematics)
	{
		return -1;
	}

	int	i;
	for(i=0;i<MAX_VIDEO_HANDLES;i++) 
	{
		if(cinTable[i].vidplay_handle==-1) 
		{
			return i;
		}
	}
	Com_Error( ERR_DROP, "CIN_HandleForVideo: none free" );
	return -1;
}



static void NextMap(void) 
{
	const char *s;
	cls.state = CA_DISCONNECTED;
	// we can't just do a vstr nextmap, because
	// if we are aborting the intro cinematic with
	// a devmap command, nextmap would be valid by
	// the time it was referenced
	s = Cvar_VariableString( "nextmap" );
	if(s[0]) 
	{
		Cbuf_ExecuteText( EXEC_APPEND, va("%s\n", s) );
		Cvar_Set("nextmap", "");
	}
}

/*
==================
SCR_StopCinematic
==================
*/
e_status CIN_StopCinematic(int handle) 
{
	if(g_no_cinematics)
	{
		return FMV_EOF;
	}

	int alter=0;
	if (handle < 0 || handle>= MAX_VIDEO_HANDLES) 
	{
		return FMV_EOF;
	}
	if(cinTable[handle].vidplay_handle==-1) 
	{
		return FMV_EOF;
	}


	currentHandle = handle;

	Com_DPrintf("trFMV::stop(), closing cinematic\n");

	VidPlay_Stop(cinTable[handle].vidplay_handle);
	VidPlay_Close(cinTable[handle].vidplay_handle);
	
	if(cinTable[handle].altergamestate)
	{
		alter=1;
	}

	cinTable[handle].vidplay_handle=-1;
	cinTable[handle].x=0;
	cinTable[handle].y=0;
	cinTable[handle].w=0;
	cinTable[handle].h=0;
	cinTable[handle].loop=0;
	cinTable[handle].altergamestate=0;

	if(alter)
	{
		NextMap();
	}

	if(CL_handle==handle)
	{
		CL_handle=-1;
	}

	currentHandle=-1;
	
	return FMV_EOF;
}

/*
==================
SCR_RunCinematic

Fetch and decompress the pending frame
==================
*/


e_status CIN_RunCinematic (int handle)
{
	if(g_no_cinematics)
	{
		return FMV_EOF;
	}

	int res;

	if (handle < 0 || handle>= MAX_VIDEO_HANDLES) 
	{
		return FMV_EOF;
	}
	if(cinTable[handle].vidplay_handle==-1) 
	{
		return FMV_EOF;
	}
	
	currentHandle=handle;

	res=VidPlay_Process(cinTable[handle].vidplay_handle);
	if(res<=0)
	{
		return CIN_StopCinematic(handle);
	}

	return FMV_PLAY;
}

void CIN_AdjustFrom640( gfixed *x, gfixed *y, gfixed *w, gfixed *h ) {
	gfixed	xscale;
	gfixed	yscale;
	int vx,vy;

	vx=cls.glconfig.vidWidth;
	vy=cls.glconfig.vidHeight;

	// scale for screen sizes
	
	xscale = FIXED_INT32RATIO_G(vx,640);
	yscale = FIXED_INT32RATIO_G(vy,480);
	
	if(x) *x *= xscale;
	if(y) *y *= yscale;
	if(w) *w *= xscale;
	if(h) *h *= yscale;
}


/*
==================
CL_PlayCinematic

==================
*/
int CIN_PlayCinematic( const char *arg, int x, int y, int w, int h, int systemBits ) 
{
	if(g_no_cinematics)
	{
		return -1;
	}

	char name[MAX_OSPATH];
	int namelen;
	int vidhandle;
	gfixed fx,fy,fw,fh;
	int alter=1;
	int loop=0;
	

	if(!cin_init)
	{
		CIN_Init();
	}
	
	if (strstr(arg, "/") == NULL && strstr(arg, "\\") == NULL) 
	{
		Com_sprintf (name, sizeof(name), "video/%s", arg);
	}
	else 
	{
		Com_sprintf (name, sizeof(name), "%s", arg);
	}

	// Convert all RoQ names to avi
	namelen=strlen(name);
	if(namelen>3 && _strnicmp(name+namelen-3,"roq",3)==0)
	{
		name[namelen-3]='a';
		name[namelen-2]='v';
		name[namelen-1]='i';
	}


	Com_DPrintf("SCR_PlayCinematic( %s )\n", arg);

	alter=((systemBits & CIN_shader)==0)?1:0;
	loop=((systemBits & CIN_loop)!=0)?1:0;

	if(alter)
	{
		// close the menu
		SysCallArgs args(1);		
		args[0]=UIMENU_NONE;
		VM_Call( uivm, UI_SET_ACTIVE_MENU, args);		
	}

	fx=MAKE_GFIXED(x);
	fy=MAKE_GFIXED(y);
	fw=MAKE_GFIXED(w);
	fh=MAKE_GFIXED(h);
	CIN_AdjustFrom640( &fx, &fy, &fw, &fh );
	x=FIXED_TO_INT(fx);
	y=FIXED_TO_INT(fy);
	w=FIXED_TO_INT(fw);
	h=FIXED_TO_INT(fh);

	vidhandle=VidPlay_Open(name,x,y,w,h,loop);
	if(vidhandle<=0)
	{
		return -1;
	}	

	currentHandle=CIN_HandleForVideo();
	cinTable[currentHandle].vidplay_handle=vidhandle;
	cinTable[currentHandle].x=x;
	cinTable[currentHandle].y=y;
	cinTable[currentHandle].w=w;
	cinTable[currentHandle].h=h;
	cinTable[currentHandle].loop=loop;
	cinTable[currentHandle].altergamestate=alter;


	VidPlay_Play(vidhandle);

	return currentHandle;
}

void CIN_SetExtents (int handle, int x, int y, int w, int h) 
{
	if(g_no_cinematics)
	{
		return;
	}

	gfixed fx,fy,fw,fh;
	if (handle < 0 || handle>= MAX_VIDEO_HANDLES) return;
	if(cinTable[handle].vidplay_handle==-1) return;


	fx=MAKE_GFIXED(x);
	fy=MAKE_GFIXED(y);
	fw=MAKE_GFIXED(w);
	fh=MAKE_GFIXED(h);
	SCR_AdjustFrom640( &fx, &fy, &fw, &fh );
	x=FIXED_TO_INT(fx);
	y=FIXED_TO_INT(fy);
	w=FIXED_TO_INT(fw);
	h=FIXED_TO_INT(fh);
	
	VidPlay_SetExtents(cinTable[handle].vidplay_handle,x,y,w,h);
}

void CIN_SetLooping(int handle, int loop) 
{
	if(g_no_cinematics)
	{
		return;
	}

	if (handle < 0 || handle>= MAX_VIDEO_HANDLES) return;
	if(cinTable[handle].vidplay_handle==-1) return;

	VidPlay_SetLoop(cinTable[handle].vidplay_handle,(loop==1)?1:0);
}

/*
==================
SCR_DrawCinematic

==================
*/
void CIN_DrawCinematic (int handle) 
{
	if(g_no_cinematics)
	{
		return;
	}
	//xxx
}

void CL_PlayCinematic_f(void) 
{
	if(g_no_cinematics)
	{
		return;
	}
	const char	*arg, *s;
	int	holdatend;
	int bits = CIN_system;

	Com_DPrintf("CL_PlayCinematic_f\n");
	if (cls.state == CA_CINEMATIC) {
		SCR_StopCinematic();
	}

	arg = Cmd_Argv( 1 );
	s = Cmd_Argv(2);

	holdatend = 0;
	if ((s && s[0] == '1') || Q_stricmp(arg,"demoend.roq")==0 || Q_stricmp(arg,"end.roq")==0) {
		bits |= CIN_hold;
	}
	if (s && s[0] == '2') {
		bits |= CIN_loop;
	}
	
	S_StopAllSounds ();

	CL_handle = CIN_PlayCinematic( arg, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bits );
	if (CL_handle >= 0) 
	{
		while(CIN_RunCinematic(CL_handle)==FMV_PLAY)
		{
			sysEvent_t evt;
			evt=Sys_GetEvent();
			if(evt.evType==SE_KEY && evt.evValue2)
			{
				CIN_StopCinematic(CL_handle);
				break;
			}
			Sleep(4);
		}
	}
}


void SCR_DrawCinematic (void) 
{
	if(g_no_cinematics)
	{
		return;
	}
	if (CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES) 
	{
		CIN_DrawCinematic(CL_handle);
	}
}

void SCR_RunCinematic (void)
{
	if(g_no_cinematics)
	{
		return;
	}
	if (CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES) 
	{
		CIN_RunCinematic(CL_handle);
	}
}

void SCR_StopCinematic(void) 
{
	if(g_no_cinematics)
	{
		return;
	}
	if (CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES) 
	{
		CIN_StopCinematic(CL_handle);
		S_StopAllSounds();
	}
}

void CIN_UploadCinematic(int handle) 
{
	if(g_no_cinematics)
	{
		return;
	}

	/*
	if (handle >= 0 && handle < MAX_VIDEO_HANDLES) {
		if (!cinTable[handle].buf) {
			return;
		}
		if (cinTable[handle].playonwalls <= 0 && cinTable[handle].dirty) {
			if (cinTable[handle].playonwalls == 0) {
				cinTable[handle].playonwalls = -1;
			} else {
				if (cinTable[handle].playonwalls == -1) {
					cinTable[handle].playonwalls = -2;
				} else {
					cinTable[handle].dirty = 0;
				}
			}
		}
		re.UploadCinematic( cinTable[handle].width, cinTable[handle].height, cinTable[handle].width, cinTable[handle].height, cinTable[handle].buf, handle, cinTable[handle].dirty);
		if (cl_inGameVideo->integer == 0 && cinTable[handle].playonwalls == 1) {
			cinTable[handle].playonwalls--;
		}
	}
*/
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VidPlay/tcpmp PK3 filesystem driver


static int PK3Get(pk3stream* p, int No, void* Data, int Size)
{
	int Result = ERR_INVALID_PARAM;
	switch (No)
	{
	case STREAM_URL: GETSTRING(p->URL); break;
	case STREAM_SILENT: GETVALUE(p->Silent,bool_t); break;
	case STREAM_LENGTH: GETVALUECOND(p->Length,int,p->Length>=0); break;
	case STREAM_CREATE: GETVALUE(p->Create,bool_t); break;
	}
	return Result;
}

static int PK3Open(pk3stream* p, const tchar_t* URL, bool_t ReOpen)
{
//DebugBreak();
	if (p->Handle)
		FS_FCloseFile(p->Handle);
		
	p->Handle = 0;
	p->Length = -1;
	if (!ReOpen)
		p->URL[0] = 0;
	else
		Sleep(200);

	if (URL && URL[0])
	{
		char name[256];
		fileHandle_t Handle=0;

		_snprintf(name,256,"%S",URL);

		p->Length=FS_FOpenFileRead(name,&Handle,qfalse);
		if(p->Length<=0)
		{
			if (!ReOpen && !p->Silent)
				ShowError(0,ERR_ID,ERR_FILE_NOT_FOUND,URL);
			return ERR_FILE_NOT_FOUND;
		}

		TcsNCpy(p->URL,URL,MAXPATH);
		p->Handle = Handle;

		if (ReOpen)
		{
			PK3Seek(p,p->Pos,SEEK_SET);
		}
		else
		{
			p->Pos = 0;
		}
	}
	return ERR_NONE;
}

static int PK3Set(pk3stream* p, int No, const void* Data, int Size)
{
	int Result = ERR_INVALID_PARAM;
	switch (No)
	{
	case STREAM_SILENT: SETVALUE(p->Silent,bool_t,ERR_NONE); break;
	case STREAM_CREATE: SETVALUE(p->Create,bool_t,ERR_NONE); break;
	case STREAM_URL:
		Result = PK3Open(p,(const tchar_t*)Data,0);
		break;
	}
	return Result;
}

static int PK3Read(void * _p,void* Data,int Size)
{
	pk3stream *p=(pk3stream *)_p;
	int Readed;
	
	//Sleep(100); 

	//DEBUG_MSG3(-1,"FileRead: %08x %08x %d",p->Pos,SetFilePointer(p->Handle,0,NULL,FILE_CURRENT),Size);

	Readed=FS_Read(Data,Size,p->Handle);
	if(Readed>=0)
	{
		//DEBUG_MSG2(T("READ pos:%d len:%d"),p->Pos,Readed);
		p->Pos += Readed;
		return Readed;
	}


	return -1;
}

static int PK3ReadBlock(void * _p,block* Block,int Ofs,int Size)
{
	pk3stream *p=(pk3stream *)_p;
	
	return PK3Read(p,(char*)(Block->Ptr+Ofs),Size);
}

static int PK3Seek(void * _p,int Pos,int SeekMode)
{
	pk3stream *p=(pk3stream *)_p;
	
	int Diff;

	if(SeekMode==SEEK_CUR)
	{
		Diff=Pos;
	}
	else if (SeekMode==SEEK_SET)
	{
		FS_Seek( p->Handle, 0, FS_SEEK_SET);
		p->Pos=0;
		Diff=Pos;
	}
	else if (SeekMode==SEEK_END)
	{
		FS_Seek( p->Handle, 0, FS_SEEK_SET);
		p->Pos=0;
		Diff=p->Length+Pos;		
	}

	if(Diff==0)
	{
		return p->Pos;
	}
	else if(Diff>0)
	{
		int r=Diff;
		int rn;
		char x[4096];

		while(r!=0)
		{
			rn=r;
			if(rn>4096)
			{
				rn=4096;
			}
			FS_Read(x,rn,p->Handle);
			r-=rn;
		}
	}
	else if (Diff<0)
	{
		int rn;
		int r;
		char x[4096];

		FS_Seek( p->Handle, 0, FS_SEEK_SET);

		r=p->Pos+Diff;
		
		while(r!=0)
		{
			rn=r;
			if(rn>4096)
			{
				rn=4096;
			}
			FS_Read(x,rn,p->Handle);
			r-=rn;
		}
	}


	p->Pos+=Diff;
		
	return p->Pos;
}

static int PK3Write(void* _p,const void* Data,int Size)
{
//	pk3stream *p=(pk3stream *)_p;
	
	return -1;
}

static int PK3EnumDir(void* _p,const tchar_t* URL,const tchar_t* Exts,bool_t ExtFilter,streamdir* Item)
{
	//pk3stream *p=(pk3stream *)_p;
	
	return ERR_NONE;
}

static int PK3Create(pk3stream* p)
{
	p->Stream.Get = (nodeget)PK3Get,
	p->Stream.Set = (nodeset)PK3Set,
	p->Stream.Read = PK3Read;
	p->Stream.ReadBlock = PK3ReadBlock;
	p->Stream.Write = PK3Write;
	p->Stream.Seek = PK3Seek;
	p->Stream.EnumDir = PK3EnumDir;
	return ERR_NONE;
}

static void PK3Delete(pk3stream* p)
{
	PK3Open(p,NULL,0);
}

void PK3FileInit()
{
	if(g_no_cinematics)
	{
		return;
	}

	NodeRegisterClass(&PK3File);
}

void PK3FileDone()
{
	if(g_no_cinematics)
	{
		return;
	}

	NodeUnRegisterClass(PK3FILE_ID);
}

#else

void CIN_Init(void) {}
void CIN_CloseAllVideos(void) {}
int CIN_HandleForVideo(void) { return 1; }
e_status CIN_StopCinematic(int) { return FMV_IDLE; }
e_status CIN_RunCinematic (int) { return FMV_IDLE; }
void CIN_AdjustFrom640( gfixed *, gfixed *, gfixed *, gfixed *) {}
int CIN_PlayCinematic( const char *, int, int, int, int, int)  { return 1;}
void CIN_SetExtents (int, int, int, int, int)  {}
void CIN_SetLooping(int, int)  {}
void CIN_DrawCinematic (int) {}
void CL_PlayCinematic_f(void) {}
void SCR_DrawCinematic (void) {}
void SCR_RunCinematic (void) {}
void SCR_StopCinematic(void) {}
void CIN_UploadCinematic(int) {}

#endif

