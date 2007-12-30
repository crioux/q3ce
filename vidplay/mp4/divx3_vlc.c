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
 * $Id: divx3_vlc.c 202 2005-01-25 01:27:33Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/
 
#include "stdafx.h"

#ifdef MSMPEG4

#define VLC_BITS 9

// 16bit vlc table
// 
// data = 4bit bits | 12bit data
// subtable = 1|1|4bit tabbits|10bit offset*4

static int buildvlc( mp4_decode* dec, const uint8_t* codetab, int codesize, const uint16_t* datatab, int tabbits, int count,
			  int prefix, int prefixbits )
{
	int data,code,bits,i,j,n;
	int size = 1 << tabbits;
	int vlcpos = dec->vlcpos;
	uint16_t* p = dec->vlctable + vlcpos;
	dec->vlcpos += size;
	if (dec->vlcpos > MAX_VLC)
		return 0;

	for (i=0;i<size;++i)
		p[i] = ESCAPE;

	for (i=0;i<count;++i)
	{
		code = 0;
		for (j=0;j<codesize;++j)
			code |= codetab[i*codesize+j] << (j*8);
		if (codesize < 4)
			bits = datatab[i] >> 12;
		else
		{
			bits = code & 31;
			code >>= 5;
		}
		if (datatab)
			data = datatab[i] & 0xFFF;
		else
			data = i;

        bits -= prefixbits;
        if (bits > 0 && (code >> bits) == prefix) 
		{
            if (bits <= tabbits) 
			{
				// store data and bits

                j = (code << (tabbits - bits)) & (size - 1);
                n = j+(1 << (tabbits - bits));
                for (;j<n;j++) 
					p[j] = (uint16_t)((bits << 12) | data);
            } 
			else 
			{
				// subtable

                bits -= tabbits;
                j = (code >> bits) & (size - 1);
				if (p[j] == ESCAPE)
					n = bits;
				else
				{
					n = p[j] & 31;
					if (bits > n)
						n = bits;
				}
                p[j] = (uint16_t)(0xC000 | n);
            }
        }
	}
	
	// create subtables

	for (i=0;i<size;++i)
		if ((p[i] >> 12) >= 12)
		{
			bits = p[i] & 31;
			if (bits > tabbits)
				bits = tabbits;

			j = dec->vlcpos - vlcpos;

			// align to 4*offset
			if (j & 3)
			{
				n = 4 - (j & 3);
				dec->vlcpos += n;
				j += n;
			}

			assert(j<1024*4);

			p[i] = (uint16_t)(0xC000 | (bits << 10) | (j >> 2));

			buildvlc(dec,codetab,codesize,datatab,bits,count,
				(prefix << tabbits) | i, prefixbits+tabbits );
		}

	return vlcpos;
}

uint16_t* initvlc( mp4_decode* dec, const void* code, int codesize, const uint16_t* data, int count )
{
	return dec->vlctable + buildvlc(dec,code,codesize,data,VLC_BITS,count,0,0);
}

int getvlc( mp4_decode* dec, const uint16_t* table )
{
	int n,code;
	int bits = VLC_BITS;

	code = table[showbits(dec,bits)];
	n = code >> 12;
	if (n >= 12)
	{
		flushbits(dec,bits);
		bits = (code >> 10) & 15;
		table += (code & 1023) << 2;

		code = table[showbits(dec,bits)];
		n = code >> 12;
		if (n >= 12)
		{
			flushbits(dec,bits);
			bits = (code >> 10) & 15;
			table += (code & 1023) << 2;

			inlineloadbits(dec);

			code = table[showbits(dec,bits)];
			n = code >> 12;
			assert(n < 12);
		}
	}

	flushbits(dec,n);
	return code & 0xFFF;
}

#endif
