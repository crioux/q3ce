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
 * $Id: mp4_header.h 183 2005-01-11 03:29:39Z picard $
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
	
#ifndef _MP4_HEADER_H_
#define _MP4_HEADER_H_

#define VOL_START_CODE					0x120
#define VOP_START_CODE					0x1b6
#define USERDATA_START_CODE				0x1b2

#define VOL_START_CODE_MASK				0x0f

#define I_VOP							0
#define P_VOP							1
#define B_VOP							2
#define S_VOP							3
#define N_VOP							4

#define RECTANGULAR						0
#define BINARY							1
#define BINARY_SHAPE_ONLY				2 
#define GREY_SCALE						3

#define SPRITE_STATIC					1
#define SPRITE_GMC						2

#define INTER							0
#define INTER_Q							1
#define INTER4V							2
#define INTRA							3	
#define INTRA_Q	 						4

#define DIRECT							0
#define INTERPOLATE						1
#define BACKWARD						2
#define FORWARD							3	

#define RESCUE							1

#define ASPECT_SQUARE					1
#define ASPECT_625TYPE_43				2
#define ASPECT_525TYPE_43				3
#define ASPECT_625TYPE_169				8
#define ASPECT_525TYPE_169				9
#define ASPECT_CUSTOM					15

#define TOP 1
#define LEFT 0

extern int gethdr_msmpeg( mp4_decode* );
extern int gethdr_mpeg4( mp4_decode* );
extern int gethdr_h263( mp4_decode* );
extern int gethdr_intel_h263( mp4_decode* );

extern bool_t findnext_mpeg4( mp4_decode* );

extern _CONST uint8_t def_quant_inter[64];
extern _CONST uint8_t def_quant_intra[64];

static INLINE int _log2(uint32_t data)
{
	int i;
	if (!data) ++data;
	for (i=0;data;++i)
		data >>= 1;
    return i;
}

#endif

