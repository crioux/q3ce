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
 * $Id: video.h 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __VIDEO_H
#define __VIDEO_H

//---------------------------------------------------------------
//pixelformat flags
#define	PF_FOURCC		0x0001
#define	PF_PALETTE		0x0002
#define	PF_RGB			0x0004
#define	PF_YUV			0x0008
#define	PF_YUV420		0x0010
#define	PF_INVERTED		0x0020
#define PF_READONLY		0x0040
#define PF_FRAGMENTED	0x0080
#define PF_PIXELDOUBLE	0x0100
#define PF_16BITACCESS	0x0200
#define PF_16ALIGNED	0x0400
#define PF_SAFEBORDER	0x0800
#define	PF_YUV422		0x1000
#define	PF_YUV444		0x2000

//---------------------------------------------------------------
//planar YUV420 formats
#define	FOURCC_YV12 	FOURCC('Y','V','1','2')
#define	FOURCC_IYUV 	FOURCC('I','Y','U','V')
#define	FOURCC_I420 	FOURCC('I','4','2','0')
#define	FOURCC_IMC2 	FOURCC('I','M','C','2')
#define	FOURCC_IMC4 	FOURCC('I','M','C','4')

//planar YUV422 formats
#define	FOURCC_YV16 	FOURCC('Y','V','1','6')
//---------------------------------------------------------------
//packed YUV formats
#define	FOURCC_YUY2 	FOURCC('Y','U','Y','2')
#define	FOURCC_YUNV 	FOURCC('Y','U','N','V')
#define	FOURCC_V422 	FOURCC('V','4','2','2')
#define	FOURCC_YUYV 	FOURCC('Y','U','Y','V')
#define	FOURCC_YVYU 	FOURCC('Y','V','Y','U')
#define	FOURCC_UYVY 	FOURCC('U','Y','V','Y')
#define	FOURCC_Y422		FOURCC('Y','4','2','2')
#define	FOURCC_UYNV		FOURCC('U','Y','N','V')
#define	FOURCC_VYUY 	FOURCC('V','Y','U','Y')

//---------------------------------------------------------------
//direction flags
#define	DIR_SWAPXY			0x001
#define	DIR_MIRRORLEFTRIGHT	0x002
#define	DIR_MIRRORUPDOWN	0x004

//---------------------------------------------------------------
//vide caps
#define	VC_DITHER		0x0001
#define	VC_BRIGHTNESS	0x0002
#define	VC_CONTRAST		0x0004
#define	VC_SATURATION	0x0008
#define	VC_RGBADJUST	0x0010

#define SCALE_ONE	(1<<16)
#define ASPECT_ONE	(1<<16)

typedef struct rgb
{
	unsigned char r,g,b,a;

} rgb;

typedef struct point
{
	int x;
	int y;

} point;

typedef struct rect
{
	int x;
	int y;
	int Width;
	int Height;

} rect;

typedef struct pixel
{
	int Flags;
	int FourCC;
	int BitCount;
	unsigned int BitMask[3];
	rgb *Palette;

} pixel;

typedef struct video
{
	int Direction;	// direction flags
	int Aspect;		// 16.16 fixed point
	int Width;		// phyisical width of surface
	int Height;		// phyisical height of surface
	int Pitch;
	pixel Pixel;

} video;

//---------------------------------------------------------------

#define VBUFFER_ID			FOURCC('V','B','U','F')

//---------------------------------------------------------------
// video output (abstract)

#define VOUT_CLASS			FOURCC('V','O','U','T')
#define VOUT_IDCT_CLASS(n)	INT32BE((INT32BE(n) & ~0xFF)|'I')

// primary display (bool_t readonly)
#define VOUT_PRIMARY		0x70
// hardware accelerated idct (idct* readonly)
#define VOUT_IDCT			0x74
// allow showahead display or not
#define VOUT_PLAY			0x7D
// blit fx options (blitfx)
#define VOUT_FX				0x75
// window handle (int)
#define VOUT_WND			0x76
// overlay visible (bool_t)
#define VOUT_VISIBLE		0x77
// gui clipping needed (bool_t)
#define VOUT_CLIPPING		0x7F
// viewport rectangle (rect)
#define VOUT_VIEWPORT		0x78
// actuall output rectangle (rect readonly)
#define VOUT_OUTPUTRECT		0x79
// color key (rgb_t)
#define VOUT_COLORKEY		0x7A
// prerotate portrait input
#define VOUT_AUTOPREROTATE	0x7C
// aspect ratio
#define VOUT_ASPECT			0x81
// reset
#define VOUT_RESET			0x80
// voutput caps
#define VOUT_CAPS			0x84
// begin/end update (bool_t)
#define VOUT_UPDATING		0x85
// overlay display (bool_t readonly)
#define VOUT_OVERLAY		0x86
// fullscren				
#define VOUT_FULLSCREEN		0x87

#define VOUT_ERROR_SIZE		0x100

void Video_Init();
void Video_Done();

int VOutEnum(void* p, int* No, datadef* Param);
DLL bool_t VOutIDCT(int Class);

#endif
