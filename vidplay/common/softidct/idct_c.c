/*****************************************************************************
 *
 * This code has been developed by Project Mayo. This software is an
 * implementation of a part of one or more MPEG-4 Video tools as
 * specified in ISO/IEC 14496-2 standard.  Those intending to use this
 * software module in hardware or software products are advised that its
 * use may infringe existing patents or copyrights, and any such use
 * would be at such party's own risk.  The original developer of this
 * software module and his/her company, and subsequent editors and their
 * companies (including Project Mayo), will have no liability for use of
 * this software or modifications or derivatives thereof.
 *
 *****************************************************************************
 *																				*	
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
 * $Id: idct_c.c 117 2004-11-28 02:45:11Z picard $
 *
 *****************************************************************************
 *
 * Authors:
 *
 *	Andrea	Graziani  (Ag): Original source code (Open Divx Decoder 0.4a).
 *	Pedro	Mateu     (Pm) and
 *	Gabor	Kovacs    (Kg) Heavily modified and optimized code
 *
 ****************************************************************************/

#include "../common.h"
#include "softidct.h"

// 2D Inverse Discrete Cosine Transform (iDCT)

#define W1 2841                 // 2048*sqrt(2)*cos(1*pi/16) 
#define W2 2676                 // 2048*sqrt(2)*cos(2*pi/16) 
#define W3 2408                 // 2048*sqrt(2)*cos(3*pi/16) 
#define W5 1609                 // 2048*sqrt(2)*cos(5*pi/16) 
#define W6 1108                 // 2048*sqrt(2)*cos(6*pi/16) 
#define W7 565                  // 2048*sqrt(2)*cos(7*pi/16) 

#define W1_minus_W7 2276
#define W1_plus_W7 3406
#define W3_minus_W5 799
#define W3_plus_W5 4017
#define W2_minus_W6 1568
#define W2_plus_W6 3784

#define ADDSAT32(a,Dst,Add32)		\
	b = a + Add32;					\
	c = a & Add32;					\
	a ^= Add32;						\
	a &= ~b;						\
	a |= c;							\
	a &= MaskCarry;					\
	c = a << 1;						\
	b -= c;	/* adjust neighbour */	\
	b |= c - (a >> 7); /* mask */	\
	Dst = b;						

#define SUBSAT32(a,Dst,Add32)		\
	a = ~a;							\
	b = a + Add32;					\
	c = a & Add32;					\
	a ^= Add32;						\
	a &= ~b;						\
	a |= c;							\
	a &= MaskCarry;					\
	c = a << 1;						\
	b -= c;	/* adjust neighbour */	\
	b |= c - (a >> 7); /* mask */	\
	Dst = ~b;						

#if !defined(MIPS64) && !defined(MIPS32) && !defined(ARM) 

void IDCT_Col8(idct_block_t *Blk)
{
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;
	int x567,x123;

	x0 = Blk[0];
	x4 = Blk[8];
	x3 = Blk[16];
	x7 = Blk[24];
	x1 = Blk[32] << 11;
	x6 = Blk[40];
	x2 = Blk[48];	
	x5 = Blk[56];
	
	x123=x1|x2|x3;
	x567=x5|x6|x7;

	if (!(x123|x567))
	{
		if (!x4) { // x0

			if (x0) 
				Blk[0] = Blk[8] = Blk[16] = Blk[24] = Blk[32] = Blk[40] = Blk[48] = Blk[56] = (idct_block_t)(x0 << 3);
		}
		else { // x0,x4

			x0 = (x0 << 11) + 128;    
			x5 = W7 * x4;
			x1 = W1 * x4;
			x2 = ((181 * W1_plus_W7 + 128) >> 8) * x4;
			x4 = ((181 * W1_minus_W7 + 128) >> 8) * x4;

			Blk[0] = (idct_block_t)((x0 + x1) >> 8);
			Blk[8] = (idct_block_t)((x0 + x2) >> 8);
			Blk[16] = (idct_block_t)((x0 + x4) >> 8);
			Blk[24] = (idct_block_t)((x0 + x5) >> 8);
			Blk[32] = (idct_block_t)((x0 - x5) >> 8);
			Blk[40] = (idct_block_t)((x0 - x4) >> 8);
			Blk[48] = (idct_block_t)((x0 - x2) >> 8);
			Blk[56] = (idct_block_t)((x0 - x1) >> 8);
		}
	}
	else if (!(x4|x567)) { // x0,x1,x2,x3
	
		x0 = (x0 << 11) + 128;    
		
		x8 = x0 + x1;
		x0 -= x1;
		x1 = W6 * (x3 + x2);
		x2 = x1 - (W2_plus_W6) * x2;
		x3 = x1 + (W2_minus_W6) * x3;
		
		x7 = x8 + x3;
		x8 -= x3;
		x3 = x0 + x2;
		x0 -= x2;
		
		Blk[0] = (idct_block_t)(x7 >> 8);
		Blk[8] = (idct_block_t)(x3 >> 8);
		Blk[16] = (idct_block_t)(x0 >> 8);
		Blk[24] = (idct_block_t)(x8 >> 8);
		Blk[32] = (idct_block_t)(x8 >> 8);
		Blk[40] = (idct_block_t)(x0 >> 8);
		Blk[48] = (idct_block_t)(x3 >> 8);
		Blk[56] = (idct_block_t)(x7 >> 8);

		return;
	}
	else { //x0,x1,x2,x3,x4,x5,x6,x7

		x0 = (x0 << 11) + 128;    
		x8 = W7 * (x4 + x5);
		x4 = x8 + (W1_minus_W7) * x4;
		x5 = x8 - (W1_plus_W7) * x5;
		x8 = W3 * (x6 + x7);
		x6 = x8 - (W3_minus_W5) * x6;
		x7 = x8 - (W3_plus_W5) * x7;

		x8 = x0 + x1;
		x0 -= x1;
		x1 = W6 * (x3 + x2);
		x2 = x1 - (W2_plus_W6) * x2;
		x3 = x1 + (W2_minus_W6) * x3;
		x1 = x4 + x6;
		x4 -= x6;
		x6 = x5 + x7;
		x5 -= x7;

		x7 = x8 + x3;
		x8 -= x3;
		x3 = x0 + x2;
		x0 -= x2;
		x2 = (181 * (x4 + x5) + 128) >> 8;
		x4 = (181 * (x4 - x5) + 128) >> 8;

		Blk[0] = (idct_block_t)((x7 + x1) >> 8);
		Blk[8] = (idct_block_t)((x3 + x2) >> 8);
		Blk[16] = (idct_block_t)((x0 + x4) >> 8);
		Blk[24] = (idct_block_t)((x8 + x6) >> 8);
		Blk[32] = (idct_block_t)((x8 - x6) >> 8);
		Blk[40] = (idct_block_t)((x0 - x4) >> 8);
		Blk[48] = (idct_block_t)((x3 - x2) >> 8);
		Blk[56] = (idct_block_t)((x7 - x1) >> 8);
	}
}

static INLINE void IDCT_RowConst(int v, uint8_t *Dst, const uint8_t *Src)
{
	if (Src) {
		
		uint32_t MaskCarry = 0x80808080U;
		uint32_t a,b,c,d;

		a = ((uint32_t*)Src)[0];
		d = ((uint32_t*)Src)[1];

		if (v>0)
		{
			v |= v << 8;
			v |= v << 16;

			ADDSAT32(a,((uint32_t*)Dst)[0],v);
			ADDSAT32(d,((uint32_t*)Dst)[1],v);
		}
		else
		if (v<0)
		{
			v = -v;
			v |= v << 8;
			v |= v << 16;

			SUBSAT32(a,((uint32_t*)Dst)[0],v);
			SUBSAT32(d,((uint32_t*)Dst)[1],v);
		}
		else
		{
			((uint32_t*)Dst)[0] = a;
			((uint32_t*)Dst)[1] = d;
		}
	}
	else	
	{
		SAT(v);

		v &= 255;
		v |= v << 8;
		v |= v << 16;

		((uint32_t*)Dst)[1] = ((uint32_t*)Dst)[0] = v;
	}
}   

void IDCT_Row8(idct_block_t *Blk, uint8_t *Dst, const uint8_t *Src)
{
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;

	x4 = Blk[1];
  	x3 = Blk[2];
  	x7 = Blk[3];
	x1 = Blk[4];
	x6 = Blk[5];
	x2 = Blk[6];
	x5 = Blk[7];
	
	if (!(x1|x2|x3|x4|x5|x6|x7))
	{
		IDCT_RowConst((Blk[0] + 32) >> 6,Dst,Src);
		return;
	}

	x1 <<= 8;
	x0 = (Blk[0] << 8) + 8192;

	x8 = W7 * (x4 + x5) + 4;
	x4 = (x8 + (W1_minus_W7) * x4) >> 3;
	x5 = (x8 - (W1_plus_W7) * x5) >> 3;
	x8 = W3 * (x6 + x7) + 4;
	x6 = (x8 - (W3_minus_W5) * x6) >> 3;
	x7 = (x8 - (W3_plus_W5) * x7) >> 3;

	x8 = x0 + x1;
	x0 -= x1;
	x1 = W6 * (x3 + x2) + 4;
	x2 = (x1 - (W2_plus_W6) * x2) >> 3;
	x3 = (x1 + (W2_minus_W6) * x3) >> 3;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;

	x7 = x8 + x3;
	x8 -= x3;
	x3 = x0 + x2;
	x0 -= x2;
	x2 = (181 * (x4 + x5) + 128) >> 8;
	x4 = (181 * (x4 - x5) + 128) >> 8;

	x5 = (x7 + x1) >> 14;
	x1 = (x7 - x1) >> 14;
	x7 = (x3 + x2) >> 14;
	x2 = (x3 - x2) >> 14;
	x3 = (x0 + x4) >> 14;
	x4 = (x0 - x4) >> 14;
	x0 = (x8 + x6) >> 14;
	x6 = (x8 - x6) >> 14;

	if (Src)
	{
		x5 += Src[0];
		x1 += Src[7];
		x7 += Src[1];
		x2 += Src[6];
		x3 += Src[2];
		x4 += Src[5];
		x0 += Src[3];
		x6 += Src[4];
	}
	
	x8 = (x5|x1|x7|x2|x3|x4|x0|x6)>>8;

	if (x8)
	{
		SAT(x5)
		SAT(x7)
		SAT(x3)
		SAT(x0)
		SAT(x6)
		SAT(x4)
		SAT(x2)
		SAT(x1)
	}

	Dst[0] = (uint8_t)x5;
	Dst[1] = (uint8_t)x7;
	Dst[2] = (uint8_t)x3;
	Dst[3] = (uint8_t)x0;
	Dst[4] = (uint8_t)x6;
	Dst[5] = (uint8_t)x4;
	Dst[6] = (uint8_t)x2;
	Dst[7] = (uint8_t)x1;
}

void IDCT_Row4(idct_block_t *Blk, uint8_t *Dst, const uint8_t *Src)
{
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;
  
	x4 = Blk[1];
  	x3 = Blk[2];
  	x7 = Blk[3];
	
	if (!(x3|x4|x7))
	{
		IDCT_RowConst((Blk[0] + 32) >> 6,Dst,Src);
		return;
	}

	x0 = (Blk[0] << 8) + 8192;

	x5 = (W7 * x4 + 4) >> 3;
	x4 = (W1 * x4 + 4) >> 3;
	x6 = (W3 * x7 + 4) >> 3;
	x7 = (-W5 * x7 + 4) >> 3;

	x2 = (W6 * x3 + 4) >> 3;
	x3 = (W2 * x3 + 4) >> 3;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;

	x7 = x0 + x3;
	x8 = x0 - x3;
	x3 = x0 + x2;
	x0 -= x2;
	x2 = (181 * (x4 + x5) + 128) >> 8;
	x4 = (181 * (x4 - x5) + 128) >> 8;

	x5 = (x7 + x1) >> 14;
	x1 = (x7 - x1) >> 14;
	x7 = (x3 + x2) >> 14;
	x2 = (x3 - x2) >> 14;
	x3 = (x0 + x4) >> 14;
	x4 = (x0 - x4) >> 14;
	x0 = (x8 + x6) >> 14;
	x6 = (x8 - x6) >> 14;

	if (Src)
	{
		x5 += Src[0];
		x1 += Src[7];
		x7 += Src[1];
		x2 += Src[6];
		x3 += Src[2];
		x4 += Src[5];
		x0 += Src[3];
		x6 += Src[4];
	}
	
	x8 = (x5|x1|x7|x2|x3|x4|x0|x6)>>8;

	if (x8)
	{
		SAT(x5)
		SAT(x7)
		SAT(x3)
		SAT(x0)
		SAT(x6)
		SAT(x4)
		SAT(x2)
		SAT(x1)
	}

	Dst[0] = (uint8_t)x5;
	Dst[1] = (uint8_t)x7;
	Dst[2] = (uint8_t)x3;
	Dst[3] = (uint8_t)x0;
	Dst[4] = (uint8_t)x6;
	Dst[5] = (uint8_t)x4;
	Dst[6] = (uint8_t)x2;
	Dst[7] = (uint8_t)x1;
}

#endif

#if !defined(ARM) 

void STDCALL IDCT_Block8x8(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src)
{
	int SrcStride;

	IDCT_Col8(Block);
	IDCT_Col8(Block+1);
	IDCT_Col8(Block+2);
	IDCT_Col8(Block+3);
	IDCT_Col8(Block+4);
	IDCT_Col8(Block+5);
	IDCT_Col8(Block+6);
	IDCT_Col8(Block+7);

	SrcStride = 0;
#ifdef MIPS64
	if (Src) SrcStride = DestStride;
#else
	if (Src) SrcStride = 8;
#endif

	IDCT_Row8(Block,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row8(Block+8,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row8(Block+16,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row8(Block+24,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row8(Block+32,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row8(Block+40,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row8(Block+48,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row8(Block+56,Dest,Src);
}

void STDCALL IDCT_Block4x8(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src)
{
	int SrcStride;

	IDCT_Col8(Block);
	IDCT_Col8(Block+1);
	IDCT_Col8(Block+2);
	IDCT_Col8(Block+3);

	SrcStride = 0;
#ifdef MIPS64
	if (Src) SrcStride = DestStride;
#else
	if (Src) SrcStride = 8;
#endif

	IDCT_Row4(Block,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row4(Block+8,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row4(Block+16,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row4(Block+24,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row4(Block+32,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row4(Block+40,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row4(Block+48,Dest,Src);
	Dest+=DestStride;
	Src+=SrcStride;
	IDCT_Row4(Block+56,Dest,Src);
}

#ifdef IDCTSWAP
// just for testing
void STDCALL IDCT_Block4x8Swap(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src)
{
	int x;
	for (x=0;x<64;++x)
		Block[64+x] = Block[((x&7)<<3)+(x>>3)];

	IDCT_Block4x8(Block+64,Dest,DestStride,Src);
}
void STDCALL IDCT_Block8x8Swap(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src)
{
	int x;
	for (x=0;x<64;++x)
		Block[64+x] = Block[((x&7)<<3)+(x>>3)];

	IDCT_Block8x8(Block+64,Dest,DestStride,Src);
}
#endif

#endif

#ifndef MMX

void STDCALL IDCT_Const8x8(int v,uint8_t * Dst, int DstPitch, uint8_t * Src)
{
#ifndef MIPS64
	int SrcPitch = 8;
#else
	int SrcPitch = DstPitch;
#endif
	const uint8_t* SrcEnd = Src + 8*SrcPitch;
	uint32_t MaskCarry = 0x80808080U;
	uint32_t a,b,c,d;

	if (v>0)
	{
		v |= v << 8;
		v |= v << 16;

		do
		{
			a = ((uint32_t*)Src)[0];
			d = ((uint32_t*)Src)[1];
			ADDSAT32(a,((uint32_t*)Dst)[0],v);
			ADDSAT32(d,((uint32_t*)Dst)[1],v);
			Dst += DstPitch;
			Src += SrcPitch;
		}
		while (Src != SrcEnd);
	}
	else
	if (v<0)
	{
		v = -v;
		v |= v << 8;
		v |= v << 16;

		do
		{
			a = ((uint32_t*)Src)[0];
			d = ((uint32_t*)Src)[1];
			SUBSAT32(a,((uint32_t*)Dst)[0],v);
			SUBSAT32(d,((uint32_t*)Dst)[1],v);
			Dst += DstPitch;
			Src += SrcPitch;
		}
		while (Src != SrcEnd);
	}
#ifndef MIPS64
	else
		CopyBlock8x8(Src,Dst,8,DstPitch);
#endif
}

#endif
