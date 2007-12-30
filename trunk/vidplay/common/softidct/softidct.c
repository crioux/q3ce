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
 * $Id: softidct.c 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/
 
#include "../common.h"
#include "softidct.h"

#ifdef MIPS64

// 8 byte steps
static void edge_copy(uint8_t *p_border_top,uint8_t *p_border_top_ref,int width){
	__asm (	"srl	$8,$6,4;"
			"lab1: ldr	$10,0($5);"
			"ldr	$11,8($5);"
#ifdef MIPSVR41XX
			".set noreorder;"
			"cache	13,0($4);" //cache without loading
			".set reorder;"
#endif
			"addi	$5,$5,16;"
			"sdr	$10,0($4);"
			"sdr	$11,8($4);"
			"addi	$8,$8,-1;"
			"addi	$4,$4,16;"
			"bgtz	$8,lab1;"
			"andi	$6,$6,0x08;"
			"beq	$6,$0,skip;"
			"ldr	$10,0($5);"
			"sdr	$10,0($4);"
			"skip:"
			);
}

// 16 byte steps
static void edge_set(uint8_t *p_border_top,int p_border_top_ref,int width) {
	__asm (	"srl	$8,$6,4;"
			"dsll	$10,$5,56;"
			"dsrl	$11,$10,8;"
			"or		$10,$10,$11;"
			"dsrl	$11,$10,16;"
			"or		$10,$10,$11;"
			"dsrl	$11,$10,32;"
			"or		$10,$10,$11;"

			"lab2:"
#ifdef MIPSVR41XX
			".set noreorder;"
			"cache	13,0($4);" //cache without loading
			".set reorder;"
#endif
			"sdr	$10,0($4);"
			"sdr	$10,8($4);"
			"addi	$8,$8,-1;"
			"addi	$4,$4,16;"
			"bgtz	$8,lab2;"
			);
}
#else
#define edge_copy memcpy
#define edge_set memset
#endif

static void FillEdge(uint8_t *Ptr, int Width, int Height, int EdgeX, int EdgeY)
{
	int n;
	uint8_t *p;
	int InnerWidth = Width - EdgeX*2;
	int InnerHeight = Height - EdgeY*2;
		
	// left and right
	p = Ptr + EdgeX + EdgeY * Width;
	for (n=0;n<InnerHeight;++n)
	{
		edge_set(p-EdgeX, p[0], EdgeX);
		edge_set(p+InnerWidth,p[InnerWidth-1], EdgeX);
		p += Width;
	}

	// top
	p = Ptr;
	for (n=0;n<EdgeY;++n)
	{
		edge_copy(p, Ptr+EdgeY * Width, Width);
		p += Width;
	}

	// bottom
	p = Ptr + (Height-EdgeY)*Width;
	for (n=0;n<EdgeY;++n)
	{
		edge_copy(p, Ptr+(Height-EdgeY-1)*Width, Width);
		p += Width;
	}
}

static void FillEdgeYUV(softidct* p,uint8_t *Ptr)
{
	FillEdge(Ptr, p->BufWidth, p->BufHeight, EDGE, EDGE);
	Ptr += p->YToU;
	FillEdge(Ptr, p->BufWidth>>p->UVX2, p->BufHeight>>p->UVY2, EDGE>>p->UVX2, EDGE>>p->UVY2);
	Ptr += p->YToU >> (p->UVX2+p->UVY2);
	FillEdge(Ptr, p->BufWidth>>p->UVX2, p->BufHeight>>p->UVY2, EDGE>>p->UVX2, EDGE>>p->UVY2);
}

#if defined(MIPS64_WIN32) 
static int TouchPages(softidct* p)
{
	uint8_t* i;
	int Sum = 0;
	for (i=p->CodeBegin;i<p->CodeEnd;i+=p->CodePage)
		Sum += *i;
	return Sum;
}
#endif

static int Lock(softidct* p,int No,planes Planes,int* Brightness,video* Format)
{
	if (Brightness)
		*Brightness = 0;
	if (Format)
	{
		*Format = p->Out.Format.Video;
		Format->Pitch = p->BufWidth;
		Format->Width = p->BufWidth - 2*EDGE;
		Format->Height = p->BufHeight - 2*EDGE;
	}

	if (No<0 || No>=p->BufCount)
	{
		Planes[0] = NULL;
		Planes[1] = NULL;
		Planes[2] = NULL;
		return ERR_INVALID_PARAM;
	}

	Planes[0] = p->Buffer[No] + EDGE + EDGE*p->BufWidth;
	Planes[1] = p->Buffer[No] + p->YToU + (EDGE >> p->UVX2) + (EDGE >> p->UVY2)*(p->BufWidth >> p->UVX2);
	Planes[2] = (uint8_t*)Planes[1] + (p->YToU >> (p->UVX2+p->UVY2));
	return ERR_NONE;
}

static void Unlock(softidct* p,int No)
{
}

static int UpdateRounding(softidct* p)
{
	p->CopyBlock = p->AllCopyBlock[0][p->Rounding];
	p->CopyMBlock = p->AllCopyBlock[1][p->Rounding];
	return ERR_NONE;
}

static int SetBufferCount(softidct* p, int n, int Temp)
{
	int i;
	n += Temp;
	if (n>p->MaxCount) return ERR_NOT_SUPPORTED;

	for (i=n;i<p->BufCount;++i)
	{
		FreeBlock(&p->_Buffer[i]);
		p->Buffer[i] = NULL;
	}
	if (p->BufCount > n)
		p->BufCount = n;

	for (i=p->BufCount;i<n;++i)
	{
#ifndef MIPS
		int Align = 32;
		int Offset = 0;
#else
		int Align = 8192;
		int Offset = ((i>>1)+(i&1)*2)*2048;
#endif
		if (!AllocBlock(p->BufSize+Align+Offset,&p->_Buffer[i],0,HEAP_ANYWR))
		{
			p->MaxCount = p->BufCount;
			break;
		}
		if (i>=2 && AvailMemory()<64*1024)
		{
			FreeBlock(&p->_Buffer[i]);
			ShowOutOfMemory();
			p->MaxCount = p->BufCount;
			break;
		}
		p->Buffer[i] = (uint8_t*)(((int)p->_Buffer[i].Ptr + Align-1) & ~(Align-1));
		p->Buffer[i] += Offset;
		p->BufBorder[i] = 0;
		p->BufFrameNo[i] = -1;
		p->BufCount = i+1;
	}

	if (p->ShowNext >= p->BufCount)
		p->ShowNext = -1;
	if (p->ShowCurr >= p->BufCount)
		p->ShowCurr = -1;
	p->LastTemp = Temp && p->BufCount == n;

	if (p->BufCount != n)
		return ERR_OUT_OF_MEMORY;

	return ERR_NONE;
}

static int UpdateFormat(softidct* p)
{
	if (p->Out.Format.Type == PACKET_VIDEO)
	{
		int AlignX,AlignY,Mode;

		PlanarYUV(&p->Out.Format.Video.Pixel,&p->UVX2,&p->UVY2,NULL);

		AlignX = (8 << p->UVX2) - 1;
		AlignY = (8 << p->UVY2) - 1;

		p->BufWidth = ((p->BufferWidth+AlignX)&~AlignX)+2*EDGE;
		p->BufHeight = ((p->BufferHeight+AlignY)&~AlignY)+2*EDGE;
		if (p->Out.Format.Video.Direction & DIR_SWAPXY)
			SwapInt(&p->BufWidth,&p->BufHeight);

		p->YToU = p->BufHeight * p->BufWidth;
		p->BufSize = p->YToU + 2*(p->YToU >> (p->UVX2+p->UVY2));
		p->BufWidthUV = p->BufWidth >> p->UVX2;

		p->Out.Format.Video.Pitch = p->BufWidth;

		if (p->Out.Format.Video.Pixel.Flags & PF_YUV420)
		{
			Mode = 0;
			p->Tab[0] = 2*(8);					//Y[0;0] -> Y[0;1]
			p->Tab[1] = 2*(8*p->BufWidth-8);	//Y[0;1] -> Y[1;0]
			p->Tab[2] = 2*(8);					//Y[1;0] -> Y[1;1]
			p->Tab[4] = 2*(p->YToU >> 2);		//U->V
			p->Tab[5] = 2*(0);	
		}
		else 
		if (p->Out.Format.Video.Pixel.Flags & PF_YUV422)
		{
			Mode = 1;
			p->Tab[0] = 2*(8);					//Y[0;0] -> Y[0;1]
			p->Tab[2] = 2*(p->YToU >> 1);		//U->V
			p->Tab[3] = 2*(0);	
		}
		else
		if (p->Out.Format.Video.Pixel.Flags & PF_YUV444)
		{
			Mode = 2;
			p->Tab[0] = 2*(p->YToU);			//Y->U
			p->Tab[1] = 2*(p->YToU);			//U->V
			p->Tab[2] = 2*(0);					
		}
		else
			return ERR_NOT_SUPPORTED;

#ifdef IDCTSWAP
		if (p->Out.Format.Video.Direction & DIR_SWAPXY)
		{
			p->IDCT.Process = p->ProcessSwap[Mode];
			p->IDCT.Copy16x16 = p->Copy16x16Swap[Mode];
			p->IDCT.Intra8x8 = p->Intra8x8Swap;
			p->IDCT.MComp8x8 = (idctmcomp) SoftMComp8x8Swap;
			p->IDCT.MComp16x16 = (idctmcomp) SoftMComp16x16Swap;

			if (Mode == 0)
			{
				p->Tab[0] = 2*(8*p->BufWidth);		//Y[0;0] -> Y[1;0]
				p->Tab[1] = 2*(8-8*p->BufWidth);	//Y[1;0] -> Y[0;1]
				p->Tab[2] = 2*(8*p->BufWidth);		//Y[0;1] -> Y[1;1]
			}
		}
		else
#endif
		{
			p->IDCT.Process = p->Process[Mode];
			p->IDCT.Copy16x16 = p->Copy16x16[Mode];
			p->IDCT.Intra8x8 = p->Intra8x8;
			p->IDCT.MComp8x8 = (idctmcomp) SoftMComp8x8;
			p->IDCT.MComp16x16 = (idctmcomp) SoftMComp16x16;
		}

#ifdef MIPS64
		p->IDCT.Inter8x8 = Inter8x8Add;
#endif

	}
	return ConnectionUpdate((node*)p,IDCT_OUTPUT,p->Out.Pin.Node,p->Out.Pin.No);
}

#ifdef IDCTSWAP
static int ChangeSwap(softidct* p)
{
	if (p->BufCount > 0)
	{
		int No;
		planes TmpPlanes;
		video TmpFormat;
		int Brightness;
		video Format[2];
		planes Planes[2];

		TmpFormat = p->Out.Format.Video;
		TmpFormat.Width = p->BufWidth - 2*EDGE;
		TmpFormat.Height = p->BufHeight - 2*EDGE;

		if (SurfaceAlloc(TmpPlanes,&TmpFormat) == ERR_NONE)
		{
			if (Lock(p,0,Planes[0],&Brightness,&Format[0])==ERR_NONE)
			{
				SurfaceCopy(&Format[0],&TmpFormat,Planes[0],TmpPlanes,NULL);
				Unlock(p,0);
			}

			for (No=1;No<p->BufCount;++No)
				if (Lock(p,No,Planes[0],&Brightness,&Format[0])==ERR_NONE)
				{
					SwapInt(&p->BufWidth,&p->BufHeight);
					if (Lock(p,0,Planes[1],&Brightness,&Format[1])==ERR_NONE)
					{
						block Tmp;
						SurfaceRotate(&Format[0],&Format[1],Planes[0],Planes[1],DIR_SWAPXY);
						Unlock(p,0);

						SwapPByte(&p->Buffer[No],&p->Buffer[0]);
						Tmp = p->_Buffer[0];
						p->_Buffer[0] = p->_Buffer[No];
						p->_Buffer[No] = Tmp;
						p->BufBorder[No] = 0;
					}
					SwapInt(&p->BufWidth,&p->BufHeight);
					Unlock(p,No);
				}

			SwapInt(&p->BufWidth,&p->BufHeight);
			if (Lock(p,0,Planes[0],&Brightness,&Format[0])==ERR_NONE)
			{
				SurfaceRotate(&TmpFormat,&Format[0],TmpPlanes,Planes[0],DIR_SWAPXY);
				Unlock(p,0);
				p->BufBorder[0] = 0;
			}
			SwapInt(&p->BufWidth,&p->BufHeight);

			SurfaceFree(TmpPlanes);
		}
	}

	p->Out.Format.Video.Direction ^= DIR_SWAPXY;
	SwapInt(&p->Out.Format.Video.Width,&p->Out.Format.Video.Height);
	return UpdateFormat(p);
}
#endif

static int SetFormat(softidct* p, const video* Format)
{
	SetBufferCount(p,0,0);
	PacketFormatClear(&p->Out.Format);

	if (Format)
	{
		if (!(Format->Pixel.Flags & (PF_YUV420|PF_YUV422|PF_YUV444)))
			return ERR_NOT_SUPPORTED;

		p->Out.Format.Type = PACKET_VIDEO;
		p->Out.Format.Video = *Format;
		if (p->Out.Format.Video.Direction & DIR_SWAPXY)
			SwapInt(&p->Out.Format.Video.Width,&p->Out.Format.Video.Height);
		p->Out.Format.Video.Direction = 0;
		p->Out.Format.Video.Pixel.Flags |= PF_16ALIGNED | PF_SAFEBORDER;
		p->MaxCount = MAXBUF;
		p->Rounding = 0;
		UpdateRounding(p);

	}

#ifdef FREESCALE_MX1
	if (p->MX1)
	{
		volatile int* Cmd = p->MX1-64; //0x22400
		volatile int* GCCR = p->MX1-6972; //0x1B810
		if (Format)
		{
			*GCCR |= 2;
			ThreadSleep(1);
			*Cmd = 0x284C+0x20;
			ThreadSleep(1);
			*Cmd = 0x284D;
		}
		else
		{
			*Cmd = 0x00;
			*GCCR &= ~2;
		}
	}
#endif

	return UpdateFormat(p);
}

static int Send(softidct* p,tick_t RefTime,const flowstate* State);

static int Set(softidct* p, int No, const void* Data, int Size)
{
	flowstate State;
	int Result = ERR_INVALID_PARAM;

	switch (No)
	{
	case NODE_SETTINGSCHANGED:
		p->NeedLast = QueryAdvanced(ADVANCED_SLOW_VIDEO);
		break;

	case IDCT_SHOW: 
		SETVALUE(p->ShowNext,int,ERR_NONE); 
		if (p->ShowNext >= p->BufCount) p->ShowNext = -1;
		break;

	case FLOW_FLUSH:
		p->ShowCurr = -1;
		p->ShowNext = -1;
		return ERR_NONE;

	case IDCT_BACKUP: 
		assert(Size == sizeof(idctbackup));
		Result = IDCTRestore(&p->IDCT,(idctbackup*)Data);
		break;

	case IDCT_BUFFERWIDTH: SETVALUE(p->BufferWidth,int,ERR_NONE); break;
	case IDCT_BUFFERHEIGHT: SETVALUE(p->BufferHeight,int,ERR_NONE); break;
	case IDCT_FORMAT:
		assert(Size == sizeof(video) || !Data);
		Result = SetFormat(p,(const video*)Data);
		break;

	case IDCT_OUTPUT|PIN_PROCESS: SETVALUE(p->Out.Process,packetprocess,ERR_NONE); break;

#ifdef IDCTSWAP
	case IDCT_OUTPUT|PIN_FORMAT:
		assert(Size == sizeof(packetformat) || !Data);
		if (Data && QueryAdvanced(ADVANCED_IDCTSWAP) && 
			!(p->Out.Format.Video.Pixel.Flags & PF_YUV422) &&
			PacketFormatRotatedVideo(&p->Out.Format,(packetformat*)Data,DIR_SWAPXY))
			Result = ChangeSwap(p);
		break;
#endif

	case IDCT_OUTPUT: SETVALUE(p->Out.Pin,pin,ERR_NONE); break;
	case IDCT_ROUNDING: SETVALUE(p->Rounding,bool_t,UpdateRounding(p)); break;
	case IDCT_BUFFERCOUNT:
		assert(Size == sizeof(int));
		Result = ERR_NONE;
		if (p->BufCount < *(const int*)Data)
			Result = SetBufferCount(p,*(const int*)Data,0);
		break;

	case FLOW_RESEND:
		State.CurrTime = TIME_RESEND;
		State.DropLevel = 0;
		Result = Send(p,-1,&State);
		break;
	}

	if (No>=IDCT_FRAMENO && No<IDCT_FRAMENO+p->BufCount)
		SETVALUE(p->BufFrameNo[No-IDCT_FRAMENO],int,ERR_NONE);

	return Result;
}

static int Get(softidct* p, int No, void* Data, int Size)
{
	int Result = ERR_INVALID_PARAM;
	switch (No)
	{
	case FLOW_BUFFERED: GETVALUE(1,bool_t); break;
	case IDCT_OUTPUT|PIN_PROCESS: GETVALUE(p->Out.Process,packetprocess); break;
	case IDCT_OUTPUT|PIN_FORMAT: GETVALUE(p->Out.Format,packetformat); break;
	case IDCT_OUTPUT: GETVALUE(p->Out.Pin,pin); break;
	case IDCT_FORMAT: GETVALUE(p->Out.Format.Video,video); break;
	case IDCT_ROUNDING: GETVALUE(p->Rounding,bool_t); break;
	case IDCT_BUFFERCOUNT: GETVALUE(p->BufCount - p->LastTemp,int); break;
	case IDCT_BUFFERWIDTH: GETVALUE(p->BufferWidth,int); break;
	case IDCT_BUFFERHEIGHT: GETVALUE(p->BufferHeight,int); break;
	case IDCT_SHOW: GETVALUE(p->ShowNext,int); break;
	case IDCT_BACKUP: 
		assert(Size == sizeof(idctbackup));
		Result = IDCTBackup(&p->IDCT,(idctbackup*)Data);
		break;
	}
	if (No>=IDCT_FRAMENO && No<IDCT_FRAMENO+p->BufCount)
		GETVALUE(p->BufFrameNo[No-IDCT_FRAMENO],int);
	return Result;
}

static void Drop(softidct* p)
{
	int i;
	for (i=0;i<p->BufCount;++i)
		p->BufFrameNo[i] = -1;
}

static int Null(softidct* p,const flowstate* State,bool_t Empty)
{
	packet* Packet = NULL;
	packet EmptyPacket;
	flowstate DropState;
	if (!State)
	{
		DropState.CurrTime = TIME_UNKNOWN;
		DropState.DropLevel = 1;
		State = &DropState;
	}
	if (Empty)
	{
		memset(&EmptyPacket,0,sizeof(EmptyPacket));
		EmptyPacket.RefTime = TIME_UNKNOWN;
		Packet = &EmptyPacket;
	}
	return p->Out.Process(p->Out.Pin.Node,Packet,State);
}

static int Send(softidct* p,tick_t RefTime,const flowstate* State)
{
	packet Packet;
	int Result = ERR_INVALID_DATA;

	if (State->DropLevel)
		return p->Out.Process(p->Out.Pin.Node,NULL,State);

	if (p->ShowNext < 0)
		return ERR_NEED_MORE_DATA;

	if (p->ShowCurr == p->ShowNext)
		p->ShowCurr = -1;

	if (Lock(p,p->ShowNext,*(planes*)&Packet.Data,NULL,NULL) == ERR_NONE)
	{
		Lock(p,p->ShowCurr,*(planes*)&Packet.LastData,NULL,NULL);

		Packet.RefTime = RefTime;
		Result = p->Out.Process(p->Out.Pin.Node,&Packet,State);

		Unlock(p,p->ShowCurr);
		Unlock(p,p->ShowNext);
	}

	if (Result != ERR_BUFFER_FULL)
		p->ShowCurr = p->ShowNext;

	return Result;
}

static int FrameStart(softidct* p,int FrameNo,int* OldFrameNo,int DstNo,int BackNo,int FwdNo,int ShowNo,bool_t Drop)
{
#if defined(MIPS64_WIN32) 
	// win32 doesn't support 64bit natively. thread and process
	// changes won't save upper part of 64bit registers.
	// we have to disable interrupt and make sure all 1KB
	// code pages are in memory so no page loading is called
	// during code execution 
	TouchPages(p);	
	DisableInterrupts();
	p->NextIRQ = 0;
#endif

#ifdef ARM
	// ShowCurr will be the next "LastData" in output packet
	// we don't want to lose it's content
	if (p->NeedLast && p->ShowCurr == DstNo)
	{
		int SwapNo;
		for (SwapNo=0;SwapNo<p->BufCount;++SwapNo)
			if (SwapNo != DstNo && SwapNo != BackNo && SwapNo != FwdNo && SwapNo != ShowNo)
				break;

		if (SwapNo < p->BufCount || SetBufferCount(p,p->BufCount,1)==ERR_NONE)
		{
			//DEBUG_MSG2("IDCT Swap %d,%d",SwapNo,DstNo);
			block Tmp;
			
			Tmp = p->_Buffer[SwapNo];
			p->_Buffer[SwapNo] = p->_Buffer[DstNo];
			p->_Buffer[DstNo] = Tmp;

			SwapPByte(&p->Buffer[SwapNo],&p->Buffer[DstNo]);
			SwapInt(&p->BufFrameNo[SwapNo],&p->BufFrameNo[DstNo]);
			SwapBool(&p->BufBorder[SwapNo],&p->BufBorder[DstNo]);

			p->ShowCurr = SwapNo;
		}
	}
#endif

	p->ShowNext = ShowNo;

	if (OldFrameNo)
		*OldFrameNo = p->BufFrameNo[DstNo];
	p->BufFrameNo[DstNo] = FrameNo;

	p->Dst = p->Buffer[DstNo];
	p->Ref[0] = NULL;
	p->Ref[1] = NULL;
	p->RefMin[0] = NULL;
	p->RefMin[1] = NULL;
	p->RefMax[0] = NULL;
	p->RefMax[1] = NULL;

	if (BackNo>=0)
	{
		p->Ref[0] = p->Buffer[BackNo];
		if (!p->BufBorder[BackNo])
		{
			p->BufBorder[BackNo] = 1;
			FillEdgeYUV(p,p->Ref[0]);
		}
		p->RefMin[0] = p->Ref[0];
		p->RefMax[0] = p->Ref[0] + p->BufSize -8-8*p->BufWidthUV;
	}

	if (FwdNo>=0)
	{
		p->Ref[1] = p->Buffer[FwdNo];
		if (!p->BufBorder[FwdNo])
		{
			p->BufBorder[FwdNo] = 1;
			FillEdgeYUV(p,p->Ref[1]);
		}
		p->RefMin[1] = p->Ref[1];
		p->RefMax[1] = p->Ref[1] + p->BufSize -8-8*p->BufWidthUV;
	}

	p->BufBorder[DstNo] = 0; //invalidate border for dst

#ifndef MIPS64
#ifdef IDCTSWAP
	if (p->Out.Format.Video.Direction & DIR_SWAPXY)
		p->IDCT.Inter8x8 = p->Inter8x8Swap[p->Ref[1]!=NULL];
	else
#endif
		p->IDCT.Inter8x8 = p->Inter8x8[p->Ref[1]!=NULL];
#endif

#ifdef FREESCALE_MX1
	if (p->MX1)
		p->MX1Pop = MX1PopNone;
#endif

	return ERR_NONE;
} 

static void FrameEnd(softidct* p)
{
#ifdef MIPS64_WIN32
	EnableInterrupts();
#endif
#ifdef FREESCALE_MX1
	if (p->MX1)
		p->MX1Pop(p);
#endif
}

static const copyblock AllCopyBlock[2][2][4] = //[8x8/16x16][Rounding][x][y]
{
   {{ CopyBlock, CopyBlockHor, CopyBlockVer, CopyBlockHorVer },
	{ CopyBlock, CopyBlockHorRound, CopyBlockVerRound, CopyBlockHorVerRound }},
#ifdef MIPS64
   {{ CopyMBlock, CopyMBlockHor, CopyMBlockVer, CopyMBlockHorVer },
	{ CopyMBlock, CopyMBlockHorRound, CopyMBlockVerRound, CopyMBlockHorVerRound }},
#endif
};

static int Create(softidct* p)
{
#if defined(MMX)
	if (!(QueryPlatform(PLATFORM_CAPS) & CAPS_X86_MMX))
		return ERR_NOT_SUPPORTED;
#endif

#if defined(MIPSVR41XX)
	if (!QueryAdvanced(ADVANCED_VR41XX) || 
		!(QueryPlatform(PLATFORM_CAPS) & CAPS_MIPS_VR4120))
		return ERR_NOT_SUPPORTED;
#endif

	p->IDCT.Get = (nodeget)Get;
	p->IDCT.Set = (nodeset)Set;
	p->IDCT.Send = (idctsend)Send;
	p->IDCT.Null = (idctnull)Null;
	p->IDCT.Drop = (idctdrop)Drop;
	p->IDCT.Lock = (idctlock)Lock;
	p->IDCT.Unlock = (idctunlock)Unlock;
	p->IDCT.FrameStart = (idctframestart)FrameStart;
	p->IDCT.FrameEnd = (idctframeend)FrameEnd;

#if !defined(MIPS64) && !defined(MIPS32)
	p->AddBlock[0] = AddBlock;
	p->AddBlock[1] = AddBlockHor;
	p->AddBlock[2] = AddBlockVer;
	p->AddBlock[3] = AddBlockHorVer;
#endif

	memcpy(p->AllCopyBlock,AllCopyBlock,sizeof(AllCopyBlock));

	p->Copy16x16[0] = (idctcopy)Copy420;
	p->Process[0] = (idctprocess)Process420;
	p->Process[1] = (idctprocess)Process422;
	p->Process[2] = (idctprocess)Process444;
	p->Intra8x8 = (idctintra)Intra8x8;

#ifndef MIPS64
	p->Inter8x8[0] = (idctinter)Inter8x8Back;
	p->Inter8x8[1] = (idctinter)Inter8x8BackFwd;
#endif

#ifdef IDCTSWAP
	p->Copy16x16Swap[0] = (idctcopy)CopySwap420;
	p->ProcessSwap[0] = (idctprocess)ProcessSwap420;
	p->ProcessSwap[1] = (idctprocess)ProcessSwap422;
	p->ProcessSwap[2] = (idctprocess)ProcessSwap444;
	p->Intra8x8Swap = (idctintra)Intra8x8Swap;

#ifndef MIPS64
	p->Inter8x8Swap[0] = (idctinter)Inter8x8BackSwap;
	p->Inter8x8Swap[1] = (idctinter)Inter8x8BackFwdSwap;
#endif
#endif //IDCTSWAP

#if defined(MIPS64_WIN32) 
	CodeFindPages(*(void**)&p->IDCT.Enum,&p->CodeBegin,&p->CodeEnd,&p->CodePage);
#endif

#if defined(ARM)

#if defined(FREESCALE_MX1)
	if (QueryPlatform(PLATFORM_MODEL)==MODEL_ZODIAC)
	{
		p->MX1 = (int*)0x80022500;

		p->Intra8x8 = (idctintra)MX1Intra8x8;
		p->Inter8x8[0] = (idctinter)MX1Inter8x8Back;
		p->Inter8x8[1] = (idctinter)MX1Inter8x8BackFwd;

#ifdef IDCTSWAP
		p->Intra8x8Swap = (idctintra)MX1Intra8x8Swap;
		p->Inter8x8Swap[0] = (idctinter)MX1Inter8x8BackSwap;
		p->Inter8x8Swap[1] = (idctinter)MX1Inter8x8BackFwdSwap;
#endif //IDCTSWAP
	}
	else
#endif

#if !defined(TARGET_PALMOS) || (__GNUC__>=4) || (__GNUC__==3 && __GNUC_MINOR__>=4)
	if (!QueryAdvanced(ADVANCED_NOWMMX) && 
		(QueryPlatform(PLATFORM_CAPS) & CAPS_ARM_WMMX))
	{
		p->Intra8x8 = (idctintra)WMMXIntra8x8;
		p->Inter8x8[0] = (idctinter)WMMXInter8x8Back;
		p->Inter8x8[1] = (idctinter)WMMXInter8x8BackFwd;

#ifdef IDCTSWAP
		p->Intra8x8Swap = (idctintra)WMMXIntra8x8Swap;
		p->Inter8x8Swap[0] = (idctinter)WMMXInter8x8BackSwap;
		p->Inter8x8Swap[1] = (idctinter)WMMXInter8x8BackFwdSwap;
#endif //IDCTSWAP

		p->AllCopyBlock[0][0][0] = WMMXCopyBlock;
		p->AllCopyBlock[0][0][1] = WMMXCopyBlockHor;
		p->AllCopyBlock[0][0][2] = WMMXCopyBlockVer;
		p->AllCopyBlock[0][0][3] = WMMXCopyBlockHorVer;
		p->AllCopyBlock[0][1][0] = WMMXCopyBlock;
		p->AllCopyBlock[0][1][1] = WMMXCopyBlockHorRound;
		p->AllCopyBlock[0][1][2] = WMMXCopyBlockVerRound;
		p->AllCopyBlock[0][1][3] = WMMXCopyBlockHorVerRound;
		p->AddBlock[0] = WMMXAddBlock;
		p->AddBlock[1] = WMMXAddBlockHor;
		p->AddBlock[2] = WMMXAddBlockVer;
		p->AddBlock[3] = WMMXAddBlockHorVer;
	}
	else
#endif // TARGET_PALMOS
	{}
/*	if (QueryPlatform(PLATFORM_CAPS) & CAPS_ARM_5E)
	{
		if (QueryPlatform(PLATFORM_ICACHE) >= 32768)
		{
			p->AllCopyBlock[0][0][0] = FastCopyBlock;
			p->AllCopyBlock[0][0][1] = FastCopyBlockHor;
			p->AllCopyBlock[0][0][2] = FastCopyBlockVer;
			p->AllCopyBlock[0][0][3] = FastCopyBlockHorVer;
			p->AllCopyBlock[0][0][0] = FastCopyBlock;
			p->AllCopyBlock[0][1][1] = FastCopyBlockHorRound;
			p->AllCopyBlock[0][1][2] = FastCopyBlockVerRound;
			p->AllCopyBlock[0][1][3] = FastCopyBlockHorVerRound;
			p->AddBlock[0] = FastAddBlock;
			p->AddBlock[1] = FastAddBlockHor;
			p->AddBlock[2] = FastAddBlockVer;
			p->AddBlock[3] = FastAddBlockHorVer;
		}
		else
		{
			p->AllCopyBlock[0][0][0] = PreLoadCopyBlock;
			p->AllCopyBlock[0][0][1] = PreLoadCopyBlockHor;
			p->AllCopyBlock[0][0][2] = PreLoadCopyBlockVer;
			p->AllCopyBlock[0][0][3] = PreLoadCopyBlockHorVer;
			p->AllCopyBlock[0][0][0] = PreLoadCopyBlock;
			p->AllCopyBlock[0][1][1] = PreLoadCopyBlockHorRound;
			p->AllCopyBlock[0][1][2] = PreLoadCopyBlockVerRound;
			p->AllCopyBlock[0][1][3] = PreLoadCopyBlockHorVerRound;
			p->AddBlock[0] = PreLoadAddBlock;
			p->AddBlock[1] = PreLoadAddBlockHor;
			p->AddBlock[2] = PreLoadAddBlockVer;
			p->AddBlock[3] = PreLoadAddBlockHorVer;
		}
	}
*/

#endif //ARM

	p->Tmp = (uint8_t*)ALIGN16(p->_Tmp);
	p->MaxCount = MAXBUF;
	p->BufCount = 0;
	p->LastTemp = 0;
	p->ShowCurr = -1;
	p->ShowNext = -1;

	return ERR_NONE;
}

static const nodedef SoftIDCT =
{
	sizeof(softidct),
#if defined(MPEG4_EXPORTS)
	MPEG4IDCT_ID,
	IDCT_CLASS,
	PRI_DEFAULT+20,
#elif defined(MSMPEG4_EXPORTS)
	MSMPEG4IDCT_ID,
	IDCT_CLASS,
	PRI_DEFAULT+10,
#else
	SOFTIDCT_ID,
	IDCT_CLASS,
	PRI_DEFAULT,
#endif
	(nodecreate)Create,
};

void SoftIDCT_Init()
{
	NodeRegisterClass(&SoftIDCT);
}

void SoftIDCT_Done()
{
	NodeUnRegisterClass(SoftIDCT.Class);
}
