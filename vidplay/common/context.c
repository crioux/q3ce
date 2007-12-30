/*****************************************************************************
 *
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: context.c 202 2005-01-25 01:27:33Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"
#include "softidct/softidct.h"

void OverlayDirect_Init();
void OverlayDirect_Done();
void OverlayRAW_Init();
void OverlayRAW_Done();
void OverlayGAPI_Init();
void OverlayGAPI_Done();
void OverlayHIRES_Init();
void OverlayHIRES_Done();
void OverlayDDraw_Init();
void OverlayDDraw_Done();
void OverlayFlyTV_Init();
void OverlayFlyTV_Done();
void OverlayGDI_Init();
void OverlayGDI_Done();
void OverlayXScale_Init();
void OverlayXScale_Done();

#ifdef TARGET_PALMOS
#include "../splitter/avi.h"
#include "../splitter/asf.h"
#include "../splitter/wav.h"
#include "../splitter/mov.h"
#include "../splitter/mpg.h"
#include "../interface/win.h"
#include "../interface/about.h"
#include "../interface/benchresult.h"
#include "../interface/mediainfo.h"
#include "../interface/settings.h"
#endif

//#define PREALLOC	12*16

extern void SetContext(context* Context);

int DefaultLang();

#ifdef PREALLOC
static void* q[PREALLOC] = {NULL};
#endif

bool_t Context_Init(const tchar_t* Name,const tchar_t* Version,int Id,const tchar_t* CmdLine)
{
	context* p = malloc(sizeof(context));
	if (!p) return 0;

#ifdef PREALLOC
	{ int i; for (i=0;i<PREALLOC;++i) q[i] = malloc(65536); }
#endif

	memset(p,0,sizeof(context));
	p->Version = CONTEXT_VERSION;
	p->ProgramId = Id;
	p->ProgramName = Name;
	p->ProgramVersion = Version;
	p->CmdLine = CmdLine;
	p->Lang = DefaultLang();
	p->StartUpMemory = AvailMemory();
	SetContext(p);

	SAFE_BEGIN		
	Mem_Init();
	String_Init();
	PCM_Init();
	Blit_Init();
	Node_Init();
	Platform_Init();
	Stream_Init();
	Advanced_Init();
	Flow_Init();
	Codec_Init();
	Audio_Init();
	Video_Init();
	Format_Init();
	FormatBase_Init();
	NullOutput_Init();
	RawAudio_Init();
	Timer_Init();
	IDCT_Init();
	Overlay_Init();
	WaveOut_Init();
	SoftIDCT_Init();
#if defined(TARGET_PALMOS)
	OverlayHIRES_Init();
	Win_Init();
	About_Init();
	BenchResult_Init();
	MediaInfo_Init();
	Settings_Init();
	ASF_Init();
	AVI_Init();
	WAV_Init();
	MP4_Init();
	MPG_Init();
#elif defined(TARGET_WIN32) || defined(TARGET_WINCE)
#if defined(TARGET_WINCE)
	OverlayXScale_Init();
	if (QueryPlatform(PLATFORM_TYPENO) != TYPE_SMARTPHONE)
	{
		OverlayRAW_Init();
		OverlayGAPI_Init();
	}
	else
	{
		OverlayGAPI_Init(); // prefer GAPI with smartphones (Sagem MyS-7 crashes with Raw FrameBuffer?)
		OverlayRAW_Init();
	}
	OverlayDirect_Init();
#endif
	OverlayDDraw_Init(); // after GAPI and RAW
	OverlayFlyTV_Init();
	OverlayGDI_Init();
#endif
	Plugins_Init();
#ifdef NO_PLUGINS
	Static_Init();
#endif
	Association_Init(); // after all formats are registered
	Color_Init(); 
	Equalizer_Init();
	Player_Init(); // after all output drivers are registered
	SAFE_END
	return 1;
}

void Context_Done()
{
	context* p = Context();
	if (!p) return;

	SAFE_BEGIN
	Player_Done();
	Color_Done();
	Equalizer_Done();
#ifdef NO_PLUGINS
	Static_Done();
#endif
	Plugins_Done();
#ifdef TARGET_PALMOS
	About_Done();
	BenchResult_Done();
	MediaInfo_Done();
	Settings_Done();
	Win_Done();
	MPG_Done();
	MP4_Done();
	ASF_Done();
	AVI_Done();
	WAV_Done();
	OverlayHIRES_Done();
#elif defined(TARGET_WIN32) || defined(TARGET_WINCE)
	OverlayDDraw_Done();
	OverlayFlyTV_Done();
	OverlayGDI_Done();
	OverlayXScale_Done();
#if defined(TARGET_WINCE)
	OverlayRAW_Done();
	OverlayGAPI_Done();
	OverlayDirect_Done();
#endif
#endif
	SoftIDCT_Done();
	WaveOut_Done();
	Overlay_Done();
	IDCT_Done();
	Timer_Done();
	NullOutput_Done();
	RawAudio_Done();
	FormatBase_Done();
	Format_Done();
	Association_Done();
	Video_Done();
	Audio_Done();
	Codec_Done();
	Flow_Done();
	Playlist_Done();
	Advanced_Done();
	Stream_Done();

	Platform_Done();
	Node_Done();
	Blit_Done();
	PCM_Done();
	String_Done();
	Mem_Done();
	Log_Done();
	SAFE_END

#ifdef PREALLOC
	{ int i; for (i=0;i<PREALLOC;++i) free(q[i]); }
#endif
	free(p);
}

void Context_Wnd(void* Wnd)
{
	context* p = Context();
	if (p)
	{
#ifdef REGISTRY_GLOBAL
		if (Wnd)
			NodeRegLoadGlobal();
		else
			NodeRegSaveGlobal();
#endif
		p->Wnd = Wnd; // only set after globals are loaded
		NodeSettingsChanged();
	}
}

