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
 * $Id: divx3_header.c 207 2005-03-19 15:23:49Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/
 
#include "stdafx.h"

#ifdef MSMPEG4

#define get012(dec) (getbits(dec,1) ? getbits(dec,1)+1 : 0)

int gethdr_msmpeg( mp4_decode *dec )
{
	int i;
	loadbits(dec);

    dec->prediction_type = getbits(dec,2);
    dec->quantizer = getbits(dec, 5);

    switch (dec->prediction_type) 
	{
	case I_VOP:
		dec->slice_height = dec->mb_ysize;
        i = getbits(dec,5)-0x16;
        if (i>1)
			dec->slice_height /= i;

		i = 3+get012(dec);
		dec->rl_vlc[1] = dec->rl_table[i];
		dec->rlmax_vlc[1] = dec->rlmax+i*192;

		i = get012(dec);
		dec->rl_vlc[0] = dec->rl_table[i];
		dec->rlmax_vlc[0] = dec->rlmax+i*192;

		i = getbits(dec,1);
		dec->dc_chr_vlc = dec->dc_chr_table[i];
		dec->dc_lum_vlc = dec->dc_lum_table[i];
	    dec->rounding = 0;

		break;

	case P_VOP:
		dec->use_coded_bit = getbits(dec, 1);

		i = get012(dec);

		dec->rl_vlc[0] = dec->rl_table[i];
		dec->rlmax_vlc[0] = dec->rlmax+i*192;

		dec->rl_vlc[1] = dec->rl_table[3+i];
		dec->rlmax_vlc[1] = dec->rlmax+(3+i)*192;

		i = getbits(dec,1);
		dec->dc_chr_vlc = dec->dc_chr_table[i];
		dec->dc_lum_vlc = dec->dc_lum_table[i];

		dec->mv_vlc = dec->mv_table[getbits(dec,1)];

		if (dec->flip_rounding)
			dec->rounding ^= 1;
		else
			dec->rounding = 0;
		break;

	default:
		return 0;
    }

	dec->Codec.IDCT.Ptr->Set(dec->Codec.IDCT.Ptr,IDCT_ROUNDING,&dec->rounding,sizeof(bool_t));
	return 1;
}

#endif
