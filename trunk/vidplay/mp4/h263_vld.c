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
 * $Id: mp4_stream.c 131 2004-12-04 20:36:04Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "stdafx.h"

#ifdef MPEG4

#define vld_write									\
	block[scan[len & 63]] = (idct_block_t)level;	\
	DEBUG_MSG2(DEBUG_VCODEC,T("block[%i] %i"), scan[len & 63], level<0 ? (level+q_add)/q_scale : (level-q_add)/q_scale);\
	if (++len < 64)	continue;						\
	endbits_pos(dec);								\
	return len - 64;							

#define vld_quant									\
	level *= q_scale;								\
	level += q_add;									\
	if (getbits1_pos(dec))							\
		level = -level;

int vld_block_h263( mp4_decode* dec, const uint16_t *table, const uint8_t *scan, int len ) 
{
	idct_block_t* block = dec->blockptr;
	int q_scale = dec->quantizer << 1;
	int q_add = dec->quantizer;
	beginbits_pos(dec);

	for (;;) // event vld
	{
		int code,level;
		loadbits_pos(dec);

		vld_code;
		level = code & 31; // 0x7FFF ESCAPE -> level = 31
		if (level != 31) {
			code >>= 5;
			code &= 127;
			len += code; // last|run
			vld_quant
			vld_write
		} else {
			// this value is escaped

			loadbits_pos(dec);
			code = showbits_pos(dec,15);
			flushbits_pos(dec,15);
			level = (code << 24) >> 24; //sign extend the lower 8 bits
            if (level == -128)
			{
				level = showbits_pos(dec,5);
				flushbits_pos(dec,5);
				loadbits_pos(dec);
				level |= showbits_pos(dec,6) << 5;
				flushbits_pos(dec,6);
				level = (level << 21) >> 21;
			}
			level *= q_scale;
			len   += code >> 8;  // last|run
			if (level > 0)
				level += q_add;
			else
				level -= q_add;

			vld_write;
		}
	}
	return 0;
}

#endif
