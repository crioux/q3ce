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
 * $Id: bitstream.h 3 2004-07-13 11:26:13Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __BITSTREAM_H
#define __BITSTREAM_H

typedef struct bitstream
{
	int bits;
	int bitpos;
	const uint8_t *bitptr;
	const uint8_t *bitend;
	
} bitstream;

DLL void bitinit(bitstream*,const uint8_t *stream, int len);

DLL void bitload(bitstream*);

// n=0..32
DLL int bitshowlarge(bitstream*,int n);

// n=1..24 (or ..32 after bytealign)
#define bitshow(dec,n) ((uint32_t)((dec)->bits << (dec)->bitpos) >> (32-(n)))

#define bitflush(dec,n) (dec)->bitpos += n;

// use this only in boolean expression. to get a 1 bit value use getbits(dec,1)
#define bitget1(dec) (((dec)->bits << (dec)->bitpos++) < 0)

static INLINE int bitget(bitstream* dec,int n)
{
	int i = bitshow(dec,n);
	bitflush(dec,n);
	return i;
}

static INLINE int biteof(bitstream* dec)
{
	return dec->bitptr >= dec->bitend;
}

static INLINE void bitbytealign(bitstream* dec)
{
	dec->bitpos = (dec->bitpos + 7) & ~7;
}

static INLINE int bittonextbyte(bitstream* dec)
{
	return 8-(dec->bitpos & 7);
}

static INLINE const uint8_t *bitbytepos(bitstream* dec)
{
	return dec->bitptr - 4 + ((dec->bitpos+7) >> 3);
}

//**************************************************************************

#define bitload_pos(dec, bitpos)				\
	bitpos -= 8;								\
	if (bitpos >= 0) {							\
		int bits = (dec)->bits;					\
		const uint8_t* bitptr = (dec)->bitptr;	\
		do {									\
			bits = (bits << 8) | *bitptr++;		\
			bitpos -= 8;						\
		} while (bitpos>=0);					\
		(dec)->bits = bits;						\
		(dec)->bitptr = bitptr;					\
	}											\
	bitpos += 8;

#define bitshow_pos(dec,bitpos,n) ((uint32_t)((dec)->bits << bitpos) >> (32-(n)))
#define bitflush_pos(dec,bitpos,n) bitpos += n;
#define bitget1_pos(dec,bitpos) (((dec)->bits << bitpos++) < 0)
#define bitbytealign_pos(dec,bitpos) bitpos = (bitpos + 7) & ~7;

#define bitgetx_pos(dec,bitpos,n,tmp)			\
{												\
	tmp = (dec)->bits << bitpos;				\
	bitflush_pos(dec,bitpos,n);					\
	tmp >>= 1;									\
	tmp ^= 0x80000000;							\
	tmp >>= 31-n;								\
	n = tmp - (tmp >> 31);						\
}

#endif
