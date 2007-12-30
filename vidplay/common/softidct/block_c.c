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
 * $Id: block_c.c 131 2004-12-04 20:36:04Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/
 
#include "../common.h"
#include "softidct.h"

//#define STAT

#ifdef STAT

int Len[16] = {0};
int Count = 0;
int Type[2][3] = {0};
int Spec[2][5] = {0};
int Skip = 0;
int Max = 0;

void Statistics(idct_block_t *Block, int Length, int ScanType, int Add)
{
	int i;
	for (i=0;i<Length;++i)
	{
		int v = Block[i];
		if (v<0) v=-v;
		if (v>Max) Max=v;
	}

	++Len[Length >> 2];
	++Count;
	++Type[Add][ScanType];
	if (Length==1)
		Spec[Add][0]++;
	else
	if (ScanType==0 && Length==2)
		Spec[Add][1]++;
	else
	if (!Add && ScanType!=1 && Length < 15)
	{
		Spec[Add][2]++;
	}
	else
	if (Add && ScanType==0 && (Length < 15 || (Length<26 && ((uint32_t*)Block)[2]==0 && ((uint32_t*)Block)[6]==0)))
	{
		Spec[Add][2]++;
	}
}
#else
#define Statistics(a,b,c,d)
#endif

void Copy420(softidct* p,int x,int y,int Forward)
{
	SetPtr420(p,x,y);
	CopyBlock16x16(p->RefPtr[Forward],p->DstPtr,p->CurrPitch,p->CurrPitch);
	IncPtrLum(p);
	CopyBlock8x8(p->RefPtr[Forward],p->DstPtr,p->CurrPitch,p->CurrPitch);
	IncPtr(p,1,1);
	CopyBlock8x8(p->RefPtr[Forward],p->DstPtr,p->CurrPitch,p->CurrPitch);
	EMMS();
}

void Process420(softidct* p,int x,int y)
{
#ifdef MIPS64_WIN32
	int Pos = y*256+x;
	if (Pos >= p->NextIRQ)
		EnableInterrupts();
#endif

	SetPtr420(p,x,y);

#ifdef MIPS64_WIN32
	if (Pos >= p->NextIRQ)
	{
		p->NextIRQ = Pos+16;
		DisableInterrupts();
	}
#endif
}

void Process422(softidct* p,int x,int y)
{
#ifdef MIPS64_WIN32
	int Pos = y*256+x;
	if (Pos >= p->NextIRQ)
		EnableInterrupts();
#endif

	SetPtr422(p,x,y);

#ifdef MIPS64_WIN32
	if (Pos >= p->NextIRQ)
	{
		p->NextIRQ = Pos+16;
		DisableInterrupts();
	}
#endif
}

void Process444(softidct* p,int x,int y)
{
#ifdef MIPS64_WIN32
	int Pos = y*256+x;
	if (Pos >= p->NextIRQ)
		EnableInterrupts();
#endif

	SetPtr444(p,x,y);

#ifdef MIPS64_WIN32
	if (Pos >= p->NextIRQ)
	{
		p->NextIRQ = Pos+16;
		DisableInterrupts();
	}
#endif
}

#ifndef MIPS64
void SoftMComp8x8(softidct* p,const int* MVBack,const int* MVFwd)
{
	p->MVBack = MVBack;
	p->MVFwd = MVFwd;
}
#endif

#include "block.h"

#ifdef IDCTSWAP

void CopySwap420(softidct* p,int x,int y,int Forward)
{
	SetPtr420(p,y,x);
	CopyBlock16x16(p->RefPtr[Forward],p->DstPtr,p->CurrPitch,p->CurrPitch);
	IncPtrLum(p);
	CopyBlock8x8(p->RefPtr[Forward],p->DstPtr,p->CurrPitch,p->CurrPitch);
	IncPtr(p,1,1);
	CopyBlock8x8(p->RefPtr[Forward],p->DstPtr,p->CurrPitch,p->CurrPitch);
	EMMS();
}

void ProcessSwap420(softidct* p,int x,int y)
{
	SetPtr420(p,y,x);
}
void ProcessSwap422(softidct* p,int x,int y)
{
	SetPtr422(p,y,x);
}
void ProcessSwap444(softidct* p,int x,int y)
{
	SetPtr444(p,y,x);
}

#ifndef MIPS64
void SoftMComp8x8Swap(softidct* p,const int* MVBack,const int* MVFwd)
{
	p->MVBack = MVBack;
	p->MVFwd = MVFwd;
}
#endif

#define SWAPXY
#define SWAP8X4
#define Intra8x8 Intra8x8Swap
#define Inter8x8Back Inter8x8BackSwap
#define Inter8x8BackFwd Inter8x8BackFwdSwap

#include "block.h"

#endif
