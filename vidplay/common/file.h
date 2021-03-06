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
 * $Id: file.h 192 2005-01-13 17:02:00Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __FILE_H
#define __FILE_H

#define FILE_ID			FOURCC('F','I','L','E')
#define FILEDB_ID		FOURCC('F','I','D','B')
#define VFS_ID			FOURCC('V','F','S','_')

DLL bool_t FileExits(const tchar_t*);
DLL int64_t FileDate(const tchar_t*); // yyyyhhddhhmmssmmm

#endif
