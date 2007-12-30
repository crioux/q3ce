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
 * $Id: blit_soft.h 157 2005-01-02 00:39:09Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __BLIT_SOFT_H
#define __BLIT_SOFT_H

#define LOOKUP_FIX		4

struct blit_soft;
typedef void (*blitsoftentry)(struct blit_soft* This, 
							  uint8_t** DstPtr,uint8_t** SrcPtr,
							  int DstPitch,int SrcPitch,
							  int Width,int Height,int Src2SrcLast);

typedef struct blit_soft
{
	blitsoftentry Entry;
	void* LookUp_Data; //sh3 need it somewhere in the begining

	dyncode Code;
	pixel Dst;
	pixel Src;
	blitfx FX;

	int SrcBPP;
	int SrcBPP2;
	int DstBPP;
	int DstBPP2;
	bool_t OnlyDiff;
	int SrcType;
	int DstType;
	bool_t ArithStretch;
	bool_t SwapXY;
	bool_t Slices;
	bool_t SlicesReverse;
	bool_t DstLeftRight;
	bool_t DstUpDown;
	bool_t SrcUpDown;

	int Caps;
	int SrcAlignPos;
	int DstAlignPos;
	int DstAlignSize;

	int SrcUVX2;
	int SrcUVY2;
	int SrcUVPitch2;
	int DstUVX2;
	int DstUVY2;
	int DstUVPitch2;
	int DirX; // destination space
	int DstSize[3];
	int DstPos[3];
	int SrcSize[3];
	int SrcPos[3];
	int RScaleX; // reverse scale, unit=16
	int RScaleY; // reverse scale, unit=16
	int DitherSize;
	const rgb* SrcPalette;
	const rgb* DstPalette;

	int SrcHalfX;
	int SrcHalfY;
	int SrcDoubleX;
	int SrcDoubleY;
	int DstHalfX;
	int DstHalfY;
	int DstDoubleX;
	int DstDoubleY;

	bool_t ARM5;
	bool_t QAdd;
	bool_t WMMX;
	bool_t ColorLookup;
	int DstStepX;

	bool_t Dither;
	bool_t RealOnlyDiff;
	bool_t HalfX;
	bool_t HalfY;
	bool_t DoubleX;
	bool_t DoubleY;
	bool_t Scale;
	int Double;
	bool_t OnlyDiff2;// adjusted OnlyDiff (diff mode not always available)

	dyninst* PalPtr;
	dyninst* DiffMask;
	dyninst* InvertMask;
	dyninst* Skip;
	dyninst* YMul;
	dyninst* GAdd;
	dyninst* RAdd;
	dyninst* BAdd;
	dyninst* GVMul;
	dyninst* RVMul;
	dyninst* GUMul;
	dyninst* BUMul;

	dyninst* LookUp;
	int LookUp_U;
	int LookUp_V;
	int LookUp_Size;

	int _YMul;
	int _GAdd;
	int _RAdd;
	int _BAdd;
	int _GVMul;
	int _RVMul;
	int _GUMul;
	int _BUMul;
	bool_t IsYNextRow;
	bool_t IsUVNextRow;

	bool_t Upper;
	int Pos;
	int LookUpOfs;

} blit_soft;

bool_t BlitCompile(blit_soft* This,
			  	   const video* Dst,const video* Src,
				   const blitfx* FX,bool_t OnlyDiff);

void BlitUniversal(blit_soft*,uint8_t** Dst,uint8_t** Src,int DstPitch,int SrcPitch,
                   int Width,int Height,int Src2SrcLast);

void Stretch_RGB_UV(blit_soft* This);
void Double_RGB_UV(blit_soft* This);
void Fix_RGB_UV(blit_soft* This);
void Fix_Gray_UV(blit_soft* This);
void Half_RGB_UV(blit_soft* This);
void Universal_RGB_UV(blit_soft* This);
void Fix_YUV_YUV(blit_soft* This);
void WMMXFix_RGB_UV(blit_soft* This);
void Any_RGB_RGB(blit_soft* This);
void Stretch_RGB_RGB(blit_soft* This);

void CalcColor(blit_soft* This);
void CalcYUVLookUp(blit_soft* This);

// 4*256:Y + 768*n:SAT + 4*256:U + 4*256:V 
void CalcLookUp(blit_soft* This, bool_t Dither);

#endif
