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
 * $Id: tools.h 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __BUFFER_H
#define __BUFFER_H

struct stream;
typedef struct buffer
{
	uint8_t* Data;
	int WritePos;
	int ReadPos;
	int Allocated;

} buffer;

DLL void BufferClear(buffer*);
DLL void BufferDrop(buffer*);
DLL bool_t BufferAlloc(buffer* p, int Size, int Align);
DLL bool_t BufferWrite(buffer*, const void* Ptr, int Length, int Align);
DLL bool_t BufferRead(buffer*, const uint8_t** Ptr, int Length);
DLL void BufferPack(buffer*, int Readed);
DLL bool_t BufferStream(buffer*, struct stream*);

typedef struct array
{
	// these are private members
	uint8_t* _Begin;
	uint8_t* _End;
	int _Allocated;
	block _Block;

} array;

typedef	int (*arraycmp)(const void* a,const void* b);

DLL void ArrayClear(array*);
DLL void ArrayDrop(array*);
DLL int ArrayFind(const array* p, int Count, int Width, const void* Data, arraycmp Cmp, bool_t* Found);
DLL bool_t ArrayAlloc(array* p,int Total,int Align);
DLL bool_t ArrayAppend(array* p, const void* Ptr, int Length, int Align);
DLL bool_t ArrayAdd(array* p, int Count, int Width, const void* Data, arraycmp Cmp,int Align);
DLL bool_t ArrayRemove(array* p, int Count, int Width, const void* Data, arraycmp Cmp);
DLL void ArraySort(array* p, int Count, int Width, arraycmp Cmp);
DLL void ArrayLock(array*);
DLL bool_t ArrayUnlock(array*,int Align);

#define ARRAYEMPTY(Array)			((Array)._Begin==(Array)._End)
#define ARRAYBEGIN(Array,Type)		((Type*)((Array)._Begin))
#define ARRAYEND(Array,Type)		((Type*)((Array)._End))
#define ARRAYCOUNT(Array,Type)		(ARRAYEND(Array,Type)-ARRAYBEGIN(Array,Type))
#define ARRAYALLOCATED(Array,Type)	((Array)._Allocated/(int)sizeof(Type))

#endif

