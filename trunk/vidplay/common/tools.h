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
 * $Id: tools.h 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __TOOLS_H
#define __TOOLS_H

#define ALIGN16(x) (((int)(x) + 15) & ~15)
#define ALIGN4(x) (((int)(x) + 3) & ~3)

#define GET_R(x)   ((uint8_t)(((x) >> 0) & 255))
#define GET_G(x)   ((uint8_t)(((x) >> 8) & 255))
#define GET_B(x)   ((uint8_t)(((x) >> 16) & 255))

//some helper functions

static INLINE bool_t EqInt(const int* a, const int* b) { return *a == *b; }
static INLINE bool_t EqBool(const bool_t* a, const bool_t* b) { return *a == *b; }
static INLINE bool_t EqPtr(void** a, void** b) { return *a == *b; }

DLL void* Alloc16(int); //aligned to 16 bytes
DLL void Free16(void*);

DLL bool_t EqPoint(const point* a, const point* b);
DLL bool_t EqRect(const rect* a, const rect* b);
DLL bool_t EqPixel(const pixel* a, const pixel* b);
DLL bool_t EqVideo(const video* a, const video* b);
DLL bool_t EqFrac(const fraction* a, const fraction* b);
DLL bool_t EqAudio(const audio* a, const audio* b);
DLL bool_t EqBlitFX(const blitfx* a, const blitfx* b);

DLL void Simplify(fraction*, int MaxNum, int MaxDen);

DLL void SwapPChar(tchar_t**, tchar_t**);
DLL void SwapPByte(uint8_t**, uint8_t**);
DLL void SwapInt(int*, int*);
DLL void SwapLong(long*, long*);
DLL void SwapBool(bool_t*, bool_t*);
DLL void SwapPoint(point*);
DLL void SwapRect(rect*);

DLL void UpperUntil(tchar_t* s, tchar_t Delimiter);

DLL void VirtToPhy(const rect* Virtual, rect* Physical, const video*);
DLL void PhyToVirt(const rect* Physical, rect* Virtual, const video*);
DLL void ClipRectPhy(rect* Physical, const video*);

DLL int UpperFourCC(int);
DLL bool_t Compressed(const pixel*);
DLL bool_t AnyYUV(const pixel*);
DLL bool_t PlanarYUV(const pixel*, int* UVX2, int* UVY2,int* UVStride2);
DLL bool_t PlanarYUV444(const pixel*);
DLL bool_t PlanarYUV422(const pixel*);
DLL bool_t PlanarYUV420(const pixel*);
DLL bool_t PackedYUV(const pixel*);
DLL void FillInfo(pixel*);
DLL int GetImageSize(const video*);
DLL int GetBPP(const pixel*);
DLL int BitMaskSize(uint32_t Mask);
DLL int BitMaskPos(uint32_t Mask);
DLL uint32_t RGBToFormat(uint32_t RGB, const pixel*);

DLL int CombineDir(int SrcDir,int FXDir,int DstDir);
DLL void FillColor(uint8_t* Dst,int DstPitch,int x,int y,int Width,int Height,int BPP,int Value);
DLL int AnyAlign(rect* DstRect, rect* SrcRect, const blitfx* FX, 
				 int DstAlignSize, int DstAlignPos, 
				 int MinScale, int MaxScale);

DLL int SurfaceAlloc(planes, const video*);
DLL void SurfaceFree(planes);
DLL int SurfaceCopy(const video* SrcFormat, const video* DstFormat, 
					const planes Src, planes Dst, blitfx* FX);
DLL int SurfaceRotate(const video* SrcFormat, const video* DstFormat, 
					  const planes Src, planes Dst, int Dir);

DLL void DefaultVideo(video*);
DLL void DefaultAudio(audio*);
DLL void DefaultPitch(video*);
DLL void DefaultRGB(pixel*, int BitCount, 
                     int RBits, int GBits, int BBits,
                     int RGaps, int GGaps, int BGaps);

DLL void BuildChapter(tchar_t* s,int No,int64_t Time,int Rate);

// variable names: Result, Data, Size

#define GETVALUE(Value,Type)								\
		{													\
			assert(Size == sizeof(Type));					\
			*(Type*)Data = Value;							\
			Result = ERR_NONE;								\
		}

#define GETVALUECOND(Value,Type,Cond)						\
		{													\
			assert(Size == sizeof(Type));					\
			if (Cond)										\
			{												\
				*(Type*)Data = Value;						\
				Result = ERR_NONE;							\
			}												\
		}

#define GETSTRING(Text)										\
		{													\
			TcsNCpy((tchar_t*)Data,Text,Size/sizeof(tchar_t));\
			Result = ERR_NONE;								\
		}

#define SETVALUE(Value,Type,Update)							\
		{													\
			assert(Size == sizeof(Type));					\
			Value = *(Type*)Data;							\
			Result = Update;								\
		}

#define SETVALUENULL(Value,Type,Update,Null)				\
		{													\
			if (Data)										\
			{												\
				assert(Size == sizeof(Type));				\
				Value = *(Type*)Data;						\
			}												\
			else											\
				Null;										\
			Result = Update;								\
		}

#define SETVALUECOND(Value,Type,Update,Cond)				\
		{													\
			assert(Size == sizeof(Type));					\
			if (Cond)										\
			{												\
				Value = *(Type*)Data;						\
				Result = Update;							\
			}												\
		}

#define SETVALUECMP(Value,Type,Update,EqFunc)				\
		{													\
			assert(Size == sizeof(Type));					\
			Result = ERR_NONE;								\
			if (!EqFunc(&Value,(Type*)Data))				\
			{												\
				Value = *(Type*)Data;						\
				Result = Update;							\
			}												\
		}

#define SETSTRING(Text)										\
		{													\
			TcsNCpy(Text,(tchar_t*)Data,sizeof(Text)/sizeof(tchar_t));\
			Result = ERR_NONE;								\
		}

#define SETPACKETFORMAT(Value,Type,Update)					\
		{													\
			assert(Size == sizeof(Type) || !Data);			\
			Result = PacketFormatCopy(&Value,(Type*)Data);	\
			if (Result == ERR_NONE)							\
				Result = Update;							\
		}

DLL void SplitURL(const tchar_t* URL, tchar_t* Mime, tchar_t* Dir, tchar_t* Name, tchar_t* Ext);
DLL bool_t SetFileExt(tchar_t* URL, const tchar_t* Ext);
DLL int CheckExts(const tchar_t* URL, const tchar_t* Exts);
DLL bool_t CheckContentType(const tchar_t* s, const tchar_t* List);
DLL void AbsPath(tchar_t* Abs, const tchar_t* Any, const tchar_t* Base);
DLL void RelPath(tchar_t* Rel, const tchar_t* Any, const tchar_t* Base);
DLL bool_t UpperPath(tchar_t* Path, tchar_t* Last);

DLL bool_t UniqueExts(const int* Begin,const int* Pos);

#define MAXLINE 1024
DLL bool_t ReadLine(buffer* Buffer, struct stream*, tchar_t* Out, int Len); // ascii

#define SWAP32(a) ((((uint32_t)(a) >> 24) & 0x000000FF) | (((uint32_t)(a) >> 8)  & 0x0000FF00)|\
                  (((uint32_t)(a) << 8)  & 0x00FF0000) | (((uint32_t)(a) << 24) & 0xFF000000))

#define SWAP16(a) ((uint16_t)((((uint32_t)(a) >> 8) & 0xFF) | (((uint32_t)(a) << 8) & 0xFF00)))
#define SWAP64(a) ((SWAP32(a) << 32) | SWAP32(a>>32))

#ifdef BIG_ENDIAN
#define INT64BE(a) (a)
#define INT64LE(a) SWAP64(a)
#define INT32BE(a) (a)
#define INT32LE(a) SWAP32(a)
#define INT16BE(a) (a)
#define INT16LE(a) SWAP16(a)
#else
#define INT64LE(a) (a)
#define INT64BE(a) SWAP64(a)
#define INT32LE(a) (a)
#define INT32BE(a) SWAP32(a)
#define INT16LE(a) (a)
#define INT16BE(a) SWAP16(a)
#endif

DLL int ScaleRound(int v,int Num,int Den);

static INLINE int Scale(int v,int Num,int Den)
{
	if (Den) 
		return (int)((int64_t)v * Num / Den);
	return 0;
}

static INLINE int Scale64(int v,int64_t Num,int64_t Den)
{
	if (Den) 
		return (int)((int64_t)v * Num / Den);
	return 0;
}

#define OFS(name,item) ((int)&(((name*)NULL)->item))

#define WMVF_ID		FOURCC('W','M','V','F')
#define WMAF_ID		FOURCC('W','M','A','F')

extern DLL const nodedef WMVF;
extern DLL const nodedef WMAF;

#endif
