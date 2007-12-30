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
 * $Id: portab.h 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __PORTAB_H
#define __PORTAB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#undef INLINE
#undef NOINLINE
#undef LITTLE_ENDIAN
#undef BIG_ENDIAN

#if defined(__palmos__)

#define TARGET_PALMOS
#define REGISTRY_GLOBAL

#elif defined(_WIN32_WCE)

#define TARGET_WINCE
//#define MULTITHREAD

#elif defined(_WIN32)

#define TARGET_WIN32
//#define MULTITHREAD

#else
#error Unsupported target
#endif

#if defined(ARM) || defined(MIPS) || defined(SH3) || defined(_M_IX86)
#define LITTLE_ENDIAN
#else
#define BIG_ENDIAN
#endif

#define TICKSPERSEC			16384

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define MAX_INT	0x7FFFFFFF

#ifdef _MSC_VER

#if _MSC_VER >= 1400
#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libcd.lib")
#pragma comment(linker, "/nodefaultlib:oldnames.lib")
#endif

typedef signed long int32_t;
typedef unsigned long uint32_t;  
typedef signed short int16_t; 
typedef unsigned short uint16_t; 
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#define NOINLINE 
#define INLINE __forceinline
#define STDCALL __stdcall
#else
#include <stdint.h>
#define INLINE inline
#define NOINLINE __attribute__((noinline))
#if defined(TARGET_WINCE) || defined(TARGET_PALMOS)
#define __stdcall
#define STDCALL 
#else
#define STDCALL __attribute__((stdcall))
#endif
#endif

#ifdef __MMX__
#define MMX
#endif

typedef int bool_t;
typedef int tick_t;
typedef int filepos_t;

typedef struct guid
{
    uint32_t v1;
    uint16_t v2;
    uint16_t v3;
    uint8_t v4[8];
} guid;

typedef struct fraction
{
	int Num;
	int Den;
} fraction;

typedef struct fraction64
{
	int64_t Num;
	int64_t Den;
} fraction64;

#ifndef ZLIB_INTERNAL
#ifdef UNICODE
typedef unsigned short tchar_t;
#define tcsstr wcsstr
#define tcslen wcslen
#define tcscmp wcscmp
#define tcsncmp wcsncmp
#define tcscpy wcscpy
#define tcscat wcscat
#define tcschr wcschr
#define tcsrchr wcsrchr
#define stprintf swprintf
#define vstprintf vswprintf
#define T(a) L ## a
#else
typedef char tchar_t;
#define tcsstr strstr
#define tcslen strlen
#define tcscpy strcpy
#define tcscmp strcmp
#define tcsncmp strncmp
#define tcscat strcat
#define tcschr strchr
#define tcsrchr strrchr
#define stprintf sprintf
#define vstprintf vsprintf
#define T(a) a
#endif
#endif

#if defined(_WIN32)
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLEXPORT
#define DLLIMPORT
#endif

#if !defined(SH3) && !defined(MIPS)
#define _INLINE INLINE
#define _CONST const
#else
#define _INLINE
#define _CONST
#endif

#define MAXPATH 256

#define MAXPLANES 4
typedef void* planes[MAXPLANES];
typedef const void* constplanes[MAXPLANES];

#define FOURCCBE(a,b,c,d) \
	(((unsigned char)a << 24) | ((unsigned char)b << 16) | \
	((unsigned char)c << 8) | ((unsigned char)d << 0))

#define FOURCCLE(a,b,c,d) \
	(((unsigned char)a << 0) | ((unsigned char)b << 8) | \
	((unsigned char)c << 16) | ((unsigned char)d << 24))

#ifdef BIG_ENDIAN
#define FOURCC(a,b,c,d) FOURCCBE(a,b,c,d)
#else
#define FOURCC(a,b,c,d) FOURCCLE(a,b,c,d)
#endif

#ifndef min
#  define min(x,y)  ((x)>(y)?(y):(x))
#endif

#ifndef max
#  define max(x,y)  ((x)<(y)?(y):(x))
#endif

#ifndef sign
#  define sign(x) ((x)<0?-1:1)
#endif

#if defined(__GNUC__)
#define alloca(size) __builtin_alloca(size)
#if defined(TARGET_PALMOS)
extern int rand();
extern void qsort(void* const base,size_t,size_t,int(*cmp)(const void*,const void*));

#if defined(ARM)
#define SWAPSP
static INLINE void* SwapSP(void* in)
{
	void* out;
	asm volatile(
		"mov %0, sp\n\t"
		"mov sp, %1\n\t"
		: "=&r"(out) : "r"(in) : "cc");
	return out;
}
#endif //ARM
#endif //TARGET_PALMOS
#endif //__GNUC__

#ifdef _MSC_VER
#define LIBGCC	\
//int64_t __divdi3(int64_t a,int64_t b) { return a/b; } \
//int64_t __moddi3(int64_t a,int64_t b) { return a%b; } \
//int32_t __divsi3(int32_t a,int32_t b) { return a/b; } \
//int32_t __modsi3(int32_t a,int32_t b) { return a%b; } \
//uint32_t __udivsi3(uint32_t a,uint32_t b) { return a/b; } \
//uint32_t __umodsi3(uint32_t a,uint32_t b) { return a%b; }

#define LIBGCCFLOAT	\
int __fixdfsi(double i) { return (int)i; } \
int64_t __fixdfdi(double i) { return (int64_t)i; } \
int __eqdf2(double a,double b) { return !(a==b); } \
int __nedf2(double a,double b) { return a != b; } \
int __gtdf2(double a,double b) { return a > b; } \
int __gedf2(double a,double b) { return (a>=b)-1; } \
int __ltdf2(double a,double b) { return -(a<b); } \
int __ledf2(double a,double b) { return 1-(a<=b); } \
double __floatsidf(int i) { return (double)i; } \
double __extendsfdf2(float f) { return f; } \
double __negdf2(double a) { return -a; } \
double __divdf3(double a,double b) { return a/b; } \
double __muldf3(double a,double b) { return a*b; } \
double __adddf3(double a,double b) { return a+b; } \
double __subdf3(double a,double b) { return a-b; }
#else
#define LIBGCC
#define LIBGCCFLOAT
#endif

#endif
