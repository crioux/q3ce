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
 * $Id: mp4_vld.h 178 2005-01-10 07:34:30Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef _MP4_VLD_H_
#define _MP4_VLD_H_

extern _CONST uint16_t vld_intra[];
extern _CONST uint16_t vld_inter[];

extern int vld_block_h263( mp4_decode* dec, const uint16_t *table, const uint8_t *scan, int len ); 
extern int vld_block_mpeg4( mp4_decode* dec, const uint16_t *table, const uint8_t *scan, int len ); 
extern int vld_block_mpeg( mp4_decode* dec, const uint16_t *table, const uint8_t *scan, int len ); 

#define TABLE_1			0
#define TABLE_2			112
#define TABLE_3			112+96
#define TABLE_LMAX		112+96+120
#define TABLE_RMAX		112+96+120+64

#define vld_code								\
	code = showbits_pos(dec,12);				\
	if (code >> 9)								\
		code = (code >> 5) - 16 + TABLE_1;		\
	else if (code >> 7)							\
		code = (code >> 2) - 32 + TABLE_2;		\
	else /* if (code >= 8)  but we don't care about invalid huffman codes */ \
		code = code - 8 + TABLE_3;				\
	code = table[code];							\
	flushbits_pos(dec,code >> 12);

#endif
