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
 * $Id: context_win32.c 202 2005-01-25 01:27:33Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "../common.h"

#if defined(TARGET_WIN32) || defined(TARGET_WINCE)

static context* ContextPtr = NULL;

void SetContext(context* Ptr) 
{ 
	ContextPtr=Ptr; 
	NodeRegLoadValue(PLATFORM_ID,PLATFORM_LANG,&Ptr->Lang,sizeof(int),TYPE_INT);
}

context* Context() 
{ 
	return ContextPtr; 
}

#endif
