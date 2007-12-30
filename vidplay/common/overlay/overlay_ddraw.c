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
 * $Id: overlay_ddraw.c 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "../common.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#ifndef STRICT
#define STRICT
#endif
#include <windows.h>

#if _MSC_VER > 1000
#pragma warning( push, 3 )
#endif

#include "ddraw.h"
#include "overlay_ddraw.h"

static void Desc2Surface( const DDSURFACEDESC* Desc, video* p )
{
	memset(p,0,sizeof(video));
	p->Width = Desc->dwWidth;
	p->Height = Desc->dwHeight;
	p->Aspect = ASPECT_ONE;
	p->Direction = 0;

	if (Desc->ddpfPixelFormat.dwFlags & DDPF_FOURCC)
	{
		p->Pixel.Flags = PF_FOURCC;
		p->Pixel.FourCC = Desc->ddpfPixelFormat.dwFourCC;
	}
	else
	if (Desc->ddpfPixelFormat.dwFlags & DDPF_RGB)
	{
		p->Pixel.Flags = PF_RGB;
		p->Pixel.BitCount = Desc->ddpfPixelFormat.dwRGBBitCount;
		p->Pixel.BitMask[0] = Desc->ddpfPixelFormat.dwRBitMask;
		p->Pixel.BitMask[1] = Desc->ddpfPixelFormat.dwGBitMask;
		p->Pixel.BitMask[2] = Desc->ddpfPixelFormat.dwBBitMask;
	}
	else
	if (Desc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED1)
	{
		p->Pixel.Flags = PF_PALETTE;
		p->Pixel.BitCount = 1;
	}
	else
	if (Desc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED2)
	{
		p->Pixel.Flags = PF_PALETTE;
		p->Pixel.BitCount = 2;
	}
	else
	if (Desc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
	{
		p->Pixel.Flags = PF_PALETTE;
		p->Pixel.BitCount = 4;
	}
	else
	if (Desc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
	{
		p->Pixel.Flags = PF_PALETTE;
		p->Pixel.BitCount = 8;
	}
}

static bool_t Surface2Desc( const video* p, DDSURFACEDESC* Desc, bool_t PixelFormat )
{
	memset(Desc,0,sizeof(DDSURFACEDESC));
	Desc->dwSize = sizeof(DDSURFACEDESC);
	Desc->dwFlags = DDSD_WIDTH | DDSD_HEIGHT;

	Desc->dwWidth = p->Width;
	Desc->dwHeight = p->Height;

	if (PixelFormat)
	{
		Desc->dwFlags |= DDSD_PIXELFORMAT;
		Desc->ddpfPixelFormat.dwSize = sizeof(Desc->ddpfPixelFormat);

		if (p->Pixel.Flags & PF_FOURCC)
		{
			Desc->ddpfPixelFormat.dwFlags = DDPF_FOURCC;
			Desc->ddpfPixelFormat.dwFourCC = p->Pixel.FourCC;
		}
		else
		if (p->Pixel.Flags & PF_RGB)
		{
			Desc->ddpfPixelFormat.dwFlags = DDPF_RGB;
			Desc->ddpfPixelFormat.dwRGBBitCount = p->Pixel.BitCount;
			Desc->ddpfPixelFormat.dwRBitMask = p->Pixel.BitMask[0];
			Desc->ddpfPixelFormat.dwGBitMask = p->Pixel.BitMask[1];
			Desc->ddpfPixelFormat.dwBBitMask = p->Pixel.BitMask[2];
		}
		else
		if (p->Pixel.Flags & PF_PALETTE)
		{
			switch (p->Pixel.BitCount)
			{
			case 1: Desc->ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED1; break;
			case 2: Desc->ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED2; break;
			case 4: Desc->ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED4; break;
			case 8: Desc->ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8; break;
			}
			Desc->ddpfPixelFormat.dwRGBBitCount = p->Pixel.BitCount;
		}
		else
			return 0;
	}

	return 1;
}

static bool_t CreateBuffer(ddraw* p,bool_t PixelFormat)
{
	DDSURFACEDESC Desc;

	p->BufferPixelFormat = PixelFormat;

	if (p->DDBuffer) 
	{ 
		IDirectDrawSurface_Release(p->DDBuffer); 
		p->DDBuffer = NULL;
	}

	if (!Surface2Desc(&p->Overlay,&Desc,PixelFormat))
		return 0;

	Desc.dwFlags |= DDSD_CAPS;
	Desc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
	if (p->Mode == MODE_OVERLAY)
		Desc.ddsCaps.dwCaps |= DDSCAPS_OVERLAY;

	DEBUG_MSG5(DEBUG_VIDEO,T("DDRAW CreateSurface %08x %dx%d %08x %08x"),Desc.ddsCaps.dwCaps,Desc.dwWidth,Desc.dwHeight,Desc.ddpfPixelFormat.dwFlags,Desc.ddpfPixelFormat.dwFourCC);

	if (IDirectDraw_CreateSurface(p->DD,&Desc,&p->DDBuffer,NULL) != DD_OK)
	{
		DEBUG_MSG(DEBUG_VIDEO,T("DDRAW CreateSurface Failed"));
		return 0;
	}

	if (IDirectDrawSurface_GetSurfaceDesc(p->DDBuffer,&Desc) == DD_OK)
		Desc2Surface(&Desc,&p->Overlay);

	DEBUG_MSG(DEBUG_VIDEO,T("DDRAW CreateSurface Ok"));

	// fill with black
 
	if (IDirectDrawSurface_Lock(p->DDBuffer,NULL,&Desc,DDLOCK_WAIT,NULL) == DD_OK)
	{
		int v,x2,y2,pitch2;

		FillInfo(&p->Overlay.Pixel);
		v = RGBToFormat(0,&p->Overlay.Pixel);

		if (PlanarYUV(&p->Overlay.Pixel,&x2,&y2,&pitch2) && p->Overlay.Pixel.BitCount==8)
		{
			uint8_t* i = (uint8_t*)Desc.lpSurface;
			FillColor(i,Desc.lPitch,0,0,p->Overlay.Width,p->Overlay.Height,8,v & 255);
			i += Desc.lPitch * p->Overlay.Height;
			FillColor(i,Desc.lPitch >> pitch2,0,0,p->Overlay.Width >> x2,p->Overlay.Height >> y2,8,(v>>8)&255);
			i += (Desc.lPitch >> x2) * (p->Overlay.Height >> y2);
			FillColor(i,Desc.lPitch >> pitch2,0,0,p->Overlay.Width >> x2,p->Overlay.Height >> y2,8,(v>>16)&255);
		}
		else
		if (PackedYUV(&p->Overlay.Pixel) && p->Overlay.Pixel.BitCount==16)
			FillColor(Desc.lpSurface,Desc.lPitch,0,0,p->Overlay.Width >> 1,p->Overlay.Height,32,v);
		else
			FillColor(Desc.lpSurface,Desc.lPitch,0,0,p->Overlay.Width,p->Overlay.Height,p->Overlay.Pixel.BitCount,v);

		IDirectDrawSurface_Unlock(p->DDBuffer,NULL);
	}

	return 1;
}

static int UpdateOverlay(ddraw* p)
{
	p->Src.left = p->OverlayRect.x;
	p->Src.top = p->OverlayRect.y;
	p->Src.right = p->OverlayRect.x + p->OverlayRect.Width;
	p->Src.bottom = p->OverlayRect.y + p->OverlayRect.Height;
 
	p->Dst.left = p->p.DstAlignedRect.x;
	p->Dst.top = p->p.DstAlignedRect.y;
	p->Dst.right = p->p.DstAlignedRect.x + p->p.DstAlignedRect.Width;
	p->Dst.bottom = p->p.DstAlignedRect.y + p->p.DstAlignedRect.Height;

	if (p->Mode == MODE_OVERLAY)
	{
		DWORD hResult;
		int Flags = 0;
		DDOVERLAYFX DDFX;

		memset(&DDFX,0,sizeof(DDFX));
		DDFX.dwSize = sizeof(DDFX);

		if (p->OvlFX.Flags & (BLITFX_ARITHSTRETCH50|BLITFX_ARITHSTRETCHALWAYS))
		{
			Flags |= DDOVER_DDFX;
			DDFX.dwDDFX |= DDOVERFX_ARITHSTRETCHY;
		}

		if (p->p.Show)
			Flags |= DDOVER_SHOW;
		else
			Flags |= DDOVER_HIDE;

		if (p->p.ColorKey != -1)
			Flags |= DDOVER_KEYDEST;

		hResult = IDirectDrawSurface_UpdateOverlay(p->DDBuffer,&p->Src,p->DDPrimary,&p->Dst,Flags,&DDFX);
		if (hResult != DD_OK)
			DEBUG_MSG1(DEBUG_VIDEO,T("DDRAW UpdateOverlay failed %08x"),hResult);
	}

	return ERR_NONE;
}

static void GetMode(ddraw *p)
{
	DDSURFACEDESC Desc;
	memset(&Desc,0,sizeof(DDSURFACEDESC));
	Desc.dwSize = sizeof(DDSURFACEDESC);
	IDirectDraw_GetDisplayMode(p->DD,&Desc);
	Desc2Surface(&Desc,&p->p.Output.Video);
	FillInfo(&p->p.Output.Video.Pixel);
}

static int Init(ddraw* p)
{
	DDSURFACEDESC Desc;

	p->p.ColorKey = -1;
	if (p->SetupColorKey && IsWindow(Context()->Wnd))
		p->p.ColorKey = COLORKEY;

	if (p->DDCaps.dwCaps & DDCAPS_OVERLAYSTRETCH)
	{
		p->MinScale = (p->DDCaps.dwMinOverlayStretch * SCALE_ONE) / 1000;
		p->MaxScale = (p->DDCaps.dwMaxOverlayStretch * SCALE_ONE) / 1000;
	}
	else
	{
		p->MinScale = SCALE_ONE;
		p->MaxScale = SCALE_ONE;
	}

	if (p->DDCaps.dwCaps & DDCAPS_ALIGNBOUNDARYDEST)
		p->DstAlignPos = p->DDCaps.dwAlignBoundaryDest;
	else
		p->DstAlignPos = 1;

	if (p->DDCaps.dwCaps & DDCAPS_ALIGNSIZEDEST)
		p->DstAlignSize = p->DDCaps.dwAlignSizeDest;
	else
		p->DstAlignSize = 1;

	GetMode(p);

	// get primary surface
	memset(&Desc,0,sizeof(DDSURFACEDESC));
	Desc.dwSize = sizeof(DDSURFACEDESC);
	Desc.dwFlags = DDSD_CAPS;
	Desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (IDirectDraw_CreateSurface(p->DD,&Desc,&p->DDPrimary,NULL) != DD_OK)
		return ERR_DEVICE_ERROR;

	p->p.Overlay = 1;
	p->p.UpdateShow = UpdateOverlay;
	p->Overlay = p->p.Input.Video;
	p->Mode = MODE_OVERLAY;

	// first try the optimal overlay format
	if (!p->SetupBlit && !CreateBuffer(p,1))
	{
		if (PlanarYUV(&p->Overlay.Pixel,NULL,NULL,NULL))
		{
			// try other formats
			static const int FourCC[] = 
			{ 
				// prefer planar420 formats
				FOURCC_YV12, FOURCC_IYUV, 
				FOURCC_I420, 
				// next planar422 formats
				FOURCC_YV16,
				// next packed formats
				FOURCC_YUY2, FOURCC_YUNV, 
				FOURCC_V422, FOURCC_YUYV, 
				FOURCC_YVYU, FOURCC_UYVY, 
				FOURCC_Y422, FOURCC_UYNV,
				0 
			};
			const int* i = FourCC;

			for (;*i;++i)
			{
				p->Overlay.Pixel.Flags = PF_FOURCC;
				p->Overlay.Pixel.FourCC = *i;
				if (CreateBuffer(p,1))
					break;
			}
		}

		// last hope is the device's current RGB mode (still better as blit mode)
		if (!p->DDBuffer)
			CreateBuffer(p,0);
	}

	if (!p->DDBuffer)
	{
		// try blit mode

		p->p.Overlay = 0;
		p->p.UpdateShow = NULL;
		p->p.ColorKey = -1;
		p->DstAlignPos = 1;
		p->DstAlignSize = 1;
		p->MinScale = SCALE_ONE/256;
		p->MaxScale = SCALE_ONE*256;

		if (p->SetupBlitStretch)
		{
			p->Mode = MODE_BLIT; 
			CreateBuffer(p,0);
		}

		if (!p->DDBuffer)
		{
			p->Mode = MODE_PRIMARY; // use primary mode
			DEBUG_MSG(DEBUG_VIDEO,T("DDRAW Blit Primary Mode"));
		}
		else
			DEBUG_MSG(DEBUG_VIDEO,T("DDRAW Blit Stretch Mode"));
	}
	else
		DEBUG_MSG(DEBUG_VIDEO,T("DDRAW Overlay Mode"));

	if (p->Mode != MODE_BLIT)
		p->p.SetFX = BLITFX_AVOIDTEARING;

	return ERR_NONE;
}

static void Done(ddraw* p)
{
	if (p->DDBuffer) 
	{ 
		IDirectDrawSurface_Release(p->DDBuffer); 
		p->DDBuffer = NULL;
	}
	if (p->DDPrimary) 
	{ 
		IDirectDrawSurface_Release(p->DDPrimary); 
		p->DDPrimary = NULL;
	}
}

static int Update(ddraw* p)
{
	int OvlWidth = p->p.Input.Video.Width;
	int OvlHeight = p->p.Input.Video.Height;

	if (p->Mode == MODE_PRIMARY)
		return OverlayUpdateAlign(&p->p);

	p->OvlFX = p->p.FX;
	p->SoftFX = p->p.FX;

	p->OvlFX.Brightness = 0;
	p->OvlFX.Contrast = 0;
	p->OvlFX.Saturation = 0;
	p->OvlFX.RGBAdjust[0] = p->OvlFX.RGBAdjust[1] = p->OvlFX.RGBAdjust[2] = 0;
	p->OvlFX.Direction &= ~(DIR_SWAPXY|DIR_MIRRORLEFTRIGHT|DIR_MIRRORUPDOWN); //rotate handled by SoftFX

	p->SoftFX.ScaleX = SCALE_ONE; // scale handled by overlay or blit
	p->SoftFX.ScaleY = SCALE_ONE;

	if ((p->Mode == MODE_BLIT && !(p->DDCaps.dwFXCaps & DDFXCAPS_BLTARITHSTRETCHY)) ||
	    (p->Mode == MODE_OVERLAY && !(p->DDCaps.dwFXCaps & DDFXCAPS_OVERLAYARITHSTRETCHY)))
		p->OvlFX.Flags &= ~(BLITFX_ARITHSTRETCH50|BLITFX_ARITHSTRETCHALWAYS);

	if (p->SoftFX.Direction & DIR_SWAPXY)
		SwapInt(&p->OvlFX.ScaleX,&p->OvlFX.ScaleY);

	if ((p->p.OrigFX.Direction ^ p->p.InputDirection) & DIR_SWAPXY)
		SwapInt(&OvlWidth,&OvlHeight);

	if (p->Overlay.Width != OvlWidth ||	p->Overlay.Height != OvlHeight)
	{
		p->Overlay.Width = OvlWidth;
		p->Overlay.Height = OvlHeight;
		CreateBuffer(p,p->BufferPixelFormat);
	}

	VirtToPhy(&p->p.Viewport,&p->p.DstAlignedRect,&p->p.Output.Video);
	VirtToPhy(NULL,&p->OverlayRect,&p->Overlay);

	AnyAlign(&p->p.DstAlignedRect, &p->OverlayRect, &p->OvlFX,
		p->DstAlignSize,p->DstAlignPos,p->MinScale,p->MaxScale);

	PhyToVirt(&p->p.DstAlignedRect,&p->p.GUIAlignedRect,&p->p.Output.Video);

	VirtToPhy(NULL,&p->p.SrcAlignedRect,&p->p.Input.Video);

	BlitRelease(p->p.Soft);
	p->p.Soft = BlitCreate(&p->Overlay,&p->p.Input.Video,&p->SoftFX,&p->p.Caps);

	BlitAlign(p->p.Soft,&p->OverlayRect,&p->p.SrcAlignedRect);

	if (p->p.ColorKey!=-1 && p->DDPrimary)
	{ 
		DWORD hResult;
		DDCOLORKEY Key;

		Key.dwColorSpaceLowValue = RGBToFormat(p->p.ColorKey,&p->p.Output.Video.Pixel);
		Key.dwColorSpaceHighValue = 0;

		WinInvalidate(&p->p.Viewport,1);

		if ((hResult = IDirectDrawSurface_SetColorKey(p->DDPrimary,DDCKEY_DESTOVERLAY,&Key))!=DD_OK)
		{
			DEBUG_MSG1(DEBUG_VIDEO,T("DDRAW SetColorKey failed %08x"),hResult);
			return ERR_NOT_SUPPORTED;
		}
	}

	return UpdateOverlay(p);
}

static int Blit(ddraw* p, const constplanes Data, const constplanes DataLast )
{
	DDSURFACEDESC Desc;
	HRESULT hResult;
	planes Planes;
	LPDIRECTDRAWSURFACE Output = p->Mode == MODE_PRIMARY ? p->DDPrimary:p->DDBuffer;

	if (!Output)
		return ERR_NOT_SUPPORTED;

	Desc.dwSize = sizeof(Desc);
	while ((hResult = IDirectDrawSurface_Lock(Output,NULL,&Desc,DDLOCK_WAIT,NULL)) != DD_OK)
		if (hResult != DDERR_SURFACELOST || IDirectDrawSurface_Restore(Output) != DD_OK)
			return ERR_NOT_SUPPORTED;

	Planes[0] = Desc.lpSurface;

	BlitImage(p->p.Soft,Planes,Data,DataLast,Desc.lPitch,-1);

	IDirectDrawSurface_Unlock(Output,NULL);

	if (p->Mode == MODE_BLIT)
	{
		while ((hResult = IDirectDrawSurface_Blt(p->DDPrimary,&p->Dst,Output,&p->Src,DDBLT_ASYNC,NULL)) != DD_OK)
			if (hResult != DDERR_SURFACELOST || IDirectDrawSurface_Restore(p->DDPrimary) != DD_OK)
				break;
	}

	return ERR_NONE;
}

static const datatable Params[] = 
{
	{ DDRAW_COLORKEY,		TYPE_BOOL, DF_SETUP|DF_CHECKLIST },
	{ DDRAW_BLIT,			TYPE_BOOL, DF_SETUP|DF_CHECKLIST },
	{ DDRAW_BLITSTRETCH,	TYPE_BOOL, DF_SETUP|DF_CHECKLIST },

	DATATABLE_END(DDRAW_ID)
};

static int Enum(ddraw* p, int* No, datadef* Param)
{
	// same for ce and win32 version
	if (OverlayEnum(&p->p,No,Param)==ERR_NONE)
		return ERR_NONE;
	return NodeEnumTable(No,Param,Params);
}

static int Get(ddraw* p,int No,void* Data,int Size)
{
	// same for ce and win32 version
	int Result = OverlayGet(&p->p,No,Data,Size);
	switch (No)
	{
	case DDRAW_COLORKEY: GETVALUE(p->SetupColorKey,bool_t); break;
	case DDRAW_BLIT: GETVALUE(p->SetupBlit,bool_t); break;
	case DDRAW_BLITSTRETCH: GETVALUE(p->SetupBlitStretch,bool_t); break;
	}
	return Result;
}

static int ReInit(ddraw* p)
{
	// same for ce and win32 version
	if (p->p.Inited)
	{
		player* Player;

		p->p.Done(p);
		p->p.Init(p);
		OverlayUpdateFX(&p->p,1);

		if ((Player = (player*)Context()->Player) != NULL)
			Player->Set(Player,PLAYER_UPDATEVIDEO,NULL,0);

	}
	return ERR_NONE;
}

static int Set(ddraw* p,int No,const void* Data,int Size)
{
	// same for ce and win32 version
	int Result = OverlaySet(&p->p,No,Data,Size);
	switch (No)
	{
	case NODE_CRASH: p->p.Done(p); break;
	case DDRAW_COLORKEY: SETVALUE(p->SetupColorKey,bool_t,ReInit(p)); break;
	case DDRAW_BLIT: SETVALUE(p->SetupBlit,bool_t,ReInit(p)); break;
	case DDRAW_BLITSTRETCH: SETVALUE(p->SetupBlitStretch,bool_t,ReInit(p)); break;
	}
	return Result;
}

static int Create(ddraw* p)
{
	LPDIRECTDRAW DD = NULL;
	HRESULT (WINAPI* DirectDrawCreate)( void*, LPDIRECTDRAW*, void* ) = NULL;

	p->p.Node.Enum = (nodeenum)Enum;
	p->p.Node.Get = (nodeget)Get;
	p->p.Node.Set = (nodeset)Set;

	p->p.Module = LoadLibrary(T("DDRAW.DLL"));
	GetProc(&p->p.Module,&DirectDrawCreate,T("DirectDrawCreate"),0);

	if (!p->p.Module || DirectDrawCreate(NULL,&DD,NULL)!=DD_OK ||
		IDirectDraw_QueryInterface(DD,&IID_IDirectDraw,&p->DD)!=DD_OK)
	{
		if (DD && DDrawCECreate(p,DD)==ERR_NONE)
			return ERR_NONE;

		if (DD) IDirectDraw_Release(DD);
		if (p->p.Module) { FreeLibrary(p->p.Module); p->p.Module = NULL; }
		return ERR_DEVICE_ERROR;
	}

	IDirectDraw_Release(DD);
	IDirectDraw_SetCooperativeLevel(p->DD, NULL, DDSCL_NORMAL);

	p->DDCaps.dwSize = sizeof(p->DDCaps);
	IDirectDraw_GetCaps(p->DD,&p->DDCaps,NULL);

	//{ int i; for (i=0;i<sizeof(p->DDCaps)/4;++i)
	//	DEBUG_MSG(DEBUG_VIDEO,T("DDRAW Caps %02x:%08x"),i*4,((int*)&p->DDCaps)[i]); }

	p->SetupColorKey = (p->DDCaps.dwCaps & DDCAPS_COLORKEY) != 0;
	p->SetupBlit = (p->DDCaps.dwCaps & DDCAPS_OVERLAY) == 0;
	p->SetupBlitStretch = (p->DDCaps.dwCaps & DDCAPS_BLTSTRETCH) != 0;

	p->p.DoPowerOff = 1;
	p->p.Init = Init;
	p->p.Done = Done;
	p->p.Blit = Blit;
	p->p.Update = Update;
	p->p.UpdateShow = UpdateOverlay;
	return ERR_NONE;
}

static void Delete(ddraw* p)
{
	// same for ce and win32 version
	if (p->DD)
		IDirectDraw_Release(p->DD);
}

static const nodedef DDraw = 
{
	sizeof(ddraw)|CF_GLOBAL|CF_SETTINGS,
	DDRAW_ID,
	OVERLAY_CLASS,
	PRI_DEFAULT+100,
	(nodecreate)Create,
	(nodedelete)Delete,
};

void OverlayDDraw_Init() 
{ 
	NodeRegisterClass(&DDraw);
}

void OverlayDDraw_Done()
{
	NodeUnRegisterClass(DDRAW_ID);
}

#endif
