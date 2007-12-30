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
 * $Id: mp4_mblock_ipvop.c 3 2004-07-13 11:26:13Z picard $
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

_CONST uint8_t scan[3][64] = {
{
	 0,  1,  8, 16,  9,  2,  3, 10, 
	17,	24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34, 
	27,	20, 13,  6,  7, 14, 21, 28, 
	35, 42,	49, 56, 57, 50, 43, 36, 
	29, 22,	15, 23, 30, 37, 44, 51, 
	58, 59, 52, 45, 38, 31, 39, 46, 
	53, 60, 61,	54, 47, 55, 62, 63
},
{
 	 0,  1,  2,  3,  8,  9, 16, 17, 
	10, 11,  4,  5,  6,  7, 15, 14,
	13, 12, 19, 18, 24, 25, 32, 33, 
	26, 27, 20, 21, 22, 23, 28, 29,
	30, 31, 34, 35, 40, 41, 48, 49, 
	42, 43, 36, 37, 38, 39, 44, 45,
	46, 47, 50, 51, 56, 57, 58, 59, 
	52, 53, 54, 55, 60, 61, 62, 63
},
{
	 0,  8, 16, 24,  1,  9,  2, 10, 
	17, 25, 32, 40, 48, 56, 57, 49,
	41, 33, 26, 18,  3, 11,  4, 12, 
	19, 27, 34, 42, 50, 58, 35, 43,
	51, 59, 20, 28,  5, 13,  6, 14, 
	21, 29, 36, 44, 52, 60, 37, 45,
	53, 61, 22, 30,  7, 15, 23, 31, 
	38, 46, 54, 62, 39, 47, 55, 63
}};

#ifdef MPEG4

#undef MSMPEG4
#include "mp4_predict.h"

_CONST uint8_t roundtab[16] = 
{0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2};

_CONST char DQtab[4] = {
	-1, -2, 1, 2
};

_CONST uint8_t MCBPCtabIntra[] = {
	0,0, //-1,0
	20,6, 36,6, 52,6, 4,4, 4,4, 4,4, 
	4,4, 19,3, 19,3, 19,3, 19,3, 19,3, 
	19,3, 19,3, 19,3, 35,3, 35,3, 35,3, 
	35,3, 35,3, 35,3, 35,3, 35,3, 51,3, 
	51,3, 51,3, 51,3, 51,3, 51,3, 51,3, 
	51,3
};

_CONST uint8_t MCBPCtabInter[] = {
	0,0, //-1,0
	0,9, 52,9, 36,9, 20,9, 49,9, 35,8, 35,8, 19,8, 19,8,
	50,8, 50,8, 51,7, 51,7, 51,7, 51,7, 34,7, 34,7, 34,7,
	34,7, 18,7, 18,7, 18,7, 18,7, 33,7, 33,7, 33,7, 33,7, 
	17,7, 17,7, 17,7, 17,7, 4,6, 4,6, 4,6, 4,6, 4,6, 
	4,6, 4,6, 4,6, 48,6, 48,6, 48,6, 48,6, 48,6, 48,6, 
	48,6, 48,6, 3,5, 3,5, 3,5, 3,5, 3,5, 3,5, 3,5, 
	3,5, 3,5, 3,5, 3,5, 3,5, 3,5, 3,5, 3,5, 3,5, 
	32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 
	32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 
	32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 32,4, 
	32,4, 32,4, 32,4, 32,4, 32,4, 16,4, 16,4, 16,4, 16,4, 
	16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 
	16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 
	16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 16,4, 
	16,4, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 
	2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 
	2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 
	2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 
	2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 
	2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 
	2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 2,3, 
	2,3, 2,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 
	1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 
	1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 
	1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 
	1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 
	1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 
	1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 1,3, 
	1,3, 1,3, 1,3
};

/**/

_CONST uint8_t CBPYtab[] = 
{ 
	0,0, 0,0, 6,6,  9,6,  8,5,  8,5,  4,5,  4,5,//-1,0
	2,5,  2,5,  1,5,  1,5,  0,4,  0,4,  0,4,  0,4, 
	12,4, 12,4, 12,4, 12,4, 10,4, 10,4, 10,4, 10,4,
	14,4, 14,4, 14,4, 14,4, 5,4,  5,4,  5,4,  5,4,
	13,4, 13,4, 13,4, 13,4, 3,4,  3,4,  3,4,  3,4, 
	11,4, 11,4, 11,4, 11,4, 7,4,  7,4,  7,4,  7,4 
};

static INLINE int getDCsizeLum( mp4_decode* dec )
{
	int i,code = showbits(dec,11);

	for (i=0;i<8;i++)
	{
		if ((code >> i)==1)
		{
			flushbits(dec,11-i);
			return 12-i;
		}
	}

	code >>= 8;

	if (code == 1) {
		flushbits(dec,3);
		return 4;
	} 
	else if (code == 2) {
		flushbits(dec,3);
		return 3;
	} 
	else if (code == 3) {
		flushbits(dec,3);
		return 0;
	}
	
	code >>=1;
	
	if (code == 2) {
		flushbits(dec,2);
		return 2;
	} 
	else if (code == 3) {
		flushbits(dec,2);
		return 1;
	}     

	return 0;
}

static INLINE int getDCsizeChr( mp4_decode* dec )
{
	int i,code = showbits(dec,12);

	for (i=0;i<10;i++)
	{
		if ((code >> i)==1)
		{
			flushbits(dec,12-i);
			return 12-i;
		}
	}

	return 3 - getbits(dec,2);
}

static INLINE int getDCdiff(int dct_dc_size,mp4_decode* dec)
{
	int code = showbits(dec,32); //we need only dct_dc_size bits (but in the higher bits)
	int adj = 0;
	flushbits(dec,dct_dc_size);
	if (code >= 0)
		adj = (-1 << dct_dc_size) + 1;
	return adj + ((uint32_t)code >> (32-dct_dc_size));
}

static void blockIntra( mp4_decode *dec, int pos)
{
	int j;
	idct_block_t *block;

	dec->Codec.IDCT.Ptr->Process(dec->Codec.IDCT.Ptr,POSX(pos),POSY(pos));

	block = dec->blockptr;
	for (j=0;j<6;++j)
	{
		idct_block_t *dc_addr;
		int subpos;
		int dc_scaler = dec->quantizer;
		int len,scantype;
		
		clearblock(block);	

		inlineloadbits(dec); 

		//stream: 24bit available

		if (j < 4) {
			if (dc_scaler >24) dc_scaler = dc_scaler*2 - 16;
			else if (dc_scaler>8) dc_scaler = (dc_scaler + 8);
			else if (dc_scaler>4) dc_scaler <<= 1;
			else dc_scaler = 8;

			// convert 1x1 -> 2x2 (accodring to block number)
			subpos = 2*POSX(pos) + (POSY(pos) << (MB_X2+2));
			subpos += (j & 1) + ((j & 2) << MB_X2);
		}
		else {
			if (dc_scaler > 24) dc_scaler = (dc_scaler - 6);
			else if (dc_scaler > 4) dc_scaler = (dc_scaler + 13)>>1;
			else dc_scaler = 8;

			subpos = pos;
		}

		if (dec->quantizer < dec->intra_dc_threshold)
		{
			int dct_dc_size, dct_dc_diff;

			dct_dc_diff = 0;
			dct_dc_size = j<4 ? getDCsizeLum(dec) : getDCsizeChr(dec); //max11bit : max12bit

			if (dct_dc_size)
			{
				dct_dc_diff = getDCdiff(dct_dc_size,dec);
				if (dct_dc_size > 8)				
					flushbits(dec,1); // marker bit	
			}

			*block = (idct_block_t)(dct_dc_diff * dc_scaler);
			len = 1;

			DEBUG_MSG1(DEBUG_VCODEC,T("block[0] %i"), dct_dc_diff);
		}
		else
			len = 0;

		// dc reconstruction, prediction direction
		dc_addr = dc_recon(dec, j, subpos, dc_scaler );

		if (dec->ac_pred_flag) 
		{
			if (dec->predict_dir == TOP)
				scantype = IDCTSCAN_ALT_HORI;
			else
				scantype = IDCTSCAN_ALT_VERT;
		}
		else
			scantype = IDCTSCAN_ZIGZAG;

		if ((dec->cbp << (26+j)) < 0) 
			len = dec->vld_block(dec,vld_intra,scan[scantype],len);

		//DEBUG_MSG1(DEBUG_VCODEC,T("predictor[0] %i"), *dc_addr / (dec->dc_scaler = dc_scaler));

		// dc add 
		if (dec->quantizer >= dec->intra_dc_threshold)
		{
			if (dec->quant_type)
				*block = (idct_block_t)(((*block*8) / (dec->quant[0][0] * dec->quantizer)) * dc_scaler);
			else
				*block = (idct_block_t)((*block / (dec->quantizer*2)) * dc_scaler);
		}

		*block = *dc_addr = (idct_block_t)(*block + *dc_addr);
		
		// ac reconstruction
		ac_recon(dec, block, j, subpos);

		dec->Codec.IDCT.Ptr->Intra8x8(dec->Codec.IDCT.Ptr,block,max(len,14),scantype);
	}
}

/**/

int mvchroma(int v)
{
	v |= ((v & 0x3)!=0) << 1;		//adjust dx
	v |= ((v & 0x30000)!=0) << 17;	//adjust dy

	v >>= 1; //shift

	//replace dx sign bit with old signed bit
	v &= ~0x8000;			
	v |= (v & 0x4000) << 1;

	return v;
}

int mvchroma4(const int* mv)
{
	int dx = MVX(mv[0])+MVX(mv[1])+MVX(mv[2])+MVX(mv[3]);
	int dy = MVY(mv[0])+MVY(mv[1])+MVY(mv[2])+MVY(mv[3]);

	if (dx)
		dx = sign(dx) * (roundtab[abs(dx) & 0xF] + ((abs(dx) >> 4) <<1));
	
	if (dy)
		dy = sign(dy) * (roundtab[abs(dy) & 0xF] + ((abs(dy) >> 4) <<1));

	return MAKEMV(dx,dy);
}
 
void blockInter( mp4_decode *dec )
{
	int j;
	idct_block_t* block = dec->blockptr;

	// texture decoding add
	for (j = 0; j < 6; j++) {
		
		int len = 0;
		if ((dec->cbp << (26+j)) < 0) { //if coded

			clearblock(block);	
			len = dec->vld_block(dec,vld_inter,scan[IDCTSCAN_ZIGZAG],len);
		}
		dec->Codec.IDCT.Ptr->Inter8x8(dec->Codec.IDCT.Ptr,block,len);
	}
}

int getCBPY( mp4_decode* dec ) //max 6 bits
{
	int code = showbits(dec,6);

	if (code < 2) return -1;
	if (code >= 48) {
		flushbits(dec,2);
		code = 15;
	} 
	else {
		flushbits(dec,CBPYtab[(code<<1)+1]);
		code = CBPYtab[code<<1];
	}
	return code;
}

int resync_marker(mp4_decode *dec)
{
	int bits = bitstonextbyte(dec);
	int code = showbits(dec,bits);

	return (code == (1 << (bits-1)) - 1) &&
		(showbitslarge(dec,bits+dec->resync_marker_bits) & 
		((1 << dec->resync_marker_bits)-1)) == 1;
}

/**/
void IVOP_mpeg4( mp4_decode *dec )
{
	int mcbpc,pos;

	dec->mv_bufinvalid = 2; // invalid, but can be cleared to make it valid
	dec->lastrefframe = dec->frame;
	dec->mapofs = dec->frame;
	memset(dec->framemap,0,dec->pos_end); // set all block to current frame (and clear rescue flag)

	dec->resync_marker_bits = 17;

	for (pos=0;pos<dec->pos_end;pos+=MB_X-dec->mb_xsize) {

		for (;POSX(pos)<dec->mb_xsize;++pos) {

			inlineloadbits(dec);

			// skip stuffing
			while (showbits(dec,9) == 1)
			{
				flushbits(dec,9);
				loadbits(dec);
				if (eofbits(dec))
					return;
			}

			if (eofbits(dec))
				return;

			if (resync_marker(dec))
			{
				pos = resync(dec);
				if (pos>=dec->pos_end)
					return;
			}

			//DEBUG_MSG3(-1,T("%d %d %02X"),POSX(pos),POSY(pos),showbits(dec,8));

			DEBUG_MSG3(DEBUG_VCODEC,T("macroblock (%i,%i) %08x"), POSX(pos), POSY(pos), showbitslarge(dec,32));

			//stream: 24bits available

			mcbpc = getMCBPC_i(dec); // mcbpc

			//stream: 15bits available
			
			dec->ac_pred_flag = getbits(dec,1);
			dec->cbp = (getCBPY(dec) << 2) | ((mcbpc >> 4) & 3);
				
			if ((mcbpc & 7) == INTRA_Q) //mb_type
			{
				int q = dec->quantizer + DQtab[getbits(dec,2)]; //DQtab[dquant]
				dec->quantizer = q<1 ? 1: (q>31 ? 31:q);
			}

			blockIntra(dec, pos);
		}
	}
}

void PVOP_mpeg4( mp4_decode *dec )
{
	//if no b-frames, we need only one row of mv vectors
	int j,pos;

	dec->mv_bufinvalid = 0;
	dec->currframemap = (dec->frame - dec->mapofs) << 1;
	dec->lastrefframe = dec->frame;
	dec->resync_marker_bits = 16 + dec->fcode_for;

	for (pos=0;pos<dec->pos_end;pos+=MB_X-dec->mb_xsize) {

		int* mv = &dec->mv_buf[(pos & dec->mv_bufmask)*4];

		for (;POSX(pos)<dec->mb_xsize;++pos,mv+=4) {

			inlineloadbits(dec);

			// skip stuffing
			while (showbits(dec,10) == 1)
			{
				flushbits(dec,10);
				loadbits(dec);
				if (eofbits(dec))
					return;
			}

			if (eofbits(dec))
				return;

			if (resync_marker(dec))
			{
				pos = resync(dec);
				if (pos>=dec->pos_end)
					return;

			}

			//DEBUG_MSG3(-1,T("%d %d %02X"),POSX(pos),POSY(pos),showbits(dec,8));

			DEBUG_MSG3(DEBUG_VCODEC,T("macroblock (%i,%i) %08x"), POSX(pos), POSY(pos), showbitslarge(dec,32));

			//stream: 24bits available

			if (!getbits1(dec)) {         //if coded

				int mcbpc = getMCBPC_p(dec); // mcbpc
				int mb_type = mcbpc & 7;
				mcbpc = (mcbpc >> 4) & 3;

				DEBUG_MSG1(DEBUG_VCODEC,T("mode %i"), mb_type);
				DEBUG_MSG1(DEBUG_VCODEC,T("cbpc %i"), mcbpc);

				//stream: 14bits available

				if (mb_type >= INTRA)
				{
					dec->framemap[pos] = (uint8_t)dec->currframemap;

					dec->ac_pred_flag = getbits(dec,1);
					dec->cbp = (getCBPY(dec) << 2) | mcbpc;

					DEBUG_MSG2(DEBUG_VCODEC,T("cbpy %i mcsel %i "), dec->cbp >> 2,0);

					//stream: 7bits available

					if (mb_type == INTRA_Q)
					{
						int q = dec->quantizer + DQtab[getbits(dec,2)]; //DQtab[dquant]
						dec->quantizer = q<1 ? 1: (q>31 ? 31:q);
					}

					//stream: 5bits available

					mv[3] = mv[2] = mv[1] = mv[0] = 0;

					rescue_predict(dec, pos);  //Restore AC_DC values

					blockIntra(dec, pos );
				}
				else
				{
				 	dec->Codec.IDCT.Ptr->Process(dec->Codec.IDCT.Ptr,POSX(pos),POSY(pos));

					dec->framemap[pos] = (uint8_t)(dec->currframemap|RESCUE); // set rescue needed flag

					dec->cbp = ((15-getCBPY(dec)) << 2) | mcbpc;

					DEBUG_MSG2(DEBUG_VCODEC,T("cbpy %i mcsel %i "), (dec->cbp >> 2),0);

					//stream: 8bits available

					// we will use mv[4],mv[5] for temporary purposes
					// in the next macroblock it will be overwrite (mv incremented by 4)

					switch (mb_type) 
					{
					case INTER4V:

						for (j = 0; j < 4; j++) 
							mv[j] = getMV(dec->fcode_for,getPMV(j,pos,dec),dec);

						mv[5] = mv[4] = mvchroma4(mv);

						dec->Codec.IDCT.Ptr->MComp8x8(dec->Codec.IDCT.Ptr,mv,NULL);
						break;

					case INTER_Q:
						{
							int q = dec->quantizer + DQtab[getbits(dec,2)]; //DQtab[dquant]
							dec->quantizer = q<1 ? 1: (q>31 ? 31:q);
						}

					default: //case INTER:

						mv[3] = mv[2] = mv[1] = mv[0] = 
							getMV(dec->fcode_for,getPMV(0,pos,dec),dec);

						mv[5] = mv[4] = mvchroma(mv[0]);

						dec->Codec.IDCT.Ptr->MComp16x16(dec->Codec.IDCT.Ptr,mv,NULL);
						break;
					}

					blockInter( dec );
				}
			}
			else {
				// not coded macroblock

				int n = dec->framemap[pos];
				dec->framemap[pos] = (uint8_t)(n|RESCUE);

				// copy needed or the buffer already has this block?
				if (dec->bufframe < dec->mapofs+(n>>1))
					dec->Codec.IDCT.Ptr->Copy16x16(dec->Codec.IDCT.Ptr,POSX(pos),POSY(pos),0);

				mv[3] = mv[2] = mv[1] = mv[0] = 0;
			}
		}
	}
}

static void blockDirect( mp4_decode *dec, int pos, int dmv )
{
	int fmv[6];
	int bmv[6];

	int* mv = &dec->mv_buf[pos*4];
	
	int j;
	for (j=0;j<4;++j,++mv) {
		
		int64_t v;
		int dx,dy;

		v = dec->TRB * MVX(*mv);
		if ((int)(v>>32)<0) //best code for (v<0)
			dx = (int)((v-1) >> 32);
		else
			dx = (int)(v >> 32);
		if (dx<0) ++dx;

		v = dec->TRB * MVY(*mv);
		if ((int)(v>>32)<0) //best code for (v<0)
			dy = (int)((v-1) >> 32);
		else
			dy = (int)(v >> 32);
		if (dy<0) ++dy;

		dx += MVX(dmv);
		dy += MVY(dmv);

		fmv[j] = MAKEMV(dx,dy);
		
		if (dmv & 0xFFFF)
			dx -= MVX(*mv);
		else
		{
			v = dec->TRB_TRD * MVX(*mv);
			if ((int)(v>>32)<0) //best code for (v<0)
				dx = (int)((v-1) >> 32);
			else
				dx = (int)(v >> 32);
			if (dx<0) ++dx;
		}

		if (dmv & 0xFFFF0000)
			dy -= MVY(*mv);
		else
		{
			v = dec->TRB_TRD * MVY(*mv);
			if ((int)(v>>32)<0) //best code for (v<0)
				dy = (int)((v-1) >> 32);
			else
				dy = (int)(v >> 32);
			if (dy<0) ++dy;
		}

		bmv[j] = MAKEMV(dx,dy);
	}

	fmv[5]=fmv[4]=mvchroma4(fmv);
	bmv[5]=bmv[4]=mvchroma4(bmv);

	dec->Codec.IDCT.Ptr->MComp8x8(dec->Codec.IDCT.Ptr,bmv,fmv);

	blockInter(dec);
}

void BVOP_mpeg4( mp4_decode *dec )
{
	int fmv[6];
	int bmv[6];
	int pos;

	dec->resync_marker_bits = 16 +
		(dec->fcode_back > dec->fcode_for ? dec->fcode_back : dec->fcode_for);

	for (pos=0;pos<dec->pos_end;pos+=MB_X-dec->mb_xsize) {

		int n;
		int fprev=0, bprev=0;

		for (;POSX(pos)<dec->mb_xsize;++pos) {

			inlineloadbits(dec);

			if (eofbits(dec))
				return;

			if (resync_marker(dec))
			{
				pos = resync(dec);
				if (pos>=dec->pos_end)
					return;
				fprev = bprev = 0;
			}

			n = dec->mapofs+(dec->framemap[pos] >> 1);

			// did the last refframe touch this block?
			if (dec->lastrefframe > n)
			{
				// no change: this block is the same in both backward and forward buffer

				// copy needed?
				if (dec->bufframe < n && dec->bufframe >= -dec->lastrefframe)
					dec->Codec.IDCT.Ptr->Copy16x16(dec->Codec.IDCT.Ptr,POSX(pos),POSY(pos),1);
			}
			else
			{
				//stream: 24bits available

			 	dec->Codec.IDCT.Ptr->Process(dec->Codec.IDCT.Ptr,POSX(pos),POSY(pos));

				if (!getbits1(dec))
				{
					int mb_type;
					n = getbits(dec,1);

					for (mb_type=0; mb_type<=3; ++mb_type)
						if (getbits1(dec))
							break;

					//stream: 19bits available

					if (!n)
						dec->cbp = getbits(dec,6);
					else
						dec->cbp = 0;

					//stream: 13bits available

					if (mb_type != DIRECT)
					{
						if (dec->cbp && getbits1(dec))
						{
							int q = dec->quantizer + (getbits1(dec) ? 2:-2);
							dec->quantizer = q<1 ? 1: (q>31 ? 31:q);
						}

						switch (mb_type) 
						{
						case BACKWARD:
							bmv[3]=bmv[2]=bmv[1]=bmv[0]=bprev = getMV( dec->fcode_back, bprev, dec );
							bmv[5]=bmv[4]=mvchroma(bprev);
							dec->Codec.IDCT.Ptr->MComp16x16(dec->Codec.IDCT.Ptr,bmv,NULL);
							blockInter(dec);
							break;

						case FORWARD:
							fmv[3]=fmv[2]=fmv[1]=fmv[0]=fprev = getMV( dec->fcode_for, fprev, dec );
							fmv[5]=fmv[4]=mvchroma(fprev);
							dec->Codec.IDCT.Ptr->MComp16x16(dec->Codec.IDCT.Ptr,NULL,fmv);
							blockInter(dec);
							break;

						default: //case INTERPOLATE:
							fmv[3]=fmv[2]=fmv[1]=fmv[0]=fprev = getMV( dec->fcode_for, fprev, dec );
							fmv[5]=fmv[4]=mvchroma(fprev);
							bmv[3]=bmv[2]=bmv[1]=bmv[0]=bprev = getMV( dec->fcode_back, bprev, dec );
							bmv[5]=bmv[4]=mvchroma(bprev);
							dec->Codec.IDCT.Ptr->MComp16x16(dec->Codec.IDCT.Ptr,bmv,fmv);
							blockInter(dec);
							break;
						}
					}
					else
					{
						blockDirect(dec,pos,getMV( 1, 0, dec ));
					}
				} 
				else {
					dec->cbp = 0;
					blockDirect(dec,pos,0);
				}
			}
		}
	}
}

#endif
