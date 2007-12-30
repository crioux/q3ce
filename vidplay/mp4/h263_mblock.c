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
 * $Id: divx3_vlc.c 131 2004-12-04 20:36:04Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/
 
#include "stdafx.h"

#ifdef MPEG4

static void blockIntra_h263( mp4_decode *dec, int pos)
{
	int j;
	idct_block_t *block;

	dec->Codec.IDCT.Ptr->Process(dec->Codec.IDCT.Ptr,POSX(pos),POSY(pos));

	block = dec->blockptr;
	for (j=0;j<6;++j)
	{
		int dc;
		int len,scantype;
		
		clearblock(block);	

		inlineloadbits(dec); 

		//stream: 24bit available

        dc = getbits(dec, 8);
		if (dc == 255)
			dc = 128;

		*block = (idct_block_t)(dc << 3);
		len = 1;

		DEBUG_MSG1(DEBUG_VCODEC,T("block[0] %i"), dc);

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
			len = vld_block_h263(dec,vld_inter,scan[scantype],len);

		dec->Codec.IDCT.Ptr->Intra8x8(dec->Codec.IDCT.Ptr,block,len,scantype);
	}
}

static void blockInter_h263( mp4_decode *dec )
{
	int j;
	idct_block_t* block = dec->blockptr;

	// texture decoding add
	for (j = 0; j < 6; j++) {
		
		int len = 0;
		if ((dec->cbp << (26+j)) < 0) { //if coded

			clearblock(block);	
			len = vld_block_h263(dec,vld_inter,scan[IDCTSCAN_ZIGZAG],len);
		}
		dec->Codec.IDCT.Ptr->Inter8x8(dec->Codec.IDCT.Ptr,block,len);
	}
}

void IVOP_h263( mp4_decode *dec )
{
	int mcbpc,pos;

	dec->mv_bufinvalid = 2; // invalid, but can be cleared to make it valid
	dec->lastrefframe = dec->frame;
	dec->mapofs = dec->frame;
	memset(dec->framemap,0,dec->pos_end); // set all block to current frame (and clear rescue flag)

	for (pos=0;pos<dec->pos_end;pos+=MB_X-dec->mb_xsize) {

		for (;POSX(pos)<dec->mb_xsize;++pos) {

			inlineloadbits(dec);

			if (eofbits(dec))
				return;

			if (showbits(dec,16)==0)
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
			
			if (dec->aic)
				dec->ac_pred_flag = getbits(dec,1); //todo: aic
			else
				dec->ac_pred_flag = 0;

			dec->cbp = (getCBPY(dec) << 2) | ((mcbpc >> 4) & 3);
				
			if ((mcbpc & 7) == INTRA_Q) //mb_type
			{
				int q = dec->quantizer + DQtab[getbits(dec,2)]; //DQtab[dquant]
				dec->quantizer = q<1 ? 1: (q>31 ? 31:q);
			}

			blockIntra_h263(dec, pos);
		}
	}
}

void PVOP_h263( mp4_decode *dec )
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

			if (eofbits(dec))
				return;

			if (showbits(dec,16)==0)
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

					if (dec->aic)
						dec->ac_pred_flag = getbits(dec,1); //todo: aic
					else
						dec->ac_pred_flag = 0;

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

					blockIntra_h263(dec, pos );
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

					blockInter_h263( dec );
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

void BVOP_h263( mp4_decode *dec )
{
}

#endif
