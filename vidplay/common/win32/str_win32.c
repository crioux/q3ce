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

#define MAXTEXT		20000

#if defined(TARGET_WIN32) || defined(TARGET_WINCE)

#ifndef STRICT
#define STRICT
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void GetModulePath(tchar_t* Path,const tchar_t* Module);
void FindFiles(const tchar_t* Path, const tchar_t* Mask,void(*Process)(const tchar_t*,void*),void* Param);
void StringAlloc();
void StringFree();


void String_Init()
{
	StringAlloc();
}

void String_Done()
{
	StringFree();
}

#ifndef CP_UTF8
#define CP_UTF8 65001
#endif
void AsciiToTcs(tchar_t* Out,const char* In,int OutLen)
{
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP,0,In,-1,Out,OutLen))
		StrToTcs(Out,In,OutLen);
#else
	StrToTcs(Out,In,OutLen);
#endif
}

void TcsToAscii(char* Out,const tchar_t* In,int OutLen)
{
#ifdef UNICODE
	if (!WideCharToMultiByte(CP_ACP,0,In,-1,Out,OutLen,0,0))
		TcsToStr(Out,In,OutLen);
#else
	TcsToStr(Out,In,OutLen);
#endif
}

void UTF8ToTcs(tchar_t* Out,const char* In,int OutLen)
{
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_UTF8,0,In,-1,Out,OutLen))
		StrToTcs(Out,In,OutLen);
#else
	WCHAR Temp[512];
	if (!MultiByteToWideChar(CP_UTF8,0,In,-1,Temp,512) ||
		!WideCharToMultiByte(CP_ACP,0,Temp,-1,Out,OutLen,0,0))
		StrToTcs(Out,In,OutLen);
#endif
}

void TcsToUTF8(char* Out,const tchar_t* In,int OutLen)
{
#ifdef UNICODE
	if (!WideCharToMultiByte(CP_UTF8,0,In,-1,Out,OutLen,0,0))
		TcsToStr(Out,In,OutLen);
#else
	WCHAR Temp[512];
	if (!MultiByteToWideChar(CP_ACP,0,In,-1,Temp,512) ||
		!WideCharToMultiByte(CP_UTF8,0,Temp,-1,Out,OutLen,0,0))
		TcsToStr(Out,In,OutLen);
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
	if (!MultiByteToWideChar(CodePage,MB_PRECOMPOSED,In,-1,Out,OutLen))
	{
		OutLen = mbstowcs(Out,In,--OutLen);
		if (OutLen<0) OutLen=0;
		Out[OutLen] = 0;
	}
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

#endif
