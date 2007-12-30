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
 * $Id: timer.h 157 2005-01-02 00:39:09Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __TIMER_H
#define __TIMER_H

//---------------------------------------------------------------
// timer (abstract)

#define TIMER_CLASS			FOURCC('T','I','M','R')

// ticks counter (tick_t)
#define TIMER_TIME			0x60
// relative speed (fraction_t)
#define TIMER_SPEED			0x62
// play (bool_t)
#define TIMER_PLAY			0x63

DLL int TimerEnum(void*, int* EnumNo, datadef* Out);

//---------------------------------------------------------------
// system timer 

#define SYSTIMER_ID			FOURCC('S','Y','S','T')

void Timer_Init();
void Timer_Done();

#endif
