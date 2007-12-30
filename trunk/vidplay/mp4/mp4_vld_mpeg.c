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

int vld_block_mpeg( mp4_decode* dec, const uint16_t *table, const uint8_t *scan, int len ) 
{
	int q_add = 0;
	int q_scale = dec->quantizer << 1;
	uint8_t* matrix = dec->quant[0];
	beginbits_pos(dec);

	if (table == vld_inter)
	{
		q_add = dec->quantizer;
		matrix += 8*8;
	}

	do // event vld
	{
		int code,level;

		loadbits_pos(dec);

		vld_code;
		level = code & 31; // 0x7FFF ESCAPE -> level = 31
		if (level != 31) {
			code >>= 5;
			code &= 127;
			len += code; // last|run
		} else {
			// this value is escaped
			loadbits_pos(dec);
			if (!getbits1_pos(dec))	{ // Type 1
				vld_code;
				level = code & 31;
				code >>= 5;
				code &= 127;
				len   += code; // last|run
				level += ((uint8_t*)(table+TABLE_LMAX))[code]; // table_lmax[last][run]
			}
			else
			if (!getbits1_pos(dec)) { // Type 2	
				vld_code;
				len += (code >> 5) & 127;
				code &= 31;
				level = code;
				if (code > 11) code = 11;
				if (len >= 64) code += 16; // add (last bit << 4)
				len += ((uint8_t*)(table+TABLE_RMAX))[code]; // table_rmax[last][min(11,level)]
			}
			else { // Type 3  - fixed length codes
				code = showbits_pos(dec,20);
				flushbits_pos(dec,21);
				level = (code << 20) >> 20; //sign extend the lower 12 bits
				if (level < 0)
					level = -level;
				len   += code >> 13;  // last|run
				level *= q_scale;
				level += q_add;
				level *= matrix[scan[len & 63]];
				level >>= 4;

				if ((code << 20) < 0)
					level = -level;

				dec->blockptr[scan[len & 63]] = (idct_block_t)level;

				DEBUG_MSG2(DEBUG_VCODEC,T("block[%i] %i"), code, level);
				continue;
			}
		}

		code = scan[len & 63];

		level *= q_scale;
		level += q_add;
		level *= matrix[code];
		level >>= 4;

		if (getbits1_pos(dec)) 
			level = -level;

		dec->blockptr[code] = (idct_block_t)level;

		DEBUG_MSG2(DEBUG_VCODEC,T("block[%i] %i"), code, level);

	} while (++len < 64);

	endbits_pos(dec);
	return len - 64;
}

#endif
