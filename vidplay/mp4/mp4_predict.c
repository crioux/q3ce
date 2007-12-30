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
 * $Id: mp4_predict.c 117 2004-11-28 02:45:11Z picard $
 *
 *****************************************************************************
 *
 * Authors:
 *
 *	Andrea	Graziani  (Ag): Original source code (Open Divx Decoder 0.4a).
 *	Pedro	Mateu     (Pm) and
 *	Gabor	Kovacs    (Kg) Heavily modified and optimized code
 *
 ****************************************************************************/

#include "stdafx.h"

static INLINE void clear32(void* p)
{
	int *i = (int*)p;
	i[0] = 0; i[1] = 0;	i[2] = 0; i[3] = 0;
	i[4] = 0; i[5] = 0;	i[6] = 0; i[7] = 0;
}

void rescue_predict(mp4_decode* dec, int pos) 
{
	int lumpos = 2*POSX(pos) + (POSY(pos) << (MB_X2+2));

	if (pos>=MB_X+1 && (dec->framemap[pos-MB_X-1] & RESCUE)) {
		// rescue -A- DC value
		dec->dc_lum[(lumpos-MB_X*2) & DC_LUM_MASK] = 1024;
		dec->dc_chr[0][(pos-MB_X) & DC_CHR_MASK] = 1024;
		dec->dc_chr[1][(pos-MB_X) & DC_CHR_MASK] = 1024;
	}

	// left
	if (pos>=1 && (dec->framemap[pos-1] & RESCUE)) {
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
	if (pos>=MB_X && (dec->framemap[pos-MB_X] & RESCUE)) {
		
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
