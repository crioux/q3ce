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
 * $Id: str_win32.c 156 2004-12-23 14:07:33Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "../common.h"
#include "iconv.h"

#define MAXTEXT		20000

void GetModulePath(tchar_t* Path,const tchar_t* Module);
void FindFiles(const tchar_t* Path, const tchar_t* Mask,void(*Process)(const tchar_t*,void*),void* Param);
void StringAlloc();
void StringFree();

iconv_t g_pIConvASCtoWCHAR_T;
iconv_t g_pIConvWCHAR_TtoASC;
iconv_t g_pIConvUTF8toWCHAR_T;
iconv_t g_pIConvUTF8toASC;
iconv_t g_pIConvWCHAR_TtoUTF8;
iconv_t g_pIConvASCtoUTF8;

void String_Init()
{
	StringAlloc();
	g_pIConvASCtoWCHAR_T=iconv_open("ASCII","WCHAR_T");	
	g_pIConvWCHAR_TtoASC=iconv_open("WCHAR_T","ASCII");	
	g_pIConvUTF8toWCHAR_T=iconv_open("UTF8","WCHAR_T");	
	g_pIConvUTF8toASC=iconv_open("UTF8","ASCII");	
	g_pIConvWCHAR_TtoUTF8=iconv_open("WCHAR_T","UTF8");	
	g_pIConvASCtoUTF8=iconv_open("ASCII","UTF8");
}

void String_Done()
{
	iconv_close(g_pIConvASCtoWCHAR_T);
	iconv_close(g_pIConvWCHAR_TtoASC);
	iconv_close(g_pIConvUTF8toWCHAR_T);
	iconv_close(g_pIConvUTF8toASC);
	iconv_close(g_pIConvWCHAR_TtoUTF8);
	iconv_close(g_pIConvASCtoUTF8);
	StringFree();
}

#ifndef CP_UTF8
#define CP_UTF8 65001
#endif
void AsciiToTcs(tchar_t* Out,const char* In,int OutLen)
{
#ifdef UNICODE
	size_t x=(int)iconv(g_pIConvASCtoWCHAR_T,&In,strlen(In)+1,&Out,OutLen*sizeof(tchar_t));
	if(x==(size_t)-1)
	{
		StrToTcs(Out,In,OutLen);		
	}
#else
	StrToTcs(Out,In,OutLen);
#endif
}

void TcsToAscii(char* Out,const tchar_t* In,int OutLen)
{
#ifdef UNICODE
	size_t x=(int)iconv(g_pIConvWCHAR_TtoASC,&In,(wcslen(In)+1)*sizeof(tchar_t),&Out,OutLen);
	if(x==(size_t)-1)
	{
		TcsToStr(Out,In,OutLen);
	}	
#else
	TcsToStr(Out,In,OutLen);
#endif
}

void UTF8ToTcs(tchar_t* Out,const char* In,int OutLen)
{
#ifdef UNICODE
	size_t x=(int)iconv(g_pIConvUTF8toWCHAR_T,&In,strlen(In)+1,&Out,OutLen*sizeof(tchar_t));
	if(x==(size_t)-1)
	{
		StrToTcs(Out,In,OutLen);		
	}
#else
	size_t inlen=strlen(In+1);
	size_t outlen=(size_t)OutLen;
	size_t x=(int)iconv(g_pIConvUTF8toASC,(const char**)&In,&inlen,(const char**)&Out,&outlen);
	if(x==(size_t)-1)
	{
		StrToTcs(Out,In,OutLen);		
	}
#endif
}

void TcsToUTF8(char* Out,const tchar_t* In,int OutLen)
{
#ifdef UNICODE
	size_t x=(int)iconv(g_pIConvWCHAR_TtoUTF8,&In,(wcslen(In)+1)*sizeof(wchar_t),&Out,OutLen);
	if(x==(size_t)-1)
	{
		TcsToStr(Out,In,OutLen);		
	}
#else
	size_t x=(int)iconv(g_pIConvASCtoUTF8,&In,(strlenlen(In)+1)*sizeof(char),&Out,OutLen);
	if(x==(size_t)-1)
	{
		TcsToStr(Out,In,OutLen);		
	}
#endif
}

void TcsToStrEx(char* Out,const tchar_t* In,int OutLen,int CodePage)
{
#ifdef UNICODE
	if (!WideCharToMultiByte(CodePage,0,In,-1,Out,OutLen,0,0) && OutLen>0)
	{
		OutLen = wcstombs(Out,In,--OutLen);
		if (OutLen<0) OutLen=0;
		Out[OutLen] = 0;
	}
#else
	TcsNCpy(Out,In,OutLen);
#endif
}

void StrToTcsEx(tchar_t* Out,const char* In,int OutLen,int CodePage)
{
#ifdef UNICODE
	CodePage;
	AsciiToTcs(Out,In,OutLen);
#else
	int n = strlen(In);
	strncpy(Out,In,OutLen);
	if (n>=OutLen) 
		Out[OutLen-1] = 0;
#endif
}

void WcsToTcs(tchar_t* Out,const uint16_t* In,int OutLen)
{
#ifdef UNICODE
	TcsNCpy(Out,In,OutLen);
#else
	if (OutLen>0)
	{
		OutLen = wcstombs(Out,In,--OutLen);
		if (OutLen<0) OutLen=0;
		Out[OutLen] = 0;

	}
#endif
}

void TcsUpr(tchar_t* p) 
{
#ifdef UNICODE
	_wcsupr(p);
#else
	_strupr(p);
#endif
}

int TcsICmp(const tchar_t* a,const tchar_t* b) 
{
#ifdef UNICODE
	return _wcsicmp(a,b);
#else
	return stricmp(a,b);
#endif
}

int TcsNICmp(const tchar_t* a,const tchar_t* b,int n) 
{
#ifdef UNICODE
	return _wcsnicmp(a,b,n);
#else
	return strnicmp(a,b,n);
#endif
}
