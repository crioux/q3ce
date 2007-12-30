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
 * $Id: divx3_mblock.c 202 2005-01-25 01:27:33Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/
 
#include "stdafx.h"

#ifdef MSMPEG4

#include "mp4_predict.h"

static INLINE int mvchroma3(int v)
{
	v |= ((v & 0x3)!=0) << 1;		//adjust dx
	v |= ((v & 0x30000)!=0) << 17;	//adjust dy

	v >>= 1; //shift

	//replace dx sign bit with old signed bit
	v &= ~0x8000;			
	v |= (v & 0x4000) << 1;

	return v;
}

// code = 1:last|6:run|5:level

static int vld_block2( mp4_decode* dec, int n, const uint8_t *scan, int len, int rundiff )
{
	const uint16_t *table = dec->rl_vlc[n];
	const uint8_t *maxtable = dec->rlmax_vlc[n];
	int q_scale = (dec->quantizer) << 1;
	int q_add = (dec->quantizer-1)|1;
	idct_block_t* block = dec->blockptr;

    do
	{
		int code, level;

		inlineloadbits(dec); 

        code = getvlc(dec, table); // max 15bits

        if (code == ESCAPE) {

			inlineloadbits(dec); 

            if (!getbits1(dec)) {
                if (!getbits1(dec)) {
                    // third escape
					code = getbits(dec,7+8);
                    len += code >> 8; //last|run
                    level = (code << 24) >> 24;
					level *= q_scale;
					if (level > 0)
						level += q_add;
					else
						level -= q_add;

					block[scan[len & 63]] = (idct_block_t)level;
					continue;

                } else {
                    // second escape
                    code = getvlc(dec, table); // max 15bits
					len += code >> 5;
					code &= 31;
					level = code;
					code += (len >> 6) << 5; // add (last bit << 5)
					len += maxtable[128+code] + rundiff; //maxrun
                }
            } else {
                // first escape
                code = getvlc(dec, table); // max 15bits
				level = code & 31;
				code >>= 5;
				len += code; // last|run
                level += maxtable[code]; //maxlevel
            }

        } else {
			level = code & 31;
			len += code >> 5; // last|run
        }

		level *= q_scale;
		level += q_add;

		if (getbits1(dec)) 
			level = -level;

		block[scan[len & 63]] = (idct_block_t)level;

	} while (++len < 64);

	return len - 64;
}

static void blockIntra3( mp4_decode *dec, int pos )
{
	int j;
	idct_block_t *block = dec->blockptr;

	dec->Codec.IDCT.Ptr->Process(dec->Codec.IDCT.Ptr,POSX(pos),POSY(pos));

	for (j = 0; j < 6; j++) {
		
		idct_block_t *dc_addr;
		int subpos;
		int dc_scaler,dct_dc_diff;
		int len,scantype;

		clearblock(block);	

		inlineloadbits(dec); 

		//stream: 24bit available

		if (j < 4) {
			dct_dc_diff = getvlc(dec, dec->dc_lum_vlc); // max 26bits
			dc_scaler = dec->dc_lum_scaler;
			// convert 1x1 -> 2x2 (accodring to block number)
			subpos = pos - dec->slice_pos;
			subpos = 2*POSX(subpos) + (POSY(subpos) << (MB_X2+2));
			subpos += (j & 1) + ((j & 2) << MB_X2);
		}
		else {
			dct_dc_diff = getvlc(dec, dec->dc_chr_vlc); // max 25bits
			dc_scaler = dec->dc_chr_scaler;
			subpos = pos - dec->slice_pos;
		}

		inlineloadbits(dec); 

		if (dct_dc_diff == 119) { // dcmax
			dct_dc_diff = getbits(dec,8);
			if (getbits1(dec))
				dct_dc_diff = -dct_dc_diff;
		} else 
			if (dct_dc_diff && getbits1(dec))
				dct_dc_diff = -dct_dc_diff;

		DEBUG_MSG1(DEBUG_VCODEC,T("block[0] %i"), dct_dc_diff);

		// dc reconstruction, prediction direction
		dc_addr = dc_recon(dec, j, subpos, dc_scaler );
		// dc add
		*block = *dc_addr = (idct_block_t)(*dc_addr + dct_dc_diff * dc_scaler);

		if (dec->ac_pred_flag) {

			if (dec->predict_dir == TOP)
				scantype = IDCTSCAN_ALT_HORI;
			else
				scantype = IDCTSCAN_ALT_VERT;
		}
		else
			scantype = IDCTSCAN_ZIGZAG;

		len = 1;
		if ((dec->cbp << (26+j)) < 0) 
			len = vld_block2(dec,j>=4,scan[scantype],len,0);
		
		// ac reconstruction
		ac_recon(dec, block, j, subpos);
 
		dec->Codec.IDCT.Ptr->Intra8x8(dec->Codec.IDCT.Ptr,block,max(len,14),scantype);
	}
}

static void blockInter3( mp4_decode *dec )
{
	int j;
	idct_block_t* block = dec->blockptr;

	// texture decoding add
	for (j = 0; j < 6; j++) {
		
		int len = 0;
		if ((dec->cbp << (26+j)) < 0) { //if coded

			clearblock(block);	
			len = vld_block2(dec,1,scan[IDCTSCAN_ZIGZAG],len,1);
		}
		dec->Codec.IDCT.Ptr->Inter8x8(dec->Codec.IDCT.Ptr,block,len);
	}
}

static void dc_scaler( mp4_decode *dec )
{
    int scale;
    if (dec->quantizer < 5)
        scale = 8;
    else if (dec->quantizer < 9)
        scale = 2 * dec->quantizer;
    else 
        scale = dec->quantizer + 8;

    dec->dc_lum_scaler = scale;
    dec->dc_chr_scaler = (dec->quantizer + 13) >> 1;
}

static int getMV2(int prev, mp4_decode* dec)
{
    int code, mx, my;

	inlineloadbits(dec);

    code = getvlc(dec,dec->mv_vlc); // max 17bits

    if (code == ESCAPE) {
		inlineloadbits(dec);
        mx = getbits(dec,6);
        my = getbits(dec,6);
    } else {
        mx = code & 63;
        my = code >> 6;
    }

    mx += MVX(prev) - 32;
    my += MVY(prev) - 32;

    if (mx <= -64)
        mx += 64;
    else 
	if (mx >= 64)
        mx -= 64;

    if (my <= -64)
        my += 64;
    else 
	if (my >= 64)
        my -= 64;

	return MAKEMV(mx,my);
}

void ext_header( mp4_decode* dec )
{
	dec->flip_rounding = 0;

	if (!eofbits(dec))
	{
		loadbits(dec);
		getbits(dec,5); //fps
		dec->Codec.In.Format.ByteRate = getbits(dec,11)*128;
        dec->flip_rounding = getbits1(dec);
	}
}

void IVOP_msmpeg( mp4_decode *dec )
{
	int a,b,c,code,pos;
	char* p;

	dec->lastrefframe = dec->frame;
	dec->mapofs = dec->frame;
	memset(dec->framemap,0,dec->pos_end); // set all block to current frame

	dc_scaler(dec);

	dec->slice_pos = 0;
	for (pos=0;pos<dec->pos_end;pos+=MB_X-dec->mb_xsize) {

		if (pos == dec->slice_pos + dec->slice_height*MB_X)
			dec->slice_pos += dec->slice_height*MB_X;

		for (;POSX(pos)<dec->mb_xsize;++pos) {

			inlineloadbits(dec);

			if (eofbits(dec))
			{
				dec->flip_rounding = 0;
				return;
			}

			//DEBUG_MSG3(-1,T("%d %d %02X"),POSX(pos),POSY(pos),showbits(dec,8));

			//stream: 24bits available

			code = getvlc(dec, dec->mb_intra_vlc); // max 13bits

	        //predict coded block pattern
			p = &dec->codedmap[pos+1+MB_X];

			//(0:0)
			a = (p[-1] << 1) & 32;
			b = (p[-1-MB_X] << 3) & 32;
			c = (p[-MB_X] << 2) & 32;

			if (b==c) c=a; 
			code ^= c;
			
			//(0:1)
			a = (code >> 1) & 16;
			b = (p[-MB_X] << 1) & 16;
			c = (p[-MB_X] << 2) & 16;

			if (b==c) c=a; 
			code ^= c;

			//(1:0)
			a = (p[-1] << 1) & 8;
			b = (p[-1] >> 1) & 8;
			c = (code >> 2) & 8;

			if (b==c) c=a; 
			code ^= c;

			//(1:1)
			a = (code >> 1) & 4;
			b = (code >> 3) & 4;
			c = (code >> 2) & 4;

			if (b==c) c=a; 
			code ^= c;

			*p = (char)code;
			dec->cbp = code;

			dec->ac_pred_flag = getbits(dec,1);

			blockIntra3(dec, pos);
		}
	}

	ext_header(dec);
}

static INLINE void clear32(void* p)
{
	int *i = (int*)p;
	i[0] = 0; i[1] = 0;	i[2] = 0; i[3] = 0;
	i[4] = 0; i[5] = 0;	i[6] = 0; i[7] = 0;
}

static void rescue_predict_msmpeg(mp4_decode* dec, int pos, int pos2) 
{
	int lumpos = 2*POSX(pos) + (POSY(pos) << (MB_X2+2));

	if (pos2>=MB_X+1 && (dec->framemap[pos2-MB_X-1] & RESCUE)) {
		// rescue -A- DC value
		dec->dc_lum[(lumpos-MB_X*2) & DC_LUM_MASK] = 1024;
		dec->dc_chr[0][(pos-MB_X) & DC_CHR_MASK] = 1024;
		dec->dc_chr[1][(pos-MB_X) & DC_CHR_MASK] = 1024;
	}

	// left
	if (pos2>=1 && (dec->framemap[pos2-1] & RESCUE)) {
		// rescue -B- DC values
		dec->dc_lum[(lumpos) & DC_LUM_MASK] = 1024;
		dec->dc_lum[(lumpos+MB_X*2) & DC_LUM_MASK] = 1024;
		dec->dc_chr[0][pos & DC_CHR_MASK] = 1024;
		dec->dc_chr[1][pos & DC_CHR_MASK] = 1024;
		
		if (dec->ac_pred_flag) {
			// rescue -B- AC values
			clear32(dec->ac_left_lum);
			clear32(dec->ac_left_chr);
		}
	}

	// top
	if (pos2>=MB_X && (dec->framemap[pos2-MB_X] & RESCUE)) {
		
		// rescue -C- DC values
		dec->dc_lum[(lumpos+1-MB_X*2) & DC_LUM_MASK] = 1024;
		dec->dc_lum[(lumpos+2-MB_X*2) & DC_LUM_MASK] = 1024;
		dec->dc_chr[0][(pos+1-MB_X) & DC_CHR_MASK] = 1024;
		dec->dc_chr[1][(pos+1-MB_X) & DC_CHR_MASK] = 1024;
		
		if (dec->ac_pred_flag) {
			// rescue -C- AC values
			clear32(dec->ac_top_lum[lumpos & (MB_X*2-1)]);
			clear32(dec->ac_top_chr[pos & (MB_X-1)]);
		}
	}
}

void PVOP_msmpeg( mp4_decode *dec )
{
	int pos;

	dec->currframemap = (dec->frame - dec->mapofs) << 1;
	dec->lastrefframe = dec->frame;

	dc_scaler(dec);

	dec->slice_pos = 0;
	for (pos=0;pos<dec->pos_end;pos+=MB_X-dec->mb_xsize) {

		int* mv = &dec->mv_buf[(pos & dec->mv_bufmask)*4];

		if (pos == dec->slice_pos + dec->slice_height*MB_X)
		{
			dec->slice_pos += dec->slice_height*MB_X;
		}

		for (;POSX(pos)<dec->mb_xsize;++pos,mv+=4) {

			inlineloadbits(dec);

			if (eofbits(dec))
				return;

			//stream: 24bits available

			//DEBUG_MSG3(-1,T("%d %d %02X"),POSX(pos),POSY(pos),showbits(dec,8));

			if (dec->use_coded_bit && getbits1(dec)) {

				// not coded macroblock
				int n = dec->framemap[pos];
				dec->framemap[pos] = (uint8_t)(n|1); // set rescue needed flag

				if (dec->bufframe < dec->mapofs+(n>>1))
					dec->Codec.IDCT.Ptr->Copy16x16(dec->Codec.IDCT.Ptr,POSX(pos),POSY(pos),0);

				mv[3] = mv[2] = mv[1] = mv[0] = 0;
			}
			else {
        
				int code = getvlc(dec, dec->mb_inter_vlc); // max 21bits
				dec->cbp = code & 0x3F;

				if (code & 0x40) { //inter

					dec->framemap[pos] = (uint8_t)(dec->currframemap|1); // set rescue needed flag
					mv[3] = mv[2] = mv[1] = mv[0] = 
						getMV2(getPMV(0,pos - dec->slice_pos,dec),dec);

					mv[5] = mv[4] = mvchroma3(mv[0]);			        

				 	dec->Codec.IDCT.Ptr->Process(dec->Codec.IDCT.Ptr,POSX(pos),POSY(pos));
					dec->Codec.IDCT.Ptr->MComp16x16(dec->Codec.IDCT.Ptr,mv,NULL);
					blockInter3( dec );
				}
				else { //intra

					dec->framemap[pos] = (uint8_t)dec->currframemap;
					dec->ac_pred_flag = getbits(dec,1);

					mv[3] = mv[2] = mv[1] = mv[0] = 0;

					rescue_predict_msmpeg(dec, pos - dec->slice_pos, pos);  //restore AC_DC values

					blockIntra3(dec, pos );
				}
			}
		}
	}
}

void BVOP_msmpeg( mp4_decode *dec )
{
	// low_delay always 1 -> no B-frame
}

#endif
