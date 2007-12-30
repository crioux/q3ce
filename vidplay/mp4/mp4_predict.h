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
 * $Id: mp4_predict.h 175 2005-01-08 03:10:07Z picard $
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

#ifndef _MP4_PREDICT_H_
#define _MP4_PREDICT_H_

static INLINE idct_block_t* dc_recon(mp4_decode* dec,int block_num, int pos, int dc_scaler )
{
	int	Fa,Fb,Fc;
	idct_block_t *dc;

	Fb = 1024;
	Fc = 1024;

	if (block_num < 4) {
		dc = dec->dc_lum;
		if (pos >= MB_X*2)
		{
			Fb=dc[(pos-MB_X*2) & DC_LUM_MASK];
			Fc=dc[(pos+1-MB_X*2) & DC_LUM_MASK];
		}
		dc += pos & DC_LUM_MASK;
	}
	else {
		dc = dec->dc_chr[block_num & 1];
		if (pos >= MB_X)
		{
			Fb=dc[(pos-MB_X) & DC_CHR_MASK];
			Fc=dc[(pos+1-MB_X) & DC_CHR_MASK];
		}
		dc += pos & DC_CHR_MASK;
	}

	Fa=dc[0];

#ifdef MSMPEG4
    Fa = (Fa + (dc_scaler >> 1)) / dc_scaler;
    Fb = (Fb + (dc_scaler >> 1)) / dc_scaler;
    Fc = (Fc + (dc_scaler >> 1)) / dc_scaler;

	if (abs(Fb - Fa) <= abs(Fb - Fc)) {
#else
	if (abs(Fb - Fa) < abs(Fb - Fc)) {
#endif
		dec->predict_dir = TOP;
		Fa = Fc;
	}
	else {
		dec->predict_dir = LEFT;
		//Fa = Fa;
	}

#ifndef MSMPEG4
	if (Fa>0)
		Fa+=dc_scaler>>1;
	else
		Fa-=dc_scaler>>1;
	Fa /= dc_scaler;
#endif

	++dc;
	Fa *= dc_scaler;
	*dc = (idct_block_t)Fa;
	return dc;
}

static INLINE void ac_recon(mp4_decode* dec, idct_block_t *block, int block_num, int pos)
{
	int i;
	idct_block_t *ac_top;
	idct_block_t *ac_left;

	if (block_num < 4) {
		ac_top = dec->ac_top_lum[pos & (MB_X*2-1)];
		ac_left = dec->ac_left_lum[(pos >> (MB_X2+1)) & 1];
		i = MB_X*2-1; //stride-1
	}
	else {
		ac_top = dec->ac_top_chr[pos & (MB_X-1)][block_num & 1];
		ac_left = dec->ac_left_chr[block_num & 1];
		i = MB_X-1; //stride-1
	}

	if (dec->ac_pred_flag)
	{
		if (dec->predict_dir == TOP)
		{
			if (pos > i)
				for (i = 1; i < 8; i++) 
				{
					block[i] = (idct_block_t)(block[i] + ac_top[i]);
					//DEBUG_MSG2(DEBUG_VCODEC,T("predictor[%i] %i"), i, ac_top[i] / dec->dc_scaler);
				}
		}
		else // left prediction
		{
			if (pos & i)
				for (i = 1; i < 8; i++)
				{
					block[i<<3] = (idct_block_t)(block[i<<3] + ac_left[i]);
					//DEBUG_MSG2(DEBUG_VCODEC,T("predictor[%i] %i"), i<<3, ac_left[i] / dec->dc_scaler);
				}
		}
	}

	for (i = 1; i < 8; i++) 
	{
		ac_top[i] = block[i];
		ac_left[i] = block[i<<3];
	}
}

extern void rescue_predict(mp4_decode*, int pos);

#endif

