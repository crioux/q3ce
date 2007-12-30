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

void initbits(mp4_decode* dec,const uint8_t *stream, int len)
{
	dec->bits = 0;
	dec->bitpos = 32;
	dec->bitptr = stream;
	dec->bitend = stream+len+6; //adding more just to be safe
}

void loadbits( mp4_decode* dec )
{
	beginbits_pos(dec);
	loadbits_pos(dec);
	endbits_pos(dec);
}

int showbitslarge( mp4_decode* dec, int n )
{
	int i = showbits(dec,n);
	i |= *dec->bitptr >> (40-n-dec->bitpos);
	return i;
}
