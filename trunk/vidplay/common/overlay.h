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
 * $Id: overlay.h 142 2004-12-12 07:30:52Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __OVERLAY_H
#define __OVERLAY_H

// default colorkey to use
#define COLORKEY	0x200020

#define OVERLAY_CLASS		FOURCC('O','V','L','A')
#define DDRAW_ID			FOURCC('D','R','A','W')
#define DIRECT_ID			FOURCC('D','I','R','C')
#define FLYTV_ID			FOURCC('F','L','Y','T')
#define GAPI_ID				FOURCC('G','A','P','I')
#define GDI_ID				FOURCC('G','D','I','_')
#define RAW_ID				FOURCC('R','A','W','D')
#define XSCALE_ID			FOURCC('X','S','C','2')
#define XSCALE_LOW_ID		FOURCC('X','S','L','2')
#define HIRES_ID			FOURCC('H','I','R','S')

#define DDRAW_COLORKEY		0x100
#define DDRAW_BLIT			0x101
#define DDRAW_BLITSTRETCH	0x102

#define RAW_WIDTH		0x100
#define RAW_HEIGHT		0x101
#define RAW_PITCHX		0x102
#define RAW_PITCHY		0x103
#define RAW_BPP			0x104
#define RAW_FORMAT		0x105
#define RAW_POINTER		0x106

#define GAPI_WIDTH		0x100
#define GAPI_HEIGHT		0x101
#define GAPI_PITCHX		0x102
#define GAPI_PITCHY		0x103
#define GAPI_BPP		0x104
#define GAPI_FORMAT		0x105
#define GAPI_POINTER	0x106
#define GAPI_DRAM		0x107
#define GAPI_OPEN_ERROR	0x200

#define HIRES_TRIPLEBUFFER			0x100
#define HIRES_USEBORDER				0x101
#define HIRES_USEBORDER_INFO		0x102

#define XSCALEDRIVER_ID				FOURCC('X','S','C','D')
#define XSCALEDRIVER_STRETCH		0x100
#define XSCALEDRIVER_FLIP			0x101
#define XSCALEDRIVER_ALLOC_AT_START	0x102


typedef int (*ovlfunc)(void* This);
typedef void (*ovldone)(void* This);
typedef int (*ovlblit)(void* This,const constplanes Data,const constplanes LastData);
typedef int (*ovllock)(void* This,planes Planes, bool_t OnlyAligned);

typedef struct overlay
{
	node Node;

	ovlfunc Init;
	ovldone Done;
	ovlblit Blit;
	ovlfunc Update;
	ovlfunc UpdateShow;
	ovlfunc Reset;
	ovllock Lock;
	ovlfunc Unlock;

	idct* AccelIDCT;
	int SetFX;	
	int ClearFX;
	int Caps;
	packetformat Output;

	// these are handled by overlay
	bool_t DoPowerOff; // use backup/restore in background
	bool_t Primary;
	bool_t Overlay;
	bool_t Disabled; // created with width=0 or height=0
	bool_t Inited;
	bool_t Background;
	bool_t Updating;
	bool_t ViewportChanged; //during Updating
	bool_t FullScreenViewport;
	bool_t Dirty;
	bool_t Visible;
	bool_t Clipping;
	bool_t AutoPrerotate;
	bool_t Show;
	bool_t InputHintDir;
	bool_t Play;
	bool_t TurningOff;
	tick_t LastTime;
	tick_t CurrTime;
	int Total;
	int Dropped;
	int32_t ColorKey;
	void* Soft;
	rect Viewport;
	rect SrcAlignedRect;
	rect GUIAlignedRect;
	rect DstAlignedRect;
	blitfx FX;
	blitfx OrigFX;
	fraction Aspect;
	bool_t PreRotate;
	int InputDirection;
	int PrefDirection;
	bool_t UpdateInputFormat;
	void* Module;
	pin Pin;
	packetformat Input;
	idctbackup Backup;
	int ErrorTime;

} overlay;

DLL int OverlayEnum(overlay* p,int* No,datadef* DataType);
DLL int OverlayGet(overlay* p,int No,void* Data,int Size);
DLL int OverlaySet(overlay* p,int No,const void* Data,int Size);
DLL int OverlayUpdateAlign(overlay* p);
DLL int OverlayUpdateShow(overlay* p, bool_t Temp);
DLL int OverlayBlitImage(overlay* p, const planes Data, const planes DataLast);
DLL int OverlayUpdateFX(overlay* p, bool_t ForceUpdate);

void Overlay_Init();
void Overlay_Done();

#endif
