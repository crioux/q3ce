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
 * $Id: block.h 23 2004-07-21 12:08:19Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef SWAPXY
#define Block4x8 IDCT_Block4x8
#define Block8x8 IDCT_Block8x8
#define MV_X(v) ((v<<16)>>17)
#define MV_Y(v) (v>>17)
#define MV_SUB(v) (v&1)+((v>>15)&2)
#else
#define Block4x8 IDCT_Block4x8Swap
#define Block8x8 IDCT_Block8x8Swap
#define MV_X(v) (v>>17)
#define MV_Y(v) ((v<<16)>>17)
#define MV_SUB(v) ((v<<1)&2)+((v>>16)&1)
#endif

void Intra8x8(softidct* p,idct_block_t *Block,int Length,int ScanType)
{
	Statistics(Block,Length,ScanType,0);

#ifdef SWAP8X4
	if (ScanType!=IDCTSCAN_ALT_VERT && (Length < 11 || (Length<20 && Block[32]==0)))
		Block4x8(Block,p->DstPtr,p->CurrPitch,NULL);
#else
	if (ScanType!=IDCTSCAN_ALT_HORI && Length < 15)
		Block4x8(Block,p->DstPtr,p->CurrPitch,NULL);
#endif
	else
		Block8x8(Block,p->DstPtr,p->CurrPitch,NULL);

	EMMS();
	IncPtr(p,0,0);
}

#ifndef MIPS64

void Inter8x8BackFwd(softidct* p,idct_block_t *Block,int Length)
{
	uint8_t* Ptr;
	int MV;

	if (Length)
	{
		// mcomp and idct (using tmp buffer)

		if (p->MVBack)
		{
			MV = *(p->MVBack++);
			Ptr = p->RefPtr[0] + MV_X(MV) + p->CurrPitch * MV_Y(MV);
			if (Ptr >= p->RefMin[0] && Ptr < p->RefMax[0])
				p->CopyBlock[MV_SUB(MV)](Ptr,p->Tmp,p->CurrPitch,8);

			if (p->MVFwd)
			{
				MV = *(p->MVFwd++);
				Ptr = p->RefPtr[1] + MV_X(MV) + p->CurrPitch * MV_Y(MV);
				if (Ptr >= p->RefMin[1] && Ptr < p->RefMax[1])
				{
#if defined(MIPS32)
					p->CopyBlock[MV_SUB(MV)](Ptr,p->Tmp+64,p->CurrPitch,8);
					AddBlock8x8(p->Tmp+64,p->Tmp,8,8);
#else
					p->AddBlock[MV_SUB(MV)](Ptr,p->Tmp,p->CurrPitch);
#endif
				}
			}
		}
		else
		if (p->MVFwd)
		{
			MV = *(p->MVFwd++);
			Ptr = p->RefPtr[1] + MV_X(MV) + p->CurrPitch * MV_Y(MV);
			if (Ptr >= p->RefMin[1] && Ptr < p->RefMax[1])
				p->CopyBlock[MV_SUB(MV)](Ptr,p->Tmp,p->CurrPitch,8);
		}

		if (Length == 1)
			IDCT_Const8x8((Block[0]+4) >> 3,p->DstPtr,p->CurrPitch,p->Tmp);
#ifdef SWAP8X4
		else if (Length < 11 || (Length<20 && Block[32]==0))
			Block4x8(Block,p->DstPtr,p->CurrPitch,p->Tmp);
#else
		else if (Length < 15)
			Block4x8(Block,p->DstPtr,p->CurrPitch,p->Tmp);
#endif
		else
			Block8x8(Block,p->DstPtr,p->CurrPitch,p->Tmp);
	}
	else 
	{
		// interpolate back and foward (using tmp buffer)

		if (p->MVBack && p->MVFwd)
		{
			MV = *(p->MVBack++);
			Ptr = p->RefPtr[0] + MV_X(MV) + p->CurrPitch * MV_Y(MV);
			if (Ptr >= p->RefMin[0] && Ptr < p->RefMax[0])
				p->CopyBlock[MV_SUB(MV)](Ptr,p->Tmp,p->CurrPitch,8);

			MV = *(p->MVFwd++);
			Ptr = p->RefPtr[1] + MV_X(MV) + p->CurrPitch * MV_Y(MV);
			if (Ptr >= p->RefMin[1] && Ptr < p->RefMax[1])
			{
#if defined(MIPS32)
				p->CopyBlock[MV_SUB(MV)](Ptr,p->DstPtr,p->CurrPitch,p->CurrPitch);
				AddBlock8x8(p->Tmp,p->DstPtr,8,p->CurrPitch);
#else
				p->AddBlock[MV_SUB(MV)](Ptr,p->Tmp,p->CurrPitch);
				// copy Tmp to Dst
				CopyBlock(p->Tmp,p->DstPtr,8,p->CurrPitch);
#endif
			}
		}
		else
		if (p->MVBack)
		{
			MV = *(p->MVBack++);
			Ptr = p->RefPtr[0] + MV_X(MV) + p->CurrPitch * MV_Y(MV);
			if (Ptr >= p->RefMin[0] && Ptr < p->RefMax[0])
				p->CopyBlock[MV_SUB(MV)](Ptr,p->DstPtr,p->CurrPitch,p->CurrPitch);
		}
		else
		if (p->MVFwd)
		{
			MV = *(p->MVFwd++);
			Ptr = p->RefPtr[1] + MV_X(MV) + p->CurrPitch * MV_Y(MV);
			if (Ptr >= p->RefMin[1] && Ptr < p->RefMax[1])
				p->CopyBlock[MV_SUB(MV)](Ptr,p->DstPtr,p->CurrPitch,p->CurrPitch);
		}
	}

	EMMS();
	IncPtr(p,1,1);
}

void Inter8x8Back(softidct* p,idct_block_t *Block,int Length)
{
	uint8_t* Ptr;
	int MV;

	if (Length)
	{
		// mcomp and idct (using tmp buffer)

		MV = *(p->MVBack++);
		Ptr = p->RefPtr[0] + MV_X(MV) + p->CurrPitch * MV_Y(MV);
		if (Ptr >= p->RefMin[0] && Ptr < p->RefMax[0])
			p->CopyBlock[MV_SUB(MV)](Ptr,p->Tmp,p->CurrPitch,8);
		Statistics(Block,Length,0,1);

		if (Length == 1)
			IDCT_Const8x8((Block[0]+4) >> 3,p->DstPtr,p->CurrPitch,p->Tmp);
#ifdef SWAP8X4
		else if (Length < 11 || (Length<20 && Block[32]==0))
			Block4x8(Block,p->DstPtr,p->CurrPitch,p->Tmp);
#else
		else if (Length < 15 || (Length<26 && ((uint32_t*)Block)[2]==0 && ((uint32_t*)Block)[6]==0))
			Block4x8(Block,p->DstPtr,p->CurrPitch,p->Tmp);
#endif
		else
			Block8x8(Block,p->DstPtr,p->CurrPitch,p->Tmp);
	}
	else 
	{
		// only back mcomp
		MV = *(p->MVBack++);
		Ptr = p->RefPtr[0] + MV_X(MV) + p->CurrPitch * MV_Y(MV);
		if (Ptr >= p->RefMin[0] && Ptr < p->RefMax[0])
			p->CopyBlock[MV_SUB(MV)](Ptr,p->DstPtr,p->CurrPitch,p->CurrPitch);
	}

	EMMS();
	IncPtr(p,1,0);
}

#endif

#undef MV_X
#undef MV_Y
#undef MV_SUB
#undef Block4x8
#undef Block8x8



