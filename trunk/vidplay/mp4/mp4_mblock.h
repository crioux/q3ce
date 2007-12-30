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
 * $Id: mp4_mblock.h 206 2005-03-19 15:03:03Z picard $
 *
 *****************************************************************************
 *
 * Authors:
 *
 *	Andrea	Graziani  (Ag): Original source code (Open Divx Decoder 0.4a).
 *	Marc	Dukette   (Md) and
 *	Pedro	Mateu     (Pm) and
 *	Gabor	Kovacs    (Kg) Heavily modified and optimized code
 *
 ****************************************************************************/

#ifndef _MP4_MBLOCK_H_
#define _MP4_MBLOCK_H_

extern void IVOP_h263( mp4_decode* );
extern void PVOP_h263( mp4_decode* );
extern void BVOP_h263( mp4_decode* );

extern void IVOP_mpeg4( mp4_decode* );
extern void PVOP_mpeg4( mp4_decode* );
extern void BVOP_mpeg4( mp4_decode* );

extern void IVOP_msmpeg( mp4_decode* );
extern void PVOP_msmpeg( mp4_decode* );
extern void BVOP_msmpeg( mp4_decode* );

extern _CONST uint8_t scan[3][64];

#define POSX(pos) ((pos) & (MB_X-1))
#define POSY(pos) ((pos) >> MB_X2)

extern int resync(mp4_decode *dec);
extern int resync_marker(mp4_decode *dec);
extern void blockInter( mp4_decode *dec );
extern int mvchroma4(const int* mv);
extern int mvchroma(int v);

extern int getCBPY( mp4_decode* dec ); //max 6 bits

extern _CONST uint8_t MCBPCtabIntra[];
extern _CONST uint8_t MCBPCtabInter[];
extern _CONST char DQtab[4];

static INLINE int getMCBPC_i( mp4_decode* dec ) //max 9bits
{
	int code = showbits(dec,9);

	if (code == 1) {
		flushbits(dec,9); // stuffing
		return 0;
	}
	else if (code < 8) return -1;
	code >>= 3;
	if (code >= 32) {
		flushbits(dec,1);
		return 3;
	}
	flushbits(dec,MCBPCtabIntra[(code<<1)+1]);
	return MCBPCtabIntra[code<<1];
}

static INLINE int getMCBPC_p( mp4_decode* dec ) //max 9bits
{
	int code = showbits(dec,9);

	if (code == 0)	return -1;
	if (code >= 256)
	{
		flushbits(dec,1);
		return 0;
	}
	flushbits(dec,MCBPCtabInter[(code<<1)+1]);
	return MCBPCtabInter[code<<1];
}

static INLINE void clearblock(idct_block_t *psblock) 
{
	int *i=(int*)psblock,*ie=(int*)(psblock+64);
	do
	{
		i[3] = i[2] = i[1] = i[0] = 0;
		i[7] = i[6] = i[5] = i[4] = 0;
		i+=8;
	}
	while (i!=ie);
}

#endif
