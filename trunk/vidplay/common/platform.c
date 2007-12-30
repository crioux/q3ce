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
 * $Id: platform.c 153 2004-12-20 16:46:49Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"
#include "cpu/cpu.h"

#define LOWMEMORY_LIMIT		768*1024

int DebugMask() 
{
	return DEBUG_SYS;//|DEBUG_FORMAT|DEBUG_PLAYER;//|DEBUG_VIDEO;//|DEBUG_AUDIO|DEBUG_VIDEO|DEBUG_PLAYER;//|DEBUG_VIDEO;//DEBUG_TEST;//|DEBUG_FORMAT|DEBUG_PLAYER;
}

static const datatable Params[] = 
{
	{ PLATFORM_LANG,		TYPE_INT, DF_SETUP|DF_ENUMSTRING|DF_RESTART, LANG_ID },
	{ PLATFORM_TYPE,		TYPE_STRING, DF_SETUP|DF_RDONLY|DF_GAP },
	{ PLATFORM_VER,			TYPE_INT, DF_SETUP|DF_RDONLY|DF_HIDDEN },
	{ PLATFORM_VERSION,		TYPE_STRING, DF_SETUP|DF_RDONLY },
	{ PLATFORM_OEMINFO,		TYPE_STRING, DF_SETUP|DF_RDONLY },
	{ PLATFORM_TYPENO,		TYPE_INT, DF_SETUP|DF_RDONLY|DF_HIDDEN },
	{ PLATFORM_MODEL,		TYPE_INT, DF_SETUP|DF_RDONLY|DF_HIDDEN },
	{ PLATFORM_CAPS,		TYPE_INT, DF_SETUP|DF_RDONLY|DF_HIDDEN|DF_HEX },
	{ PLATFORM_CPU,			TYPE_STRING, DF_SETUP|DF_RDONLY },
	{ PLATFORM_CPUMHZ,		TYPE_INT, DF_SETUP|DF_RDONLY|DF_MHZ },
	{ PLATFORM_ICACHE,		TYPE_INT, DF_SETUP|DF_RDONLY|DF_HIDDEN },
	{ PLATFORM_DCACHE,		TYPE_INT, DF_SETUP|DF_RDONLY|DF_HIDDEN },
	{ PLATFORM_WMPVERSION,	TYPE_INT, DF_SETUP|DF_RDONLY|DF_HIDDEN },

	DATATABLE_END(PLATFORM_ID)
};

static int Enum(platform* p, int* No, datadef* Param)
{
	return NodeEnumTable(No,Param,Params);
}

static int Get(platform* p, int No, void* Data, int Size)
{
	int Result = ERR_INVALID_PARAM;
	switch (No)
	{
	case PLATFORM_LANG: GETVALUE(Context()->Lang,int); break;
	case PLATFORM_TYPE:	GETSTRING(p->PlatformType); break;
	case PLATFORM_WMPVERSION: GETVALUE(p->WMPVersion,int); break;
	case PLATFORM_CPU: GETSTRING(p->CPU); break;
	case PLATFORM_OEMINFO: if (p->OemInfo[0]) GETSTRING(p->OemInfo); break;
	case PLATFORM_VER: GETVALUE(p->Ver,int); break;
	case PLATFORM_TYPENO: GETVALUE(p->Type,int); break;
	case PLATFORM_MODEL: GETVALUE(p->Model,int); break;
	case PLATFORM_CAPS: GETVALUE(p->Caps,int); break;
	case PLATFORM_ICACHE: GETVALUECOND(p->ICache,int,p->ICache>0); break;
	case PLATFORM_DCACHE: GETVALUECOND(p->DCache,int,p->DCache>0); break;
	case PLATFORM_VERSION: GETSTRING(p->Version); break;
	case PLATFORM_LOWMEMORY: GETVALUE(p->LowMemory,bool_t); break;
	case PLATFORM_CPUMHZ: 
		assert(Size==sizeof(int));
		if ((*(int*)Data = CPUSpeed())>0)
			Result = ERR_NONE;
		break;
	}
	return Result;
}

static int Set(platform* p, int No, const void* Data, int Size)
{
	int Result = ERR_INVALID_PARAM;
	switch (No)
	{
	case PLATFORM_LANG: SETVALUE(Context()->Lang,int,ERR_NONE); break;
	}
	return Result;
}

int QueryPlatform(int Param)
{
	node* Platform = Context()->Platform;
	int Value = 0;
	if (Platform)
		Platform->Get(Platform,Param,&Value,sizeof(Value));
	return Value;
}

extern void PlatformDetect(platform*);

static int Create(platform* p)
{
	p->Node.Enum = (nodeenum)Enum;
	p->Node.Get = (nodeget)Get;
	p->Node.Set = (nodeset)Set;
	p->Model = MODEL_UNKNOWN;
	p->LowMemory = Context()->StartUpMemory < LOWMEMORY_LIMIT;
	CPUDetect(p->CPU,&p->Caps,&p->ICache,&p->DCache);
	PlatformDetect(p);
	return ERR_NONE;
}

const nodedef Platform = 
{
	sizeof(platform)|CF_GLOBAL|CF_SETTINGS,
	PLATFORM_ID,
	NODE_CLASS,
	PRI_MAXIMUM+650,
	(nodecreate)Create,
	NULL,
};

