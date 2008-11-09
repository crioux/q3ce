/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#ifndef __Q_SHARED_H
#define __Q_SHARED_H

#ifdef _WIN32
#include<windows.h>
#else
#include"../qcommon/unixdefs.h"
#endif

#include"../qcommon/fixed.h"

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file

#define	Q3_VERSION		"Q3 1.32b"
// 1.32 released 7-10-2002

#define MAX_TEAMNAME 32

#ifdef _WIN32

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable : 4152)		// nonstandard extension, function/data pointer conversion in expression
//#pragma warning(disable : 4201)
//#pragma warning(disable : 4214)
#pragma warning(disable : 4244)
#pragma warning(disable : 4142)		// benign redefinition
//#pragma warning(disable : 4305)		// truncation from const double to gfixed
//#pragma warning(disable : 4310)		// cast truncates constant value
//#pragma warning(disable:  4505) 	// unreferenced local function has been removed
#pragma warning(disable : 4514)
#pragma warning(disable : 4702)		// unreachable code
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4220)		// varargs matches remaining parameters
#endif

/**********************************************************************
  VM Considerations

  The VM can not use the standard system headers because we aren't really
  using the compiler they were meant for.  We use bg_lib.h which contains
  prototypes for the functions we define for our own use in bg_lib.c.

  When writing mods, please add needed headers HERE, do not start including
  stuff like <stdio.h> in the various .c files that make up each of the VMs
  since you will be including system headers files can will have issues.

  Remember, if you use a C library function that is not defined in bg_lib.c,
  you will have to add your own version for support in the VM.

 **********************************************************************/

#ifdef Q3_VM

#include "bg_lib.h"

#else

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

#endif


// this is the define for determining if we have an asm version of a C function
#if (defined _M_IX86 || defined __i386__ || defined _i386_) && !defined __sun__  && !defined __LCC__
#define id386	1
#else
#define id386	0
#endif

#if (defined ARM)
#define idARM	1
#else
#define idARM	0
#endif

// for windows fastcall option

#define	QDECL

short   ShortSwap (short l);
int		LongSwap (int l);
float FloatSwap (const float *f);


//======================= WINCE DEFINES =================================

#ifdef _WIN32_WCE

#define	MAC_STATIC

#undef QDECL
#define	QDECL	__cdecl

// buildstring will be incorporated into the version string
#ifdef _DEBUG
#ifdef _M_IX86
#define	CPUSTRING	"wce-x86-debug"
#elif defined ARM
#define	CPUSTRING	"wce-ARM-debug"
#endif
#else
#ifdef _M_IX86
#define	CPUSTRING	"wce-x86"
#elif defined ARM
#define	CPUSTRING	"wce-ARM"
#endif
#endif

#define ID_INLINE __inline 

ID_INLINE short BigShort(short l) { return ShortSwap(l); }
ID_INLINE short LittleShort(short l) { return l; }
ID_INLINE int BigLong(int l) { return LongSwap(l); }
ID_INLINE int LittleLong(int l) { return l; }
ID_INLINE float BigFloat(float f) { return FloatSwap(&f); }
ID_INLINE float LittleFloat(float f) { return f; }
//ID_INLINE gfixed BigFixed(gfixed f) { return INT_AS_FIXED(LongSwap(GFIXED_AS_INT(f))); }
ID_INLINE gfixed LittleFixed(gfixed f) { return f; }
//ID_INLINE afixed BigAFixed(afixed f) { return INT_AS_FIXED(LongSwap(AFIXED_AS_INT(f))); }
//ID_INLINE bfixed BigBFixed(bfixed f) { return INT_AS_FIXED(LongSwap(BFIXED_AS_INT(f))); }
ID_INLINE afixed LittleAFixed(afixed f) { return f; }
ID_INLINE bfixed LittleBFixed(bfixed f) { return f; }

#ifdef FIXED_IS_FLOAT
ID_INLINE gfixed LittleFixed_IS_FLOAT(gfixed f) { return LittleFixed(f); }
ID_INLINE afixed LittleFixed_IS_FLOAT_A(afixed f) { return LittleAFixed(f); }
ID_INLINE bfixed LittleFixed_IS_FLOAT_B(bfixed f) { return LittleBFixed(f); }
#else
ID_INLINE gfixed LittleFixed_IS_FLOAT(gfixed f) { return gfixed::Construct(LittleFloat(*(float *)&f)); }
ID_INLINE afixed LittleFixed_IS_FLOAT_A(afixed f) { return afixed::Construct(LittleFloat(*(float *)&f)); }
ID_INLINE bfixed LittleFixed_IS_FLOAT_B(bfixed f) { return bfixed::Construct(LittleFloat(*(float *)&f)); }
#endif

#define	PATH_SEP '\\'

#endif

//======================= LINUX DEFINES =================================

#ifdef _LINUX

#define	MAC_STATIC

#undef QDECL
#define	QDECL

// buildstring will be incorporated into the version string
#ifdef _DEBUG
#ifdef _M_IX86
#define	CPUSTRING	"linux-x86-debug"
#elif defined ARM
#define	CPUSTRING	"linux-ARM-debug"
#endif
#else
#ifdef _M_IX86
#define	CPUSTRING	"linux-x86"
#elif defined ARM
#define	CPUSTRING	"linux-ARM"
#endif
#endif

#define ID_INLINE __inline 

ID_INLINE short BigShort(short l) { return ShortSwap(l); }
ID_INLINE short LittleShort(short l) { return l; }
ID_INLINE int BigLong(int l) { return LongSwap(l); }
ID_INLINE int LittleLong(int l) { return l; }
ID_INLINE float BigFloat(float f) { return FloatSwap(&f); }
ID_INLINE float LittleFloat(float f) { return f; }
//ID_INLINE gfixed BigFixed(gfixed f) { return INT_AS_FIXED(LongSwap(GFIXED_AS_INT(f))); }
ID_INLINE gfixed LittleFixed(gfixed f) { return f; }
//ID_INLINE afixed BigAFixed(afixed f) { return INT_AS_FIXED(LongSwap(AFIXED_AS_INT(f))); }
//ID_INLINE bfixed BigBFixed(bfixed f) { return INT_AS_FIXED(LongSwap(BFIXED_AS_INT(f))); }
ID_INLINE afixed LittleAFixed(afixed f) { return f; }
ID_INLINE bfixed LittleBFixed(bfixed f) { return f; }

#ifdef FIXED_IS_FLOAT
ID_INLINE gfixed LittleFixed_IS_FLOAT(gfixed f) { return LittleFixed(f); }
ID_INLINE afixed LittleFixed_IS_FLOAT_A(afixed f) { return LittleAFixed(f); }
ID_INLINE bfixed LittleFixed_IS_FLOAT_B(bfixed f) { return LittleBFixed(f); }
#else
ID_INLINE gfixed LittleFixed_IS_FLOAT(gfixed f) { return gfixed::Construct(LittleFloat(*(float *)&f)); }
ID_INLINE afixed LittleFixed_IS_FLOAT_A(afixed f) { return afixed::Construct(LittleFloat(*(float *)&f)); }
ID_INLINE bfixed LittleFixed_IS_FLOAT_B(bfixed f) { return bfixed::Construct(LittleFloat(*(float *)&f)); }
#endif

#define	PATH_SEP '/'

#endif


//=============================================================

typedef unsigned char 		byte;

typedef int qboolean;

#define qfalse (0)
#define qtrue (1)

typedef int		qhandle_t;
typedef int		sfxHandle_t;
typedef int		fileHandle_t;
typedef int		clipHandle_t;


#ifndef NULL
#define NULL ((void *)0)
#endif

#define	MAX_QINT			0x7fffffff
#define	MIN_QINT			(-MAX_QINT-1)


// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		  1024
#define	MAX_INFO_VALUE		1024

#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		  8192
#define	BIG_INFO_VALUE		8192


#define	MAX_QPATH			64		// max length of a quake game pathname
#ifdef PATH_MAX
#define MAX_OSPATH			PATH_MAX
#else
#define	MAX_OSPATH			256		// max length of a filesystem pathname
#endif

#define	MAX_NAME_LENGTH		32		// max length of a client name

#define	MAX_SAY_TEXT	150

// paramters for command buffer stuffing
typedef enum {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
						// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
} cbufExec_t;


//
// these aren't needed by any of the VMs.  put in another header?
//
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility


// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum {
	PRINT_ALL,
	PRINT_DEVELOPER,		// only print when "developer 1"
	PRINT_WARNING,
	PRINT_ERROR
} printParm_t;


#ifdef ERR_FATAL
#undef ERR_FATAL			// this is be defined in malloc.h
#endif

// parameters to the main Error routine
typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
	ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;


// font rendering values used by ui and cgame

#define PROP_GAP_WIDTH			3
#define PROP_SPACE_WIDTH		8
#define PROP_HEIGHT				28
#define PROP_SMALL_SIZE_SCALE	GFIXED(0,75)

#define BLINK_DIVISOR			200
#define PULSE_DIVISOR			75

#define UI_LEFT			0x00000000	// default
#define UI_CENTER		0x00000001
#define UI_RIGHT		0x00000002
#define UI_FORMATMASK	0x00000007
#define UI_SMALLFONT	0x00000010
#define UI_BIGFONT		0x00000020	// default
#define UI_GIANTFONT	0x00000040
#define UI_DROPSHADOW	0x00000800
#define UI_BLINK		0x00001000
#define UI_INVERSE		0x00002000
#define UI_PULSE		0x00004000

#if defined(_DEBUG) && !defined(BSPC)
	#define HUNK_DEBUG
#endif

typedef enum {
	h_high,
	h_low,
	h_dontcare
} ha_pref;

#ifdef HUNK_DEBUG
#define Hunk_Alloc( size, preference )				Hunk_AllocDebug(size, preference, #size, __FILE__, __LINE__)
void *Hunk_AllocDebug( int size, ha_pref preference, const char *label, const char *file, int line );
#else
void *Hunk_Alloc( int size, ha_pref preference );
#endif

#ifdef __linux__
// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=371
// custom Snd_Memset implementation for glibc memset bug workaround
void Snd_Memset (void* dest, const int val, const size_t count);
#else
#define Snd_Memset Com_Memset
#endif

#ifdef __cplusplus
extern "C" {
#endif

void Com_Memset (void* dest, const int val, const size_t count);
void Com_Memcpy (void* dest, const void* src, const size_t count);

#ifdef __cplusplus
}
#endif

#define CIN_system	1
#define CIN_loop	2
#define	CIN_hold	4
#define CIN_silent	8
#define CIN_shader	16

/*
==============================================================

MATHLIB

==============================================================
*/

#include "../splines/math_angles.h"
#include "../splines/math_quaternion.h"
#include "../splines/math_matrix.h"
#include "../splines/math_vector.h"

typedef gfixed vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef afixed avec_t;
typedef avec_t avec2_t[2];
typedef avec_t avec3_t[3];
typedef avec_t avec4_t[4];
typedef avec_t avec5_t[5];

typedef bfixed bvec_t;
typedef bvec_t bvec2_t[2];
typedef bvec_t bvec3_t[3];
typedef bvec_t bvec4_t[4];
typedef bvec_t bvec5_t[5];

#define NUMVERTEXNORMALS	162
extern	avec3_t	bytedirs[NUMVERTEXNORMALS];

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		480

#define TINYCHAR_WIDTH		8
#define TINYCHAR_HEIGHT		10	

#define SMALLCHAR_WIDTH		12
#define SMALLCHAR_HEIGHT	20

#define BIGCHAR_WIDTH		14
#define BIGCHAR_HEIGHT		22

#define	GIANTCHAR_WIDTH		32
#define	GIANTCHAR_HEIGHT	48

extern	vec4_t		colorBlack;
extern	vec4_t		colorRed;
extern	vec4_t		colorGreen;
extern	vec4_t		colorBlue;
extern	vec4_t		colorYellow;
extern	vec4_t		colorMagenta;
extern	vec4_t		colorCyan;
extern	vec4_t		colorWhite;
extern	vec4_t		colorLtGrey;
extern	vec4_t		colorMdGrey;
extern	vec4_t		colorDkGrey;

#define Q_COLOR_ESCAPE	'^'
#define Q_IsColorString(p)	( p && *(p) == Q_COLOR_ESCAPE && *((p)+1) && *((p)+1) != Q_COLOR_ESCAPE )

#define COLOR_BLACK		'0'
#define COLOR_RED		'1'
#define COLOR_GREEN		'2'
#define COLOR_YELLOW	'3'
#define COLOR_BLUE		'4'
#define COLOR_CYAN		'5'
#define COLOR_MAGENTA	'6'
#define COLOR_WHITE		'7'
#define ColorIndex(c)	( ( (c) - '0' ) & 7 )

#define S_COLOR_BLACK	"^0"
#define S_COLOR_RED		"^1"
#define S_COLOR_GREEN	"^2"
#define S_COLOR_YELLOW	"^3"
#define S_COLOR_BLUE	"^4"
#define S_COLOR_CYAN	"^5"
#define S_COLOR_MAGENTA	"^6"
#define S_COLOR_WHITE	"^7"

extern vec4_t	g_color_table[8];

#define	MAKERGB( v, r, g, b ) v[0]=r;v[1]=g;v[2]=b
#define	MAKERGBA( v, r, g, b, a ) v[0]=r;v[1]=g;v[2]=b;v[3]=a

#define DEG2RAD_A( a ) ( ( (a) * AFIXED_PI ) / AFIXED(180,0) )
#define RAD2DEG_A( a ) ( ( (a) * AFIXED(180,0) ) / AFIXED_PI )

#define DEG2RAD_B( a ) ( ( (a) * BFIXED_PI ) / BFIXED(180,0) )
#define RAD2DEG_B( a ) ( ( (a) * BFIXED(180,0) ) / BFIXED_PI )

struct cplane_s;

extern	vec3_t	vec3_origin;
extern	vec3_t	axisDefault[3];

extern	avec3_t	avec3_origin;
extern	avec3_t	aaxisDefault[3];

extern	bvec3_t	bvec3_origin;
extern	bvec3_t	baxisDefault[3];


#define	nanmask (255<<23)

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

int Q_log2(int val);

#define SQRTFAST( x ) ( (x) * FIXED_FASTINVSQRT( x ) )

signed char ClampChar( int i );
signed short ClampShort( int i );

// this isn't a real cheap function to call!
int DirToByte( bvec3_t dir );
#ifndef FIXED_IS_FLOAT
int DirToByte( avec3_t dir );
#endif

void ByteToDir( int b, avec3_t dir );





#ifdef FIXED_IS_FLOAT
inline afixed MSECTIME_A(int time)
{
	return ((float)(time))/1000.0f;
}
inline bfixed MSECTIME_B(int time)
{
	return ((float)(time))/1000.0f;
}
inline cfixed MSECTIME_C(int time)
{
	return ((float)(time))/1000.0f;
}
inline dfixed MSECTIME_D(int time)
{
	return ((float)(time))/1000.0f;
}
inline gfixed MSECTIME_G(int time)
{
	return ((float)(time))/1000.0f;
}
inline lfixed MSECTIME_L(int time)
{
	return ((double)(time))/1000.0;
}
#else
inline afixed MSECTIME_A(int time)
{
	return afixed::FromRep((afixed::basetype)(((INT64)time)*(((INT64)1)<<afixed::prec))/((INT64)1000));
}
inline bfixed MSECTIME_B(int time)
{
	return bfixed::FromRep((bfixed::basetype)(((INT64)time)*(((INT64)1)<<bfixed::prec))/((INT64)1000));
}
inline cfixed MSECTIME_C(int time)
{
	return cfixed::FromRep((cfixed::basetype)(((INT64)time)*(((INT64)1)<<cfixed::prec))/((INT64)1000));
}
inline dfixed MSECTIME_D(int time)
{
	return dfixed::FromRep((dfixed::basetype)(((INT64)time)*(((INT64)1)<<dfixed::prec))/((INT64)1000));
}
inline gfixed MSECTIME_G(int time)
{
	return gfixed::FromRep((gfixed::basetype)(((INT64)time)*(((INT64)1)<<gfixed::prec))/((INT64)1000));
}
inline lfixed MSECTIME_L(int time)
{
	return lfixed::FromRep((lfixed::basetype)(((INT64)time)*(((INT64)1)<<lfixed::prec))/((INT64)1000));
}

#endif



 
inline void VectorSubtract(const gfixed *a,const gfixed *b,gfixed *c)
{
	c[0]=a[0]-b[0];
	c[1]=a[1]-b[1];
	c[2]=a[2]-b[2];
}
inline void VectorAdd(const gfixed *a,const gfixed *b,gfixed *c)
{
	c[0]=a[0]+b[0];
	c[1]=a[1]+b[1];
	c[2]=a[2]+b[2];
}
inline void VectorCopy(const gfixed *a, gfixed *b)
{
	b[0]=a[0];
	b[1]=a[1];
	b[2]=a[2];
}
inline void VectorSet(gfixed *v, gfixed x, gfixed y, gfixed z)
{
	v[0]=x;
	v[1]=y;
	v[2]=z;
}

inline void VectorClear(gfixed *v)
{
	v[0]=v[1]=v[2]=GFIXED_0;
}

inline void SnapVector(gfixed *v)
{
	v[0]=FIXED_SNAP(v[0]);
	v[1]=FIXED_SNAP(v[1]);
	v[2]=FIXED_SNAP(v[2]);
}

inline void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

gfixed NormalizeColor( const vec3_t in, vec3_t out );

gfixed RadiusFromBounds( const vec3_t mins, const vec3_t maxs );
void ClearBounds( vec3_t mins, vec3_t maxs );
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );


inline int VectorCompare( const vec3_t v1, const vec3_t v2 ) {
	if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) {
		return 0;
	}			
	return 1;
}

inline vec_t Distance( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return FIXED_VEC3LEN( v );
}

inline vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return FIXED_VEC3LEN_SQ(v);
}

inline void VectorInverse( vec3_t v ){
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out );
void VectorRotate( bvec3_t in, bvec3_t matrix[3], bvec3_t out );


gfixed Q_acos(gfixed c);

inline void VectorCopyG2B(const gfixed *a, bfixed *b)
{
	b[0]=MAKE_BFIXED(a[0]);
	b[1]=MAKE_BFIXED(a[1]);
	b[2]=MAKE_BFIXED(a[2]);
}
inline void VectorCopyB2G(const bfixed *a, gfixed *b)
{
	b[0]=MAKE_GFIXED(a[0]);
	b[1]=MAKE_GFIXED(a[1]);
	b[2]=MAKE_GFIXED(a[2]);
}
inline void VectorCopyB2A(const bfixed *a, afixed *b)
{
	b[0]=MAKE_AFIXED(a[0]);
	b[1]=MAKE_AFIXED(a[1]);
	b[2]=MAKE_AFIXED(a[2]);
}
inline void VectorCopyA2B(const afixed *a, bfixed *b)
{
	b[0]=MAKE_BFIXED(a[0]);
	b[1]=MAKE_BFIXED(a[1]);
	b[2]=MAKE_BFIXED(a[2]);
}


bfixed VectorNormalizeB2A(const bvec3_t bv, avec3_t av);
afixed VectorNormalizeA2B(const avec3_t av, bvec3_t bv);

lfixed VectorNormalizeLong2( const vec3_t v, vec3_t out);
#ifndef FIXED_IS_FLOAT
lfixed VectorNormalizeLong2( const bvec3_t v, bvec3_t out);
lfixed VectorNormalizeLong2( const avec3_t v, avec3_t out);
#endif
inline gfixed VectorNormalize2( const vec3_t v, vec3_t out) { return MAKE_GFIXED(VectorNormalizeLong2(v,out)); }
#ifndef FIXED_IS_FLOAT
inline bfixed VectorNormalize2( const bvec3_t v, bvec3_t out) { return MAKE_BFIXED(VectorNormalizeLong2(v,out)); }
inline afixed VectorNormalize2( const avec3_t v, avec3_t out) { return MAKE_AFIXED(VectorNormalizeLong2(v,out)); }
#endif
inline gfixed VectorNormalize( vec3_t v ) { return VectorNormalize2(v,v); }
#ifndef FIXED_IS_FLOAT
inline bfixed VectorNormalize( bvec3_t v ) { return VectorNormalize2(v,v); }
inline afixed VectorNormalize( avec3_t v ) { return VectorNormalize2(v,v); }
#endif
inline lfixed VectorNormalizeLong( vec3_t v ) { return VectorNormalizeLong2(v,v); }
#ifndef FIXED_IS_FLOAT
inline lfixed VectorNormalizeLong( bvec3_t v ) { return VectorNormalizeLong2(v,v); }
inline lfixed VectorNormalizeLong( avec3_t v ) { return VectorNormalizeLong2(v,v); }
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef FIXED_IS_FLOAT



extern avec3_t avec3_origin;
extern avec3_t aaxisDefault[3];

afixed Q_acos(afixed c);

inline void VectorSubtract(const afixed *a,const afixed *b,afixed *c)
{
	c[0]=a[0]-b[0];
	c[1]=a[1]-b[1];
	c[2]=a[2]-b[2];
}
inline void VectorAdd(const afixed *a,const afixed *b,afixed *c)
{
	c[0]=a[0]+b[0];
	c[1]=a[1]+b[1];
	c[2]=a[2]+b[2];
}
inline void VectorCopy(const afixed *a, afixed *b)
{
	b[0]=a[0];
	b[1]=a[1];
	b[2]=a[2];
}				
inline void VectorSet(afixed *v, afixed x, afixed y, afixed z)
{
	v[0]=x;
	v[1]=y;
	v[2]=z;
}

inline void VectorClear(afixed *v)
{
	v[0]=v[1]=v[2]=AFIXED_0;
}
inline void SnapVector(afixed *v)
{
	v[0]=FIXED_SNAP(v[0]);
	v[1]=FIXED_SNAP(v[1]);
	v[2]=FIXED_SNAP(v[2]);
}

inline void CrossProduct( const avec3_t v1, const avec3_t v2, avec3_t cross ) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

inline void CrossProduct( const bvec3_t v1, const avec3_t v2, bvec3_t cross ) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

inline void CrossProduct( const avec3_t v1, const bvec3_t v2, bvec3_t cross ) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}


afixed RadiusFromBounds( const avec3_t mins, const avec3_t maxs );
void ClearBounds( avec3_t mins, avec3_t maxs );
void AddPointToBounds( const avec3_t v, avec3_t mins, avec3_t maxs );

inline int VectorCompare( const avec3_t v1, const avec3_t v2 ) {
	if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) {
		return 0;
	}			
	return 1;
}

inline avec_t Distance( const avec3_t p1, const avec3_t p2 ) {
	avec3_t	v;

	VectorSubtract (p2, p1, v);
	return FIXED_VEC3LEN( v );
}

inline avec_t DistanceSquared( const avec3_t p1, const avec3_t p2 ) {
	avec3_t	v;

	VectorSubtract (p2, p1, v);
	return FIXED_VEC3LEN_SQ(v);
}

inline void VectorInverse( avec3_t v ){
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}


void Vector4Scale( const avec4_t in, avec_t scale, avec4_t out );


extern bvec3_t bvec3_origin;
extern bvec3_t baxisDefault[3];

bfixed Q_acos(bfixed c);

inline void VectorSubtract(const bfixed *a,const bfixed *b,bfixed *c)
{
	c[0]=a[0]-b[0];
	c[1]=a[1]-b[1];
	c[2]=a[2]-b[2];
}
inline void VectorAdd(const bfixed *a,const bfixed *b,bfixed *c)
{
	c[0]=a[0]+b[0];
	c[1]=a[1]+b[1];
	c[2]=a[2]+b[2];
}
inline void VectorCopy(const bfixed *a, bfixed *b)
{
	b[0]=a[0];
	b[1]=a[1];
	b[2]=a[2];
}				
inline void VectorSet(bfixed *v, bfixed x, bfixed y, bfixed z)
{
	v[0]=x;
	v[1]=y;
	v[2]=z;
}

inline void VectorClear(bfixed *v)
{
	v[0]=v[1]=v[2]=BFIXED_0;
}
inline void SnapVector(bfixed *v)
{
	v[0]=FIXED_SNAP(v[0]);
	v[1]=FIXED_SNAP(v[1]);
	v[2]=FIXED_SNAP(v[2]);
}

inline void CrossProduct( const bvec3_t v1, const bvec3_t v2, bvec3_t cross ) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

bfixed RadiusFromBounds( const bvec3_t mins, const bvec3_t maxs );
void ClearBounds( bvec3_t mins, bvec3_t maxs );
void AddPointToBounds( const bvec3_t v, bvec3_t mins, bvec3_t maxs );

inline int VectorCompare( const bvec3_t v1, const bvec3_t v2 ) {
	if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) {
		return 0;
	}			
	return 1;
}

inline bvec_t Distance( const bvec3_t p1, const bvec3_t p2 ) {
	bvec3_t	v;

	VectorSubtract (p2, p1, v);
	return FIXED_VEC3LEN( v );
}

inline bvec_t DistanceSquared( const bvec3_t p1, const bvec3_t p2 ) {
	bvec3_t	v;

	VectorSubtract (p2, p1, v);
	return FIXED_VEC3LEN_SQ(v);
}

inline void VectorInverse( bvec3_t v ){
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void Vector4Scale( const bvec4_t in, bvec_t scale, bvec4_t out );
void VectorRotate( bvec3_t in, avec3_t matrix[3], bvec3_t out );
void VectorRotate( avec3_t in, avec3_t matrix[3], avec3_t out );


#endif



inline lfixed DotProductLong(const bfixed *x,const bfixed *y)
{
	lfixed lvecx[3],lvecy[3];
	lvecx[0]=MAKE_LFIXED(x[0]);
	lvecx[1]=MAKE_LFIXED(x[1]);
	lvecx[2]=MAKE_LFIXED(x[2]);
	lvecy[0]=MAKE_LFIXED(y[0]);
	lvecy[1]=MAKE_LFIXED(y[1]);
	lvecy[2]=MAKE_LFIXED(y[2]);
	return FIXED_VEC3DOT(lvecx,lvecy);
}

inline lfixed DistanceLong( const bvec3_t p1, const bvec3_t p2 ) {
	bvec3_t	v;
	VectorSubtract (p2, p1, v);

	lfixed lvec[3];
	lvec[0]=MAKE_LFIXED(v[0]); lvec[1]=MAKE_LFIXED(v[1]); lvec[2]=MAKE_LFIXED(v[2]);

	return FIXED_VEC3LEN( lvec );
}

inline lfixed DistanceSquaredLong( const bvec3_t p1, const bvec3_t p2 ) {
	bvec3_t	v;
	VectorSubtract (p2, p1, v);

	lfixed lvec[3];
	lvec[0]=MAKE_LFIXED(v[0]); lvec[1]=MAKE_LFIXED(v[1]); lvec[2]=MAKE_LFIXED(v[2]);

	return FIXED_VEC3LEN_SQ(lvec);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned ColorBytes3 (gfixed r, gfixed g, gfixed b);
unsigned ColorBytes4 (gfixed r, gfixed g, gfixed b, gfixed a);





int		Q_rand( int *seed );
gfixed	Q_random( int *seed );
gfixed	Q_crandom( int *seed );
afixed	Q_arandom( int *seed );
afixed	Q_acrandom( int *seed );
bfixed	Q_brandom( int *seed );
bfixed	Q_bcrandom( int *seed );


#define random()	(GFIXED_RANDOM())
#define crandom()	(GFIXED(2,0) * (random() - GFIXED(0,5)))

#define random_a()	(AFIXED_RANDOM())
#define crandom_a()	(AFIXED(2,0) * (random_a() - AFIXED(0,5)))

#define random_b()	(BFIXED_RANDOM())
#define crandom_b()	(BFIXED(2,0) * (random_b() - BFIXED(0,5)))

void vectoangles( const bvec3_t value1, avec3_t angles);
#ifndef FIXED_IS_FLOAT
void vectoangles( const avec3_t value1, avec3_t angles);
#endif
void AnglesToAxis( const avec3_t angles, avec3_t axis[3] );

void AxisClear( avec3_t axis[3] );
void AxisCopy( avec3_t in[3], avec3_t out[3] );

void SetPlaneSignbits( struct cplane_s *out );
int BoxOnPlaneSide (bvec3_t emins, bvec3_t emaxs, struct cplane_s *plane);



inline afixed	AngleMod(afixed a) { return FIXED_MOD_CLAMP(a,AFIXED(360,0)); }
inline afixed	AngleModB2A(bfixed a) { return MAKE_AFIXED(FIXED_MOD_CLAMP(a,BFIXED(360,0))); }
inline bfixed	AngleModB(bfixed a) { return FIXED_MOD_CLAMP(a,BFIXED(360,0)); }

afixed	LerpAngle (afixed from, afixed to, afixed frac);
afixed	AngleSubtract( afixed a1, afixed a2 );
void	AnglesSubtract( avec3_t v1, avec3_t v2, avec3_t v3 );

afixed AngleNormalize360 ( afixed angle );
afixed AngleNormalize180 ( afixed angle );
afixed AngleDelta ( afixed angle1, afixed angle2 );

qboolean PlaneFromPoints( avec3_t planenormal, bfixed &planedist, const bvec3_t a, const bvec3_t b, const bvec3_t c );
void ProjectPointOnPlane( bvec3_t dst, const bvec3_t p, const avec3_t normal );
void RotatePointAroundVector( bvec3_t dst, const avec3_t dir, const bvec3_t point, afixed degrees );
void MakeNormalVectors( const bvec3_t forward, bvec3_t right, bvec3_t up );
#ifndef FIXED_IS_FLOAT
void ProjectPointOnPlane( avec3_t dst, const avec3_t p, const avec3_t normal );
void RotatePointAroundVector( bvec3_t dst, const avec3_t dir, const bvec3_t point, afixed degrees );
void RotatePointAroundVector( avec3_t dst, const avec3_t dir, const avec3_t point, afixed degrees );
void MakeNormalVectors( const avec3_t forward, avec3_t right, avec3_t up );
#endif
void RotateAroundDirection( avec3_t axis[3], afixed yaw );

// perpendicular vector could be replaced by this

//int	PlaneTypeForNormal (bvec3_t normal);
#ifdef __cplusplus

typedef bfixed bmat_33[3][3];
typedef afixed amat_33[3][3];
void MatrixMultiply(bmat_33 &in1, bmat_33 &in2, bmat_33 &out);
void AngleVectors( const avec3_t angles, avec3_t forward, avec3_t right, avec3_t up);
void AngleVectors( const avec3_t angles, bvec3_t forward, bvec3_t right, bvec3_t up);
void PerpendicularVector( bvec3_t dst, const bvec3_t src );
#ifndef FIXED_IS_FLOAT
void MatrixMultiply(amat_33 &in1, amat_33 &in2, amat_33 &out);
void MatrixMultiply(bmat_33 &in1, amat_33 &in2, bmat_33 &out);
void PerpendicularVector( avec3_t dst, const avec3_t src );
#endif

#endif


//=============================================

gfixed Com_Clamp( gfixed min, gfixed max, gfixed value );
#ifndef FIXED_IS_FLOAT
afixed Com_Clamp( afixed min, afixed max, afixed value );
bfixed Com_Clamp( bfixed min, bfixed max, bfixed value );
#endif

char	*COM_SkipPath( char *pathname );
void	COM_StripExtension( const char *in, char *out );
void	COM_DefaultExtension( char *path, int maxSize, const char *extension );

void	COM_BeginParseSession( const char *name );
int		COM_GetCurrentParseLine( void );
const char	*COM_Parse( const char **data_p );
const char	*COM_ParseExt( const char * *data_p, qboolean allowLineBreak );
int		COM_Compress( char *data_p );
void	COM_ParseError( const char *format, ... );
void	COM_ParseWarning( const char *format, ... );
//int		COM_ParseInfos( const char *buf, int max, char infos[][MAX_INFO_STRING] );

#define MAX_TOKENLENGTH		1024

#ifndef TT_STRING
//token types
#define TT_STRING					1			// string
#define TT_LITERAL					2			// literal
#define TT_NUMBER					3			// number
#define TT_NAME						4			// name
#define TT_PUNCTUATION				5			// punctuation
#endif

typedef struct pc_token_s
{
	int type;
	int subtype;
	int intvalue;
	lfixed floatvalue;
	char string[MAX_TOKENLENGTH];
} pc_token_t;

// data is an in/out parm, returns a parsed out token

void	COM_MatchToken( const char**buf_p, const char *match );

void SkipBracedSection (const char * *program);
void SkipRestOfLine (const char * *data );

void Parse1DMatrix (const char **buf_p, int x, gfixed *m);
void Parse2DMatrix (const char **buf_p, int y, int x, gfixed *m);
void Parse3DMatrix (const char **buf_p, int z, int y, int x, gfixed *m);

void	QDECL Com_sprintf (char *dest, int size, const char *fmt, ...);


// mode parm for FS_FOpenFile
typedef enum {
	FS_READ,
	FS_WRITE,
	FS_APPEND,
	FS_APPEND_SYNC
} fsMode_t;

typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

//=============================================

int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );

// portable case insensitive compare
int		Q_stricmp (const char *s1, const char *s2);
int		Q_strncmp (const char *s1, const char *s2, int n);
int		Q_stricmpn (const char *s1, const char *s2, int n);
char	*Q_strlwr( char *s1 );
char	*Q_strupr( char *s1 );
char	*Q_strrchr( const char* string, int c );

// buffer size safe library replacements
void	Q_strncpyz( char *dest, const char *src, int destsize );
void	Q_strcat( char *dest, int size, const char *src );

// strlen that discounts Quake color sequences
int Q_PrintStrlen( const char *string );
// removes color sequences from string
char *Q_CleanStr( char *string );

//=============================================

// 64-bit integers for global rankings interface
// implemented as a struct for qvm compatibility
typedef struct
{
	byte	b0;
	byte	b1;
	byte	b2;
	byte	b3;
	byte	b4;
	byte	b5;
	byte	b6;
	byte	b7;
} qint64;

//=============================================
/*
short	BigShort(short l);
short	LittleShort(short l);
int		BigLong (int l);
int		LittleLong (int l);
qint64  BigLong64 (qint64 l);
qint64  LittleLong64 (qint64 l);
gfixed	BigFloat (const gfixed *l);
gfixed	LittleFloat (const gfixed *l);

void	Swap_Init (void);
*/
const char	* QDECL va(const char *format, ...);

//=============================================

//
// key / value info strings
//
const char *Info_ValueForKey( const char *s, const char *key );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_big( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
qboolean Info_Validate( const char *s );
void Info_NextPair( const char **s, char *key, char *value );

// this is only here so the functions in q_shared.c and bg_*.c can link
void	QDECL Com_Error( int level, const char *error, ... );
void	QDECL Com_Printf( const char *msg, ... );


/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

#define	CVAR_ARCHIVE		1	// set to cause it to be saved to vars.rc
								// used for system variables, not for player
								// specific configurations
#define	CVAR_USERINFO		2	// sent to server on connect or change
#define	CVAR_SERVERINFO		4	// sent in response to front end requests
#define	CVAR_SYSTEMINFO		8	// these cvars will be duplicated on all clients
#define	CVAR_INIT			16	// don't allow change from console at all,
								// but can be set from the command line
#define	CVAR_LATCH			32	// will only change when C code next does
								// a Cvar_Get(), so it can't be changed
								// without proper initialization.  modified
								// will be set, even though the value hasn't
								// changed yet
#define	CVAR_ROM			64	// display only, cannot be set by user at all
#define	CVAR_USER_CREATED	128	// created by a set command
#define	CVAR_TEMP			256	// can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT			512	// can not be changed if cheats are disabled
#define CVAR_NORESTART		1024	// do not clear when a cvar_restart is issued

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s {
	char		*name;
	char		*string;
	char		*resetString;		// cvar_restart will reset to this value
	char		*latchedString;		// for CVAR_LATCH vars
	int			flags;
	qboolean	modified;			// set each time the cvar is changed
	int			modificationCount;	// incremented each time the cvar is changed
	lfixed		value;				// atof( string )
	int			integer;			// atoi( string )
	struct cvar_s *next;
	struct cvar_s *hashNext;
} cvar_t;

#define	MAX_CVAR_VALUE_STRING	256

typedef int	cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct {
	cvarHandle_t	handle;
	int			modificationCount;
	lfixed		value;
	int			integer;
	char		string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#include "surfaceflags.h"			// shared with the q3map utility

// plane types are used to speed some tests
// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2
#define	PLANE_NON_AXIAL	3


/*
=================
PlaneTypeForNormal
=================
*/

#define PlaneTypeForNormal(x) (x[0] == AFIXED_1 ? PLANE_X : (x[1] == AFIXED_1 ? PLANE_Y : (x[2] == AFIXED_1 ? PLANE_Z : PLANE_NON_AXIAL) ) )

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s {
	avec3_t	normal;
	bfixed	dist;
	byte	type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
	byte	signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
	byte	pad[2];
} cplane_t;


// a trace is returned when a box is swept through the world
typedef struct {
	qboolean	allsolid;	// if true, plane is not valid
	qboolean	startsolid;	// if true, the initial point was in a solid area
	gfixed		fraction;	// time completed, GFIXED_1 = didn't hit anything
	bvec3_t		endpos;		// final position
	cplane_t	plane;		// surface normal at impact, transformed to world space
	int			surfaceFlags;	// surface hit
	int			contents;	// contents on other side of surface hit
	int			entityNum;	// entity the contacted sirface is a part of
} trace_t;

// trace->entityNum can also be 0 to (MAX_GENTITIES-1)
// or ENTITYNUM_NONE, ENTITYNUM_WORLD


// markfragments are returned by CM_MarkFragments()
typedef struct {
	int		firstPoint;
	int		numPoints;
} markFragment_t;



typedef struct {
	bvec3_t		origin;
	avec3_t		axis[3];
} orientation_t;

//=====================================================================


// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define KEYCATCH_CONSOLE		0x0001
#define	KEYCATCH_UI					0x0002
#define	KEYCATCH_MESSAGE		0x0004
#define	KEYCATCH_CGAME			0x0008


// sound channels
// channel 0 never willingly overrides
// other channels will allways override a playing sound on that channel
typedef enum {
	CHAN_AUTO,
	CHAN_LOCAL,		// menu sounds, etc
	CHAN_WEAPON,
	CHAN_VOICE,
	CHAN_ITEM,
	CHAN_BODY,
	CHAN_LOCAL_SOUND,	// chat messages, etc
	CHAN_ANNOUNCER		// announcer voices, etc
} soundChannel_t;


/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/

#define	ANGLE2SHORT(x)	(FIXED_INT32SCALE(65536,(x)/AFIXED(360,0)) & 65535)
#define	SHORT2ANGLE(x)	(FIXED_INT32RATIO_A(x*360,65536))

#define	SNAPFLAG_RATE_DELAYED	1
#define	SNAPFLAG_NOT_ACTIVE		2	// snapshot used during connection and for zombies
#define SNAPFLAG_SERVERCOUNT	4	// toggled every map_restart so transitions can be detected

//
// per-level limits
//
#define	MAX_CLIENTS			64		// absolute limit
#define MAX_LOCATIONS		64

#define	GENTITYNUM_BITS		10		// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define	ENTITYNUM_NONE		(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD		(MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)


#define	MAX_MODELS			256		// these are sent over the net as 8 bits
#define	MAX_SOUNDS			256		// so they cannot be blindly increased


#define	MAX_CONFIGSTRINGS	1024

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define	CS_SERVERINFO		0		// an info string with all the serverinfo cvars
#define	CS_SYSTEMINFO		1		// an info string for server system to client system configuration (timescale, etc)

#define	RESERVED_CONFIGSTRINGS	2	// game can't modify below this, only the system can

#define	MAX_GAMESTATE_CHARS	16000
typedef struct {
	int			stringOffsets[MAX_CONFIGSTRINGS];
	char		stringData[MAX_GAMESTATE_CHARS];
	int			dataCount;
} gameState_t;

//=========================================================

// bit field limits
#define	MAX_STATS				16
#define	MAX_PERSISTANT			16
#define	MAX_POWERUPS			16
#define	MAX_WEAPONS				16		

#define	MAX_PS_EVENTS			2

#define PS_PMOVEFRAMECOUNTBITS	6

// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
typedef struct playerState_s {
	int			commandTime;	// cmd->serverTime of last executed command
	int			pm_type;
	int			bobCycle;		// for view bobbing and footstep generation
	int			pm_flags;		// ducked, jump_held, etc
	int			pm_time;

	bvec3_t		origin;
	bvec3_t		velocity;
	int			weaponTime;
	int			gravity;
	int			speed;
	int			delta_angles[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters

	int			groundEntityNum;// ENTITYNUM_NONE = in air

	int			legsTimer;		// don't change low priority animations until this runs out
	int			legsAnim;		// mask off ANIM_TOGGLEBIT

	int			torsoTimer;		// don't change low priority animations until this runs out
	int			torsoAnim;		// mask off ANIM_TOGGLEBIT

	int			movementDir;	// a number 0 to 7 that represents the reletive angle
								// of movement to the view angle (axial and diagonals)
								// when at rest, the value will remain unchanged
								// used to twist the legs during strafing

	bvec3_t		grapplePoint;	// location of grapple to pull towards if PMF_GRAPPLE_PULL

	int			eFlags;			// copied to entityState_t->eFlags

	int			eventSequence;	// pmove generated events
	int			events[MAX_PS_EVENTS];
	int			eventParms[MAX_PS_EVENTS];

	int			externalEvent;	// events set on player from another source
	int			externalEventParm;
	int			externalEventTime;

	int			clientNum;		// ranges from 0 to MAX_CLIENTS-1
	int			weapon;			// copied to entityState_t->weapon
	int			weaponstate;

	avec3_t		viewangles;		// for views
	int			viewheight;

	// damage feedback
	int			damageEvent;	// when it changes, latch the other parms
	int			damageYaw;
	int			damagePitch;
	int			damageCount;

	int			stats[MAX_STATS];
	int			persistant[MAX_PERSISTANT];	// stats that aren't cleared on death
	int			powerups[MAX_POWERUPS];	// level.time that the powerup runs out
	int			ammo[MAX_WEAPONS];

	int			generic1;
	int			loopSound;
	int			jumppad_ent;	// jumppad entity hit this frame

	// not communicated over the net at all
	int			ping;			// server to game info for scoreboard
	int			pmove_framecount;	// FIXME: don't transmit over the network
	int			jumppad_frame;
	int			entityEventSequence;
} playerState_t;


//====================================================================


//
// usercmd_t->button bits, many of which are generated by the client system,
// so they aren't game/cgame only definitions
//
#define	BUTTON_ATTACK		1
#define	BUTTON_TALK			2			// displays talk balloon and disables actions
#define	BUTTON_USE_HOLDABLE	4
#define	BUTTON_GESTURE		8
#define	BUTTON_WALKING		16			// walking can't just be infered from MOVE_RUN
										// because a key pressed late in the frame will
										// only generate a small move value for that frame
										// walking will use different animations and
										// won't generate footsteps
#define BUTTON_AFFIRMATIVE	32
#define	BUTTON_NEGATIVE		64

#define BUTTON_GETFLAG		128
#define BUTTON_GUARDBASE	256
#define BUTTON_PATROL		512
#define BUTTON_FOLLOWME		1024

#define	BUTTON_ANY			2048			// any key whatsoever

#define	MOVE_RUN			120			// if forwardmove or rightmove are >= MOVE_RUN,
										// then BUTTON_WALKING should be set

// usercmd_t is sent to the server each client frame
typedef struct usercmd_s {
	int				serverTime;
	int				angles[3];
	int 			buttons;
	byte			weapon;           // weapon 
	signed char	forwardmove, rightmove, upmove;
} usercmd_t;

//===================================================================

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define	SOLID_BMODEL	0xffffff

typedef enum {
	TR_STATIONARY,
	TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	TR_SINE,					// value = base + sin( time / duration ) * delta
	TR_GRAVITY
} trType_t;

typedef struct {
	trType_t	trType;
	int		trTime;
	int		trDuration;			// if non 0, trTime + trDuration = stop time
	bvec3_t	trBase;
	bvec3_t	trDelta;			// velocity, etc
} btrajectory_t;

typedef struct {
	trType_t	trType;
	int		trTime;
	int		trDuration;			// if non 0, trTime + trDuration = stop time
	avec3_t	trBase;
	avec3_t	trDelta;			// velocity, etc
} atrajectory_t;


// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

typedef struct entityState_s {
	int		number;			// entity index
	int		eType;			// entityType_t
	int		eFlags;

	btrajectory_t	pos;	// for calculating position
	atrajectory_t	apos;	// for calculating angles

	int		time;
	int		time2;

	bvec3_t	origin;
	bvec3_t	origin2;

	avec3_t	angles;
	avec3_t	angles2;

	int		otherEntityNum;	// shotgun sources, etc
	int		otherEntityNum2;

	int		groundEntityNum;	// -1 = in air

	int		constantLight;	// r + (g<<8) + (b<<16) + (intensity<<24)
	int		loopSound;		// constantly loop this sound

	int		modelindex;
	int		modelindex2;
	int		clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses
	int		frame;

	int		solid;			// for client side prediction, _G_trap_linkentity sets this properly

	int		event;			// impulse events -- muzzle flashes, footsteps, etc
	int		eventParm;

	// for players
	int		powerups;		// bit flags
	int		weapon;			// determines weapon and flash model, etc
	int		legsAnim;		// mask off ANIM_TOGGLEBIT
	int		torsoAnim;		// mask off ANIM_TOGGLEBIT

	int		generic1;
} entityState_t;

typedef enum {
	CA_UNINITIALIZED,
	CA_DISCONNECTED, 	// not talking to a server
	CA_AUTHORIZING,		// not used any more, was checking cd key 
	CA_CONNECTING,		// sending request packets to the server
	CA_CHALLENGING,		// sending challenge packets to the server
	CA_CONNECTED,		// netchan_t established, getting gamestate
	CA_LOADING,			// only during cgame initialization, never during main loop
	CA_PRIMED,			// got gamestate, waiting for first frame
	CA_ACTIVE,			// game views should be displayed
	CA_CINEMATIC		// playing a cinematic or a static pic, not connected to a server
} connstate_t;

// font support 

#define GLYPH_START 0
#define GLYPH_END 255
#define GLYPH_CHARSTART 32
#define GLYPH_CHAREND 127
#define GLYPHS_PER_FONT GLYPH_END - GLYPH_START + 1
typedef struct {
  int height;       // number of scan lines
  int top;          // top of glyph in buffer
  int bottom;       // bottom of glyph in buffer
  int pitch;        // width for copying
  int xSkip;        // x adjustment
  int imageWidth;   // width of actual image
  int imageHeight;  // height of actual image
  gfixed s;          // x offset in image where glyph starts
  gfixed t;          // y offset in image where glyph starts
  gfixed s2;
  gfixed t2;
  qhandle_t glyph;  // handle to the shader with the glyph
  char shaderName[32];
} glyphInfo_t;

typedef struct {
  glyphInfo_t glyphs [GLYPHS_PER_FONT];
  gfixed glyphScale;
  char name[MAX_QPATH];
} fontInfo_t;

#define Square(x) ((x)*(x))

// real time
//=============================================


typedef struct qtime_s {
	int tm_sec;     /* seconds after the minute - [0,59] */
	int tm_min;     /* minutes after the hour - [0,59] */
	int tm_hour;    /* hours since midnight - [0,23] */
	int tm_mday;    /* day of the month - [1,31] */
	int tm_mon;     /* months since January - [0,11] */
	int tm_year;    /* years since 1900 */
	int tm_wday;    /* days since Sunday - [0,6] */
	int tm_yday;    /* days since January 1 - [0,365] */
	int tm_isdst;   /* daylight savings time flag */
} qtime_t;


// server browser sources
// TTimo: AS_MPLAYER is no longer used
#define AS_LOCAL			0
#define AS_MPLAYER		1
#define AS_GLOBAL			2
#define AS_FAVORITES	3


// cinematic states
typedef enum {
	FMV_IDLE,
	FMV_PLAY,		// play
	FMV_EOF,		// all other conditions, i.e. stop/EOF/abort
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
} e_status;

typedef enum _flag_status {
	FLAG_ATBASE = 0,
	FLAG_TAKEN,			// CTF
	FLAG_TAKEN_RED,		// One Flag CTF
	FLAG_TAKEN_BLUE,	// One Flag CTF
	FLAG_DROPPED
} flagStatus_t;



#define	MAX_GLOBAL_SERVERS				4096
#define	MAX_OTHER_SERVERS					128
#define MAX_PINGREQUESTS					32
#define MAX_SERVERSTATUSREQUESTS	16

#define SAY_ALL		0
#define SAY_TEAM	1
#define SAY_TELL	2

#define CDKEY_LEN 16
#define CDCHKSUM_LEN 2

ID_INLINE long ReadLittleLong(const byte *buf)
{
	return (long) (
	(((unsigned long)(unsigned char)(buf[0]))<<0) | 
	(((unsigned long)(unsigned char)(buf[1]))<<8) |
	(((unsigned long)(unsigned char)(buf[2]))<<16) | 
	(((unsigned long)(unsigned char)(buf[3]))<<24)
	); 

}

ID_INLINE short ReadLittleShort(const byte *buf)
{ 
	return (short) (
	(((unsigned short)(unsigned char)(buf[0]))<<0) | 
	(((unsigned short)(unsigned char)(buf[1]))<<8)
	); 
}


class SysCallArg
{
private:
	union
	{
		int m_i;
		void *m_p;
		const void *m_cp;
		afixed m_a;
		bfixed m_b;
		cfixed m_c;
		dfixed m_d;
		gfixed m_g;
		lfixed m_l;
	};
	enum 
	{
		T_EMPTY=0,
		T_I,
		T_P,
		T_CP,
		T_A,
		T_B,
		T_C,
		T_D,
		T_G,
		T_L
	} m_type;

public:
	SysCallArg() { m_i=0xDE4DB3AF; m_type=T_EMPTY; }

	inline static SysCallArg Int(int i) { SysCallArg arg; arg.m_i=i; arg.m_type=T_I; return arg; }
	inline static SysCallArg Ptr(void *p) { SysCallArg arg; arg.m_p=p; arg.m_type=T_P; return arg; }
	inline static SysCallArg ConstPtr(const void *cp) { SysCallArg arg; arg.m_cp=cp; arg.m_type=T_CP; return arg; }
#ifndef FIXED_IS_FLOAT
	inline static SysCallArg Fixed(afixed a) { SysCallArg arg; arg.m_a=a; arg.m_type=T_A; return arg; }
	inline static SysCallArg Fixed(bfixed b) { SysCallArg arg; arg.m_b=b; arg.m_type=T_B; return arg; }
	inline static SysCallArg Fixed(cfixed c) { SysCallArg arg; arg.m_c=c; arg.m_type=T_C; return arg; }
	inline static SysCallArg Fixed(dfixed d) { SysCallArg arg; arg.m_d=d; arg.m_type=T_D; return arg; }
#endif
	inline static SysCallArg Fixed(gfixed g) { SysCallArg arg; arg.m_g=g; arg.m_type=T_G; return arg; }
	inline static SysCallArg Fixed(lfixed l) { SysCallArg arg; arg.m_l=l; arg.m_type=T_L; return arg; }
	
	inline operator int() const { if(m_type!=T_I) { DebugBreak(); } return m_i; }
	
	template<class T>
	inline operator T *() const { if(m_type!=T_P) { DebugBreak(); } return (T *)m_p; }
	template<class T>
	inline operator const T *() const { if(m_type!=T_CP) { DebugBreak(); }  return (const T *)m_cp; }
#ifndef FIXED_IS_FLOAT
	inline operator afixed() const { if(m_type!=T_A) { DebugBreak(); } return m_a; }
	inline operator bfixed() const { if(m_type!=T_B) { DebugBreak(); } return m_b; }
	inline operator cfixed() const { if(m_type!=T_C) { DebugBreak(); } return m_c; }
	inline operator dfixed() const { if(m_type!=T_D) { DebugBreak(); } return m_d; }
#endif
	inline operator gfixed() const { if(m_type!=T_G) { DebugBreak(); } return m_g; }
	inline operator lfixed() const { if(m_type!=T_L) { DebugBreak(); } return m_l; }

	inline SysCallArg & operator=(int i) { m_i=i; m_type=T_I; return *this; }
	inline SysCallArg & operator=(void *p) { m_p=p; m_type=T_P; return *this; }
	inline SysCallArg & operator=(const void *cp) { m_cp=cp; m_type=T_CP; return *this; }
#ifndef FIXED_IS_FLOAT
	inline SysCallArg & operator=(afixed a) { m_a=a; m_type=T_A; return *this; }
	inline SysCallArg & operator=(bfixed b) { m_b=b; m_type=T_B; return *this; }
	inline SysCallArg & operator=(cfixed c) { m_c=c; m_type=T_C; return *this; }
	inline SysCallArg & operator=(dfixed d) { m_d=d; m_type=T_D; return *this; }
#endif
	inline SysCallArg & operator=(gfixed g) { m_g=g; m_type=T_G; return *this; }
	inline SysCallArg & operator=(lfixed l) { m_l=l; m_type=T_L; return *this; }
};

class SysCallArgs
{
	SysCallArg  m_args[16];
	int m_nArgs;
public:
	SysCallArgs(int nArgs) { m_nArgs=nArgs; ASSERT(m_nArgs<=16); }
	~SysCallArgs() { }
	
	SysCallArg & operator[] (int pos) { if(pos<0 || pos>m_nArgs) { DebugBreak(); } return m_args[pos]; }
	const SysCallArg & operator[] (int pos) const  { if(pos<0 || pos>m_nArgs) { DebugBreak(); } return m_args[pos]; }
};


#ifdef _WIN32

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

#else

#define DLLEXPORT __attribute__((visibility("default")))
#define DLLIMPORT 

#endif

#endif	// __Q_SHARED_H
