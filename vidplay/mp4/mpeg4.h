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
 * $Id: mpeg4.h 178 2005-01-10 07:34:30Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __MPEG4_H
#define __MPEG4_H

#define MSMPEG4_ID		FOURCC('M','P','4','3')
#define MPEG4_ID		FOURCC('D','I','V','X')
#define H263_ID			FOURCC('H','2','6','3')
#define I263_ID			FOURCC('I','2','6','3')

#define MPEG4_ERROR_QPEL		0x200
#define MPEG4_ERROR_INTERLACE	0x201
#define MPEG4_ERROR_GMC			0x202
#define MPEG4_ERROR_H263		0x204
#define MPEG4_ERROR_PARTITIONING 0x205

void mpeg4_init();
void mpeg4_done();

void msmpeg4_init();
void msmpeg4_done();

#endif
