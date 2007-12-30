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
 * $Id: id3tag.h 115 2004-11-27 12:55:48Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/
 
#ifndef __ID3TAG_H
#define __ID3TAG_H

#define ID3TAG1SIZE			(3+30+30+30+4+30+1)
#define ID3TAGQUERYSIZE		10

DLL int Id3TagQuery(const void* Ptr, int Len);
DLL void Id3TagParse(const void* Ptr, int Len, pin* Pin);
DLL bool_t Id3Genre(int No, tchar_t* Genre, int Size);

#endif
