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
 * $Id: color.c 157 2005-01-02 00:39:09Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"

typedef struct equalizersettings
{
	node Node;
	equalizer Eq;

} equalizersettings;

static const datatable Params[] = 
{
	{ EQUALIZER_ENABLED,	TYPE_BOOL,DF_SETUP|DF_NOSAVE|DF_CHECKLIST|DF_RESYNC },
	{ EQUALIZER_PREAMP,		TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_SHORTLABEL|DF_RESYNC, -20, 20 },

	{ EQUALIZER_1,			TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_NOWRAP|DF_SHORTLABEL|DF_RESYNC|DF_GAP, -20, 20 },
	{ EQUALIZER_2,			TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_NOWRAP|DF_SHORTLABEL|DF_RESYNC, -20, 20 },
	{ EQUALIZER_3,			TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_NOWRAP|DF_SHORTLABEL|DF_RESYNC, -20, 20 },
	{ EQUALIZER_4,			TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_NOWRAP|DF_SHORTLABEL|DF_RESYNC, -20, 20 },
	{ EQUALIZER_5,			TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_NOWRAP|DF_SHORTLABEL|DF_RESYNC, -20, 20 },
	{ EQUALIZER_6,			TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_NOWRAP|DF_SHORTLABEL|DF_RESYNC, -20, 20 },
	{ EQUALIZER_7,			TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_NOWRAP|DF_SHORTLABEL|DF_RESYNC, -20, 20 },
	{ EQUALIZER_8,			TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_NOWRAP|DF_SHORTLABEL|DF_RESYNC, -20, 20 },
	{ EQUALIZER_9,			TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_NOWRAP|DF_SHORTLABEL|DF_RESYNC, -20, 20 },
	{ EQUALIZER_10,			TYPE_INT, DF_SETUP|DF_NOSAVE|DF_MINMAX|DF_NOWRAP|DF_SHORTLABEL|DF_RESYNC, -20, 20 },

	{ EQUALIZER_RESET,		TYPE_RESET,DF_SETUP|DF_NOSAVE|DF_GAP },
	{ EQUALIZER_EQ,			TYPE_EQUALIZER,DF_SETUP|DF_HIDDEN },

	DATATABLE_END(EQUALIZER_ID)
};

static int Enum( equalizersettings* p, int* No, datadef* Param)
{
	return NodeEnumTable(No,Param,Params);
}

static int Get( equalizersettings* p, int No, void* Data, int Size )
{
	int Result = ERR_INVALID_PARAM;
	switch (No)
	{
	case EQUALIZER_RESET: Result = ERR_NONE; break;
	case EQUALIZER_ENABLED: GETVALUE(p->Eq.Enabled,bool_t); break;
	case EQUALIZER_PREAMP: GETVALUE(p->Eq.Amplify,int); break;
	case EQUALIZER_1: GETVALUE(p->Eq.Eq[0],int); break;
	case EQUALIZER_2: GETVALUE(p->Eq.Eq[1],int); break;
	case EQUALIZER_3: GETVALUE(p->Eq.Eq[2],int); break;
	case EQUALIZER_4: GETVALUE(p->Eq.Eq[3],int); break;
	case EQUALIZER_5: GETVALUE(p->Eq.Eq[4],int); break;
	case EQUALIZER_6: GETVALUE(p->Eq.Eq[5],int); break;
	case EQUALIZER_7: GETVALUE(p->Eq.Eq[6],int); break;
	case EQUALIZER_8: GETVALUE(p->Eq.Eq[7],int); break;
	case EQUALIZER_9: GETVALUE(p->Eq.Eq[8],int); break;
	case EQUALIZER_10:GETVALUE(p->Eq.Eq[9],int); break;
	case EQUALIZER_EQ:GETVALUE(p->Eq,equalizer); break;
	}
	return Result;
}

static NOINLINE int UpdateEqualizer()
{
	context* p = Context();
	if (!p->Player)
		return ERR_NONE;
	return p->Player->Set(p->Player,PLAYER_UPDATEEQUALIZER,NULL,0);
}

static int Set( equalizersettings* p, int No, const void* Data, int Size )
{
	int Result = ERR_INVALID_PARAM;
	switch (No)
	{
	case EQUALIZER_ENABLED: SETVALUE(p->Eq.Enabled,bool_t,UpdateEqualizer()); break;
	case EQUALIZER_PREAMP: SETVALUE(p->Eq.Amplify,int,UpdateEqualizer()); break;
	case EQUALIZER_1: SETVALUE(p->Eq.Eq[0],int,UpdateEqualizer()); break;
	case EQUALIZER_2: SETVALUE(p->Eq.Eq[1],int,UpdateEqualizer()); break;
	case EQUALIZER_3: SETVALUE(p->Eq.Eq[2],int,UpdateEqualizer()); break;
	case EQUALIZER_4: SETVALUE(p->Eq.Eq[3],int,UpdateEqualizer()); break;
	case EQUALIZER_5: SETVALUE(p->Eq.Eq[4],int,UpdateEqualizer()); break;
	case EQUALIZER_6: SETVALUE(p->Eq.Eq[5],int,UpdateEqualizer()); break;
	case EQUALIZER_7: SETVALUE(p->Eq.Eq[6],int,UpdateEqualizer()); break;
	case EQUALIZER_8: SETVALUE(p->Eq.Eq[7],int,UpdateEqualizer()); break;
	case EQUALIZER_9: SETVALUE(p->Eq.Eq[8],int,UpdateEqualizer()); break;
	case EQUALIZER_10:SETVALUE(p->Eq.Eq[9],int,UpdateEqualizer()); break;
	case EQUALIZER_EQ:SETVALUE(p->Eq,equalizer,UpdateEqualizer()); break;
	}
	return Result;
}

static int Create(equalizersettings* p)
{
	p->Node.Enum = (nodeenum)Enum;
	p->Node.Set = (nodeset)Set;
	p->Node.Get = (nodeget)Get;
	return ERR_NONE;
}

static const nodedef Equalizer =
{
	sizeof(equalizersettings)|CF_GLOBAL|CF_SETTINGS,
	EQUALIZER_ID,
	NODE_CLASS,
	PRI_MAXIMUM+580,
	(nodecreate)Create,
};

void Equalizer_Init()
{
	NodeRegisterClass(&Equalizer);
}

void Equalizer_Done()
{
	NodeUnRegisterClass(EQUALIZER_ID);
}
