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
 * $Id: mp4_decode.h 207 2005-03-19 15:23:49Z picard $
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
 
#ifndef __MP4_DECODE_H
#define __MP4_DECODE_H

typedef struct mp4_decode 
{
	codecidct Codec;

	int bits;
	int bitpos;
	const uint8_t *bitptr;
	const uint8_t *bitend;
	int resyncpos;

	int startstate;
	int frame;
	int bufframe;		// frame number of buffer's previous state
	int lastrefframe;	// frame number of last refframe
	int mapofs;			// mapofs + (framemap[pos] >> 1) is the last time that block was updated
	int currframemap;	// (frame - mapofs) << 1

	int (*gethdr)( struct mp4_decode* );
	void (*IVOP)( struct mp4_decode* );
	void (*PVOP)( struct mp4_decode* );
	void (*BVOP)( struct mp4_decode* );

	// vol
	int time_increment_resolution;
	int time_increment_bits;
	int mb_xsize;
	int mb_ysize;
	int mb_bits; //log2(mb_xsize*mb_ysize)
	int pos_end;
	int quant_precision;
	int quant_type;
	int sprite;
	int sprite_warping_points;
	int sprite_warping_accuracy;
	int sprite_brightness_change;
	int flip_rounding;

	char startfound;
	char low_delay;
	char validvol;
	char showerror;
	char needvol;
	char ignoresize;
	char interlaced;
	char long_vectors;
	char aic;
	bool_t rounding;
	
	// vop
	int prediction_type;
	int quantizer;
	int fcode_for;
	int fcode_back;
	int resync_marker_bits;
	int intra_dc_threshold;
	
	// macroblock
	idct_block_t* blockptr;
	int derived_mb_type;
	int ac_pred_flag;
	int cbp;
	int (*vld_block)( struct mp4_decode* dec, const uint16_t *table, const uint8_t *scan, int len ); 
	IDCT_BLOCK_DECLARE

	// motion compensation buffer
	int* mv_buf; //32bytes aligned
	int* _mv_buf;
	int mv_bufmask;
	int mv_bufinvalid;

	// b-frame
	int time_pp;
	int time_bp;
	int last_reftime;
	int last_bframe;

	int64_t TRB;
	int64_t TRB_TRD;

	uint8_t* packed_data;
	int packed_allocated;
	int packed_length;

	// idct
	idct *ownidct;
	int refframe;

	// predict
	int predict_dir;

	uint8_t quant[2][64]; // first intra, second inter

	//int dc_scaler; // only for debug

#ifdef MSMPEG4
	// msmpeg4
	int use_coded_bit;
	int dc_lum_scaler;
	int dc_chr_scaler;
	int vlcpos;
	int slice_pos;
	int slice_height;
	uint8_t* rlmax;
	uint16_t* vlcdata;
	uint16_t* vlctable;

	// vlc pointer
	uint16_t* dc_chr_vlc;
	uint16_t* dc_lum_vlc;
	uint16_t* mv_vlc;
	uint16_t* rl_vlc[2];
	uint8_t* rlmax_vlc[2];

	// vlc tables
	uint16_t* mb_inter_vlc;
	uint16_t* mb_intra_vlc;
	uint16_t* dc_chr_table[2];
	uint16_t* dc_lum_table[2];
	uint16_t* mv_table[2];
	uint16_t* rl_table[6];
#endif

	idct_block_t dc_lum[4*MB_X*2];		//[4][double width row]
	idct_block_t ac_left_lum[2][8];		//[lower/upper][8]
	idct_block_t ac_top_lum[2*MB_X][8]; //[double witdh row][8]

	idct_block_t dc_chr[2][2*MB_X];		//[U/V][2][normal width row]
	idct_block_t ac_left_chr[2][8];		//[U/V][8]
	idct_block_t ac_top_chr[MB_X][2][8];//[normal width row][U/V][8]

	uint8_t framemap[MB_X*MB_Y]; // when the last time the block changed (and resuce needed flag)
	
	int _mv_bufrow[4*(MB_X+1)+32/sizeof(int)]; // add one extra block for left border + 32align

	char codedmap[MB_X*MB_Y]; // only for msmpeg4 (must be at the end!)

} mp4_decode;

extern void mp4_create(mp4_decode*);
extern void mp4_delete(mp4_decode*);

#endif
