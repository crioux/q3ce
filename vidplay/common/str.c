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
 * $Id: str.c 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"

typedef struct stringdef
{	
	int Class;
	int Id;
	// tchar_t s[]
} stringdef;

#define TABLEINIT	3072
#define TABLEALIGN	1024

#ifdef TARGET_PALMOS
#define BUFFERSIZE	512*sizeof(tchar_t)
#else
#define BUFFERSIZE	2048*sizeof(tchar_t)
#endif

void StringAlloc()
{
	context* p = Context();
	if (p->Lang != LANG_DEFAULT)
		ArrayAlloc(&p->StrTable[0],TABLEINIT,TABLEALIGN);
	ArrayAlloc(&p->StrTable[1],TABLEINIT,TABLEALIGN);
	p->StrLock = LockCreate();
}

void StringFree()
{
	context* p = Context();
	array* i;
	for (i=ARRAYBEGIN(p->StrBuffer,array);i!=ARRAYEND(p->StrBuffer,array);++i)
		ArrayClear(i);
	ArrayClear(&p->StrTable[0]);
	ArrayClear(&p->StrTable[1]);
	ArrayClear(&p->StrBuffer);
	LockDelete(p->StrLock);
	p->StrLock = NULL;
}

static int CmpDef(const stringdef* const* pa, const stringdef* const* pb)
{
	const stringdef* a = *pa;
	const stringdef* b = *pb;
	if (a->Class < b->Class || (a->Class == b->Class && a->Id < b->Id))
		return -1;
	if (a->Class > b->Class || a->Id > b->Id)
		return 1;
	return 0;
}

static void AddDef(context* p, bool_t Default, const stringdef* Def)
{
	array* Table = &p->StrTable[Default?1:0];
	ArrayAdd(Table,ARRAYCOUNT(*Table,stringdef*),sizeof(stringdef*),&Def,(arraycmp)CmpDef,TABLEALIGN);
}

static void Filter(tchar_t* i)
{
	tchar_t* j=i;
	for (;*i;++i,++j)
	{
		if (i[0]=='\\' && i[1]=='n')
		{
			*j=10;
			++i;
		}
		else
			*j=*i;
	}
	*j=0;
}

void StringAddText(const char* p, int n)
{
	int UserLang = Context()->Lang;
	tchar_t* s = (tchar_t*)malloc(MAXLINE*sizeof(tchar_t));
	char* s8 = (char*)malloc(MAXLINE*sizeof(char));
	int CodePage = -1;
	int Lang = 0;
	tchar_t* q;
	int i,No;

	if (s && s8)
	{
		while (n>0)
		{
			for (i=0;n>0 && *p!=13 && *p!=10;++p,--n)
				if (i<MAXLINE-1)
					s8[i++] = *p;
			for (;n>0 && (*p==13 || *p==10);++p,--n)
			s8[i]=0;

			if (CodePage>=0)
				StrToTcsEx(s,s8,MAXLINE,CodePage);
			else
				AsciiToTcs(s,s8,MAXLINE);

			for (i=0;IsSpace(s[i]);++i);
			if (s[i]==0) continue;
			if (s[i]==';')
			{
				Scanf(s+i,T(";CODEPAGE = %d"),&CodePage);
				continue;
			}
			if (s[i]=='[')
			{
				++i;
				q = tcschr(s+i,']');
				if (!q || CodePage<0) break; // invalid language file
				*q = 0;

				Lang = StringToFourCC(s+i,1);
				if (Lang == FOURCC('D','E','F','A'))
					Lang = LANG_DEFAULT;

				if (Lang != LANG_DEFAULT)
				{
					if (Lang != UserLang)
						break;
					Context()->CodePage = CodePage;
				}
			}
			else
			{
				q = tcschr(s+i,'=');
				if (!q || !Lang) break; // invalid language file
				*q = 0;
				++q;

				if (tcslen(s+i)>4)
				{
					if (tcslen(s+i)<8)
						No = StringToFourCC(s+i+4,1);
					else
					{
						No = StringToInt(s+i+4,1);
						if (No >= 32768)
							No -= 65536;
					}
				}
				else
					No = 0;

				Filter(q);
				StringAdd(Lang==LANG_DEFAULT,StringToFourCC(s+i,1),No,q);
			}
		}
	}

	free(s);
	free(s8);
}

bool_t StringIsBinary(int Class,int Id)
{
	bool_t Result = 1;
	context* p = Context();
	array* i;
	const tchar_t* Def = StringDef(Class,Id);
	if (Def[0])
	{
		LockEnter(p->StrLock);
		for (i=ARRAYBEGIN(p->StrBuffer,array);i!=ARRAYEND(p->StrBuffer,array);++i)
			if (ARRAYBEGIN(*i,const tchar_t)<=Def && ARRAYEND(*i,const tchar_t)>Def)
			{
				Result = 0;
				break;
			}
		LockLeave(p->StrLock);
	}
	return Result;
}

bool_t StringAddBinary(const void* Data, int Length)
{
	context* p = Context();
	const uint8_t* Def = (const uint8_t*)Data + 2*sizeof(int);
	int Lang = *(const int*)Data;
	int Len;

	if (Lang != LANG_DEFAULT && Lang != p->Lang)
		return 0;

	LockEnter(p->StrLock);
	while (Length>sizeof(stringdef))
	{
		AddDef(p,Lang==LANG_DEFAULT,(const stringdef*)Def);

		Len = (tcslen((const tchar_t*)(Def+sizeof(stringdef)))+1)*sizeof(tchar_t);
		Len = sizeof(stringdef)+((Len+sizeof(int)-1) & ~(sizeof(int)-1));
		Def += Len;
		Length -= Len;
	}
	LockLeave(p->StrLock);
	return 1;
}

void StringAdd(bool_t Default, int Class, int Id, const tchar_t* s)
{
	context* p = Context();
	int Pos,Len,Total;
	bool_t Found;
	array* Last = &p->StrTable[Default?1:0];
	stringdef Def;
	stringdef* Ptr = &Def;
	Def.Class = Class;
	Def.Id = Id;

	if (!s)	s = T("");

	LockEnter(p->StrLock);
	// already the same?
	Pos = ArrayFind(Last,ARRAYCOUNT(*Last,stringdef*),sizeof(stringdef*),&Ptr,(arraycmp)CmpDef,&Found);
	if (Found && tcscmp(s,(tchar_t*)(ARRAYBEGIN(*Last,stringdef*)[Pos]+1))==0)
	{
		LockLeave(p->StrLock);
		return;
	}

	// add to buffer
	Len = (tcslen(s)+1)*sizeof(tchar_t);
	Total = sizeof(stringdef)+((Len+sizeof(int)-1) & ~(sizeof(int)-1));
	Last = ARRAYEND(p->StrBuffer,array)-1;

	if (ARRAYEMPTY(p->StrBuffer) || ARRAYCOUNT(*Last,uint8_t)+Total > ARRAYALLOCATED(*Last,uint8_t))
	{
		// add new buffer
		if (!ArrayAppend(&p->StrBuffer,NULL,sizeof(array),128))
		{
			LockLeave(p->StrLock);
			return;
		}
		Last = ARRAYEND(p->StrBuffer,array)-1;
		memset(Last,0,sizeof(array));
		if (!ArrayAlloc(Last,BUFFERSIZE,1))
		{
			LockLeave(p->StrLock);
			return;
		}
	}

	// no allocation needed here (can't fail)
	Ptr = (stringdef*)ARRAYEND(*Last,uint8_t);
	ArrayAppend(Last,&Def,sizeof(stringdef),1);
	ArrayAppend(Last,s,Len,1);
	ArrayAppend(Last,NULL,Total-Len-sizeof(stringdef),1);

	AddDef(p,Default,Ptr);
	LockLeave(p->StrLock);
}

void StringAddPrint(bool_t Default, int Class,int No,const tchar_t* Msg, ...)
{
	tchar_t s[256];
	va_list Arg;
	va_start(Arg, Msg);
	vstprintf(s, Msg, Arg);
	va_end(Arg);
	StringAdd(Default,Class,No,s);
}

int StringEnum(int Class, int No)
{
	int Result = 0;
	context* p = Context();
	stringdef **i;
	LockEnter(p->StrLock);

	for (i=ARRAYBEGIN(p->StrTable[1],stringdef*);i!=ARRAYEND(p->StrTable[1],stringdef*);++i)
		if ((*i)->Class==Class && No--==0)
		{
			Result = (*i)->Id;
			break;
		}

	LockLeave(p->StrLock);
	return Result;
}

const tchar_t* String(int Class, int Id)
{
	int n;
	context* p = Context();
	bool_t Found;
	stringdef Def;
	stringdef* Ptr = &Def;
	Def.Class = Class;
	Def.Id = Id;

	LockEnter(p->StrLock);
	for (n=0;n<2;++n)
	{
		int Pos = ArrayFind(&p->StrTable[n],ARRAYCOUNT(p->StrTable[n],stringdef*),
		                    sizeof(stringdef*),&Ptr,(arraycmp)CmpDef,&Found);
		if (Found)
		{
			LockLeave(p->StrLock);
			return (tchar_t*)(ARRAYBEGIN(p->StrTable[n],stringdef*)[Pos]+1);
		}
	}
	LockLeave(p->StrLock);
	return T("");
}

const tchar_t* StringDef(int Class, int Id)
{
	int n;
	context* p = Context();
	bool_t Found;
	stringdef Def;
	stringdef* Ptr = &Def;
	Def.Class = Class;
	Def.Id = Id;

	LockEnter(p->StrLock);
	for (n=1;n>=0;--n)
	{
		int Pos = ArrayFind(&p->StrTable[n],ARRAYCOUNT(p->StrTable[n],stringdef*),
		                    sizeof(stringdef*),&Ptr,(arraycmp)CmpDef,&Found);
		if (Found)
		{
			LockLeave(p->StrLock);
			return (tchar_t*)(ARRAYBEGIN(p->StrTable[n],stringdef*)[Pos]+1);
		}
	}
	LockLeave(p->StrLock);
	return T("");
}

#ifdef UNICODE
void TcsNCpy(tchar_t* Out,const tchar_t* In,int OutLen)
{
	if (OutLen>0)
	{
		int n = wcslen(In);
		wcsncpy(Out,In,OutLen);
		if (n>=OutLen) 
			Out[OutLen-1] = 0;
	}
}
#else
void TcsNCpy(tchar_t* Out,const tchar_t* In,int OutLen)
{
	if (OutLen>0)
	{
		int n = strlen(In);
		strncpy(Out,In,OutLen);
		if (n>=OutLen) 
			Out[OutLen-1] = 0;
	}
}
#endif

void BoolToString(tchar_t* Out, bool_t Bool, int OutSize)
{
	OutSize /= sizeof(tchar_t);
	TcsNCpy(Out,StringDef(PLATFORM_ID,Bool?PLATFORM_YES:PLATFORM_NO),OutSize);
};

int StringToFourCC(const tchar_t* In, bool_t Upper)
{
	char s[4+1];
	tchar_t Up[4+1];
	int i;

	if (Upper)
	{
		TcsNCpy(Up,In,4+1);
		TcsUpr(Up);
		In = Up;
	}
	TcsToAscii(s,In,5);

	for (i=0;i<4;++i)
		if (!s[i])
			for (;i<4;++i)
				s[i] = '_';

	return FOURCC(s[0],s[1],s[2],s[3]);
}

void FourCCToString(tchar_t* Out, int FourCC, int OutSize)
{
	char s[4+1];
	*(uint32_t*)s = (uint32_t)FourCC;
	s[4] = 0;
	AsciiToTcs(Out,s,OutSize/sizeof(tchar_t));
}

void FractionToString(tchar_t* Out, const fraction* p, int OutSize, bool_t Percent, int Decimal)
{
	int a,b,i;
	int Num = p->Num;
	int Den = p->Den;

	if (Percent)
	{
		while (abs(Num) > MAX_INT/100)
		{
			Num >>= 1;
			Den >>= 1;
		}
		Num *= 100;
	}

	if (Den)
	{
		if (Den<0)
		{
			Num = -Num;
			Den = -Den;
		}
		for (i=0,b=1;i<Decimal;++i,b*=10);
		if (Num>0)
		{
			// rounding
			a = Den/(2*b);
			if (Num<MAX_INT-a)
				Num += a;
			else
				Num = MAX_INT;
		}
		a=Num/Den;
		Num -= a*Den;
		b=(int)(((int64_t)Num*b)/Den);
	}
	else
		a=b=0;

	if (Decimal)
		stprintf(Out,T("%d.%0*d"),a,Decimal,b);
	else
		stprintf(Out,T("%d"),a);

	if (Percent)
		tcscat(Out,T("%"));

}

int StringToInt(const tchar_t* In, bool_t Hex)
{
	int v=0;
	Scanf(In,Hex ? T("%X"):T("%d"),&v);
	return v;
}

void IntToString(tchar_t* Out, int p, int OutSize, bool_t Hex)
{
	stprintf(Out,Hex ? T("0x%08X"):T("%d"),p);
}

void TickToString(tchar_t* Out, tick_t Tick, int OutSize, bool_t MS, bool_t Extended, bool_t Fix)
{
	if (!MS)
	{
		tchar_t Sign[2] = {0};
		int Hour,Min,Sec;
		if (Tick<0) 
		{
			Sign[0] = '-';
			Tick = -Tick;
		}
		Hour = Tick / 3600 / TICKSPERSEC;
		Tick -= Hour * 3600 * TICKSPERSEC;
		Min = Tick / 60 / TICKSPERSEC;
		Tick -= Min * 60 * TICKSPERSEC;
		Sec = Tick / TICKSPERSEC;
		Tick -= Sec * TICKSPERSEC;
		if (Hour)
			stprintf(Out,T("%s%d:%02d"),Sign,Hour,Min);
		else
			stprintf(Out,Fix?T("%s%02d"):T("%s%d"),Sign,Min);
		stprintf(Out+tcslen(Out),T(":%02d"),Sec);
		if (Extended)
			stprintf(Out+tcslen(Out),T(".%03d"),(Tick*1000)/TICKSPERSEC);
	}
	else
		stprintf(Out,T("%0.2f ms"),(Tick*1000.f)/TICKSPERSEC);
}

int Scanf(const tchar_t* In, const tchar_t* Mask, ...)
{
	va_list Arg;
	int n = 0;
	int Sign;
	int v;
	int Width;
	const tchar_t* In0;

	va_start(Arg, Mask);
	while (In && *In && *Mask)
	{
		switch (*Mask)
		{
		case '%':
			++Mask;

			Width = -1;
			if (IsDigit(*Mask))
			{
				Width = 0;
				for (;IsDigit(*Mask);++Mask)
					Width = Width*10 + (*Mask-'0');
			}

			switch (*Mask)
			{
			case 'X':
			case 'x':

				for (;IsSpace(*In);++In);
				v = 0;
				Sign = *In == '-';
				In0 = In;
				if (Sign) { ++In; --Width; }
				for (;Width!=0 && *In;++In,--Width)
				{
					int h = Hex(*In);
					if (h<0) break;
					v=v*16+h;
				}
				if (Sign) v=-v;
				if (In != In0)
				{
					*va_arg(Arg,int*) = v;
					++n;
				}
				else
					In = NULL;
				break;

			case 'd':
			case 'i':

				for (;IsSpace(*In);++In);
				v = 0;
				Sign = *In == '-';
				In0 = In;
				if (Sign) ++In;
				for (;Width!=0 && IsDigit(*In);++In,--Width)
					v=v*10+(*In-'0');
				if (Sign) v=-v;
				if (In != In0)
				{
					*va_arg(Arg,int*) = v;
					++n;
				}
				else
					In = NULL;
				break;

			case 'o':

				for (;IsSpace(*In);++In);
				v = 0;
				Sign = *In == '-';
				In0 = In;
				if (Sign) ++In;
				for (;Width!=0 && *In;++In,--Width)
				{
					if (*In >= '0' && *In <= '7')
						v=v*8+(*In-'0');
					else
						break;
				}
				if (Sign) v=-v;
				if (In != In0)
				{
					*va_arg(Arg,int*) = v;
					++n;
				}
				else
					In = NULL;
				break;
			}
			break;
		case 9:
		case ' ':
			for (;IsSpace(*In);++In);
			break;
		default:
			if (*Mask == *In)
				++In;
			else
			{
				In = NULL;
				n = -1;
			}
		}
		++Mask;
	}

	va_end(Arg);
	return n;
}

void GUIDToString(tchar_t* Out, const guid* p, int OutSize)
{
	stprintf(Out,T("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
		(int)p->v1,p->v2,p->v3,p->v4[0],p->v4[1],p->v4[2],p->v4[3],
		p->v4[4],p->v4[5],p->v4[6],p->v4[7]);
}

bool_t StringToGUID(const tchar_t* In, guid* p)
{
	int i,v[10];
	if (Scanf(In,T("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
			&p->v1,v+0,v+1,v+2,v+3,v+4,v+5,v+6,v+7,v+8,v+9) < 11)
	{
		memset(p,0,sizeof(guid));
		return 0;
	}
	p->v2 = (uint16_t)v[0];
	p->v3 = (uint16_t)v[1];
	for (i=0;i<8;++i)
		p->v4[i] = (uint8_t)v[2+i];
	return 1;
}

void StrToTcs(tchar_t* Out,const char* In,int OutLen)
{
	StrToTcsEx(Out,In,OutLen,Context()->CodePage);
}

void TcsToStr(char* Out,const tchar_t* In,int OutLen)
{
	TcsToStrEx(Out,In,OutLen,Context()->CodePage);
}

bool_t IsSpace(int ch) { return ch==' ' || ch==9; }
bool_t IsAlpha(int ch) { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'); }
bool_t IsDigit(int ch) { return ch>='0' && ch<='9'; }

int Hex(int ch) 
{
	if (IsDigit(ch))
		return ch-'0';
	if (ch >= 'a' && ch <= 'f')
		return ch-'a'+10;
	if (ch >= 'A' && ch <= 'F')
		return ch-'A'+10;
	return -1;
}


