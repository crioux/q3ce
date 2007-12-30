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
 * $Id: mp4_header.c 183 2005-01-11 03:29:39Z picard $
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

#ifdef MPEG4

_CONST uint8_t intra_dc_threshold[] = 
{
	32,
	13,
	15,
	17,
	19,
	21,
	23,
	1,
};

_CONST uint8_t def_quant_intra[64] = 
{
	 8, 17, 18, 19, 21, 23, 25, 27,
	17, 18, 19, 21, 23, 25, 27, 28,
	20, 21, 22, 23, 24, 26, 28, 30,
	21, 22, 23, 24, 26, 28, 30, 32,
	22, 23, 24, 26, 28, 30, 32, 35,
	23, 24, 26, 28, 30, 32, 35, 38,
	25, 26, 28, 30, 32, 35, 38, 41,
	27, 28, 30, 32, 35, 38, 41, 45
};

_CONST uint8_t def_quant_inter[64] = 
{
	16, 17, 18, 19, 20, 21, 22, 23,
	17, 18, 19, 20, 21, 22, 23, 24,
	18, 19, 20, 21, 22, 23, 24, 25,
	19, 20, 21, 22, 23, 24, 26, 27,
	20, 21, 22, 23, 25, 26, 27, 28,
	21, 22, 23, 24, 26, 27, 28, 30,
	22, 23, 24, 26, 27, 28, 30, 31,
	23, 24, 25, 27, 28, 30, 31, 33
};

void getmatrix( mp4_decode* dec, uint8_t* m )
{
	int i = 0;
	int last,value = 0;

	do 
	{
		last = value;
		loadbits(dec);
		value = getbits(dec,8);
		m[scan[0][i++]] = (uint8_t)value;
	}
	while (value!=0 && i<64);

    i--;
	while (i < 64) 
		m[scan[0][i++]] = (uint8_t)last;
}

static bool_t notsupported( mp4_decode* dec, int msg )
{
	pin Pin;
	Pin.No = CODECIDCT_INPUT;
	Pin.Node = &dec->Codec.Node;

	if (!dec->Codec.NotSupported.Node || 
		dec->Codec.NotSupported.Node->Set(dec->Codec.NotSupported.Node,dec->Codec.NotSupported.No,
		&Pin,sizeof(pin))!=ERR_NONE)
	{
		ShowError(dec->Codec.Node.Class,MPEG4_ID,msg);
		return 0; // still contiune buggy decode
	}
	else
	{
		dec->validvol = 0;
		return 1; // codec will be replaced anyway so skip decoding
	}
}

void gethdr_vol( mp4_decode* dec )
{
	int width,height,aspect;
	int visual_object_layer_verid;
	int is_object_layer_identifier;
	flushbits(dec,32+1+8); // start_code + random_accessible_vol + video_object_type_indication
	loadbits(dec);
	 
	is_object_layer_identifier = getbits(dec,1);

	if (is_object_layer_identifier) {
		visual_object_layer_verid = getbits(dec,4);
		flushbits(dec,3); // video_object_layer_priority
	} 
	else {
		visual_object_layer_verid = 1;
	}

	aspect = getbits(dec,4); // aspect ratio
	if (aspect == ASPECT_CUSTOM)
	{
		loadbits(dec);
		getbits(dec,8); //aspect_width
		getbits(dec,8); //aspect_height
	}

	if (getbits1(dec)) // vol control parameters
	{
		flushbits(dec,2); // chroma_format
		if (!getbits(dec,1)) // b-frames
			CodecIDCTSetCount(&dec->Codec,3);
		if (getbits1(dec)) // vbv parameters
			flushbits(dec,15+1+15+1+15+1+3+11+1+15+1);
	}

	flushbits(dec,2+1); // shape + marker
	loadbits(dec);

	dec->time_increment_resolution = getbits(dec,16);
	if (dec->time_increment_resolution <= 0)
		dec->time_increment_resolution = 1;
	dec->time_increment_bits = _log2(dec->time_increment_resolution-1);

	flushbits(dec,1); // marker
	if (getbits1(dec)) //fixed_vop_rate
		flushbits(dec,dec->time_increment_bits);

	flushbits(dec,1); // marker
	loadbits(dec);
	width = getbits(dec,13); //width
	flushbits(dec,1);
	loadbits(dec);
	height = getbits(dec,13); //height
	flushbits(dec,1); // marker
	dec->interlaced = (char)getbits(dec,1);
	if (dec->interlaced && dec->showerror && notsupported(dec,MPEG4_ERROR_INTERLACE)) // interlace
		return;

	flushbits(dec,1); // obmc_disable

	if (!dec->ignoresize && CodecIDCTSetFormat(&dec->Codec,PF_YUV420,
		width,height,width,height) != ERR_NONE)
	{
		dec->validvol = 0;
		return;
	}

	loadbits(dec);
	dec->sprite = getbits(dec,(visual_object_layer_verid==1)?1:2);

	if (dec->sprite == SPRITE_STATIC || dec->sprite == SPRITE_GMC)
	{
		if (dec->showerror && notsupported(dec,MPEG4_ERROR_GMC))
			return;

		if (dec->sprite != SPRITE_GMC)
		{
			flushbits(dec,13+1+13+1+13+1+13+1); //width+marker+height+marker+left+marker+top+marker
			loadbits(dec);
		}
		dec->sprite_warping_points = getbits(dec,6);	
		dec->sprite_warping_accuracy = getbits(dec,2);
		getbits(dec,1); // brightness change not supported

		if (dec->sprite != SPRITE_GMC)
			flushbits(dec,1); // low_latency_sprite_enable
	}
		
	loadbits(dec);
	if (getbits1(dec))  // not 8 bit
	{
		dec->quant_precision = getbits(dec,4);
		flushbits(dec,4); // bit per pixel
	}
	else 
		dec->quant_precision = 5;

	dec->quant_type = getbits(dec,1);
	if (dec->quant_type) // quant type
	{
		if (getbits1(dec))	// load intra
			getmatrix(dec,dec->quant[0]);
		else
			memcpy(dec->quant[0],def_quant_intra,sizeof(def_quant_intra));

		if (getbits1(dec))	// load inter
			getmatrix(dec,dec->quant[1]);
		else
			memcpy(dec->quant[1],def_quant_inter,sizeof(def_quant_inter));

		dec->vld_block = vld_block_mpeg;
	}
	else
		dec->vld_block = vld_block_mpeg4;

	loadbits(dec);
	if (visual_object_layer_verid != 1) 
	{
		if (getbits1(dec) && dec->showerror && notsupported(dec,MPEG4_ERROR_QPEL))
			return;
	} 

	flushbits(dec,1); // complexity estimation
	flushbits(dec,1); // resync marker disabled (?)

	if (getbits1(dec))
	{
		flushbits(dec,1);
		if (dec->showerror && notsupported(dec,MPEG4_ERROR_PARTITIONING))
			return;
	}

    if (visual_object_layer_verid != 1) 
	{
		if (getbits1(dec)) //new pred not supported
			flushbits(dec,2+1); // req msg type, seg type
		flushbits(dec,1); //reduced res not supported
    }

	if (getbits1(dec)) // scalability
		flushbits(dec,1+4+1+5+5+5+5+1); // not supported

	dec->validvol = 1;
	dec->showerror = 0;
}

bool_t findnext_mpeg4( mp4_decode* dec )
{
	const uint8_t* ptr = dec->Codec.Buffer.Data;
	int len = dec->Codec.Buffer.WritePos;
	int pos = dec->Codec.FrameEnd;
	int32_t v = dec->startstate;

	if (!dec->startfound)
		for (;pos<len;++pos)
		{
			v = (v<<8) | ptr[pos];
			if (v == VOP_START_CODE)
			{
				++pos;
				dec->startfound = 1;
				break;
			}
		}

	if (dec->startfound)
		for (;pos<len;++pos)
		{
			v = (v<<8) | ptr[pos];
			if (v == VOP_START_CODE)
			{
				dec->Codec.FrameEnd = pos-3;
				dec->startfound = 0;
				dec->startstate = -1;
				return 1;
			}
		}

	dec->Codec.FrameEnd = pos;
	dec->startstate = v;
	return 0;
}

int gethdr_mpeg4( mp4_decode *dec )
{
	int code;
	for (;;)
	{
		bytealign(dec);
		loadbits(dec);
		if (eofbits(dec))
			break;
		code = showbits(dec,32);

		if ((code & ~VOL_START_CODE_MASK) == VOL_START_CODE)
		{
			gethdr_vol( dec );
		}
		else if (code == VOP_START_CODE) {

			int time_increment;
			bool_t rounding;

			flushbits(dec,32);

			loadbits(dec);
			dec->prediction_type = getbits(dec,2);

			while (getbits1(dec)) 
				loadbits(dec);

			flushbits(dec,1); //marker
			loadbits(dec);
			time_increment = getbits(dec,dec->time_increment_bits);

			if (!dec->low_delay && dec->time_increment_resolution)
			{
				if (dec->prediction_type != B_VOP) {
					dec->time_pp = (dec->time_increment_resolution + 
						time_increment - dec->last_reftime) % dec->time_increment_resolution;
					dec->last_reftime = time_increment;
				} else {
					dec->time_bp = (dec->time_increment_resolution + 
						dec->last_reftime - time_increment) % dec->time_increment_resolution;
				}
			}

			flushbits(dec,1); //marker

			loadbits(dec);
			if (!getbits1(dec)) 
			{
				dec->prediction_type = N_VOP;
				return 1;
			}  

			rounding = 0;
			if (dec->prediction_type == P_VOP || dec->prediction_type == S_VOP)
				rounding = getbits(dec,1);

			dec->Codec.IDCT.Ptr->Set(dec->Codec.IDCT.Ptr,IDCT_ROUNDING,&rounding,sizeof(bool_t));

			dec->intra_dc_threshold = intra_dc_threshold[getbits(dec,3)];

			if (dec->interlaced)
				flushbits(dec,2); //top_field_first, alternate_scan

		    //if (dec->prediction_type == S_VOP && (dec->sprite == SPRITE_STATIC || dec->sprite == SPRITE_GMC))
			//	spritetrajectory(dec);

			dec->quantizer = getbits(dec,dec->quant_precision); // vop quant

			loadbits(dec);
			if (dec->prediction_type != I_VOP) 
				dec->fcode_for = getbits(dec,3); 

			if (dec->prediction_type == B_VOP) 
				dec->fcode_back = getbits(dec,3); 

			return 1;
		}
		else
			flushbits(dec,8);
	}
	return 0;
}

int resync(mp4_decode* dec)
{
	int i,x,y;
	bool_t mpeg4 = dec->Codec.Node.Class == MPEG4_ID;

	i=bitstonextbyte(dec);
	if (i==8 && !mpeg4)
		i=0; // h.263
	flushbits(dec,i);

	if (!mpeg4)
	{
		flushbits(dec,16);
		loadbits(dec);

		while (!eofbits(dec) && !getbits1(dec))
			loadbits(dec);

		x = 0;
	    y = getbits(dec,5); //gob number
	    if (dec->mb_xsize > 25)
			y <<= 1;
		if (dec->mb_xsize > 50)
			y <<= 1;

		getbits(dec,2); //gfid
	    i = getbits(dec,5);
		if (i)
			dec->quantizer = i;
	}
	else
	{
		loadbits(dec);
		for (i=0;i<32;++i)
		{
			if (getbits1(dec))
				break;
			if (i==16) 
				loadbits(dec);
		}

		if (i+1 != dec->resync_marker_bits)
			return dec->pos_end;

		loadbits(dec);
		x = getbits(dec,dec->mb_bits);
		y = x/dec->mb_xsize;
		x -= y*dec->mb_xsize;

		loadbits(dec);
		i = getbits(dec,dec->quant_precision);
		if (i)
			dec->quantizer = i;

		if (getbits1(dec))
		{
			// extension
			return dec->pos_end; //not supported yet
		}
	}

	for (i = 0; i < 4*MB_X*2; i++)
		dec->dc_lum[i] = 1024;

	for (i = 0; i < 2*MB_X; i++) 
	{
		dec->dc_chr[0][i] = 1024;
		dec->dc_chr[1][i] = 1024;
	}

	memset(dec->ac_left_lum,0,sizeof(dec->ac_left_lum));
	memset(dec->ac_top_lum,0,sizeof(dec->ac_top_lum));
	memset(dec->ac_left_chr,0,sizeof(dec->ac_left_chr));
	memset(dec->ac_top_chr,0,sizeof(dec->ac_top_chr));

	dec->resyncpos = x+y*MB_X;
    return dec->resyncpos;
}

#endif
