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
 * $Id: str.h 202 2005-01-25 01:27:33Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __STR_H
#define __STR_H

#define LANG_ID			FOURCC('L','A','N','G')

#define LANG_EN			FOURCC('E','N','_','_')
#define LANG_DEFAULT	LANG_EN

DLL void String_Init();
DLL void String_Done();

DLL int StringEnum(int Class, int No);
DLL const tchar_t* String(int Class, int Id);
DLL const tchar_t* StringDef(int Class, int Id); // in English

DLL void StringAdd(bool_t Default, int Class,int No,const tchar_t*);
DLL void StringAddPrint(bool_t Default, int Class,int No,const tchar_t*, ...);

DLL void StringAddText(const char* Data,int Length);
DLL bool_t StringAddBinary(const void* Data, int Length);
DLL bool_t StringIsBinary(int Class, int Id);

DLL bool_t IsSpace(int);
DLL bool_t IsAlpha(int);
DLL bool_t IsDigit(int);
DLL int Hex(int ch);

DLL void StrToTcs(tchar_t* Out,const char* In,int OutLen);
DLL void StrToTcsEx(tchar_t* Out,const char* In,int OutLen,int CodePage);
DLL void WcsToTcs(tchar_t* Out,const uint16_t* In,int OutLen);
DLL void UTF8ToTcs(tchar_t* Out,const char* In,int OutLen);
DLL void AsciiToTcs(tchar_t* Out,const char* In,int OutLen);
DLL void TcsToAscii(char* Out,const tchar_t* In,int OutLen);
DLL void TcsToUTF8(char* Out,const tchar_t* In,int OutLen);
DLL void TcsToStr(char* Out,const tchar_t* In,int OutLen);
DLL void TcsToStrEx(char* Out,const tchar_t* In,int OutLen,int CodePage);
DLL void TcsNCpy(tchar_t* Out,const tchar_t* In,int OutLen);
DLL int TcsICmp(const tchar_t* a,const tchar_t* b);
DLL int TcsNICmp(const tchar_t* a,const tchar_t* b,int n);
DLL void TcsUpr(tchar_t* a);

DLL int Scanf(const tchar_t* In, const tchar_t* Mask, ...);

DLL void FourCCToString(tchar_t* Out, int FourCC, int OutSize);
DLL int StringToFourCC(const tchar_t* In, bool_t Upper);
DLL void FractionToString(tchar_t* Out, const fraction*, int OutSize, bool_t Percent, int Decimal);
DLL int StringToInt(const tchar_t* In, bool_t Hex);
DLL void IntToString(tchar_t* Out, int, int OutSize, bool_t Hex);
DLL void TickToString(tchar_t* Out, tick_t, int OutSize, bool_t MS, bool_t Extended, bool_t Fix);
DLL void HotKeyToString(tchar_t* Out, int HotKey, int OutSize);
DLL void BoolToString(tchar_t* Out, bool_t Bool, int OutSize);
DLL void GUIDToString(tchar_t* Out, const guid*, int OutSize);
DLL bool_t StringToGUID(const tchar_t* In, guid*);

#endif

