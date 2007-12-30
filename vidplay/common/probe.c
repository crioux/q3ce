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
 * $Id: probe.c 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"

static NOINLINE void SkipSpace(const tchar_t** p)
{
	while (IsSpace(**p)) ++(*p);
}

static NOINLINE bool_t IsToken(const tchar_t** p,const tchar_t* Name)
{
	int Len = tcslen(Name);
	SkipSpace(p);
	if (TcsNICmp(*p,Name,Len)==0 && !IsAlpha((*p)[Len]))
	{
		(*p) += Len;
		return 1;
	}
	return 0;
}

static NOINLINE bool_t IsSymbol(const tchar_t** p,int ch)
{
	SkipSpace(p);
	if (**p == ch)
	{
		++(*p);
		return 1;
	}
	return 0;
}

static NOINLINE bool_t IsTokenWithParam(const tchar_t** p,const tchar_t* Name)
{
	return IsToken(p,Name) && IsSymbol(p,'(');
}

static INLINE void ParamEnd(const tchar_t** p)
{
	IsSymbol(p,')');
}

static NOINLINE bool_t ParamNext(const tchar_t** p)
{
	if (IsSymbol(p,','))
		return 1;
	ParamEnd(p);
	return 0;
}

static int Run(const tchar_t** p, const uint8_t** Data, int* Length);

static NOINLINE int GetOfs(const tchar_t** p, const uint8_t** Data, int* Length)
{
	int Ofs = 0;
 	if (IsSymbol(p,'('))
	{
		Ofs = Run(p,Data,Length);
		ParamEnd(p);
	}
	return Ofs;
}

static int Run(const tchar_t** p, const uint8_t** Data, int* Length)
{
	int a,b,Ofs,v = 0;
	
	assert(!IsSymbol(p,')') && !IsSymbol(p,',') && **p);

	if (IsTokenWithParam(p,T("scan")))
	{
		const uint8_t* SaveData = *Data;
		int Result = -1;
		int SaveLength = *Length;
		const tchar_t* Start;
		int End = *Length - Run(p,Data,Length); // limit
		if (End < 0)
			End = 0;
		ParamNext(p);

		Start = *p;
		while (*Length > End && Result<0)
		{
			*p = Start;
			if (Run(p,Data,Length)) //success
				Result = 1;
			ParamNext(p);
			if (Run(p,Data,Length) && Result<=0) //fail
				Result = 0;
			ParamNext(p);
			Run(p,Data,Length); //step
			ParamEnd(p);
		}

		if (Result>=0)
			v = Result;

		*Data = SaveData;
		*Length = SaveLength;
	}
	else
	if (IsToken(p,T("fwd")))
	{
		v = 1;
		if (IsSymbol(p,'('))
		{
			v = Run(p,Data,Length);
			ParamEnd(p);
		}
		*Length -= v;
		*Data += v;
	}
	else
	if (IsToken(p,T("le32")))
	{
		Ofs = GetOfs(p,Data,Length);
		if (*Length >= Ofs+4)
		{
			v  = (*Data)[Ofs+0];
			v |= (*Data)[Ofs+1] << 8;
			v |= (*Data)[Ofs+2] << 16;
			v |= (*Data)[Ofs+3] << 24;
		}
	}
	else
	if (IsToken(p,T("be32")))
	{
		Ofs = GetOfs(p,Data,Length);
		if (*Length >= Ofs+4)
		{
			v  = (*Data)[Ofs+0] << 24;
			v |= (*Data)[Ofs+1] << 16;
			v |= (*Data)[Ofs+2] << 8;
			v |= (*Data)[Ofs+3];
		}
	}
	else
	if (IsToken(p,T("le16")))
	{
		Ofs = GetOfs(p,Data,Length);
		if (*Length >= Ofs+2)
		{
			v  = (*Data)[Ofs+0];
			v |= (*Data)[Ofs+1] << 8;
		}
	}
	else
	if (IsToken(p,T("be16")))
	{
		Ofs = GetOfs(p,Data,Length);
		if (*Length >= Ofs+2)
		{
			v  = (*Data)[Ofs+0] << 8;
			v |= (*Data)[Ofs+1];
		}
	}
	else
	if (IsTokenWithParam(p,T("eq")))
	{
		a = Run(p,Data,Length);
		ParamNext(p);
		b = Run(p,Data,Length);
		ParamEnd(p);
		v = a == b;
	}
	else
	if (IsTokenWithParam(p,T("ne")))
	{
		a = Run(p,Data,Length);
		ParamNext(p);
		b = Run(p,Data,Length);
		ParamEnd(p);
		v = a != b;
	}
	else
	if (IsTokenWithParam(p,T("mask")))
	{
		a = Run(p,Data,Length);
		ParamNext(p);
		b = Run(p,Data,Length);
		ParamEnd(p);
		v = a & b;
	}
	else
	if (IsTokenWithParam(p,T("or")))
	{
		do
		{
			if (Run(p,Data,Length))
				v = 1;
		}
		while (ParamNext(p));
	}
	else
	if (IsTokenWithParam(p,T("and")))
	{
		v = 1;
		do
		{
			if (!Run(p,Data,Length))
				v = 0;
		}
		while (ParamNext(p));
	}
	else
	if (IsSymbol(p,'!'))
		v = !Run(p,Data,Length);
	else
	if (IsSymbol(p,'~'))
		v = ~Run(p,Data,Length);
	else
	if (IsSymbol(p,'\''))
	{
		int i,c[4];
		c[0] = c[1] = c[2] = c[3] = '_';
		for (i=0;**p && **p!='\'';++i,++(*p))
			if (i<4) c[i] = **p;
		IsSymbol(p,'\'');
		v = FOURCCLE(c[0],c[1],c[2],c[3]);
	}
	else
	if (IsDigit(**p) || **p=='-')
	{
		bool_t Neg = IsSymbol(p,'-');

		if ((*p)[0] == '0' && (*p)[1] == 'x')
		{
			(*p)+=2;
			for (;**p;++(*p))
			{
				int h = Hex(**p);
				if (h<0) break;
				v=v*16+h;
			}		
		}
		else
		{
			for (;IsDigit(**p);++(*p))
				v = v*10 + (**p-'0');
		}

		if (Neg)
			v = -v;
	}

	return v;
}

bool_t DataProbe(const void* Ptr, int Length,const tchar_t* Code)
{
	const uint8_t* Data = (const uint8_t*)Ptr;
	if (!Code[0])
		return 0;
	return Run(&Code,&Data,&Length);
}
