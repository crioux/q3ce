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
 * $Id: flow.c 131 2004-12-04 20:36:04Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"

static const datatable FlowParams[] =
{
	{ FLOW_BUFFERED,TYPE_BOOL, DF_HIDDEN },

	DATATABLE_END(FLOW_CLASS)
};

int FlowEnum(void* p, int* No, datadef* Param)
{
	return NodeEnumTable(No,Param,FlowParams);
}

static const nodedef Flow =
{
	0,
	FLOW_CLASS,
	NODE_CLASS,
	PRI_DEFAULT,
};

static const datatable OutParams[] =
{
	{ OUT_INPUT,	TYPE_PACKET, DF_INPUT },
	{ OUT_OUTPUT,	TYPE_PACKET, DF_OUTPUT|DF_RDONLY },
	{ OUT_TOTAL,	TYPE_INT,	 DF_HIDDEN },
	{ OUT_DROPPED,	TYPE_INT,	 DF_HIDDEN },

	DATATABLE_END(OUT_CLASS)
};

int OutEnum(void* p, int* No, datadef* Param)
{
	return NodeEnumTable(No,Param,OutParams);
}

static const nodedef Out =
{
	0,
	OUT_CLASS,
	FLOW_CLASS,
	PRI_DEFAULT,
};

void Flow_Init()
{
	NodeRegisterClass(&Flow);
	NodeRegisterClass(&Out);
}

void Flow_Done()
{
	NodeUnRegisterClass(OUT_CLASS);
	NodeUnRegisterClass(FLOW_CLASS);
}

void Disconnect(node* Src,int SrcNo,node* Dst,int DstNo)
{
	ConnectionUpdate(NULL,0,Dst,DstNo);
	ConnectionUpdate(Src,SrcNo,NULL,0);
}

int ConnectionUpdate(node* Src,int SrcNo,node* Dst,int DstNo)
{
	int Result;
	pin Pin;
	packetformat Format;
	packetprocess Process;

	memset(&Format,0,sizeof(Format));

	// first setup connection via Pin, so when PIN_FORMAT is set the nodes can negotiate
	// ConnectionUpdate should not disconnect nodes even is something fails

	if (Dst && Dst->Get(Dst,DstNo,&Pin,sizeof(Pin))==ERR_NONE && (Pin.Node != Src || Pin.No != SrcNo))
	{
		Pin.Node = Src;
		Pin.No = SrcNo;
		Result = Dst->Set(Dst,DstNo,&Pin,sizeof(Pin));
		if (Result != ERR_NONE)
			return Result;
	}

	if (Src && Src->Get(Src,SrcNo,&Pin,sizeof(Pin))==ERR_NONE && (Pin.Node != Dst || Pin.No != DstNo))
	{
		Pin.Node = Dst;
		Pin.No = DstNo;
		Result = Src->Set(Src,SrcNo,&Pin,sizeof(Pin));
		if (Result != ERR_NONE)
			return Result;
	}

	if (Src && Dst)
	{
		Src->Get(Src,SrcNo|PIN_FORMAT,&Format,sizeof(Format));
		Result = Dst->Set(Dst,DstNo|PIN_FORMAT,&Format,sizeof(Format));
		if (Result != ERR_NONE)
			Dst->Set(Dst,DstNo|PIN_FORMAT,NULL,0);

		Dst->Get(Dst,DstNo|PIN_PROCESS,&Process,sizeof(packetprocess));
		Src->Set(Src,SrcNo|PIN_PROCESS,&Process,sizeof(packetprocess));

		if (Result != ERR_NONE)
			return Result;
	}
	else
	if (Dst)
		Dst->Set(Dst,DstNo|PIN_FORMAT,NULL,0);

	return ERR_NONE;
}

int DummyProcess(void* p, const packet* Packet, const flowstate* State)
{
	if (State->CurrTime >= 0 && Packet && Packet->RefTime > State->CurrTime + SHOWAHEAD)
		return ERR_BUFFER_FULL;
	return ERR_NONE;
}

static bool_t ContentType(const packetformat* Format,tchar_t* s,int Size)
{
	switch (Format->Type)
	{
	case PACKET_VIDEO:
		tcscpy(s,T("vcodec/"));
		FourCCToString(s+7,Format->Video.Pixel.FourCC,Size-7*sizeof(tchar_t));
		break;
	case PACKET_AUDIO:
		stprintf(s,T("acodec/0x%04x"),Format->Audio.Format);
		break;
	case PACKET_SUBTITLE:
		tcscpy(s,T("subtitle/"));
		FourCCToString(s+9,Format->Subtitle.FourCC,Size-9*sizeof(tchar_t));
		break;
	default:
		return 0;
	}
	return 1;
}

void PacketFormatEnumClass(array* List, const packetformat* Format)
{
	tchar_t s[16];
	if (ContentType(Format,s,sizeof(s)))
		NodeEnumClassEx(List,FLOW_CLASS,s,NULL,NULL,0);
	else
		memset(List,0,sizeof(array));
}

bool_t PacketFormatMatch(int Class, const packetformat* Format)
{
	tchar_t s[16];
	const tchar_t* Supported = String(Class,NODE_CONTENTTYPE);
	assert(Supported[0]!=0);
	if (Supported[0]==0)
		return 0;
	if (!ContentType(Format,s,sizeof(s)))
		return 0;
	return CheckContentType(s,Supported);
}

bool_t PacketFormatRotatedVideo(const packetformat* Current, const packetformat* New,int Mask)
{
	if (Current && Current->Type == PACKET_VIDEO && New && New->Type == PACKET_VIDEO &&
		(Current->Video.Direction ^ New->Video.Direction) & Mask)
	{
		video Tmp = New->Video;
		Tmp.Pitch = Current->Video.Pitch;
		if ((Current->Video.Direction ^ Tmp.Direction) & DIR_SWAPXY)
			SwapInt(&Tmp.Width,&Tmp.Height);
		Tmp.Direction = Current->Video.Direction;
		return EqVideo(&Current->Video,&Tmp);
	}
	return 0;
}

int PacketFormatCopy(packetformat* Dst, const packetformat* Src)
{
	PacketFormatClear(Dst);
	if (Src)
	{
		*Dst = *Src;
		if (Src->ExtraLength >= 0)
		{
			Dst->Extra = NULL;
			Dst->ExtraLength = 0;
			if (Src->ExtraLength && PacketFormatExtra(Dst,Src->ExtraLength))
				memcpy(Dst->Extra,Src->Extra,Dst->ExtraLength);
		}
	}
	return ERR_NONE;
}

void PacketFormatCombine(packetformat* Dst, const packetformat* Src)
{
	if (Dst->Type == Src->Type)
	{
		if (!Dst->ByteRate)
			Dst->ByteRate = Src->ByteRate;
		if (!Dst->PacketRate.Num)
			Dst->PacketRate = Src->PacketRate;

		switch (Dst->Type)
		{
		case PACKET_VIDEO:
			if (!Dst->Video.Width && !Dst->Video.Height)
			{
				Dst->Video.Width = Src->Video.Width;
				Dst->Video.Height = Src->Video.Height;
				Dst->Video.Direction = Src->Video.Direction;
			}
			if (!Dst->Video.Aspect)
				Dst->Video.Aspect = Src->Video.Aspect;
			break;

		case PACKET_AUDIO:
			// force update
			Dst->Audio.Channels = Src->Audio.Channels;
			Dst->Audio.SampleRate = Src->Audio.SampleRate; 
			if (!Dst->Audio.Bits)
			{
				Dst->Audio.Bits = Src->Audio.Bits;
				Dst->Audio.FracBits = Src->Audio.FracBits;
			}
			break;
		}
	}
}

void PacketFormatClear(packetformat* p)
{
	if (p->ExtraLength>=0)
		free(p->Extra);
	memset(p,0,sizeof(packetformat));
}

bool_t PacketFormatExtra(packetformat* p, int Length)
{
	void* Extra = realloc(p->Extra,Length);
	if (!Extra && Length)
		return 0;
	p->Extra = Extra;
	p->ExtraLength = Length;
	return 1;
}

void PacketFormatPCM(packetformat* p, const packetformat* In, int Bits)
{
	PacketFormatClear(p);
	p->Type = PACKET_AUDIO;
	p->Audio.Format = AUDIOFMT_PCM;
	p->Audio.Bits = Bits;
	p->Audio.SampleRate = In->Audio.SampleRate;
	p->Audio.Channels = In->Audio.Channels;
	if (p->Audio.Channels > 2)
		p->Audio.Channels = 2;
	PacketFormatDefault(p);
}

void PacketFormatDefault(packetformat* p)
{
	switch (p->Type)
	{
	case PACKET_VIDEO:
		if (p->Video.Pixel.FourCC==0) // DIB?
		{
			switch (p->Video.Pixel.BitCount)
			{
			case 16:
				DefaultRGB(&p->Video.Pixel,p->Video.Pixel.BitCount,5,5,5,0,0,0);
				break;
			case 24:
			case 32:
				DefaultRGB(&p->Video.Pixel,p->Video.Pixel.BitCount,8,8,8,0,0,0);
				break;
			}
			p->Video.Direction = DIR_MIRRORUPDOWN;
		}
		else
		if (p->Video.Pixel.FourCC==3)
		{
			p->Video.Pixel.Flags = PF_RGB;
			p->Video.Direction = DIR_MIRRORUPDOWN;
		}
		else
		{
			p->Video.Pixel.Flags = PF_FOURCC;
			p->Video.Pixel.FourCC = UpperFourCC(p->Video.Pixel.FourCC);
			p->Video.Direction = 0;
		}

		DefaultPitch(&p->Video);

		if (p->Video.Height<0)
		{
			p->Video.Height = -p->Video.Height;
			p->Video.Direction ^= DIR_MIRRORUPDOWN;
		}
		break;

	case PACKET_AUDIO:
		// detect fake PCM
		if (p->Audio.Format > 8192 && 
			p->Audio.BlockAlign > 0 &&
			p->ByteRate > 0 &&
			p->Audio.BlockAlign == ((p->Audio.Channels * p->Audio.Bits) >> 3) &&
			p->ByteRate == p->Audio.SampleRate * p->Audio.BlockAlign)
			p->Audio.Format = AUDIOFMT_PCM;

		if (p->Audio.Format == AUDIOFMT_PCM)
		{
			p->Audio.FracBits = p->Audio.Bits - 1;
			if (p->Audio.Bits <= 8)
				p->Audio.Flags = PCM_UNSIGNED;
			p->Audio.Bits = (p->Audio.Bits + 7) & ~7;
			p->Audio.BlockAlign = (p->Audio.Channels * p->Audio.Bits) >> 3;
			p->ByteRate = p->Audio.SampleRate * p->Audio.BlockAlign;
		}
		break;
	}
}	

bool_t PacketFormatName(packetformat* p, tchar_t* Name)
{
	tchar_t Id[8];
	switch (p->Type)
	{
	case PACKET_SUBTITLE:
		FourCCToString(Id,p->Subtitle.FourCC,sizeof(Id));
		tcscpy(Name,String(p->Subtitle.FourCC,0x4400));
		if (Name[0])
			tcscat(Name,T(", "));
		tcscat(Name,Id);
		return 1;
	case PACKET_AUDIO:
		if (p->Audio.Format != AUDIOFMT_PCM)
		{
			stprintf(Id,T("%04X"),p->Audio.Format);
			tcscpy(Name,String(FOURCC(Id[0],Id[1],Id[2],Id[3]),0x4400));
			if (Name[0])
				tcscat(Name,T(", "));
			tcscat(Name,Id);
		}
		else
			tcscpy(Name,T("PCM"));
		return 1;
	case PACKET_VIDEO:
		if (Compressed(&p->Video.Pixel))
		{
			FourCCToString(Id,p->Video.Pixel.FourCC,sizeof(Id));
			tcscpy(Name,String(p->Video.Pixel.FourCC,0x4400));
			if (Name[0])
				tcscat(Name,T(", "));
			tcscat(Name,Id);
		}
		else
		if (AnyYUV(&p->Video.Pixel))
			tcscpy(Name,T("YUV"));
		else
			stprintf(Name,T("RGB %d bits"),p->Video.Pixel.BitCount);
		return 1;
	}
	return 0;
}

