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
 * $Id: vlc.c 3 2004-07-13 11:26:13Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"

// 16bit vlc table
// 
// data = 4bit bits | 12bit data
// subtable = 1|1|4bit tabbits|10bit offset*8

static int buildvlc(vlc* v, vlcitem* item, int count, int tabbits, int prefix, int prefixbits)
{
	int data,code,bits,i,j,n;
	uint16_t* p;
	int size = 1 << tabbits;
	int pos = v->pos;
	v->pos += size;

	if (v->pos > v->size)
	{
		// realloc table
		int size = (v->pos + 511) & ~511; 
		uint16_t* table;
		table = (uint16_t*) realloc(v->table,size*sizeof(uint16_t));
		if (!table)
			return ERR_OUT_OF_MEMORY;
		v->size = size;
		v->table = table;
	}

	p = v->table + pos;
	for (i=0;i<size;++i)
		p[i] = v->escape;

	for (i=0;i<count;++i)
	{
        bits = item[i].bits - prefixbits;
		code = item[i].code;

        if (bits > 0 && (code >> bits) == prefix) 
		{
            if (bits <= tabbits) 
			{
				// store data and bits

				data = item[i].value & 0xFFF;
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
				if (p[j] == v->escape)
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

			j = v->pos - pos;

			// align to 8*offset
			if (j & 7)
			{
				n = 8 - (j & 7);
				v->pos += n;
				j += n;
			}

			assert(j<1024*8);

			p[i] = (uint16_t)(0xC000 | (bits << 10) | (j >> 3));

			if (buildvlc(v,item,count,bits,(prefix << tabbits)|i, prefixbits+tabbits) != ERR_NONE)
				return ERR_OUT_OF_MEMORY;

			p = v->table + pos;
		}

	return ERR_NONE;
}

int vlcinit(vlc* v, vlcitem* item, int num, int escape)
{
	v->pos = 0;
	v->escape = (uint16_t)(escape & 0xFFF);
	return buildvlc(v,item,num,VLC_BITS,0,0);
}

void vlcdone(vlc* v)
{
	free(v->table);
	v->table = NULL;
	v->size = 0;
	v->pos = 0;
}

int vlcget3(const uint16_t* table, bitstream* stream)
{
	int n,code;
	int bits = VLC_BITS;

	bitload(stream);
	code = table[bitshow(stream,bits)];
	n = code >> 12;
	if (n >= 12)
	{
		bitflush(stream,bits);
		bits = (code >> 10) & 15;
		table += (code & 1023) << 3;

		code = table[bitshow(stream,bits)];
		n = code >> 12;
		if (n >= 12)
		{
			bitflush(stream,bits);
			bits = (code >> 10) & 15;
			table += (code & 1023) << 3;

			bitload(stream);

			code = table[bitshow(stream,bits)];
			n = code >> 12;
			assert(n < 12);
		}
	}

	bitflush(stream,n);
	return code & 0xFFF;
}

int vlcget2(const uint16_t* table, bitstream* stream)
{
	int n,code;
	int bits = VLC_BITS;

	bitload(stream);
	code = table[bitshow(stream,bits)];
	n = code >> 12;
	if (n >= 12)
	{
		bitflush(stream,bits);
		bits = (code >> 10) & 15;
		table += (code & 1023) << 3;

		code = table[bitshow(stream,bits)];
		n = code >> 12;
		assert(n < 12);
	}

	bitflush(stream,n);
	return code & 0xFFF;
}
