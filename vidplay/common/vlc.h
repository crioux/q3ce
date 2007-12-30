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
 * $Id: vlc.h 3 2004-07-13 11:26:13Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __VLC_H
#define __VLC_H

#define VLC_BITS 9

typedef struct vlc
{
	uint16_t *table;
	int pos;
	int size;
	uint16_t escape;
	
} vlc;

typedef struct vlcitem
{
	uint32_t code;
	uint16_t bits;
	uint16_t value;

} vlcitem;

// item: code[32] bits[16] value[16]
DLL int vlcinit(vlc*, vlcitem* code, int num, int escape);
DLL void vlcdone(vlc*);

DLL int vlcget2(const uint16_t*, bitstream*);
DLL int vlcget3(const uint16_t*, bitstream*);

#define vlcget2_pos(code, table, stream, bitpos, tmp, mask)	\
{													\
	bitload_pos(stream,bitpos);						\
	code = table[bitshow_pos(stream,bitpos,VLC_BITS)];\
	tmp = code >> 12;								\
	if (tmp >= 12)									\
	{												\
		bitflush_pos(stream,bitpos,VLC_BITS);		\
		bitload_pos(stream,bitpos);					\
		tmp = (code >> 10) & 15;					\
		code = (code & 1023) << 3;					\
													\
		code = table[code+bitshow_pos(stream,bitpos,tmp)];\
		assert((code >> 12) < 12);					\
		tmp = code >> 12;							\
	}												\
													\
	bitflush_pos(stream,bitpos,tmp);				\
	code &= mask;									\
}

#endif
