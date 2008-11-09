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
 * $Id: association_win32.c 166 2005-01-05 01:31:02Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "../common.h"

static void SetEditFlags(const tchar_t* Base,DWORD Value)
{
	DWORD Disp=0;
	HKEY Key;
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, Base, 0, NULL, 0, KEY_READ|KEY_WRITE, NULL, &Key, &Disp) == ERROR_SUCCESS)
	{
		RegSetValueEx(Key, T("EditFlags"), 0, REG_DWORD, (LPBYTE)&Value, sizeof(Value));
		RegCloseKey(Key);
	}
}

static void SetReg(const tchar_t* Base,const tchar_t* New,bool_t State)
{
	tchar_t Old[MAX_PATH];
	tchar_t Backup[MAX_PATH];
	DWORD Disp=0;
	HKEY Key;
	DWORD RegSize;
	DWORD RegType;

	stprintf(Backup,T("%s.bak"),Context()->ProgramName);

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, Base, 0, KEY_READ|KEY_WRITE, &Key) == ERROR_SUCCESS)
	{
		RegSize=sizeof(Old);
		if (RegQueryValueEx(Key, NULL, 0, &RegType, (LPBYTE)Old, &RegSize) == ERROR_SUCCESS)
		{
			if (TcsICmp(Old,New)!=0)
			{
				if (State)
				{
					RegSetValueEx(Key, NULL, 0, REG_SZ, (LPBYTE)New, (tcslen(New)+1)*sizeof(tchar_t));
					RegSetValueEx(Key, Backup, 0, REG_SZ, (LPBYTE)Old, RegSize);
				}
			}
			else 
			if (!State)
			{
				RegSize = sizeof(Old);
				if (RegQueryValueEx(Key, Backup, 0, &RegType, (LPBYTE)Old, &RegSize) == ERROR_SUCCESS)
				{
					RegSetValueEx(Key, NULL, 0, REG_SZ, (LPBYTE)Old, RegSize);
					RegDeleteValue(Key, Backup);
				}
			}
		}
		RegCloseKey(Key);
	}
	else 
	if (State)
	{
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, Base, 0, NULL, 0, KEY_READ|KEY_WRITE, NULL, &Key, &Disp) == ERROR_SUCCESS)
		{
			RegSetValueEx(Key, NULL, 0, REG_SZ, (LPBYTE)New, (tcslen(New)+1)*sizeof(New));
			RegSetValueEx(Key, Backup, 0, REG_SZ, (LPBYTE)T(""), sizeof(tchar_t));
			RegCloseKey(Key);
		}
	}
}

static bool_t CmpReg(const tchar_t* Base, const tchar_t* Value)
{
	bool_t Result = 0;
	HKEY Key;
	DWORD RegSize;
	DWORD RegType;
	tchar_t s[MAXPATH];

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, Base, 0, KEY_READ, &Key) == ERROR_SUCCESS)
	{
		RegSize = sizeof(s);
		if (RegQueryValueEx(Key, NULL, 0, &RegType, (LPBYTE)s, &RegSize) == ERROR_SUCCESS && RegType == REG_SZ)
			Result = TcsICmp(s,Value)==0;

		RegCloseKey(Key);
	}
	return Result;
}

static void SetFileAssociation(const tchar_t* Ext,bool_t State)
{
	tchar_t Base[64];
	tchar_t Type[64];
	tchar_t Path[MAXPATH];
	tchar_t Open[MAXPATH];
	tchar_t Icon[MAXPATH];

	GetModuleFileName(NULL,Path,MAXPATH);
	if (tcschr(Path,' '))
		stprintf(Open,T("\"%s\" \"%%1\""),Path);
	else
		stprintf(Open,T("%s \"%%1\""),Path);
	stprintf(Icon,T("%s, -%d"),Path,1000);

	stprintf(Base,T(".%s"),Ext);
	stprintf(Type,T("%sFile"),Ext);
	TcsUpr(Type);
	SetReg(Base,Type,State);

	if (State)
		SetEditFlags(Type,268500992);

	stprintf(Base,T("%s\\DefaultIcon"),Type);
	SetReg(Base,Icon,State);

	stprintf(Base,T("%s\\Shell\\Open\\Command"),Type);
	SetReg(Base,Open,State);
}

#ifdef NDEBUG
static bool_t GetReg(const tchar_t* Base, tchar_t* Value,int Size)
{
	bool_t Result = 0;
	HKEY Key;
	DWORD RegSize;
	DWORD RegType;

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, Base, 0, KEY_READ, &Key) == ERROR_SUCCESS)
	{
		RegSize = Size;
		if (RegQueryValueEx(Key, NULL, 0, &RegType, (LPBYTE)Value, &RegSize) == ERROR_SUCCESS && RegType == REG_SZ)
			Result = 1;

		RegCloseKey(Key);
	}
	return Result;
}

static bool_t EmptyFileAssociation(const tchar_t* Ext)
{
	tchar_t Base[MAXPATH];
	tchar_t Type[MAXPATH];

	stprintf(Base,T(".%s"),Ext);
	if (GetReg(Base,Type,sizeof(Type)))
	{
		stprintf(Base,T("%s\\Shell\\Open\\Command"),Type);
		if (GetReg(Base,Type,sizeof(Type)))
			return 0;
	}
	return 1;
}
#endif

static bool_t GetFileAssociation(const tchar_t* Ext)
{
	tchar_t Base[64];
	tchar_t Type[64];

	stprintf(Base,T(".%s"),Ext);
	stprintf(Type,T("%sFile"),Ext);
	TcsUpr(Type);

	if (CmpReg(Base,Type))
	{
		tchar_t Path[MAXPATH];
		tchar_t Open[MAXPATH];

		GetModuleFileName(NULL,Path,MAXPATH);
		if (tcschr(Path,' '))
			stprintf(Open,T("\"%s\" \"%%1\""),Path);
		else
			stprintf(Open,T("%s \"%%1\""),Path);

		stprintf(Base,T("%s\\Shell\\Open\\Command"),Type);
		if (CmpReg(Base,Open))
			return 1;
	}

	return 0;
}

static int Enum( node* p, int* No, datadef* Param )
{
	int Result = ERR_INVALID_PARAM;
	array List;

	NodeEnumClass(&List,MEDIA_CLASS);

	if (*No>=0 && *No<ARRAYCOUNT(List,int))
	{
		memset(Param,0,sizeof(datadef));
		Param->No = ARRAYBEGIN(List,int)[*No];
		Param->Name = String(Param->No,NODE_NAME);
		Param->Type = TYPE_BOOL;
		Param->Size = sizeof(bool_t);
		Param->Flags = DF_SETUP|DF_NOSAVE|DF_CHECKLIST;
		Param->Class = ASSOCIATION_ID;
		if (!UniqueExts(ARRAYBEGIN(List,int),ARRAYBEGIN(List,int)+*No))
			Param->Flags = DF_HIDDEN;
		Result = ERR_NONE;
	}

	ArrayClear(&List);
	return Result;
}

static int Get(node* p, int No, void* Data, int Size)
{
	const tchar_t* Exts = String(No,NODE_EXTS);
	if (Exts && Exts[0])
	{
		tchar_t s[16];
		const tchar_t *r,*q;

		assert(Size==sizeof(bool_t));
		*(bool_t*)Data = 1;
		for (r=Exts;r;r=q)
		{
			q = tcschr(r,':');
			if (q)
			{
				TcsNCpy(s,r,q-r+1);
				if (!GetFileAssociation(s))
				{
					*(bool_t*)Data = 0;
					break;
				}
				q = tcschr(q,';');
				if (q) ++q;
			}
		}
		return ERR_NONE;

	}
	return ERR_INVALID_PARAM;
}

static int Set(node* p, int No, const void* Data, int Size)
{
	const tchar_t* Exts = String(No,NODE_EXTS);
	if (Exts && Exts[0])
	{
		tchar_t s[16];
		const tchar_t *r,*q;
		bool_t State;

		assert(Size==sizeof(bool_t));
		State = *(bool_t*)Data;

		for (r=Exts;r;r=q)
		{
			q = tcschr(r,':');
			if (q)
			{
				TcsNCpy(s,r,q-r+1);
				SetFileAssociation(s,State);
				q = tcschr(q,';');
				if (q) ++q;
			}
		}
		return ERR_NONE;
	}
	return ERR_INVALID_PARAM;
}

static void AssignEmpty(node* p)
{
#ifdef NDEBUG
	int v;
	if (!NodeRegLoadValue(0,REG_ASSOCIATIONS,&v,sizeof(v),TYPE_INT) || !v)
	{
		// set default associations for empty extensions
		array List;
		int* i;

		NodeEnumClass(&List,MEDIA_CLASS);

		for (i=ARRAYBEGIN(List,int);i!=ARRAYEND(List,int);++i)
		{
			const tchar_t* Exts = String(*i,NODE_EXTS);
			if (Exts && Exts[0])
			{
				bool_t Empty = 1;
				tchar_t s[16];
				const tchar_t *r,*q;

				for (r=Exts;r;r=q)
				{
					q = tcschr(r,':');
					if (q)
					{
						TcsNCpy(s,r,q-r+1);
						if (!EmptyFileAssociation(s))
						{
							Empty = 0;
							break;
						}
						q = tcschr(q,';');
						if (q) ++q;
					}
				}

				if (Empty)
				{
					bool_t True = 1;
					Set(p,*i,&True,sizeof(True));
				}
			}
		}

		ArrayClear(&List);

		v = 1;
		NodeRegSaveValue(0,REG_ASSOCIATIONS,&v,sizeof(v),TYPE_INT);
	}
#endif
}

static int Create(node* p)
{
	p->Enum = (nodeenum)Enum;
	p->Get = (nodeget)Get;
	p->Set = (nodeset)Set;
	AssignEmpty(p);
	return ERR_NONE;
}

static const nodedef Association =
{
	sizeof(node)|CF_GLOBAL|CF_SETTINGS,
	ASSOCIATION_ID,
	NODE_CLASS,
	PRI_MAXIMUM+500,
	(nodecreate)Create,
};

void Association_Init()
{
	NodeRegisterClass(&Association);
}

void Association_Done()
{
	NodeUnRegisterClass(ASSOCIATION_ID);
}

