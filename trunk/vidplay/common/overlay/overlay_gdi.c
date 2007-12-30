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
 * $Id: overlay_gdi.c 200 2005-01-14 22:09:26Z picard $
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

typedef struct gdi
{
	overlay p;
	int DIBSection;
	HBITMAP Bitmap;
	HGDIOBJ Bitmap0; //bitmap returned by selectobject
	HDC DC2;
	video Overlay;
	rect OverlayRect;
	planes Planes;
} gdi;

static int AllocBitmap(gdi* p)
{
	int OldFlags = p->Overlay.Pixel.Flags;
	p->p.Dirty = 1;

	if (p->DIBSection)
	{
		int i;
		struct 
		{
			BITMAPINFOHEADER Head;
			int BitMask[3];
		} Info;
		HDC DC = GetDC(NULL);

		memset(&Info,0,sizeof(Info));
		Info.Head.biSize = sizeof(Info.Head);
		Info.Head.biWidth = p->Overlay.Width;
		Info.Head.biHeight = -p->Overlay.Height;
		Info.Head.biPlanes = 1;
		Info.Head.biBitCount = (WORD)p->Overlay.Pixel.BitCount;
		Info.Head.biCompression = BI_BITFIELDS;
		Info.Head.biSizeImage = (p->Overlay.Width * p->Overlay.Height * p->Overlay.Pixel.BitCount) >> 3;
		for (i=0;i<3;++i)
			Info.BitMask[i] = p->Overlay.Pixel.BitMask[i];

		p->Bitmap = CreateDIBSection(DC,(BITMAPINFO*)&Info,DIB_RGB_COLORS,&p->Planes[0],NULL,0);

		if (p->Bitmap)
			p->Bitmap0 = SelectObject(p->DC2,p->Bitmap);

		ReleaseDC(NULL,DC);
	}
	else
		p->Planes[0] = Alloc16(p->Overlay.Pitch * p->Overlay.Height);

	if (((int)p->Planes[0] & 15) || (p->Overlay.Pitch & 15))
		p->Overlay.Pixel.Flags &= ~PF_16ALIGNED;
	else
		p->Overlay.Pixel.Flags |= PF_16ALIGNED;

	if (OldFlags != p->Overlay.Pixel.Flags)
	{
		BlitRelease(p->p.Soft);
		p->p.Soft = BlitCreate(&p->Overlay,&p->p.Input.Video,&p->p.FX,&p->p.Caps);
		BlitAlign(p->p.Soft, &p->OverlayRect, &p->p.SrcAlignedRect);
	}

	return p->Planes[0] != NULL;
}

static void FreeBitmap(gdi* p)
{
	if (p->Bitmap) 
	{ 
		SelectObject(p->DC2,p->Bitmap0);
		DeleteObject(p->Bitmap); 
		p->Bitmap = 0;
	}

	if (p->Planes[0])
	{
		if (!p->DIBSection)
			Free16(p->Planes[0]);

		p->Planes[0] = NULL;
	}
}

static void Done(gdi* p)
{
	FreeBitmap(p);

	if (p->DC2) 
	{
		DeleteDC(p->DC2); 
		p->DC2 = 0;
	}
}

static int Init(gdi* p)
{
	HDC DC;
	QueryDesktop(&p->p.Output.Video);

	p->Planes[0] = NULL;

	p->Overlay.Width = 0;
	p->Overlay.Height = 0;
	p->Overlay.Direction = 0;
	p->Overlay.Aspect = ASPECT_ONE;
	p->Overlay.Pixel = p->p.Output.Video.Pixel;
	p->Overlay.Pixel.Flags |= PF_16ALIGNED; // assume it will be aligned

	p->DIBSection = (p->Overlay.Pixel.Flags & PF_RGB) != 0;
	p->Bitmap = 0;
	p->Bitmap0 = 0;

	DC = GetDC(NULL);
	p->DC2 = CreateCompatibleDC(DC);
	ReleaseDC(NULL,DC);

	p->p.ClearFX = BLITFX_ONLYDIFF;
	return ERR_NONE;
}

static int Reset(gdi* p)
{
	Done(p);
	Init(p);
	return ERR_NONE;
}

static int Update(gdi* p)
{
	rect OldGUI = p->p.GUIAlignedRect;
	rect Old = p->p.DstAlignedRect;

	int OldWidth = p->Overlay.Width;
	int OldHeight = p->Overlay.Height;

	VirtToPhy(&p->p.Viewport,&p->p.DstAlignedRect,&p->p.Output.Video);
	VirtToPhy(NULL,&p->p.SrcAlignedRect,&p->p.Input.Video);

	AnyAlign(&p->p.DstAlignedRect, &p->p.SrcAlignedRect, &p->p.FX, 2, 2, 1, SCALE_ONE*1024 ); 

	PhyToVirt(&p->p.DstAlignedRect,&p->p.GUIAlignedRect,&p->p.Output.Video);

	p->Overlay.Width = ALIGN16(p->p.GUIAlignedRect.Width); // we need PF_16ALIGNED for pitch
	p->Overlay.Height = p->p.GUIAlignedRect.Height;
	p->Overlay.Pitch = (p->Overlay.Width * p->Overlay.Pixel.BitCount) >> 3;
	p->Overlay.Pitch = (p->Overlay.Pitch+1) & ~1; //word aligned (it doesn't mater anymore, Width is aligned)

	p->OverlayRect.x = p->OverlayRect.y = 0;
	p->OverlayRect.Width = p->p.GUIAlignedRect.Width;
	p->OverlayRect.Height = p->p.GUIAlignedRect.Height;

	BlitRelease(p->p.Soft);
	p->p.Soft = BlitCreate(&p->Overlay,&p->p.Input.Video,&p->p.FX,&p->p.Caps);
	BlitAlign(p->p.Soft, &p->OverlayRect, &p->p.SrcAlignedRect);

	p->p.GUIAlignedRect.x += p->OverlayRect.x;
	p->p.GUIAlignedRect.y += p->OverlayRect.y;
	p->p.GUIAlignedRect.Width = p->OverlayRect.Width;
	p->p.GUIAlignedRect.Height = p->OverlayRect.Height;

	VirtToPhy(&p->p.GUIAlignedRect,&p->p.DstAlignedRect,&p->p.Output.Video);

	if (OldWidth != p->Overlay.Width || OldHeight != p->Overlay.Height)
		FreeBitmap(p);

	if (p->p.Show && !EqRect(&Old,&p->p.DstAlignedRect))
	{
		WinInvalidate(&OldGUI,0);
		WinInvalidate(&p->p.Viewport,1);
		WinValidate(&p->p.GUIAlignedRect);
	}
	return ERR_NONE;
}

static int Blit(gdi* p, const constplanes Data, const constplanes DataLast )
{
	HDC DC;

	if (!p->Planes[0] && !AllocBitmap(p))
		return ERR_OUT_OF_MEMORY;

	BlitImage(p->p.Soft,p->Planes,Data,DataLast,-1,-1);

	if (!p->DIBSection)
	{
		if (p->Bitmap)
		{
			SelectObject(p->DC2,p->Bitmap0);
			DeleteObject(p->Bitmap);
		}

		p->Bitmap = CreateBitmap( p->Overlay.Width, p->Overlay.Height, 1, 
			p->Overlay.Pixel.BitCount, (char*)p->Planes[0]);

		if (!p->Bitmap)
			return ERR_OUT_OF_MEMORY;

		p->Bitmap0 = SelectObject(p->DC2,p->Bitmap);
	}

	DC = GetDC(NULL);
	BitBlt(DC,p->p.GUIAlignedRect.x,p->p.GUIAlignedRect.y,
		   p->OverlayRect.Width,p->OverlayRect.Height,p->DC2,p->OverlayRect.x,p->OverlayRect.y,SRCCOPY);
	ReleaseDC(NULL,DC);

	return ERR_NONE;
}

static int Create(gdi* p)
{
	p->p.Init = Init;
	p->p.Done = Done;
	p->p.Blit = Blit;
	p->p.Update = Update;
	p->p.Reset = Reset;
	return ERR_NONE;
}

static const nodedef GDI = 
{
	sizeof(gdi)|CF_GLOBAL,
	GDI_ID,
	OVERLAY_CLASS,
	PRI_DEFAULT,
	(nodecreate)Create,
};

void OverlayGDI_Init() 
{ 
	NodeRegisterClass(&GDI);
}

void OverlayGDI_Done() 
{ 
	NodeUnRegisterClass(GDI_ID);
}

#endif
