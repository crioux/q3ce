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
 * $Id: tools.c 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"

int GCD(int a,int b)
{
	int c;
	while (b)
	{
		c = b;
		b = a % b;
		a = c;
	}
	return a;
}

void BuildChapter(tchar_t* s,int No,int64_t Time,int Rate)
{
	int Hour,Min,Sec;
	Time /= Rate;
	Hour = (int)(Time / 3600000);
	Time -= Hour * 3600000;
	Min = (int)(Time / 60000);
	Time -= Min * 60000;
	Sec = (int)(Time / 1000);
	Time -= Sec * 1000;
	stprintf(s,T("CHAPTER%02d=%02d:%02d:%02d.%03d"),No,Hour,Min,Sec,(int)Time);
}

void Simplify(fraction* f, int MaxNum, int MaxDen)
{
	int Den = abs(f->Den);
	int Num = abs(f->Num);

	if ((int64_t)Num*MaxDen < (int64_t)Den*MaxNum)
	{
		if (Den > MaxDen)
		{
			f->Num = Scale(f->Num,MaxDen,Den);
			f->Den = Scale(f->Den,MaxDen,Den);
		}
	}
	else
	{
		if (Num > MaxNum)
		{
			f->Num = Scale(f->Num,MaxNum,Num);
			f->Den = Scale(f->Den,MaxNum,Num);
		}
	}
}

bool_t EqAudio(const audio* a, const audio* b)
{
	return a->Bits == b->Bits &&
		   a->Channels == b->Channels &&
		   a->SampleRate == b->SampleRate &&
		   a->FracBits == b->FracBits &&
		   a->Flags == b->Flags &&
		   a->Format == b->Format;
}

int UpperFourCC(int FourCC)
{
	return (toupper((FourCC >> 0) & 255) << 0) |
		   (toupper((FourCC >> 8) & 255) << 8) |
		   (toupper((FourCC >> 16) & 255) << 16) |
		   (toupper((FourCC >> 24) & 255) << 24);
}

bool_t EqFrac(const fraction* a, const fraction* b)
{
	if (a->Den == b->Den && a->Num == b->Num) 
		return 1;
	if (!a->Den) return b->Den==0;
	if (!b->Den) return 0;
	return (int64_t)b->Den * a->Num == (int64_t)a->Den * b->Num;
}

int BitMaskSize(uint32_t Mask)
{
	int i;
	for (i=0;Mask;++i)
		Mask &= Mask - 1;
	return i;
}

int BitMaskPos(uint32_t Mask)
{
	int i;
	for (i=0;Mask && !(Mask&1);++i) 
		Mask >>= 1;
	return i;
}

int ScaleRound(int v,int Num,int Den)
{
	int64_t i;
	if (!Den) 
		return 0;
	i = (int64_t)v * Num;
	if (i<0)
		i-=Den/2;
	else
		i+=Den/2;
	i/=Den;
	return (int)i;
}

void FillColor(uint8_t* Dst,int DstPitch,int x,int y,int Width,int Height,int BPP,int Value)
{
	if (Width>0 && Height>0)
	{
		uint16_t *p16,*p16e;
		uint32_t *p32,*p32e;
		uint8_t* End;

		Dst += y*DstPitch + (x*BPP)/8;
		End = Dst + Height * DstPitch;
		do
		{
			switch (BPP)
			{
			case 1:
				Value &= 1;
				memset(Dst,Value * 0xFF,Width >> 3);
				break;
			case 2:
				Value &= 3;
				memset(Dst,Value * 0x55,Width >> 2);
				break;
			case 4:
				Value &= 15;
				memset(Dst,Value * 0x11,Width >> 1);
				break;
			case 8:
				memset(Dst,Value,Width);
				break;
			case 16:
				p16 = (uint16_t*)Dst;
				p16e = p16+Width;
				for (;p16!=p16e;++p16)
					*p16 = (uint16_t)Value;
				break;
			case 32:
				p32 = (uint32_t*)Dst;
				p32e = p32+Width;
				for (;p32!=p32e;++p32)
					*p32 = Value;
				break;
			}
			Dst += DstPitch;
		}
		while (Dst != End);
	}
}

bool_t EqBlitFX(const blitfx* a, const blitfx* b)
{
	return a->Flags == b->Flags &&
		   a->Contrast == b->Contrast &&
		   a->Saturation == b->Saturation &&
		   a->Brightness == b->Brightness &&
		   a->Direction == b->Direction &&
		   a->RGBAdjust[0] == b->RGBAdjust[0] &&
		   a->RGBAdjust[1] == b->RGBAdjust[1] &&
		   a->RGBAdjust[2] == b->RGBAdjust[2] &&
		   a->ScaleX == b->ScaleX &&
		   a->ScaleY == b->ScaleY;
}

int CombineDir(int Src, int Blit, int Dst)
{
	//order of transformations 
	//  SrcMirror
	//  SrcSwap
	//  BlitSwap
	//  BlitMirror
	//  DstSwap
	//  DstMirror

	//should be combined to a single
	//  Swap
	//  Mirror

	if (Dst & DIR_SWAPXY)
	{
		if (Blit & DIR_MIRRORLEFTRIGHT)
			Dst ^= DIR_MIRRORUPDOWN;
		if (Blit & DIR_MIRRORUPDOWN)
			Dst ^= DIR_MIRRORLEFTRIGHT;
	}
	else
		Dst ^= Blit & (DIR_MIRRORUPDOWN|DIR_MIRRORLEFTRIGHT);
	Dst ^= Blit & DIR_SWAPXY;
	Dst ^= Src & DIR_SWAPXY;
	if (Dst & DIR_SWAPXY)
	{
		if (Src & DIR_MIRRORLEFTRIGHT)
			Dst ^= DIR_MIRRORUPDOWN;
		if (Src & DIR_MIRRORUPDOWN)
			Dst ^= DIR_MIRRORLEFTRIGHT;
	}
	else
		Dst ^= Src & (DIR_MIRRORUPDOWN|DIR_MIRRORLEFTRIGHT);

	return Dst;
}

int SurfaceRotate(const video* SrcFormat, const video* DstFormat, 
				  const planes Src, planes Dst, int Dir)
{
	blitfx FX;
	memset(&FX,0,sizeof(FX));
	FX.ScaleX = SCALE_ONE;
	FX.ScaleY = SCALE_ONE;
	FX.Direction = Dir;
	return SurfaceCopy(SrcFormat,DstFormat,Src,Dst,&FX);
}


int SurfaceCopy(const video* SrcFormat, const video* DstFormat, 
				const planes Src, planes Dst, blitfx* FX)
{
	void* Blit;
	rect SrcRect;
	rect DstRect;

	VirtToPhy(NULL,&SrcRect,SrcFormat);
	VirtToPhy(NULL,&DstRect,DstFormat);

	Blit = BlitCreate(DstFormat,SrcFormat,FX,NULL);
	if (!Blit)
		return ERR_NOT_SUPPORTED;

	BlitAlign(Blit,&DstRect,&SrcRect);
	BlitImage(Blit,Dst,*(const constplanes*)Src,NULL,-1,-1);
	BlitRelease(Blit);

	return ERR_NONE;
}

int SurfaceAlloc(planes Ptr, const video* p)
{
	int i;
	for (i=0;i<MAXPLANES;++i)
		Ptr[i] = NULL;

	if (p->Pixel.Flags & (PF_YUV420|PF_YUV422|PF_YUV444))
	{
		int x,y,s;
		PlanarYUV(&p->Pixel,&x,&y,&s);
		Ptr[0] = Alloc16(p->Height * p->Pitch);
		Ptr[1] = Alloc16((p->Height>>y) * (p->Pitch>>s));
		Ptr[2] = Alloc16((p->Height>>y) * (p->Pitch>>s));
		if (!Ptr[0] || !Ptr[1] || !Ptr[2])
		{
			SurfaceFree(Ptr);
			return ERR_OUT_OF_MEMORY;
		}
		return ERR_NONE;
	}
	Ptr[0] = Alloc16(GetImageSize(p));
	return Ptr[0] ? ERR_NONE : ERR_OUT_OF_MEMORY;
}

void SurfaceFree(planes p)
{
	int i;
	for (i=0;i<MAXPLANES;++i)
	{
		Free16(p[i]);
		p[i] = NULL;
	}
}

void DefaultPitch(video* p)
{
	p->Pitch = p->Width*GetBPP(&p->Pixel);
	if (p->Pixel.Flags & PF_RGB) 
		p->Pitch = ((p->Pitch+31)>>5)*4; // dword align
	else 
		p->Pitch = (p->Pitch+7)>>3; // byte align
}

void DefaultRGB(pixel* p, int BitCount, 
		         int RBits, int GBits, int BBits,
				 int RGaps, int GGaps, int BGaps)
{
	p->Flags = PF_RGB;
	p->BitCount = BitCount;
	p->BitMask[0] = ((1<<RBits)-1) << (RGaps+GBits+GGaps+BBits+BGaps);
	p->BitMask[1] = ((1<<GBits)-1) << (GGaps+BBits+BGaps);
	p->BitMask[2] = ((1<<BBits)-1) << (BGaps);
}

bool_t Compressed(const pixel* Fmt)
{
	return (Fmt->Flags & PF_FOURCC) && !AnyYUV(Fmt);
}

bool_t PlanarYUV(const pixel* Fmt, int* x, int* y,int *s)
{
	if (PlanarYUV420(Fmt))
	{
		if (x) *x = 1;
		if (y) *y = 1;
		if (s)
		{
			if (Fmt->Flags & PF_FOURCC &&
				((Fmt->FourCC == FOURCC_IMC2) || (Fmt->FourCC == FOURCC_IMC4)))
				*s = 0; // interleaved uv scanlines
			else
				*s = 1;
		}
		return 1;

	}
	if (PlanarYUV422(Fmt))
	{
		if (x) *x = 1;
		if (s) *s = 1;
		if (y) *y = 0;
		return 1;

	}
	if (PlanarYUV444(Fmt))
	{
		if (x) *x = 0;
		if (y) *y = 0;
		if (s) *s = 0;
		return 1;
	}

	if (x) *x = 0;
	if (y) *y = 0;
	if (s) *s = 0;
	return 0;
}

bool_t PlanarYUV420(const pixel* Fmt)
{
	if (Fmt->Flags & PF_YUV420)
		return 1;

	return (Fmt->Flags & PF_FOURCC) && 
	  	  ((Fmt->FourCC == FOURCC_YV12) ||
		   (Fmt->FourCC == FOURCC_IYUV) ||
		   (Fmt->FourCC == FOURCC_I420) ||
		   (Fmt->FourCC == FOURCC_IMC2) ||
		   (Fmt->FourCC == FOURCC_IMC4));
}

bool_t PlanarYUV422(const pixel* Fmt)
{
	if (Fmt->Flags & PF_YUV422)
		return 1;

	return (Fmt->Flags & PF_FOURCC) && (Fmt->FourCC == FOURCC_YV16);
}

bool_t PlanarYUV444(const pixel* Fmt)
{
	return (Fmt->Flags & PF_YUV444) != 0;
}

bool_t PackedYUV(const pixel* Fmt)
{
	return (Fmt->Flags & PF_FOURCC) && 
	
		  ((Fmt->FourCC == FOURCC_YUY2) ||
		   (Fmt->FourCC == FOURCC_YUNV) ||
		   (Fmt->FourCC == FOURCC_V422) ||
  		   (Fmt->FourCC == FOURCC_YUYV) ||
		   (Fmt->FourCC == FOURCC_VYUY) ||
		   (Fmt->FourCC == FOURCC_UYVY) ||
		   (Fmt->FourCC == FOURCC_Y422) ||
		   (Fmt->FourCC == FOURCC_YVYU) ||
		   (Fmt->FourCC == FOURCC_UYNV));
}

bool_t AnyYUV(const pixel* Fmt)
{
	return PlanarYUV420(Fmt) || 
		PlanarYUV422(Fmt) || 
		PlanarYUV444(Fmt) || 
		PackedYUV(Fmt);
}

uint32_t RGBToFormat(uint32_t RGB, const pixel* Fmt)
{
	uint32_t v;
	int R,G,B;
	int Y,U,V;
	int Pos[3];

	R = (RGB >> 0) & 255;
	G = (RGB >> 8) & 255;
	B = (RGB >> 16) & 255;

	if (AnyYUV(Fmt))
	{
		Y = ((2105 * R) + (4128 * G) + (802 * B))/0x2000 + 16;
		V = ((3596 * R) - (3015 * G) - (582 * B))/0x2000 + 128;
		U = (-(1212 * R) - (2384 * G) + (3596 * B))/0x2000 + 128;

		if (Fmt->Flags & PF_INVERTED)
		{
			Y ^= 255;
			U ^= 255;
			V ^= 255;
		}
		v =  (Fmt->BitMask[0] / 255) * Y;
		v += (Fmt->BitMask[1] / 255) * U;
		v += (Fmt->BitMask[2] / 255) * V;
	}
	else
	{
		if (Fmt->Flags & PF_INVERTED)
		{
			R ^= 255;
			G ^= 255;
			B ^= 255;
		}

		Pos[0] = BitMaskPos(Fmt->BitMask[0]) + BitMaskSize(Fmt->BitMask[0]);
		Pos[1] = BitMaskPos(Fmt->BitMask[1]) + BitMaskSize(Fmt->BitMask[1]);
		Pos[2] = BitMaskPos(Fmt->BitMask[2]) + BitMaskSize(Fmt->BitMask[2]);

		v = ((R << Pos[0]) & (Fmt->BitMask[0] << 8)) |
			((G << Pos[1]) & (Fmt->BitMask[1] << 8)) |
			((B << Pos[2]) & (Fmt->BitMask[2] << 8));
		v >>= 8;
	}

	return v;
}

void FillInfo(pixel* Fmt)
{
	Fmt->BitCount = GetBPP(Fmt);
	if (PlanarYUV(Fmt,NULL,NULL,NULL))
	{
		switch (Fmt->FourCC)
		{
		case FOURCC_IMC4:
		case FOURCC_I420: 
		case FOURCC_IYUV: 
			Fmt->BitMask[0] = 0x000000FF;
			Fmt->BitMask[1] = 0x0000FF00;
			Fmt->BitMask[2] = 0x00FF0000;
			break;
		case FOURCC_IMC2:
		case FOURCC_YV16:
		case FOURCC_YV12: 
			Fmt->BitMask[0] = 0x000000FF;
			Fmt->BitMask[1] = 0x00FF0000;
			Fmt->BitMask[2] = 0x0000FF00;
			break;
		}
	}
	else
	if (PackedYUV(Fmt))
		switch (Fmt->FourCC)
		{
		case FOURCC_YUY2:
		case FOURCC_YUNV:
		case FOURCC_V422:
		case FOURCC_YUYV:
			Fmt->BitMask[0] = 0x00FF00FF;
			Fmt->BitMask[1] = 0x0000FF00;
			Fmt->BitMask[2] = 0xFF000000;
			break;
		case FOURCC_YVYU:
			Fmt->BitMask[0] = 0x00FF00FF;
			Fmt->BitMask[1] = 0xFF000000;
			Fmt->BitMask[2] = 0x0000FF00;
			break;
		case FOURCC_UYVY:
		case FOURCC_Y422:
		case FOURCC_UYNV:
			Fmt->BitMask[0] = 0xFF00FF00;
			Fmt->BitMask[1] = 0x000000FF;
			Fmt->BitMask[2] = 0x00FF0000;
			break;
		}	
}

int GetImageSize(const video* p)
{
	int BPP = p->Pixel.BitCount;
	if (PlanarYUV420(&p->Pixel))
		BPP = 12;
	else
	if (PackedYUV(&p->Pixel) || PlanarYUV422(&p->Pixel))
		BPP = 16;
	else
	if (PlanarYUV444(&p->Pixel))
		BPP = 24;
	return (BPP * p->Pitch * p->Height) >> 3;
}

int GetBPP(const pixel* Fmt)
{
	if (Fmt->Flags & (PF_RGB | PF_PALETTE)) 
		return Fmt->BitCount;
	if (PlanarYUV(Fmt,NULL,NULL,NULL))
		return 8;
	if (PackedYUV(Fmt))
		return 16;
	return 0;
}

bool_t EqPoint(const point* a, const point* b)
{
	return a->x==b->x && a->y==b->y;
}

bool_t EqRect(const rect* a, const rect* b)
{
	return a->x==b->x && a->y==b->y && a->Width==b->Width && a->Height==b->Height;
}

bool_t EqPixel(const pixel* a, const pixel* b)
{
	if (a->Flags != b->Flags) 
		return 0;

	if ((a->Flags & PF_PALETTE) &&
		a->BitCount != b->BitCount)
		return 0;

	if ((a->Flags & PF_RGB) &&
		(a->BitCount != b->BitCount ||
		 a->BitMask[0] != b->BitMask[0] ||
		 a->BitMask[1] != b->BitMask[1] ||
		 a->BitMask[2] != b->BitMask[2]))
		return 0;

	if ((a->Flags & PF_FOURCC) && a->FourCC != b->FourCC)
		return 0;

	return 1;
}

bool_t EqVideo(const video* a, const video* b)
{
	// no direction check here!
	return a->Width == b->Width &&
		   a->Height == b->Height &&
		   a->Pitch == b->Pitch &&
		   EqPixel(&a->Pixel,&b->Pixel);
}

void ClipRectPhy(rect* Physical, const video* p)
{
	if (Physical->x < 0)
	{
		Physical->Width += Physical->x;
		Physical->x = 0;
	}
	if (Physical->y < 0)
	{
		Physical->Height += Physical->y;
		Physical->y = 0;
	}
	if (Physical->x + Physical->Width > p->Width)
	{
		Physical->Width = p->Width - Physical->x;
		if (Physical->Width < 0)
		{
			Physical->Width = 0;
			Physical->x = 0;
		}
	}
	if (Physical->y + Physical->Height > p->Height)
	{
		Physical->Height = p->Height - Physical->y;
		if (Physical->Height < 0)
		{
			Physical->Height = 0;
			Physical->y = 0;
		}
	}
}

void VirtToPhy(const rect* Virtual, rect* Physical, const video* p)
{
	if (Virtual)
	{
		*Physical = *Virtual;

		if (p->Pixel.Flags & PF_PIXELDOUBLE)
		{
			Physical->x >>= 1;
			Physical->y >>= 1;
			Physical->Width >>= 1;
			Physical->Height >>= 1;
		}

		if (p->Direction & DIR_SWAPXY)
			SwapRect(Physical);

		if (p->Direction & DIR_MIRRORLEFTRIGHT)
			Physical->x = p->Width - Physical->x - Physical->Width;

		if (p->Direction & DIR_MIRRORUPDOWN)
			Physical->y = p->Height - Physical->y - Physical->Height;

		ClipRectPhy(Physical,p);
	}
	else
	{
		Physical->x = 0;
		Physical->y = 0;
		Physical->Width = p->Width;
		Physical->Height = p->Height;
	}
}

void PhyToVirt(const rect* Physical, rect* Virtual, const video* p)
{
	if (Physical)
		*Virtual = *Physical;
	else
	{
		Virtual->x = 0;
		Virtual->y = 0;
		Virtual->Width = p->Width;
		Virtual->Height = p->Height;
	}

	if (p->Direction & DIR_MIRRORLEFTRIGHT)
		Virtual->x = p->Width - Virtual->x - Virtual->Width;

	if (p->Direction & DIR_MIRRORUPDOWN)
		Virtual->y = p->Height - Virtual->y - Virtual->Height;

	if (p->Direction & DIR_SWAPXY)
		SwapRect(Virtual);

	if (p->Pixel.Flags & PF_PIXELDOUBLE)
	{
		Virtual->x <<= 1;
		Virtual->y <<= 1;
		Virtual->Width <<= 1;
		Virtual->Height <<= 1;
	}
}

void SwapPByte(uint8_t** a, uint8_t** b)
{
	uint8_t* t = *a;
	*a = *b;
	*b = t;
}

void SwapPChar(tchar_t** a, tchar_t** b)
{
	tchar_t* t = *a;
	*a = *b;
	*b = t;
}

void SwapInt(int* a, int* b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

void SwapLong(long* a, long* b)
{
	long t = *a;
	*a = *b;
	*b = t;
}

void SwapBool(bool_t* a, bool_t* b)
{
	bool_t t = *a;
	*a = *b;
	*b = t;
}

void SwapPoint(point* p)
{
	int t = p->x;
	p->x = p->y;
	p->y = t;
}

void SwapRect(rect* r)
{
	int t1 = r->x;
	int t2 = r->Width;
	r->x = r->y;
	r->Width = r->Height;
	r->y = t1;
	r->Height = t2;
}

bool_t SetFileExt(tchar_t* URL, const tchar_t* Ext)
{
	tchar_t* p;
	bool_t HasHost;

	p = (tchar_t*) GetMime(URL,NULL,&HasHost);
	URL = p;
	
	p = tcsrchr(URL,'\\');
	if (!p)
		p = tcsrchr(URL,'/');
	if (p)
		URL = p+1;
	else
	if (HasHost) // only hostname
		return 0;
	
	if (!URL[0]) // no filename at all?
		return 0;

	p = tcsrchr(URL,'.');
	if (p)
		*p = 0;
	tcscat(URL,T("."));
	tcscat(URL,Ext);
	return 1;
}

void SplitURL(const tchar_t* URL, tchar_t* Mime, tchar_t* Dir, tchar_t* Name, tchar_t* Ext)
{
	const tchar_t* p;
	const tchar_t* p2;
	bool_t HasHost;
	bool_t MergeMime = Mime && Mime == Dir;

	// mime 
	p = GetMime(URL,MergeMime?NULL:Mime,&HasHost);
	if (!MergeMime)
		URL = p;

	// dir
	p2 = tcsrchr(p,'\\');
	if (!p2)
		p2 = tcsrchr(p,'/');
	if (p2)
	{
		if (Dir)
			TcsNCpy(Dir,URL,p2-URL+1);
		URL = p2+1;
	}
	else
	if (HasHost) // no filename, only host
	{
		if (Dir)
			tcscpy(Dir,URL);
		URL += tcslen(URL);
	}
	else // no directory
	{
		if (Dir)
			TcsNCpy(Dir,URL,p-URL+1);
		URL = p;
	}

	// name

	if (Name && Name == Ext)
		tcscpy(Name,URL);
	else
	{
		p = tcsrchr(URL,'.');
		if (p)
		{
			if (Name)
				TcsNCpy(Name,URL,p-URL+1);
			if (Ext)
				tcscpy(Ext,p+1);
		}
		else
		{
			if (Name)
				tcscpy(Name,URL);
			if (Ext)
				Ext[0] = 0;
		}
	}
}

void RelPath(tchar_t* Rel, const tchar_t* Any, const tchar_t* Base)
{
	unsigned int n = tcslen(Base);
	tcscpy(Rel,Any);
	if (n<tcslen(Rel) && (Rel[n]=='\\' || Rel[n]=='/'))
	{
		Rel[n] = 0;
		if (TcsICmp(Rel,Base)==0)
			Any += n+1;
		tcscpy(Rel,Any);
	}
}

bool_t UpperPath(tchar_t* Path, tchar_t* Last)
{
	tchar_t *a,*b,*c;
	bool_t HasHost;

	if (!*Path)
		return 0;

	c = (tchar_t*)GetMime(Path,NULL,&HasHost);
	
	a = tcsrchr(c,'\\');
	b = tcsrchr(c,'/');
	if (a<b)
		a=b;

	if (!a)
	{
		if (HasHost)
			return 0;
		a=c;
		if (!a[0]) // only mime left
			a=c=Path;
	}
	else
		++a;

	if (Last)
		tcscpy(Last,a);

	if (a==c)
		*a = 0;
	while (--a>=c && (*a=='\\' || *a=='/'))
		*a = 0;

	return 1;
}

void AbsPath(tchar_t* Abs, const tchar_t* Any, const tchar_t* Base)
{
	if (GetMime(Base,NULL,NULL)!=Base && (Any[0] == '/' || Any[0] == '\\'))
	{
		bool_t HasHost;
		tchar_t *a,*b;
		tcscpy(Abs,Base);
		Base = GetMime(Abs,NULL,&HasHost);
		if (!HasHost)
			++Any;
		a = tcschr(Base,'\\');
		b = tcschr(Base,'/');
		if (!a || (b && b<a))
			a=b;
		if (!a)
			a=Abs+tcslen(Abs);
		*a=0;
	}
	else
	if (GetMime(Any,NULL,NULL)==Any && Any[0] != '/' && Any[0] != '\\' &&
		!(Any[0] && Any[1]==':' && Any[2]=='\\'))
	{	
		const tchar_t* MimeEnd = GetMime(Base,NULL,NULL);
		tcscpy(Abs,Base);

#if defined(TARGET_WIN32) || defined(TARGET_WINCE)
		if (MimeEnd==Base)
			tcscat(Abs,T("\\"));
		else
#endif
		if (MimeEnd[0])
			tcscat(Abs,T("/"));
	}
	else
		Abs[0] = 0;

	tcscat(Abs,Any);

	if (GetMime(Abs,NULL,NULL)!=Abs)
		for (;*Abs;++Abs)
			if (*Abs == '\\')
				*Abs = '/';
}

bool_t CheckContentType(const tchar_t* s, const tchar_t* List)
{
	int n = tcslen(s);
	if (n)
	{
		while (List)
		{
			if (TcsNICmp(List,s,n)==0 && (!List[n] || List[n]==',' || List[n]==' '))
				return 1;
			List = tcschr(List,',');
			if (List) ++List;
		}
	}
	return 0;
}

bool_t UniqueExts(const int* Begin,const int* Pos)
{
	const tchar_t* Exts = String(*Pos,NODE_EXTS);
	if (!Exts[0])
		return 0;

	for (;Begin != Pos;++Begin)
		if (TcsICmp(Exts,String(*Begin,NODE_EXTS))==0)
			return 0;
	return 1;
}

int CheckExts(const tchar_t* URL, const tchar_t* Exts)
{
	tchar_t Ext[MAXPATH];
	tchar_t* Tail;

	SplitURL(URL,NULL,NULL,NULL,Ext);
	Tail = tcschr(Ext,'?');
	if (Tail) *Tail = 0;

	while (Exts)
	{
		const tchar_t* p = tcschr(Exts,':');
		if (p)
		{
			if (TcsNICmp(Ext,Exts,p-Exts)==0)
				return p[1]; // return type char
			p = tcschr(p,';');
			if (p) ++p;
		}
		Exts = p;
	}
	return 0;
}

void UpperUntil(tchar_t* s, tchar_t Delimiter)
{
	tchar_t Save = 0;
	tchar_t* p = tcschr(s,Delimiter);
	if (p)
	{
		Save = *p;
		*p = 0;
	}
	TcsUpr(s);
	if (p)
		*p = Save;
}

bool_t ReadLine(buffer* Buffer, stream* Stream, tchar_t* Out, int Len)
{
	bool_t Result = 0;
	char s[MAXLINE];
	char ch;
	int i=0;

	while (Buffer->WritePos>Buffer->ReadPos || BufferStream(Buffer,Stream))
	{
		ch = Buffer->Data[Buffer->ReadPos++];
		if (ch==10)
		{
			Result = 1;
			break;
		}
		else
		if (ch!=13 && i<sizeof(s)-1)
		{
			s[i++] = ch;
			Result = 1;
		}
	}

	s[i]=0;
	StrToTcs(Out,s,Len/sizeof(tchar_t));
	return Result;
}

void* Alloc16(int n)
{
	char* p = (char*) malloc(n+sizeof(void*)+16);
	if (p)
	{
		int i;
		char* p0 = p;
		p += sizeof(void*);
		i = (int)p & 15;
		if (i) p += 16-i;
		((void**)p)[-1] = p0;
	}
	return p;
}

void Free16(void* p)
{
	if (p)
		free(((void**)p)[-1]);
}

void ShowError(int Sender, int Class, int No,...)
{
	tchar_t s[1024];
	const tchar_t* Msg;

	if (Sender)
		stprintf(s, T("%s: "),String(Sender,0));
	else
		s[0] = 0;

	Msg = String(Class,No);
	if (Msg[0])
	{
		va_list Args;
		va_start(Args,No);
		vstprintf(s+tcslen(s), Msg, Args);
		va_end(Args);
	}
	else
	{
		FourCCToString(s+tcslen(s),Class,1024-tcslen(s));
		stprintf(s+tcslen(s), T("%04X"), No);
	}

	if (Context()->Error.Func)
		Context()->Error.Func(Context()->Error.This,Sender,(int)s);
	else
		ShowMessage(String(PLATFORM_ID,PLATFORM_ERROR),s);
}

