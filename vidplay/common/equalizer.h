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
 * $Id: equalizer.h 131 2004-12-04 20:36:04Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __EQUALIZER_H
#define __EQUALIZER_H

#define EQUALIZER_ID		FOURCC('E','Q','U','A')

#define EQUALIZER_RESET		0x10E
#define EQUALIZER_ENABLED	0x10F
#define EQUALIZER_PREAMP	0x100
#define EQUALIZER_1			0x101
#define EQUALIZER_2			0x102
#define EQUALIZER_3			0x103
#define EQUALIZER_4			0x104
#define EQUALIZER_5			0x105
#define EQUALIZER_6			0x106
#define EQUALIZER_7			0x107
#define EQUALIZER_8			0x108
#define EQUALIZER_9			0x109
#define EQUALIZER_10		0x10A
#define EQUALIZER_EQ		0x110

void Equalizer_Init();
void Equalizer_Done();

#endif
