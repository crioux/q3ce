/*****************************************************************************
 *
 * This code has been developed by Project Mayo. This software is an
 * implementation of a part of one or more MPEG-4 Video tools as
 * specified in ISO/IEC 14496-2 standard.  Those intending to use this
 * software module in hardware or software products are advised that its
 * use may infringe existing patents or copyrights, and any such use
 * would be at such party's own risk.  The original developer of this
 * software module and his/her company, and subsequent editors and their
 * companies (including Project Mayo), will have no liability for use of
 * this software or modifications or derivatives thereof.
 *
 *****************************************************************************
 *																				*	
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
 * $Id: mp4_mv.c 142 2004-12-12 07:30:52Z picard $
 *
 *****************************************************************************
 *
 * Authors:
 *
 *	Andrea	Graziani  (Ag): Original source code (Open Divx Decoder 0.4a).
 *	Pedro	Mateu     (Pm) and
 *	Gabor	Kovacs    (Kg) Heavily modified and optimized code
 *
 ****************************************************************************/

#include "stdafx.h"
 
#define MVTAB_0		0
#define MVTAB_1		14
#define MVTAB_2		14+96

#define _M(mv,bits) (mv<<8)|(bits+1)

_CONST int16_t MVtab[14+96+124] = {
//MVTAB_0
_M(3,4),_M(-3,4),_M(2,3),_M(2,3),_M(-2,3),_M(-2,3),_M(1,2),_M(1,2),
_M(1,2),_M(1,2),_M(-1,2),_M(-1,2),_M(-1,2),_M(-1,2),
//MVTAB_1
_M(12,10),_M(-12,10),
_M(11,10),_M(-11,10),_M(10,9),_M(10,9),_M(-10,9),_M(-10,9),_M(9,9),_M(9,9),
_M(-9,9),_M(-9,9),_M(8,9),_M(8,9),_M(-8,9),_M(-8,9),_M(7,7),_M(7,7),
_M(7,7),_M(7,7),_M(7,7),_M(7,7),_M(7,7),_M(7,7),_M(-7,7),_M(-7,7),
_M(-7,7),_M(-7,7),_M(-7,7),_M(-7,7),_M(-7,7),_M(-7,7),_M(6,7),_M(6,7),
_M(6,7),_M(6,7),_M(6,7),_M(6,7),_M(6,7),_M(6,7),_M(-6,7),_M(-6,7),
_M(-6,7),_M(-6,7),_M(-6,7),_M(-6,7),_M(-6,7),_M(-6,7),_M(5,7),_M(5,7),
_M(5,7),_M(5,7),_M(5,7),_M(5,7),_M(5,7),_M(5,7),_M(-5,7),_M(-5,7),
_M(-5,7),_M(-5,7),_M(-5,7),_M(-5,7),_M(-5,7),_M(-5,7),_M(4,6),_M(4,6),
_M(4,6),_M(4,6),_M(4,6),_M(4,6),_M(4,6),_M(4,6),_M(4,6),_M(4,6),
_M(4,6),_M(4,6),_M(4,6),_M(4,6),_M(4,6),_M(4,6),_M(-4,6),_M(-4,6),
_M(-4,6),_M(-4,6),_M(-4,6),_M(-4,6),_M(-4,6),_M(-4,6),_M(-4,6),_M(-4,6),
_M(-4,6),_M(-4,6),_M(-4,6),_M(-4,6),_M(-4,6),_M(-4,6),
//MVTAB_2
_M(32,12),_M(-32,12),
_M(31,12),_M(-31,12),_M(30,11),_M(30,11),_M(-30,11),_M(-30,11),_M(29,11),_M(29,11),
_M(-29,11),_M(-29,11),_M(28,11),_M(28,11),_M(-28,11),_M(-28,11),_M(27,11),_M(27,11),
_M(-27,11),_M(-27,11),_M(26,11),_M(26,11),_M(-26,11),_M(-26,11),_M(25,11),_M(25,11),
_M(-25,11),_M(-25,11),_M(24,10),_M(24,10),_M(24,10),_M(24,10),_M(-24,10),_M(-24,10),
_M(-24,10),_M(-24,10),_M(23,10),_M(23,10),_M(23,10),_M(23,10),_M(-23,10),_M(-23,10),
_M(-23,10),_M(-23,10),_M(22,10),_M(22,10),_M(22,10),_M(22,10),_M(-22,10),_M(-22,10),
_M(-22,10),_M(-22,10),_M(21,10),_M(21,10),_M(21,10),_M(21,10),_M(-21,10),_M(-21,10),
_M(-21,10),_M(-21,10),_M(20,10),_M(20,10),_M(20,10),_M(20,10),_M(-20,10),_M(-20,10),
_M(-20,10),_M(-20,10),_M(19,10),_M(19,10),_M(19,10),_M(19,10),_M(-19,10),_M(-19,10),
_M(-19,10),_M(-19,10),_M(18,10),_M(18,10),_M(18,10),_M(18,10),_M(-18,10),_M(-18,10),
_M(-18,10),_M(-18,10),_M(17,10),_M(17,10),_M(17,10),_M(17,10),_M(-17,10),_M(-17,10),
_M(-17,10),_M(-17,10),_M(16,10),_M(16,10),_M(16,10),_M(16,10),_M(-16,10),_M(-16,10),
_M(-16,10),_M(-16,10),_M(15,10),_M(15,10),_M(15,10),_M(15,10),_M(-15,10),_M(-15,10),
_M(-15,10),_M(-15,10),_M(14,10),_M(14,10),_M(14,10),_M(14,10),_M(-14,10),_M(-14,10),
_M(-14,10),_M(-14,10),_M(13,10),_M(13,10),_M(13,10),_M(13,10),_M(-13,10),_M(-13,10),
_M(-13,10),_M(-13,10)
};

static _INLINE int getMVData( mp4_decode* dec, const int16_t* table, int fcode ) //max 13bits
{
	int code,res;

	inlineloadbits(dec);

	code = showbits(dec,13);

	if (code >= 4096) {
		flushbits(dec,1);
		return 0;
	}
	
	if (code >= 512) 
		code = (code >> 8) - 2 + MVTAB_0;
	else
	if (code >= 128) 
		code = (code >> 2) - 32 + MVTAB_1;
	else
		code = code-4 + MVTAB_2; 

	code = table[code];
	flushbits(dec,code & 255);
	code >>= 8;

	if (fcode) {
		code <<= fcode;
		res = getbits(dec,fcode);
		res -= (1 << fcode) - 1;
		if (code < 0)
			res = -res;
		code += res;
	}

	return code;
}

int getPMV(int Block_Num,int pos, mp4_decode *dec)
{
	int p1, p2, p3, temp;
	int resyncpos = dec->resyncpos;
	int* mv = &dec->mv_buf[(pos & dec->mv_bufmask)*4];
	int* mvlast = &dec->mv_buf[((pos-MB_X) & dec->mv_bufmask)*4];

	// we are allowed to access borders on left and right (always zero)

	switch (Block_Num)
	{
		case 0:
			if (pos == resyncpos) 
				return 0;
			if (pos < resyncpos+MB_X) 
			{
				if (pos != resyncpos+MB_X-1)
					return mv[-4+1];

				if (POSX(pos)==0)
					return mvlast[4+2];

				p1 = mv[-4+1];
				p2 = 0;
				p3 = mvlast[4+2];
				break;
			}
			p1 = mv[-4+1];
			p2 = mvlast[2];
			p3 = mvlast[4+2];
			break;
		case 1:
			if (pos < resyncpos+MB_X) 
			{
				if (pos != resyncpos+MB_X-1)
					return mv[0];

				p1 = mv[0];
				p2 = 0;
				p3 = mvlast[4+2];
				break;
			}
			p1 = mv[0];
			p2 = mvlast[3];
			p3 = mvlast[4+2];
			break;	
		case 2:
			p1 = mv[-4+3];
			p2 = mv[0];
			p3 = mv[1];
			if (pos == resyncpos)
				p1 = 0;
			break;
		default: // case 3
			p1 = mv[2];
			p2 = mv[0];
			p3 = mv[1];
			break;
	}

	temp = min(max(p1, p2), min(max(p2, p3), max(p1, p3)));
	p1=(p1<<16)>>16;
	p2=(p2<<16)>>16;
	p3=(p3<<16)>>16;
	p1 = min(max(p1, p2), min(max(p2, p3), max(p1, p3)));

	return ((temp&0xFFFF0000)|(p1&0xFFFF));
}

int getMV(int fcode,int prev, mp4_decode* dec)
{
	int high,low,range;
	int mv_x, mv_y;

	--fcode;

	mv_x = getMVData(dec,MVtab,fcode);
	mv_y = getMVData(dec,MVtab,fcode); 

	DEBUG_MSG6(DEBUG_VCODEC,T("mv_diff (%i,%i) pred (%i,%i) result (%i,%i)"), mv_x, mv_y, MVX(prev), MVY(prev), mv_x+MVX(prev), mv_y+MVY(prev));

	range = 1 << (fcode+6);
	high = (range >> 1) - 1;
	low = -(range >> 1);

	mv_x += MVX(prev);
	
	if (mv_x < low)
		mv_x += range;
	
	if (mv_x > high)
		mv_x -= range;
		
	mv_y += MVY(prev);
	
	if (mv_y < low)
		mv_y += range;
	
	if (mv_y > high)
		mv_y -= range;

	return MAKEMV(mv_x,mv_y);
}
