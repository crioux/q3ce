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
 * $Id: mp4_decode.c 206 2005-03-19 15:03:03Z picard $z
 *
 *****************************************************************************
 *
 * Authors:
 *
 *	Andrea	Graziani  (Ag): Original source code (Open Divx Decoder 0.4a).
 *	Marc	Dukette   (Md) and
 *	Pedro	Mateu     (Pm) and
 *	Gabor	Kovacs    (Kg) Heavily modified and optimized code
 *
 ****************************************************************************/

#include "stdafx.h"

static int mp4_updatecount(mp4_decode* dec)
{
	int* mv;
	if (dec->Codec.IDCT.Count >= 3)
	{
		int size;
		if (dec->mv_bufmask != ~0)
			dec->mv_bufinvalid = 1;
		dec->mv_bufmask = ~0; // full size mv buffer needed

		if (!dec->_mv_buf)
		{
			size = 32 + 4*sizeof(int) * (dec->pos_end+1); // add one extra block for left border
			dec->_mv_buf = (int*)malloc(size); 
			if (!dec->_mv_buf)
				return ERR_INVALID_DATA;
			memset(dec->_mv_buf,0,size);
		}
		mv = dec->_mv_buf;
	}
	else
	{
		dec->mv_bufinvalid = 1;
		dec->mv_bufmask = MB_X-1; //only one row of mv buffer needed
		mv = dec->_mv_bufrow;
	}

	dec->mv_buf = (int*)(((int)mv + 31 + 4*sizeof(int)) & ~31);
	return ERR_NONE;
}

static int mp4_updatesize(mp4_decode* dec)
{
	if (dec->Codec.IDCT.Width > (MB_X-1)*16 || dec->Codec.IDCT.Height > (MB_Y-1)*16)
	{
		if (dec->showerror)
			ShowError(dec->Codec.Node.Class,VOUT_CLASS,VOUT_ERROR_SIZE,(MB_X-1)*16,(MB_Y-1)*16);
		return ERR_NOT_SUPPORTED;
	}

	free(dec->_mv_buf);
	dec->mb_xsize = (dec->Codec.IDCT.Width + 15) / 16; 
	dec->mb_ysize = (dec->Codec.IDCT.Height + 15) / 16;
	dec->mb_bits = _log2(dec->mb_xsize*dec->mb_ysize);
	dec->pos_end = dec->mb_ysize * MB_X;
	dec->_mv_buf = NULL;
	dec->mv_buf = NULL;
	memset(dec->_mv_bufrow,0,sizeof(dec->_mv_bufrow));
	return ERR_NONE;
}

static int mp4_discontinuity( mp4_decode* dec )
{
	dec->frame = 0;
	dec->packed_length = 0;
	return ERR_NONE;
}

static int mp4_flush( mp4_decode* dec )
{
	dec->startstate = -1;
	dec->startfound = 0;
	dec->last_bframe = 0; 
	return ERR_NONE;
}

static int mp4_updateinput( mp4_decode* dec )
{
	int result = ERR_NONE;

	free(dec->packed_data);
	dec->packed_data = NULL;
	dec->packed_allocated = 0;
	dec->packed_length = 0;

	free(dec->_mv_buf);
	dec->_mv_buf = NULL;
	dec->mv_buf = NULL;

	dec->mb_xsize = 0; 
	dec->mb_ysize = 0;
	dec->pos_end = 0;

	IDCT_BLOCK_PREPARE(dec,dec->blockptr);

	dec->quant_precision = 5;
	dec->sprite = 0;
	dec->time_increment_resolution = 30000;
	dec->time_increment_bits = 15;
	dec->last_reftime = 0;
	dec->time_pp = 0;
	dec->time_bp = 0;
	dec->validvol = 0;
	dec->needvol = 0;
	dec->refframe = 0;

	if (dec->Codec.In.Format.Type == PACKET_VIDEO)
	{
		int IDCTWidth,IDCTHeight;

		dec->showerror = 1;

		if (!dec->Codec.In.Format.PacketRate.Num)
		{
			dec->Codec.In.Format.PacketRate.Num = 25;
			dec->Codec.In.Format.PacketRate.Den = 1;
		}

		dec->Codec.FrameTime = Scale(TICKSPERSEC,dec->Codec.In.Format.PacketRate.Den,dec->Codec.In.Format.PacketRate.Num);

		IDCTWidth = dec->Codec.In.Format.Video.Width;
		IDCTHeight = dec->Codec.In.Format.Video.Height;

		if (dec->Codec.Node.Class == H263_ID)
		{
			IDCTWidth = 0;
			IDCTHeight = 0;
		}

		result = CodecIDCTSetFormat(&dec->Codec,PF_YUV420,dec->Codec.In.Format.Video.Width,dec->Codec.In.Format.Video.Height,
									 		              IDCTWidth,IDCTHeight);

		if (result==ERR_NONE && dec->Codec.Node.Class != MSMPEG4_ID)
		{
			dec->needvol = 1;
			if (dec->Codec.In.Format.ExtraLength > 0)
			{
				initbits(dec,dec->Codec.In.Format.Extra,dec->Codec.In.Format.ExtraLength);
				dec->ignoresize = (char)(dec->Codec.In.Format.Video.Width*dec->Codec.In.Format.Video.Height > 0);
				dec->gethdr(dec);
				dec->ignoresize = 0;
				if (dec->validvol)
					dec->needvol = 0;
			}
		}
	}
	return result;
}

static int mp4_vop( mp4_decode *dec, const uint8_t *start, bool_t packed )
{
	int result;
	int64_t TRB,TRD;
	
	dec->resyncpos = 0;

	switch (dec->prediction_type) 
	{
	case N_VOP:
	case P_VOP:

		if (dec->frame < 1)
		{
			if (dec->Codec.Dropping)
				dec->Codec.IDCT.Ptr->Null(dec->Codec.IDCT.Ptr,NULL,0);
			return ERR_INVALID_DATA;
		}

		if (dec->prediction_type == N_VOP)
		{
			if (!packed)
				dec->Codec.IDCT.Ptr->Null(dec->Codec.IDCT.Ptr,&dec->Codec.State,1);
			return ERR_NONE;
		}

	case I_VOP:

		// I/P-frames are the "reference" frames
		// they are decode into refframe 
		// P-frames depend on last refenrence frame: refframe^1

		dec->refframe ^= 1;

		if (dec->Codec.IDCT.Count>=3)
		{
			dec->last_bframe++; 
			if (dec->last_bframe < 3)
				dec->Codec.State.DropLevel = 0;
		}

		if (dec->Codec.IDCT.Count<3 || dec->frame==0)
			dec->Codec.Show = dec->refframe; // show this refframe
		else
			dec->Codec.Show = dec->refframe ^ 1; // show last refframe 

		if (dec->prediction_type == I_VOP)
		{
			result = dec->Codec.IDCT.Ptr->FrameStart(dec->Codec.IDCT.Ptr,dec->frame,&dec->bufframe,dec->refframe,-1,-1,dec->Codec.Show,0);
			if (result != ERR_NONE)
			{
				dec->Codec.Show = -1;
				return result;
			}

			dec->IVOP( dec );
		}
		else 
		{
			result = dec->Codec.IDCT.Ptr->FrameStart(dec->Codec.IDCT.Ptr,dec->frame,&dec->bufframe,dec->refframe,dec->refframe ^ 1,-1,dec->Codec.Show,dec->Codec.State.DropLevel);
			if (result != ERR_NONE)
			{
				dec->Codec.Show = -1;
				return result;
			}

			dec->PVOP( dec );
		}
		break;

	case B_VOP:
		if (dec->frame < 2 || dec->mv_bufinvalid)
		{
			if (dec->Codec.Dropping)
				dec->Codec.IDCT.Ptr->Null(dec->Codec.IDCT.Ptr,NULL,0);
			return ERR_INVALID_DATA;
		}

		dec->last_bframe = 0;

		if (dec->Codec.State.DropLevel)
		{
			dec->Codec.Show = -1;
			dec->Codec.IDCT.Ptr->Null(dec->Codec.IDCT.Ptr,NULL,0);
			return ERR_NONE;
		}

		if (dec->mv_bufinvalid == 2)
		{
			dec->mv_bufinvalid = 0;
			memset(dec->mv_buf,0,4*sizeof(int)*dec->pos_end);
		}

		if (dec->Codec.IDCT.Count<3)
		{
			CodecIDCTSetCount(&dec->Codec,3);
			if (dec->Codec.IDCT.Count<3)
			{
				dec->Codec.IDCT.Ptr->Null(dec->Codec.IDCT.Ptr,NULL,0);
				return ERR_INVALID_DATA;
			}
			if (dec->mv_bufinvalid)
				return ERR_INVALID_DATA;
		}

		// save this b-frame if there was already a packed one
		if (dec->Codec.Show >= 0) 
		{
			// rewind
			dec->bitpos = 32;
			dec->bitptr = start;
			return ERR_NONE;
		}

		dec->Codec.Show = 2;
		result = dec->Codec.IDCT.Ptr->FrameStart(dec->Codec.IDCT.Ptr,-dec->frame,&dec->bufframe,2,dec->refframe,dec->refframe ^ 1,2,dec->Codec.State.DropLevel);
		if (result != ERR_NONE)
			return result;
			
		TRB = dec->time_pp - dec->time_bp;
		TRD = dec->time_pp;

		if (TRD)
		{
			dec->TRB = (TRB << 32) / TRD;
			dec->TRB_TRD = ((TRB - TRD) << 32) / TRD;
		}

		dec->BVOP( dec );
		break;

	default:
		return ERR_INVALID_DATA;
	}

	dec->Codec.IDCT.Ptr->FrameEnd(dec->Codec.IDCT.Ptr);
	dec->frame++;

	// is there a possible (uint8) framemap overflow?
	if ((dec->frame - dec->mapofs) >= 128) 
	{
		int pos;
		for (pos=0;pos<dec->pos_end;pos+=MB_X-dec->mb_xsize)
			for (;POSX(pos)<dec->mb_xsize;++pos)
			{
				int i = dec->framemap[pos];
				if (i >= (120<<1))
					i -= 120<<1;
				else
					i &= 1;
				dec->framemap[pos] = (uint8_t)i;
			}

		dec->mapofs += 120;
	}
	return ERR_NONE;
}

static int mp4_frame( mp4_decode* dec, const uint8_t* ptr, int len )
{
	const uint8_t* pos;
	int result;
	bool_t packed = dec->packed_length>0;

	if (packed)
	{
		initbits(dec,dec->packed_data,dec->packed_length);

		if (dec->gethdr(dec))
			mp4_vop(dec,dec->packed_data,0);

		dec->packed_length = 0;
	}

	if (len == 0)
		return ERR_INVALID_DATA;

	initbits(dec,ptr,len);

	if (!dec->gethdr(dec) || (dec->needvol && !dec->validvol) || dec->Codec.IDCT.Count<2)
		return ERR_INVALID_DATA; // invalid header or no vol information

	result = mp4_vop(dec,ptr,packed);

	if (result == ERR_NONE && dec->Codec.Show>=0) 
	{
		// check for packed b-frame
		pos = bytepos(dec);
		len -= (pos - ptr);

		if (len > 4) 
		{
			DEBUG_MSG1(DEBUG_VCODEC,T("Packed %d"),len);

			if (dec->Codec.IDCT.Count<3)
				CodecIDCTSetCount(&dec->Codec,3);

			if (dec->Codec.IDCT.Count>=3)
			{
				// save packed b-frame
				if (dec->packed_allocated < len)
				{
					if (dec->packed_data)
						free(dec->packed_data);
					dec->packed_allocated = (len+8 + 0x1FFF) & ~0x1FFF;
					dec->packed_data = malloc(dec->packed_allocated);
					if (!dec->packed_data)
						dec->packed_allocated = 0;
				}

				if (dec->packed_data) 
				{
					dec->packed_length = len;
					memcpy(dec->packed_data,pos,len);
				}
			}
			else
			if (dec->frame >= 2 || dec->Codec.Dropping)
			{
				initbits(dec,pos,len);
				if (dec->gethdr(dec))			
					dec->Codec.IDCT.Ptr->Null(dec->Codec.IDCT.Ptr,NULL,0);
			}
		}
	}

	return result;
}

static int mp4_enum( mp4_decode* dec, int* no, datadef* param )
{
	int result = CodecIDCTEnum(dec,no,param);
	if (result == ERR_NONE && param->No == CODECIDCT_IDCT && dec->ownidct)
		param->Flags |= DF_RDONLY;
	return result;
}

void mp4_create( mp4_decode* dec )
{
	int i;

	dec->Codec.Node.Enum = (nodeenum)mp4_enum;
	dec->Codec.MinCount = 2;
	dec->Codec.Frame = (codecidctframe)mp4_frame;
	dec->Codec.UpdateSize = (nodefunc)mp4_updatesize;
	dec->Codec.UpdateCount = (nodefunc)mp4_updatecount;
	dec->Codec.UpdateInput = (nodefunc)mp4_updateinput;
	dec->Codec.Discontinuity = (nodefunc)mp4_discontinuity;
	dec->Codec.Flush = (nodefunc)mp4_flush;

	for (i = 0; i < 4*MB_X*2; i++)
		dec->dc_lum[i] = 1024;
	for (i = 0; i < 2*MB_X; i++) 
	{
		dec->dc_chr[0][i] = 1024;
		dec->dc_chr[1][i] = 1024;
	}

#if defined(MIPS64)
#ifdef MSMPEG4
	dec->ownidct = (idct*)NodeCreate(MSMPEG4IDCT_ID);
#endif
#ifdef MPEG4
	dec->ownidct = (idct*)NodeCreate(MPEG4IDCT_ID);
#endif
	dec->Codec.Node.Set(&dec->Codec.Node,CODECIDCT_IDCT,&dec->ownidct,sizeof(idct*));
#endif
}

void mp4_delete( mp4_decode* dec )
{
	if (dec->ownidct)
		NodeDelete((node*)dec->ownidct);
}

#ifdef MPEG4

static int mpeg4_create( mp4_decode* dec )
{
	mp4_create(dec);
	dec->Codec.FindNext = (codecidctnext)findnext_mpeg4;
	dec->gethdr = gethdr_mpeg4;
	dec->IVOP = IVOP_mpeg4;
	dec->PVOP = PVOP_mpeg4;
	dec->BVOP = BVOP_mpeg4;
	dec->vld_block = vld_block_mpeg4;
	return ERR_NONE;
}

static int h263_create( mp4_decode* dec )
{
	mp4_create(dec);
	dec->IVOP = IVOP_h263;
	dec->PVOP = PVOP_h263;
	dec->BVOP = BVOP_h263;
	dec->gethdr = gethdr_h263;
	return ERR_NONE;
}

static int i263_create( mp4_decode* dec )
{
	dec->gethdr = gethdr_intel_h263;
	return ERR_NONE;
}

static _CONST nodedef MPEG4Def =
{
	OFS(mp4_decode,codedmap),
	MPEG4_ID,
	CODECIDCT_CLASS,
	PRI_DEFAULT,
	(nodecreate)mpeg4_create,
	(nodedelete)mp4_delete,
};

static _CONST nodedef H263Def =
{
	OFS(mp4_decode,codedmap),
	H263_ID,
	CODECIDCT_CLASS,
	PRI_DEFAULT,
	(nodecreate)h263_create,
	(nodedelete)mp4_delete,
};

static _CONST nodedef I263Def =
{
	OFS(mp4_decode,codedmap),
	I263_ID,
	H263_ID,
	PRI_DEFAULT,
	(nodecreate)i263_create,
};

void mpeg4_init()
{
	NodeRegisterClass(&MPEG4Def);
	NodeRegisterClass(&H263Def);
	NodeRegisterClass(&I263Def);
}

void mpeg4_done()
{
	NodeUnRegisterClass(MPEG4_ID);
	NodeUnRegisterClass(H263_ID);
	NodeUnRegisterClass(I263_ID);
}

#endif
