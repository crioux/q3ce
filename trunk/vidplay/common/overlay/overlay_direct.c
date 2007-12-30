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
 * $Id: overlay_direct.c 157 2005-01-02 00:39:09Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "../common.h"

#if defined(TARGET_WINCE)

typedef struct direct
{
	overlay Overlay;
	planes Planes;
	bool_t KernelMode;
	void* PhyFrameBuffer;

} direct;

static void* LockFrameBuffer(void** PhyFrameBuffer,video* p)
{
	video Desktop;
	void* Ptr = NULL;

	memset(p,0,sizeof(video));

	//set default values (for most of the models)
	p->Width = 240;
	p->Height = 320;
	p->Pitch = 480;
	p->Aspect = ASPECT_ONE;
	p->Direction = 0;

	if (QueryPlatform(PLATFORM_CAPS) & CAPS_ONLY12BITRGB)
		DefaultRGB(&p->Pixel,16,4,4,4,1,2,1);
	else
		DefaultRGB(&p->Pixel,16,5,6,5,0,0,0);

	switch (QueryPlatform(PLATFORM_MODEL))
	{
	case MODEL_BSQUARE_ALCHEMY:
		Ptr = *PhyFrameBuffer = PhyMemBegin(0x15000000,320*480,1);
		p->Width = 240;
		p->Height = 320;
		p->Pitch = 480;
		break;

	case MODEL_IPAQ_H3600:
	case MODEL_IPAQ_H3700:
		p->Direction = DIR_SWAPXY | DIR_MIRRORLEFTRIGHT;
		Ptr = (void*)0xAC050020;
		p->Width = 320;
		p->Height = 240;
		p->Pitch = 640;
		break;

	case MODEL_IPAQ_H3800:
		p->Direction = DIR_SWAPXY | DIR_MIRRORUPDOWN;
		Ptr = (void*)0xAC050020;
		p->Width = 320;
		p->Height = 240;
		p->Pitch = 640;
		break;

	case MODEL_COMPAQ_AERO_1500:
		QueryDesktop(&Desktop);
		Ptr = (void*)0xAA000000;
		p->Direction = DIR_SWAPXY | DIR_MIRRORLEFTRIGHT;
		p->Width = 320;
		p->Height = 240;
		p->Pixel.Flags = PF_PALETTE|PF_INVERTED;
		p->Pixel.BitCount = Desktop.Pixel.BitCount;
		p->Pitch = (p->Width * p->Pixel.BitCount) >> 3;
		break;

	case MODEL_CASIO_BE300:
	case MODEL_CASIO_E105:
	case MODEL_CASIO_E115:
		Ptr = (void*)0xAA200000;
		p->Pitch = 512;
		break;

	case MODEL_JORNADA_710:
	case MODEL_JORNADA_720:
		Ptr = *PhyFrameBuffer = PhyMemBegin(0x48200000,240*1280,1);
		p->Width = 640;
		p->Height = 240;
		p->Pitch = 1280;
		break;

	case MODEL_JORNADA_680:
	case MODEL_JORNADA_690: 
		Ptr = (void*)0xB2000000; //only japan???
		p->Width = 640;
		p->Height = 240;
		p->Pitch = 1280;
		break;

	case MODEL_MOBILEGEAR2_550:
	case MODEL_MOBILEGEAR2_450:
	case MODEL_MOBILEGEAR2_530:
	case MODEL_MOBILEGEAR2_430:
	case MODEL_MOBILEPRO_780:
		Ptr = (void*)0xAA180100;
		p->Width = 640;
		p->Height = 240;
		p->Pitch = 1280;
		break;

	case MODEL_MOBILEPRO_770:
		Ptr = (void*)0xAA000000;
		p->Width = 640;
		p->Height = 240;
		p->Pitch = 1600;
		break;

	case MODEL_JVC_C33:
		Ptr = (void*)0x445C0000;
		p->Width = 1024;
		p->Height = 600;
		p->Pitch = 2048;
		break;

	case MODEL_SIGMARION:
		Ptr = (void*)0xAA000000;
		p->Width = 640;
		p->Height = 240;
		p->Pitch = 1280;
		break;

	case MODEL_SIGMARION2:
		Ptr = (void*)0xB0800000;
		p->Width = 640;
		p->Height = 240;
		p->Pitch = 1280;
		break;

	case MODEL_SIGMARION3:
		Ptr = *PhyFrameBuffer = PhyMemBegin(0x14800000,480*1600,1);
		p->Width = 800;
		p->Height = 480;
		p->Pitch = 1600;
		break;

	case MODEL_INTERMEC_6651:
		Ptr = *PhyFrameBuffer = PhyMemBegin(0x6C000000,480*1600,1);
		p->Width = 800;
		p->Height = 480;
		p->Pitch = 1600;
		break;
	}

	if (((int)Ptr & 15)==0 && (p->Pitch & 15)==0)
		p->Pixel.Flags |= PF_16ALIGNED;
	return Ptr;
}

static void UnlockFrameBuffer(void** PhyFrameBuffer)
{
	if (*PhyFrameBuffer)
	{
		PhyMemEnd(*PhyFrameBuffer);
		*PhyFrameBuffer = NULL;
	}
}

static int Init(direct* p)
{
	p->Planes[0] = LockFrameBuffer(&p->PhyFrameBuffer,&p->Overlay.Output.Video);
	if (!p->Planes[0])
		return ERR_NOT_SUPPORTED;
	AdjustOrientation(&p->Overlay.Output.Video,0);
	p->Overlay.SetFX = BLITFX_AVOIDTEARING;
	return ERR_NONE;
}

static void Done(direct* p)
{
	UnlockFrameBuffer(&p->PhyFrameBuffer);
}

static int Reset(direct* p)
{
	Done(p);
	Init(p);
	return ERR_NONE;
}

static int Lock(direct* p, planes Planes, bool_t OnlyAligned)
{
	p->KernelMode = KernelMode(1);
	Planes[0] = p->Planes[0];
	return ERR_NONE;
}

static int Unlock(direct* p)
{
	KernelMode(p->KernelMode);
	return ERR_NONE;
}

static int Create(direct* p)
{
	if (Init(p) != ERR_NONE) // check if supported
		return ERR_NOT_SUPPORTED;
	Done(p);

	p->Overlay.Init = (ovlfunc)Init;
	p->Overlay.Done = (ovldone)Done;
	p->Overlay.Reset = (ovlfunc)Reset;
	p->Overlay.Lock = (ovllock)Lock;
	p->Overlay.Unlock = (ovlfunc)Unlock;
	return ERR_NONE;
}

static const nodedef Direct = 
{
	sizeof(direct)|CF_GLOBAL,
	DIRECT_ID,
	OVERLAY_CLASS,
	PRI_DEFAULT+90,
	(nodecreate)Create,
};

void OverlayDirect_Init()
{ 
	NodeRegisterClass(&Direct);
}

void OverlayDirect_Done()
{
	NodeUnRegisterClass(DIRECT_ID);
}

#endif
