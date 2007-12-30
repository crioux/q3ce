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
 * $Id: mp4_stream.h 131 2004-12-04 20:36:04Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __MP4_STREAM_H
#define __MP4_STREAM_H

void initbits(mp4_decode*,const uint8_t *stream, int len);

void loadbits(mp4_decode*);

#ifdef MIPS
static INLINE void inlineloadbits( mp4_decode* dec )
{
	int n = dec->bitpos-8;
	if (n>=0)
	{
		const uint8_t* bitptr = dec->bitptr;
		int bits = dec->bits;

		do
		{
			bits = (bits << 8) | *bitptr++;
			n -= 8;
		}
		while (n>=0);

		dec->bits = bits;
		dec->bitptr = bitptr;
		dec->bitpos = n+8;
	}
}
#else
#define inlineloadbits(dec) loadbits(dec)
#endif

// n=0..32
int showbitslarge(mp4_decode*,int n);

// n=1..24 (or ..32 after bytealign)
#define showbits(dec,n) ((uint32_t)(dec->bits << dec->bitpos) >> (32-(n)))

#define flushbits(dec,n) dec->bitpos += n;

// use this only in boolean expression. to get a 1 bit value use getbits(dec,1)
#define getbits1(dec) ((dec->bits << dec->bitpos++) < 0)

static INLINE int getbits(mp4_decode* dec,int n)
{
	int i = showbits(dec,n);
	flushbits(dec,n);
	return i;
}

static INLINE int eofbits(mp4_decode* dec)
{
	return dec->bitptr >= dec->bitend;
}

static INLINE void bytealign(mp4_decode* dec)
{
	dec->bitpos = (dec->bitpos + 7) & ~7;
}

static INLINE int bitstonextbyte(mp4_decode* dec)
{
	return 8-(dec->bitpos & 7);
}

static INLINE const uint8_t *bytepos(mp4_decode* dec)
{
	return dec->bitptr - 4 + ((dec->bitpos+7) >> 3);
}

//**************************************************************************

#define beginbits_pos( dec )	\
	int bitpos = dec->bitpos;	\
	int bits = dec->bits;

#define endbits_pos( dec )		\
	dec->bitpos = bitpos;

#define loadbits_pos( dec )						\
	if (bitpos >= 8) {							\
		const uint8_t* bitptr = dec->bitptr;	\
		do {									\
			bits = (bits << 8) | *bitptr++;		\
			bitpos -= 8;						\
		} while (bitpos >= 8);					\
		dec->bits = bits;						\
		dec->bitptr = bitptr;					\
	}											

#define showbits_pos(dec,n) ((uint32_t)(bits << bitpos) >> (32-(n)))
#define flushbits_pos(dec,n) bitpos += n
#define getbits1_pos(dec) ((bits << bitpos++) < 0)

#endif
