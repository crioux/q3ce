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
 * $Id: softidct.h 183 2005-01-11 03:29:39Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/
 
#ifndef __SOFTIDCT_H
#define __SOFTIDCT_H

#define SOFTIDCT_ID		FOURCC('S','I','D','C')

// duplicate softidct for optimized mips code...
#define MPEG4IDCT_ID	FOURCC('M','P','I','D')
#define MSMPEG4IDCT_ID	FOURCC('M','S','I','D')

void SoftIDCT_Init();
void SoftIDCT_Done();

// IMPORTANT:
// MIPS version of all CopyBlock and AddBlock doesn't support general DstPitch, it's fixed to SrcPitch
// ARM version of AddBlock doesn't support general DstPitch, it's fixed to 8 bytes
// non MIPS64 version of CopyMBlock can be used only in Copy() (no source alignment)

#define EDGE 32
#define MAXBUF 5

typedef	void(STDCALL *copyblock)(uint8_t*,uint8_t*,int,int);
typedef	void(STDCALL *addblock)(uint8_t*,uint8_t*,int);

//#if defined(ARM) && defined(TARGET_PALMOS)
//#define FREESCALE_MX1
//#endif

#if defined(ARM) || defined(_M_IX86)
#define IDCTSWAP
#endif

#if defined(MMX)
#define CopyBlock16x16 MMXCopyBlock16x16
#define CopyBlock8x8 MMXCopyBlock
#define IDCT_Const8x8 MMXIDCT_Const8x8
#define CopyBlock MMXCopyBlock
#define CopyBlockHor MMXCopyBlockHor
#define CopyBlockVer MMXCopyBlockVer
#define CopyBlockHorVer MMXCopyBlockHorVer
#define CopyBlockHorRound MMXCopyBlockHorRound
#define CopyBlockVerRound MMXCopyBlockVerRound
#define CopyBlockHorVerRound MMXCopyBlockHorVerRound
#define AddBlock MMXAddBlock
#define AddBlockHor MMXAddBlockHor
#define AddBlockVer MMXAddBlockVer
#define AddBlockHorVer MMXAddBlockHorVer
#endif

typedef struct softidct
{
	idct IDCT;

	copyblock AllCopyBlock[2][2][4];	//[8x8/16x16][Rounding][x][y]

#if !defined(MIPS64) && !defined(MIPS32)
	addblock AddBlock[4];				//[x][y]
#endif
	idctcopy	  Copy16x16[3];
	idctprocess	  Process[3]; // pixelformat
	idctintra	  Intra8x8;
#if !defined(MIPS64)
	idctinter	  Inter8x8[2]; 
#endif
#if defined(IDCTSWAP)
	idctcopy	  Copy16x16Swap[3];
	idctprocess	  ProcessSwap[3]; // pixelformat
	idctintra	  Intra8x8Swap;
#if !defined(MIPS64)
	idctinter	  Inter8x8Swap[2]; 
#endif
#endif

	struct
	{
		pin Pin;
		packetprocess Process;
		packetformat Format;

	} Out;

	bool_t NeedLast;
	int BufWidth;	//16 aligned and edge added
	int BufHeight;	//16 aligned and edge added
	int BufSize;
	int BufWidthUV;
	int YToU;
	int UVX2;
	int UVY2;
	bool_t Rounding;

	int BufferWidth;
	int BufferHeight;

	int ShowCurr;
	int ShowNext;
	int MaxCount;
	int BufCount; //IDCT_BUFFERCOUNT + LastTemp
	int LastTemp; //last buffer is temporary
	uint8_t* Buffer[MAXBUF]; //aligned to 32bytes (example MIPSVR41XX cache hack needs cache block alignement)
	block _Buffer[MAXBUF];
	int BufFrameNo[MAXBUF];
	bool_t BufBorder[MAXBUF];
	
#ifdef MIPS64
	int NextIRQ;
#endif

	uint8_t* Dst;
	uint8_t* Ref[2];
	uint8_t* RefMin[2];
	uint8_t* RefMax[2];
	int Tab[6]; //inc[12] 4:1:1 2:1:1 1:1:1
	uint8_t *Tmp;		//aligned 8x8 or 16x16 temp buffer

#ifdef FREESCALE_MX1
	int* MX1;
	uint8_t* MX1Dst;
	int MX1Pitch;
	void (*MX1Pop)(struct softidct*);
#endif

	int CurrPitch;
	int* Ptr;
	uint8_t* DstPtr;
	uint8_t* RefPtr[2];
	const int* MVBack;
	const int* MVFwd;

	copyblock* CopyBlock; //[2][2]
	copyblock* CopyMBlock;//[2][2]

	uint8_t _Tmp[16+256];

	uint8_t* CodeBegin;
	uint8_t* CodeEnd;
	int CodePage;

} softidct;

extern void Copy420(softidct* p,int x,int y,int Forward);
extern void Process420(softidct* p,int x,int y);
extern void Process422(softidct* p,int x,int y);
extern void Process444(softidct* p,int x,int y);

extern void SoftMComp8x8(softidct* p,const int* MVBack,const int* MVFwd);
extern void SoftMComp16x16(softidct* p,const int* MVBack,const int* MVFwd);
extern void Intra8x8(softidct* p,idct_block_t* Data,int Length,int ScanType);
extern void Inter8x8Back(softidct* p,idct_block_t* Block,int Length);
extern void Inter8x8BackFwd(softidct* p,idct_block_t* Block,int Length);

extern void CopySwap420(softidct* p,int x,int y,int Forward);
extern void ProcessSwap420(softidct* p,int x,int y);
extern void ProcessSwap422(softidct* p,int x,int y);
extern void ProcessSwap444(softidct* p,int x,int y);

extern void SoftMComp8x8Swap(softidct* p,const int* MVBack,const int* MVFwd);
extern void SoftMComp16x16Swap(softidct* p,const int* MVBack,const int* MVFwd);
extern void Intra8x8Swap(softidct* p,idct_block_t* Data,int Length,int ScanType);
extern void Inter8x8BackSwap(softidct* p,idct_block_t* Block,int Length);
extern void Inter8x8BackFwdSwap(softidct* p,idct_block_t* Block,int Length);

extern void IDCT_Col8(idct_block_t *Blk);
extern void IDCT_Row8(idct_block_t *Blk,uint8_t *Dest,const uint8_t *Src);
extern void IDCT_Row4(idct_block_t *Blk,uint8_t *Dest,const uint8_t *Src);

// !MIPS64 source stride = 8
// MIPS64 source stride = dest stride
extern void STDCALL IDCT_Block4x8(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src);
extern void STDCALL IDCT_Block8x8(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src);
extern void STDCALL IDCT_Block4x8Swap(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src);
extern void STDCALL IDCT_Block8x8Swap(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src);
extern void STDCALL IDCT_Const8x8(int v,uint8_t * Dst,int DstStride,uint8_t * Src);

extern void STDCALL CopyBlock(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
extern void STDCALL CopyBlockHor(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch); 
extern void STDCALL CopyBlockVer(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
extern void STDCALL CopyBlockHorVer(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
extern void STDCALL CopyBlockHorRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
extern void STDCALL CopyBlockVerRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
extern void STDCALL CopyBlockHorVerRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);

#ifndef MIPS64
#define SoftMComp16x16Swap SoftMComp8x8Swap
#define SoftMComp16x16 SoftMComp8x8
#endif

#if defined(FREESCALE_MX1)
void MX1PopNone(softidct* p);
void MX1Intra8x8(softidct* p,idct_block_t* Data,int Length,int ScanType);
void MX1Inter8x8Back(softidct* p,idct_block_t* Block,int Length);
void MX1Inter8x8BackFwd(softidct* p,idct_block_t* Block,int Length);
void MX1Intra8x8Swap(softidct* p,idct_block_t* Data,int Length,int ScanType);
void MX1Inter8x8BackSwap(softidct* p,idct_block_t* Block,int Length);
void MX1Inter8x8BackFwdSwap(softidct* p,idct_block_t* Block,int Length);
#endif

#if defined(ARM)
void WMMXIntra8x8(softidct* p,idct_block_t* Data,int Length,int ScanType);
void WMMXInter8x8Back(softidct* p,idct_block_t* Block,int Length);
void WMMXInter8x8BackFwd(softidct* p,idct_block_t* Block,int Length);
void WMMXIntra8x8Swap(softidct* p,idct_block_t* Data,int Length,int ScanType);
void WMMXInter8x8BackSwap(softidct* p,idct_block_t* Block,int Length);
void WMMXInter8x8BackFwdSwap(softidct* p,idct_block_t* Block,int Length);

void STDCALL WMMXIDCT_Block8x4(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src);
void STDCALL WMMXIDCT_Block8x8(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src);
void STDCALL WMMXIDCT_Const8x8(int v,uint8_t * Dst,int DstStride,uint8_t * Src);

void STDCALL WMMXCopyBlock(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch); 
void STDCALL WMMXCopyBlockHor(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch); 
void STDCALL WMMXCopyBlockVer(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL WMMXCopyBlockHorVer(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL WMMXCopyBlockHorRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL WMMXCopyBlockVerRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL WMMXCopyBlockHorVerRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL WMMXAddBlock(uint8_t * Src, uint8_t * Dst, int SrcPitch);
void STDCALL WMMXAddBlockHor(uint8_t * Src, uint8_t * Dst, int SrcPitch); 
void STDCALL WMMXAddBlockVer(uint8_t * Src, uint8_t * Dst, int SrcPitch);
void STDCALL WMMXAddBlockHorVer(uint8_t * Src, uint8_t * Dst, int SrcPitch);

void STDCALL FastCopyBlock(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch); 
void STDCALL FastCopyBlockHor(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch); 
void STDCALL FastCopyBlockVer(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL FastCopyBlockHorVer(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL FastCopyBlockHorRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL FastCopyBlockVerRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL FastCopyBlockHorVerRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL FastAddBlock(uint8_t * Src, uint8_t * Dst, int SrcPitch);
void STDCALL FastAddBlockHor(uint8_t * Src, uint8_t * Dst, int SrcPitch); 
void STDCALL FastAddBlockVer(uint8_t * Src, uint8_t * Dst, int SrcPitch);
void STDCALL FastAddBlockHorVer(uint8_t * Src, uint8_t * Dst, int SrcPitch);

void STDCALL PreLoadCopyBlock(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch); 
void STDCALL PreLoadCopyBlockHor(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch); 
void STDCALL PreLoadCopyBlockVer(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL PreLoadCopyBlockHorVer(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL PreLoadCopyBlockHorRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL PreLoadCopyBlockVerRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL PreLoadCopyBlockHorVerRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
void STDCALL PreLoadAddBlock(uint8_t * Src, uint8_t * Dst, int SrcPitch);
void STDCALL PreLoadAddBlockHor(uint8_t * Src, uint8_t * Dst, int SrcPitch); 
void STDCALL PreLoadAddBlockVer(uint8_t * Src, uint8_t * Dst, int SrcPitch);
void STDCALL PreLoadAddBlockHorVer(uint8_t * Src, uint8_t * Dst, int SrcPitch);

#endif

#if defined(MIPS64)

#define IDCT_Row4 IDCT_Row8
#define CopyBlock8x8 CopyBlock
#define CopyBlock16x16 CopyMBlock
extern void STDCALL CopyMBlock(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
extern void STDCALL CopyMBlockHor(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch); 
extern void STDCALL CopyMBlockVer(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
extern void STDCALL CopyMBlockHorVer(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
extern void STDCALL CopyMBlockHorRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
extern void STDCALL CopyMBlockVerRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);
extern void STDCALL CopyMBlockHorVerRound(uint8_t * Src, uint8_t * Dst, int SrcPitch, int DstPitch);

extern void Inter8x8Add(softidct* p,idct_block_t *Block,int Length);

// Src must be aligned
extern void AddBlock8x8(uint8_t * Src, uint8_t * Dst, int SrcPitch,int DstPitch);
extern void AddBlock16x16(uint8_t * Src, uint8_t * Dst, int SrcPitch,int DstPitch);

#if defined(TARGET_WINCE) || defined(TARGET_WIN32)
#define MIPS64_WIN32
#endif

#ifdef MIPS64_WIN32
static INLINE void DisableInterrupts() {
	__asm(	".set noreorder;"
			"mfc0	$4,$12;"
			"li		$5,-1;"
			"sll	$5,$5,1;"
			"and	$4,$4,$5;"
			"mtc0	$4,$12;"
			"nop;"
			".set reorder;"
			);
}

static INLINE void EnableInterrupts() {
	__asm(	".set noreorder;"
			"mfc0	$4,$12;"
			"nop;"
			"nop;"
			"ori	$4,$4,0x001;"
			"mtc0	$4,$12;"
			"nop;"
			".set reorder;"
			);
}
#endif

#elif defined(MIPS32)

#define IDCT_Row4 IDCT_Row8
// Src must be aligned
extern void STDCALL AddBlock8x8(uint8_t * Src, uint8_t * Dst, int SrcPitch,int DstPitch);
extern void STDCALL CopyBlock8x8(uint8_t * Src, uint8_t * Dst, int SrcPitch,int DstPitch);
extern void STDCALL CopyBlock16x16(uint8_t * Src, uint8_t * Dst, int SrcPitch,int DstPitch);

#else

// AddBlock DstPitch=8
extern void STDCALL AddBlock(uint8_t * Src, uint8_t * Dst, int SrcPitch);
extern void STDCALL AddBlockHor(uint8_t * Src, uint8_t * Dst, int SrcPitch); 
extern void STDCALL AddBlockVer(uint8_t * Src, uint8_t * Dst, int SrcPitch);
extern void STDCALL AddBlockHorVer(uint8_t * Src, uint8_t * Dst, int SrcPitch);

// Src must be aligned
extern void STDCALL CopyBlock8x8(uint8_t * Src, uint8_t * Dst, int SrcPitch,int DstPitch);
extern void STDCALL CopyBlock16x16(uint8_t * Src, uint8_t * Dst, int SrcPitch,int DstPitch);

#endif

#ifdef MMX
  #ifdef _MSC_VER
    #ifdef TARGET_WINCE
      extern void STDCALL EMMS();
    #else
      static INLINE void EMMS() { __asm { emms } }
    #endif
  #else
	#define	EMMS() __asm__ __volatile__ ("emms")
  #endif
#else
  static INLINE void EMMS() {}
#endif

#ifdef ARM
  #define SAT(Value) Value = Value < 0 ? 0: (Value > 255 ? 255: Value);
#else
  // upper bits are trashed! use only lower 8 bits afterward
  #define SAT(Value) Value <<= Value >> 16; Value |= (Value << 23) >> 31;
#endif

static INLINE void SetPtr420(softidct* p,int x,int y)
{
	x <<= 4;
	y <<= 4;
	x += EDGE;
	y += EDGE;
	y *= p->BufWidth;
	p->CurrPitch = p->BufWidth;
	p->DstPtr = p->Dst+x+y;
	p->RefPtr[0] = p->Ref[0]+x+y;
	p->RefPtr[1] = p->Ref[1]+x+y;
	p->Ptr = p->Tab;
	p->Tab[3] = 1 | 2*(-y-x-8*p->BufWidth-8 + 
		p->YToU + (x>>1) + (y>>2));	//Y[1;1] -> U
}

static INLINE void SetPtr422(softidct* p,int x,int y)
{
	x <<= 4;
	y <<= 3;
	x += EDGE;
	y += EDGE;
	y *= p->BufWidth;
	p->CurrPitch = p->BufWidth;
	p->DstPtr = p->Dst+x+y;
	p->RefPtr[0] = p->Ref[0]+x+y;
	p->RefPtr[1] = p->Ref[1]+x+y;
	p->Ptr = p->Tab;
	p->Tab[1] = 1 | 2*(-y-x-8 + 
		p->YToU + (x>>1) + (y>>1));	//Y[0;1] -> U
}

static INLINE void SetPtr444(softidct* p,int x,int y)
{
	x <<= 3;
	y <<= 3;
	x += EDGE;
	y += EDGE;
	y *= p->BufWidth;
	p->CurrPitch = p->BufWidth;
	p->DstPtr = p->Dst+x+y;
	p->RefPtr[0] = p->Ref[0]+x+y;
	p->RefPtr[1] = p->Ref[1]+x+y;
	p->Ptr = p->Tab;
}

static INLINE void IncPtr(softidct* p,bool_t Ref0,bool_t Ref1)
{
	uint8_t* DstPtr = p->DstPtr;
	int v = *(p->Ptr++);
	if (v & 1) p->CurrPitch >>= 1;
	v >>= 1;
	p->DstPtr = DstPtr + v;
	if (Ref0) p->RefPtr[0] += v;
	if (Ref1) p->RefPtr[1] += v;
}

static INLINE void IncPtrLum(softidct* p)
{
	int v = (p->Tab[0]+p->Tab[1]+p->Tab[2]+p->Tab[3]) >> 1;
	p->CurrPitch >>= 1;
	p->DstPtr += v;
	p->RefPtr[0] += v;
	p->RefPtr[1] += v;
	p->Ptr = &p->Tab[4];
}

#endif

