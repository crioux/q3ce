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
 * $Id: bitstream.c 3 2004-07-13 11:26:13Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"

void bitinit(bitstream* dec,const uint8_t *stream, int len)
{
	dec->bits = 0;
	dec->bitpos = 32;
	dec->bitptr = stream;
	dec->bitend = stream+len+4;
}

void bitload(bitstream* dec)
{
	while (dec->bitpos >= 8)
	{
		dec->bits = (dec->bits << 8) | *(dec->bitptr++);
		dec->bitpos -= 8;
	}
}

int bitshowlarge(bitstream* dec, int n) 
{
	int i = bitshow(dec,n);
	i |= *dec->bitptr >> (40-n-dec->bitpos);
	return i;
}
