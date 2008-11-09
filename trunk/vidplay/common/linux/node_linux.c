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
 * $Id: node_win32.c 202 2005-01-25 01:27:33Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "../common.h"

#define TWIN(a) a

void NOINLINE NodeBase(int Class, tchar_t* Base)
{
	Class; Base;
}

bool_t NodeRegLoadValue(int Class, int Id, void* Data, int Size, int Type)
{
	Class; Id; Data; Size; Type;
}

void NodeRegSaveValue(int Class, int Id, const void* Data, int Size, int Type)
{
	Class; Id; Data; Size; Type;
}

void NodeRegLoad(node* p)
{
	p;
}

void NodeRegSave(node* p)
{
	p;
}

void NodeRegLoadGlobal() {}
void NodeRegSaveGlobal() {}

void* NodeLoadModule(const tchar_t* Path,int* Id,void** AnyFunc,void** Db)
{
	Path; Id; AnyFunc; Db;

	return NULL;
}

void NodeFreeModule(void* Module,void* Db)
{
	Module; Db;
}

void Plugins_Init()
{
}

void Plugins_Done()
{
}

