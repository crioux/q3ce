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

#include "q_shared.h"

//
// q_math.c -- stateless support routines that are included in each code module

vec3_t	vec3_origin = {GFIXED_0,GFIXED_0,GFIXED_0};
vec3_t	axisDefault[3] = { { GFIXED_1, GFIXED_0, GFIXED_0 }, { GFIXED_0, GFIXED_1, GFIXED_0 }, { GFIXED_0, GFIXED_0, GFIXED_1 } };

avec3_t	avec3_origin = {AFIXED_0,AFIXED_0,AFIXED_0};
avec3_t	aaxisDefault[3] = { { AFIXED_1, AFIXED_0, AFIXED_0 }, { AFIXED_0, AFIXED_1, AFIXED_0 }, { AFIXED_0, AFIXED_0, AFIXED_1 } };

bvec3_t	bvec3_origin = {BFIXED_0,BFIXED_0,BFIXED_0};
bvec3_t	baxisDefault[3] = { { BFIXED_1, BFIXED_0, BFIXED_0 }, { BFIXED_0, BFIXED_1, BFIXED_0 }, { BFIXED_0, BFIXED_0, BFIXED_1 } };


vec4_t		colorBlack	= {GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_1};
vec4_t		colorRed	= {GFIXED_1, GFIXED_0, GFIXED_0, GFIXED_1};
vec4_t		colorGreen	= {GFIXED_0, GFIXED_1, GFIXED_0, GFIXED_1};
vec4_t		colorBlue	= {GFIXED_0, GFIXED_0, GFIXED_1, GFIXED_1};
vec4_t		colorYellow	= {GFIXED_1, GFIXED_1, GFIXED_0, GFIXED_1};
vec4_t		colorMagenta= {GFIXED_1, GFIXED_0, GFIXED_1, GFIXED_1};
vec4_t		colorCyan	= {GFIXED_0, GFIXED_1, GFIXED_1, GFIXED_1};
vec4_t		colorWhite	= {GFIXED_1, GFIXED_1, GFIXED_1, GFIXED_1};
vec4_t		colorLtGrey	= {GFIXED(0,75), GFIXED(0,75), GFIXED(0,75), GFIXED_1};
vec4_t		colorMdGrey	= {GFIXED(0,5), GFIXED(0,5), GFIXED(0,5), GFIXED_1};
vec4_t		colorDkGrey	= {GFIXED(0,25), GFIXED(0,25), GFIXED(0,25), GFIXED_1};

vec4_t	g_color_table[8] =
	{
	{GFIXED_0, GFIXED_0, GFIXED_0, GFIXED_1},
	{GFIXED_1, GFIXED_0, GFIXED_0, GFIXED_1},
	{GFIXED_0, GFIXED_1, GFIXED_0, GFIXED_1},
	{GFIXED_1, GFIXED_1, GFIXED_0, GFIXED_1},
	{GFIXED_0, GFIXED_0, GFIXED_1, GFIXED_1},
	{GFIXED_0, GFIXED_1, GFIXED_1, GFIXED_1},
	{GFIXED_1, GFIXED_0, GFIXED_1, GFIXED_1},
	{GFIXED_1, GFIXED_1, GFIXED_1, GFIXED_1},
	};


avec3_t	bytedirs[NUMVERTEXNORMALS] =
{
{-AFIXED(0,525731), AFIXED(0,000000), AFIXED(0,850651)}, {-AFIXED(0,442863), AFIXED(0,238856), AFIXED(0,864188)}, 
{-AFIXED(0,295242), AFIXED(0,000000), AFIXED(0,955423)}, {-AFIXED(0,309017), AFIXED(0,500000), AFIXED(0,809017)}, 
{-AFIXED(0,162460), AFIXED(0,262866), AFIXED(0,951056)}, {AFIXED(0,000000), AFIXED(0,000000), AFIXED(1,000000)}, 
{AFIXED(0,000000), AFIXED(0,850651), AFIXED(0,525731)}, {-AFIXED(0,147621), AFIXED(0,716567), AFIXED(0,681718)}, 
{AFIXED(0,147621), AFIXED(0,716567), AFIXED(0,681718)}, {AFIXED(0,000000), AFIXED(0,525731), AFIXED(0,850651)}, 
{AFIXED(0,309017), AFIXED(0,500000), AFIXED(0,809017)}, {AFIXED(0,525731), AFIXED(0,000000), AFIXED(0,850651)}, 
{AFIXED(0,295242), AFIXED(0,000000), AFIXED(0,955423)}, {AFIXED(0,442863), AFIXED(0,238856), AFIXED(0,864188)}, 
{AFIXED(0,162460), AFIXED(0,262866), AFIXED(0,951056)}, {-AFIXED(0,681718), AFIXED(0,147621), AFIXED(0,716567)}, 
{-AFIXED(0,809017), AFIXED(0,309017), AFIXED(0,500000)},{-AFIXED(0,587785), AFIXED(0,425325), AFIXED(0,688191)}, 
{-AFIXED(0,850651), AFIXED(0,525731), AFIXED(0,000000)},{-AFIXED(0,864188), AFIXED(0,442863), AFIXED(0,238856)}, 
{-AFIXED(0,716567), AFIXED(0,681718), AFIXED(0,147621)},{-AFIXED(0,688191), AFIXED(0,587785), AFIXED(0,425325)}, 
{-AFIXED(0,500000), AFIXED(0,809017), AFIXED(0,309017)}, {-AFIXED(0,238856), AFIXED(0,864188), AFIXED(0,442863)}, 
{-AFIXED(0,425325), AFIXED(0,688191), AFIXED(0,587785)}, {-AFIXED(0,716567), AFIXED(0,681718), -AFIXED(0,147621)}, 
{-AFIXED(0,500000), AFIXED(0,809017), -AFIXED(0,309017)}, {-AFIXED(0,525731), AFIXED(0,850651), AFIXED(0,000000)}, 
{AFIXED(0,000000), AFIXED(0,850651), -AFIXED(0,525731)}, {-AFIXED(0,238856), AFIXED(0,864188), -AFIXED(0,442863)}, 
{AFIXED(0,000000), AFIXED(0,955423), -AFIXED(0,295242)}, {-AFIXED(0,262866), AFIXED(0,951056), -AFIXED(0,162460)}, 
{AFIXED(0,000000), AFIXED(1,000000), AFIXED(0,000000)}, {AFIXED(0,000000), AFIXED(0,955423), AFIXED(0,295242)}, 
{-AFIXED(0,262866), AFIXED(0,951056), AFIXED(0,162460)}, {AFIXED(0,238856), AFIXED(0,864188), AFIXED(0,442863)}, 
{AFIXED(0,262866), AFIXED(0,951056), AFIXED(0,162460)}, {AFIXED(0,500000), AFIXED(0,809017), AFIXED(0,309017)}, 
{AFIXED(0,238856), AFIXED(0,864188), -AFIXED(0,442863)},{AFIXED(0,262866), AFIXED(0,951056), -AFIXED(0,162460)}, 
{AFIXED(0,500000), AFIXED(0,809017), -AFIXED(0,309017)},{AFIXED(0,850651), AFIXED(0,525731), AFIXED(0,000000)}, 
{AFIXED(0,716567), AFIXED(0,681718), AFIXED(0,147621)}, {AFIXED(0,716567), AFIXED(0,681718), -AFIXED(0,147621)}, 
{AFIXED(0,525731), AFIXED(0,850651), AFIXED(0,000000)}, {AFIXED(0,425325), AFIXED(0,688191), AFIXED(0,587785)}, 
{AFIXED(0,864188), AFIXED(0,442863), AFIXED(0,238856)}, {AFIXED(0,688191), AFIXED(0,587785), AFIXED(0,425325)}, 
{AFIXED(0,809017), AFIXED(0,309017), AFIXED(0,500000)}, {AFIXED(0,681718), AFIXED(0,147621), AFIXED(0,716567)}, 
{AFIXED(0,587785), AFIXED(0,425325), AFIXED(0,688191)}, {AFIXED(0,955423), AFIXED(0,295242), AFIXED(0,000000)}, 
{AFIXED(1,000000), AFIXED(0,000000), AFIXED(0,000000)}, {AFIXED(0,951056), AFIXED(0,162460), AFIXED(0,262866)}, 
{AFIXED(0,850651), -AFIXED(0,525731), AFIXED(0,000000)},{AFIXED(0,955423), -AFIXED(0,295242), AFIXED(0,000000)}, 
{AFIXED(0,864188), -AFIXED(0,442863), AFIXED(0,238856)}, {AFIXED(0,951056), -AFIXED(0,162460), AFIXED(0,262866)}, 
{AFIXED(0,809017), -AFIXED(0,309017), AFIXED(0,500000)}, {AFIXED(0,681718), -AFIXED(0,147621), AFIXED(0,716567)}, 
{AFIXED(0,850651), AFIXED(0,000000), AFIXED(0,525731)}, {AFIXED(0,864188), AFIXED(0,442863), -AFIXED(0,238856)}, 
{AFIXED(0,809017), AFIXED(0,309017), -AFIXED(0,500000)}, {AFIXED(0,951056), AFIXED(0,162460), -AFIXED(0,262866)}, 
{AFIXED(0,525731), AFIXED(0,000000), -AFIXED(0,850651)}, {AFIXED(0,681718), AFIXED(0,147621), -AFIXED(0,716567)}, 
{AFIXED(0,681718), -AFIXED(0,147621), -AFIXED(0,716567)},{AFIXED(0,850651), AFIXED(0,000000), -AFIXED(0,525731)}, 
{AFIXED(0,809017), -AFIXED(0,309017), -AFIXED(0,500000)}, {AFIXED(0,864188), -AFIXED(0,442863), -AFIXED(0,238856)}, 
{AFIXED(0,951056), -AFIXED(0,162460), -AFIXED(0,262866)}, {AFIXED(0,147621), AFIXED(0,716567), -AFIXED(0,681718)}, 
{AFIXED(0,309017), AFIXED(0,500000), -AFIXED(0,809017)}, {AFIXED(0,425325), AFIXED(0,688191), -AFIXED(0,587785)}, 
{AFIXED(0,442863), AFIXED(0,238856), -AFIXED(0,864188)}, {AFIXED(0,587785), AFIXED(0,425325), -AFIXED(0,688191)}, 
{AFIXED(0,688191), AFIXED(0,587785), -AFIXED(0,425325)}, {-AFIXED(0,147621), AFIXED(0,716567), -AFIXED(0,681718)}, 
{-AFIXED(0,309017), AFIXED(0,500000), -AFIXED(0,809017)}, {AFIXED(0,000000), AFIXED(0,525731), -AFIXED(0,850651)}, 
{-AFIXED(0,525731), AFIXED(0,000000), -AFIXED(0,850651)}, {-AFIXED(0,442863), AFIXED(0,238856), -AFIXED(0,864188)}, 
{-AFIXED(0,295242), AFIXED(0,000000), -AFIXED(0,955423)}, {-AFIXED(0,162460), AFIXED(0,262866), -AFIXED(0,951056)}, 
{AFIXED(0,000000), AFIXED(0,000000), -AFIXED(1,000000)}, {AFIXED(0,295242), AFIXED(0,000000), -AFIXED(0,955423)}, 
{AFIXED(0,162460), AFIXED(0,262866), -AFIXED(0,951056)}, {-AFIXED(0,442863), -AFIXED(0,238856), -AFIXED(0,864188)}, 
{-AFIXED(0,309017), -AFIXED(0,500000), -AFIXED(0,809017)}, {-AFIXED(0,162460), -AFIXED(0,262866), -AFIXED(0,951056)}, 
{AFIXED(0,000000), -AFIXED(0,850651), -AFIXED(0,525731)}, {-AFIXED(0,147621), -AFIXED(0,716567), -AFIXED(0,681718)}, 
{AFIXED(0,147621), -AFIXED(0,716567), -AFIXED(0,681718)}, {AFIXED(0,000000), -AFIXED(0,525731), -AFIXED(0,850651)}, 
{AFIXED(0,309017), -AFIXED(0,500000), -AFIXED(0,809017)}, {AFIXED(0,442863), -AFIXED(0,238856), -AFIXED(0,864188)}, 
{AFIXED(0,162460), -AFIXED(0,262866), -AFIXED(0,951056)}, {AFIXED(0,238856), -AFIXED(0,864188), -AFIXED(0,442863)}, 
{AFIXED(0,500000), -AFIXED(0,809017), -AFIXED(0,309017)}, {AFIXED(0,425325), -AFIXED(0,688191), -AFIXED(0,587785)}, 
{AFIXED(0,716567), -AFIXED(0,681718), -AFIXED(0,147621)}, {AFIXED(0,688191), -AFIXED(0,587785), -AFIXED(0,425325)}, 
{AFIXED(0,587785), -AFIXED(0,425325), -AFIXED(0,688191)}, {AFIXED(0,000000), -AFIXED(0,955423), -AFIXED(0,295242)}, 
{AFIXED(0,000000), -AFIXED(1,000000), AFIXED(0,000000)}, {AFIXED(0,262866), -AFIXED(0,951056), -AFIXED(0,162460)}, 
{AFIXED(0,000000), -AFIXED(0,850651), AFIXED(0,525731)}, {AFIXED(0,000000), -AFIXED(0,955423), AFIXED(0,295242)}, 
{AFIXED(0,238856), -AFIXED(0,864188), AFIXED(0,442863)}, {AFIXED(0,262866), -AFIXED(0,951056), AFIXED(0,162460)}, 
{AFIXED(0,500000), -AFIXED(0,809017), AFIXED(0,309017)}, {AFIXED(0,716567), -AFIXED(0,681718), AFIXED(0,147621)}, 
{AFIXED(0,525731), -AFIXED(0,850651), AFIXED(0,000000)}, {-AFIXED(0,238856), -AFIXED(0,864188), -AFIXED(0,442863)}, 
{-AFIXED(0,500000), -AFIXED(0,809017), -AFIXED(0,309017)}, {-AFIXED(0,262866), -AFIXED(0,951056), -AFIXED(0,162460)}, 
{-AFIXED(0,850651), -AFIXED(0,525731), AFIXED(0,000000)}, {-AFIXED(0,716567), -AFIXED(0,681718), -AFIXED(0,147621)}, 
{-AFIXED(0,716567), -AFIXED(0,681718), AFIXED(0,147621)}, {-AFIXED(0,525731), -AFIXED(0,850651), AFIXED(0,000000)}, 
{-AFIXED(0,500000), -AFIXED(0,809017), AFIXED(0,309017)}, {-AFIXED(0,238856), -AFIXED(0,864188), AFIXED(0,442863)}, 
{-AFIXED(0,262866), -AFIXED(0,951056), AFIXED(0,162460)}, {-AFIXED(0,864188), -AFIXED(0,442863), AFIXED(0,238856)}, 
{-AFIXED(0,809017), -AFIXED(0,309017), AFIXED(0,500000)}, {-AFIXED(0,688191), -AFIXED(0,587785), AFIXED(0,425325)}, 
{-AFIXED(0,681718), -AFIXED(0,147621), AFIXED(0,716567)}, {-AFIXED(0,442863), -AFIXED(0,238856), AFIXED(0,864188)}, 
{-AFIXED(0,587785), -AFIXED(0,425325), AFIXED(0,688191)}, {-AFIXED(0,309017), -AFIXED(0,500000), AFIXED(0,809017)}, 
{-AFIXED(0,147621), -AFIXED(0,716567), AFIXED(0,681718)}, {-AFIXED(0,425325), -AFIXED(0,688191), AFIXED(0,587785)}, 
{-AFIXED(0,162460), -AFIXED(0,262866), AFIXED(0,951056)}, {AFIXED(0,442863), -AFIXED(0,238856), AFIXED(0,864188)}, 
{AFIXED(0,162460), -AFIXED(0,262866), AFIXED(0,951056)}, {AFIXED(0,309017), -AFIXED(0,500000), AFIXED(0,809017)}, 
{AFIXED(0,147621), -AFIXED(0,716567), AFIXED(0,681718)}, {AFIXED(0,000000), -AFIXED(0,525731), AFIXED(0,850651)}, 
{AFIXED(0,425325), -AFIXED(0,688191), AFIXED(0,587785)}, {AFIXED(0,587785), -AFIXED(0,425325), AFIXED(0,688191)}, 
{AFIXED(0,688191), -AFIXED(0,587785), AFIXED(0,425325)}, {-AFIXED(0,955423), AFIXED(0,295242), AFIXED(0,000000)}, 
{-AFIXED(0,951056), AFIXED(0,162460), AFIXED(0,262866)}, {-AFIXED(1,000000), AFIXED(0,000000), AFIXED(0,000000)}, 
{-AFIXED(0,850651), AFIXED(0,000000), AFIXED(0,525731)}, {-AFIXED(0,955423), -AFIXED(0,295242), AFIXED(0,000000)}, 
{-AFIXED(0,951056), -AFIXED(0,162460), AFIXED(0,262866)}, {-AFIXED(0,864188), AFIXED(0,442863), -AFIXED(0,238856)}, 
{-AFIXED(0,951056), AFIXED(0,162460), -AFIXED(0,262866)}, {-AFIXED(0,809017), AFIXED(0,309017), -AFIXED(0,500000)}, 
{-AFIXED(0,864188), -AFIXED(0,442863), -AFIXED(0,238856)}, {-AFIXED(0,951056), -AFIXED(0,162460), -AFIXED(0,262866)}, 
{-AFIXED(0,809017), -AFIXED(0,309017), -AFIXED(0,500000)}, {-AFIXED(0,681718), AFIXED(0,147621), -AFIXED(0,716567)}, 
{-AFIXED(0,681718), -AFIXED(0,147621), -AFIXED(0,716567)}, {-AFIXED(0,850651), AFIXED(0,000000), -AFIXED(0,525731)}, 
{-AFIXED(0,688191), AFIXED(0,587785), -AFIXED(0,425325)}, {-AFIXED(0,587785), AFIXED(0,425325), -AFIXED(0,688191)}, 
{-AFIXED(0,425325), AFIXED(0,688191), -AFIXED(0,587785)}, {-AFIXED(0,425325), -AFIXED(0,688191), -AFIXED(0,587785)}, 
{-AFIXED(0,587785), -AFIXED(0,425325), -AFIXED(0,688191)}, {-AFIXED(0,688191), -AFIXED(0,587785), -AFIXED(0,425325)}
};

//==============================================================

int		Q_rand( int *seed ) {
	*seed = (69069 * (*seed) + 1);
	return *seed;
}

gfixed	Q_random( int *seed ) {
#ifdef FIXED_IS_FLOAT
	return (MAKE_GFIXED(Q_rand( seed ) & 65535))/65536.0f;
#else
	return gfixed::FromRep(Q_rand( seed ) & ((1<<gfixed::prec)-1));
#endif
}

afixed	Q_arandom( int *seed ) {
#ifdef FIXED_IS_FLOAT
	return (MAKE_AFIXED(Q_rand( seed ) & 1048575))/1048576.0f;
#else
	return afixed::FromRep(Q_rand( seed ) & ((1<<afixed::prec)-1));
#endif
}

bfixed	Q_brandom( int *seed ) {
#ifdef FIXED_IS_FLOAT
	return (MAKE_BFIXED(Q_rand( seed ) & 65535))/65536.0f;
#else
	return bfixed::FromRep(Q_rand( seed ) & ((1<<bfixed::prec)-1));
#endif
}


gfixed	Q_crandom( int *seed ) {
	return GFIXED(2,0) * ( Q_random( seed ) - GFIXED(0,5) );
}

afixed	Q_acrandom( int *seed ) {
	return AFIXED(2,0) * ( Q_arandom( seed ) - AFIXED(0,5) );
}

bfixed	Q_bcrandom( int *seed ) {
	return BFIXED(2,0) * ( Q_brandom( seed ) - BFIXED(0,5) );
}


#ifdef __LCC__

int VectorCompare( const bvec3_t v1, const bvec3_t v2 ) {
	if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) {
		return 0;
	}			
	return 1;
}

bvec_t FIXED_VEC3LEN( const bvec3_t v ) {
	return (bvec_t)sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

bvec_t FIXED_VEC3LEN_SQ( const bvec3_t v ) {
	return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

bvec_t Distance( const bvec3_t p1, const bvec3_t p2 ) {
	bvec3_t	v;

	VectorSubtract (p2, p1, v);
	return FIXED_VEC3LEN( v );
}

bvec_t DistanceSquared( const bvec3_t p1, const bvec3_t p2 ) {
	bvec3_t	v;

	VectorSubtract (p2, p1, v);
	return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

// fast vector normalize routine that does not check to make sure
// that length != 0, nor does it return length, uses rsqrt approximation
void FIXED_FAST_VEC3NORM( bvec3_t v )
{
	FIXED_FASTVEC3NORM(v);
}

void VectorInverse( bvec3_t v ){
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void CrossProduct( const bvec3_t v1, const bvec3_t v2, bvec3_t cross ) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}
#endif

//=======================================================

signed char ClampChar( int i ) {
	if ( i < -128 ) {
		return -128;
	}
	if ( i > 127 ) {
		return 127;
	}
	return i;
}

signed short ClampShort( int i ) {
	if ( i < -32768 ) {
		return -32768;
	}
	if ( i > 0x7fff ) {
		return 0x7fff;
	}
	return i;
}


// this isn't a real cheap function to call!
int DirToByte( bvec3_t dir ) {
	int		i, best;
	afixed	d, bestd;

	if ( !dir ) {
		return 0;
	}

	avec3_t adir;
	adir[0]=MAKE_AFIXED(dir[0]);adir[1]=MAKE_AFIXED(dir[1]);adir[2]=MAKE_AFIXED(dir[2]);

	bestd = AFIXED_0;
	best = 0;
	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		d = FIXED_VEC3DOT (adir, bytedirs[i]);
		if (d > bestd)
		{
			bestd = d;
			best = i;
		}
	}

	return best;
}

#ifndef FIXED_IS_FLOAT
int DirToByte( avec3_t dir ) {
	int		i, best;
	afixed	d, bestd;

	if ( !dir ) {
		return 0;
	}

	bestd = AFIXED_0;
	best = 0;
	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		d = FIXED_VEC3DOT (dir, bytedirs[i]);
		if (d > bestd)
		{
			bestd = d;
			best = i;
		}
	}

	return best;
}
#endif

void ByteToDir( int b, avec3_t dir ) {
	if ( b < 0 || b >= NUMVERTEXNORMALS ) {
		VectorCopy( avec3_origin, dir );
		return;
	}
	dir[0]=bytedirs[b][0];
	dir[1]=bytedirs[b][1];
	dir[2]=bytedirs[b][2];
}


unsigned ColorBytes3 (gfixed r, gfixed g, gfixed b) {
	unsigned	i;

	( (byte *)&i )[0] = FIXED_INT32SCALE(255,r);
	( (byte *)&i )[1] = FIXED_INT32SCALE(255,g);
	( (byte *)&i )[2] = FIXED_INT32SCALE(255,b);

	return i;
}

unsigned ColorBytes4 (gfixed r, gfixed g, gfixed b, gfixed a) {
	unsigned	i;

	( (byte *)&i )[0] = FIXED_INT32SCALE(255,r);
	( (byte *)&i )[1] = FIXED_INT32SCALE(255,g);
	( (byte *)&i )[2] = FIXED_INT32SCALE(255,b);
	( (byte *)&i )[3] = FIXED_INT32SCALE(255,a);

	return i;
}

gfixed NormalizeColor( const vec3_t in, vec3_t out ) {
	gfixed	max;
	
	max = in[0];
	if ( in[1] > max ) {
		max = in[1];
	}
	if ( in[2] > max ) {
		max = in[2];
	}

	if ( FIXED_IS_ZERO(max) ) {
		VectorClear( out );
	} else {
		out[0] = in[0] / max;
		out[1] = in[1] / max;
		out[2] = in[2] / max;
	}
	return max;
}


/*
=====================
PlaneFromPoints

Returns false if the triangle is degenrate.
The normal will point out of the clock for clockwise ordered points
=====================
*/
qboolean PlaneFromPoints( avec3_t planenormal, bfixed &planedist, const bvec3_t a, const bvec3_t b, const bvec3_t c ) {
	bvec3_t	d1, d2;
	bvec3_t tmp;

	VectorSubtract( b, a, d1 );
	VectorSubtract( c, a, d2 );
	CrossProduct( d2, d1, tmp);
	if ( FIXED_IS_ZERO(VectorNormalizeB2A(tmp, planenormal ))) {
		return qfalse;
	}

	planedist = FIXED_VEC3DOT( a, planenormal );
	return qtrue;
}

/*
===============
RotatePointAroundVector

This is not implemented very well...
===============
*/
void RotatePointAroundVector( bvec3_t dst, const avec3_t dir, const bvec3_t point,
							 afixed degrees ) {
	afixed	m[3][3];
	afixed	im[3][3];
	afixed	zrot[3][3];
	afixed	tmpmat[3][3];
	afixed	rot[3][3];
	int	i;
	avec3_t vr, vup, vf;
	afixed	rad;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector( vr, dir );
	CrossProduct( vr, vf, vup );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = AFIXED_1;

	rad = DEG2RAD_A( degrees );
	zrot[0][0] = FIXED_COS( rad );
	zrot[0][1] = FIXED_SIN( rad );
	zrot[1][0] = -FIXED_SIN( rad );
	zrot[1][1] = FIXED_COS( rad );

	MatrixMultiply( m, zrot, tmpmat );
	MatrixMultiply( tmpmat, im, rot );

	for ( i = 0; i < 3; i++ ) {
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

#ifndef FIXED_IS_FLOAT
/*
===============
RotatePointAroundVector

This is not implemented very well...
===============
*/
void RotatePointAroundVector( avec3_t dst, const avec3_t dir, const avec3_t point,
							 afixed degrees ) {
	afixed	m[3][3];
	afixed	im[3][3];
	afixed	zrot[3][3];
	afixed	tmpmat[3][3];
	afixed	rot[3][3];
	int	i;
	avec3_t vr, vup, vf;
	afixed	rad;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector( vr, dir );
	CrossProduct( vr, vf, vup );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = AFIXED_1;

	rad = DEG2RAD_A( degrees );
	zrot[0][0] = FIXED_COS( rad );
	zrot[0][1] = FIXED_SIN( rad );
	zrot[1][0] = -FIXED_SIN( rad );
	zrot[1][1] = FIXED_COS( rad );

	MatrixMultiply( m, zrot, tmpmat );
	MatrixMultiply( tmpmat, im, rot );

	for ( i = 0; i < 3; i++ ) {
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

#endif

/*
===============
RotateAroundDirection
===============
*/
void RotateAroundDirection( avec3_t axis[3], afixed yaw ) {

	// create an arbitrary axis[1] 
	PerpendicularVector( axis[1], axis[0] );

	// rotate it around axis[0] by yaw
	if ( FIXED_NOT_ZERO(yaw) ) {
		avec3_t	temp;

		VectorCopy( axis[1], temp );
		RotatePointAroundVector( axis[1], axis[0], temp, yaw );
	}

	// cross to get axis[2]
	CrossProduct( axis[0], axis[1], axis[2] );
}



void vectoangles( const bvec3_t value1, avec3_t angles ) {
	bfixed	forward;
	afixed	yaw, pitch;
	
	if ( FIXED_IS_ZERO(value1[1]) && FIXED_IS_ZERO(value1[0])) {
		yaw = AFIXED_0;
		if ( value1[2] > BFIXED_0 ) {
			pitch = AFIXED(90,0);
		}
		else {
			pitch = AFIXED(270,0);
		}
	}
	else {
		if ( FIXED_NOT_ZERO(value1[0]) ) {
			yaw = MAKE_AFIXED( FIXED_ATAN2 ( value1[1], value1[0] )) * AFIXED(180,0) / AFIXED_PI ;
		}
		else if ( value1[1] > BFIXED_0 ) {
			yaw = AFIXED(90,0);
		}
		else {
			yaw = AFIXED(270,0);
		}
		if ( yaw < AFIXED_0 ) {
			yaw += AFIXED(360,0);
		}

		forward = FIXED_VEC2LEN(value1);
		pitch = MAKE_AFIXED( FIXED_ATAN2(value1[2], forward)) * AFIXED(180,0) / AFIXED_PI ;
		if ( pitch < AFIXED_0 ) {
			pitch += AFIXED(360,0);
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = AFIXED_0;
}

#ifndef FIXED_IS_FLOAT

void vectoangles( const avec3_t value1, avec3_t angles ) {
	afixed	forward;
	afixed	yaw, pitch;
	
	if ( FIXED_IS_ZERO(value1[1]) && FIXED_IS_ZERO(value1[0])) {
		yaw = AFIXED_0;
		if ( value1[2] > AFIXED_0 ) {
			pitch = AFIXED(90,0);
		}
		else {
			pitch = AFIXED(270,0);
		}
	}
	else {
		if ( FIXED_NOT_ZERO(value1[0]) ) {
			yaw = MAKE_AFIXED( FIXED_ATAN2 ( value1[1], value1[0] )) * AFIXED(180,0) / AFIXED_PI ;
		}
		else if ( value1[1] > AFIXED_0 ) {
			yaw = AFIXED(90,0);
		}
		else {
			yaw = AFIXED(270,0);
		}
		if ( yaw < AFIXED_0 ) {
			yaw += AFIXED(360,0);
		}

		forward = FIXED_VEC2LEN(value1);
		pitch = MAKE_AFIXED( FIXED_ATAN2(value1[2], forward)) * AFIXED(180,0) / AFIXED_PI ;
		if ( pitch < AFIXED_0 ) {
			pitch += AFIXED(360,0);
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = AFIXED_0;
}

#endif


/*
=================
AnglesToAxis
=================
*/
void AnglesToAxis( const avec3_t angles, avec3_t axis[3] ) {
	avec3_t	right;

	// angle vectors returns "right" instead of "y axis"
	AngleVectors( angles, axis[0], right, axis[2] );
	VectorSubtract( avec3_origin, right, axis[1] );
}

void AxisClear( avec3_t axis[3] ) {
	axis[0][0] = AFIXED_1;
	axis[0][1] = AFIXED_0;
	axis[0][2] = AFIXED_0;
	axis[1][0] = AFIXED_0;
	axis[1][1] = AFIXED_1;
	axis[1][2] = AFIXED_0;
	axis[2][0] = AFIXED_0;
	axis[2][1] = AFIXED_0;
	axis[2][2] = AFIXED_1;
}

void AxisCopy( avec3_t in[3], avec3_t out[3] ) {
	VectorCopy( in[0], out[0] );
	VectorCopy( in[1], out[1] );
	VectorCopy( in[2], out[2] );
}

void ProjectPointOnPlane( bvec3_t dst, const bvec3_t p, const avec3_t normal )
{
	bfixed d;
	avec3_t n;
	afixed inv_denom;


	inv_denom =  FIXED_VEC3LEN_SQ( normal );

	if(FIXED_NOT_ZERO(inv_denom))
	{
		inv_denom = AFIXED_1 / inv_denom;
	}
#ifdef _DEBUG
	else
	{
		DebugBreak();
	}
#endif
	
	d = (p[0]*normal[0]+p[1]*normal[1]+p[2]*normal[2]) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

#ifndef FIXED_IS_FLOAT

void ProjectPointOnPlane( avec3_t dst, const avec3_t p, const avec3_t normal )
{
	afixed d;
	avec3_t n;
	afixed inv_denom;

	inv_denom =  FIXED_VEC3LEN_SQ( normal );

	if(FIXED_NOT_ZERO(inv_denom))
	{
		inv_denom = AFIXED_1 / inv_denom;
	}
#ifdef _DEBUG
	else
	{
		DebugBreak();
	}
#endif

	
	d = (p[0]*normal[0]+p[1]*normal[1]+p[2]*normal[2]) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

#endif

/*
================
MakeNormalVectors

Given a normalized forward vector, create two
other perpendicular vectors
================
*/
void MakeNormalVectors( const bvec3_t forward, bvec3_t right, bvec3_t up) {
	bfixed		d;

	// this rotate and negate guarantees a vector
	// not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = FIXED_VEC3DOT (right, forward);
	FIXED_VEC3MA (right, -d, forward, right);
	VectorNormalize (right);
	CrossProduct (right, forward, up);
}

#ifndef FIXED_IS_FLOAT
void MakeNormalVectors( const avec3_t forward, avec3_t right, avec3_t up) {
	afixed		d;

	// this rotate and negate guarantees a vector
	// not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = FIXED_VEC3DOT (right, forward);
	FIXED_VEC3MA (right, -d, forward, right);
	VectorNormalize (right);
	CrossProduct (right, forward, up);
}
#endif




void VectorRotate( bvec3_t in, bvec3_t matrix[3], bvec3_t out )
{
	out[0] = FIXED_VEC3DOT( in, matrix[0] );
	out[1] = FIXED_VEC3DOT( in, matrix[1] );
	out[2] = FIXED_VEC3DOT( in, matrix[2] );
}

#ifndef FIXED_IS_FLOAT

void VectorRotate( bvec3_t in, avec3_t matrix[3], bvec3_t out )
{
	out[0] = FIXED_VEC3DOT( in, matrix[0] );
	out[1] = FIXED_VEC3DOT( in, matrix[1] );
	out[2] = FIXED_VEC3DOT( in, matrix[2] );
}

void VectorRotate( avec3_t in, avec3_t matrix[3], avec3_t out )
{
	out[0] = FIXED_VEC3DOT( in, matrix[0] );
	out[1] = FIXED_VEC3DOT( in, matrix[1] );
	out[2] = FIXED_VEC3DOT( in, matrix[2] );
}

#endif


//============================================================

/*
===============
LerpAngle

===============
*/
afixed LerpAngle (afixed from, afixed to, afixed frac) {
	afixed	a;

	if ( (to - from) > AFIXED(180,0) ) {
		to -= AFIXED(360,0);
	}
	if ( (to - from) < -AFIXED(180,0) ) {
		to += AFIXED(360,0);
	}
	a = from + frac * (to - from);

	return a;
}


/*
=================
AngleSubtract

Always returns a value from -180 to 180
=================
*/
afixed	AngleSubtract( afixed a1, afixed a2 ) {
	afixed	a;

	a = a1 - a2;
	while ( a > AFIXED(180,0) ) {
		a -= AFIXED(360,0);
	}
	while ( a < -AFIXED(180,0) ) {
		a += AFIXED(360,0);
	}
	return a;
}


void AnglesSubtract( avec3_t v1, avec3_t v2, avec3_t v3 ) {
	v3[0] = AngleSubtract( v1[0], v2[0] );
	v3[1] = AngleSubtract( v1[1], v2[1] );
	v3[2] = AngleSubtract( v1[2], v2[2] );
}


/*
=================
AngleNormalize360

returns angle normalized to the range [0 <= angle < 360]
=================
*/
afixed AngleNormalize360 ( afixed angle ) {
	return FIXED_MOD_CLAMP(angle,AFIXED(360,0));
}


/*
=================
AngleNormalize180

returns angle normalized to the range [-180 < angle <= 180]
=================
*/
afixed AngleNormalize180 ( afixed angle ) {
	angle = AngleNormalize360( angle );
	if ( angle > AFIXED(180,0) ) {
		angle -= AFIXED(360,0);
	}
	return angle;
}


/*
=================
AngleDelta

returns the normalized delta from angle1 to angle2
=================
*/
afixed AngleDelta ( afixed angle1, afixed angle2 ) {
	return AngleNormalize180( angle1 - angle2 );
}


//============================================================


/*
=================
SetPlaneSignbits
=================
*/
void SetPlaneSignbits (cplane_t *out) {
	int	bits, j;

	// for fast box on planeside test
	bits = 0;
	for (j=0 ; j<3 ; j++) {
		if (out->normal[j] < AFIXED_0) {
			bits |= 1<<j;
		}
	}
	out->signbits = bits;
}



/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
*/
/*
// this is the slow, general version
int BoxOnPlaneSide (bvec3_t emins, bvec3_t emaxs, struct cplane_s *p)
{
	int		i;
	bfixed	dist1, dist2;
	int		sides;
	bvec3_t	corners[2];

	for (i=0 ; i<3 ; i++)
	{
		if (p->normal[i] < AFIXED_0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist1 = FIXED_VEC3DOT_R (p->normal, corners[0]) - p->dist;
	dist2 = FIXED_VEC3DOT_R (p->normal, corners[1]) - p->dist;
	sides = 0;
	if (dist1 >= BFIXED_0)
		sides = 1;
	if (dist2 < BFIXED_0)
		sides |= 2;

	return sides;
}
*/

int BoxOnPlaneSide (bvec3_t emins, bvec3_t emaxs, struct cplane_s *p)
{
	bfixed	dist1, dist2;
	int		sides;

// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}

// general case
	switch (p->signbits)
	{
	case 0:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = BFIXED_0;		// shut up compiler
		break;
	}

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

	return sides;
}

/*
=================
RadiusFromBounds
=================
*/
gfixed RadiusFromBounds( const vec3_t mins, const vec3_t maxs ) {
	int		i;
	vec3_t	corner;
	gfixed	a, b;

	for (i=0 ; i<3 ; i++) {
		a = FIXED_ABS( mins[i] );
		b = FIXED_ABS( maxs[i] );
		corner[i] = (a > b) ? a : b;
	}

	return FIXED_VEC3LEN (corner);
}

#ifndef FIXED_IS_FLOAT
bfixed RadiusFromBounds( const bvec3_t mins, const bvec3_t maxs ) {
	int		i;
	bvec3_t	corner;
	bfixed	a, b;

	for (i=0 ; i<3 ; i++) {
		a = FIXED_ABS( mins[i] );
		b = FIXED_ABS( maxs[i] );
		corner[i] = (a > b) ? a : b;
	}

	return FIXED_VEC3LEN (corner);
}
#endif


void ClearBounds( vec3_t mins, vec3_t maxs ) {
	mins[0] = mins[1] = mins[2] = GFIXED(32767,0);//GFIXED(99999,0);
	maxs[0] = maxs[1] = maxs[2] = -GFIXED(32767,0);//-GFIXED(99999,0);
}

#ifndef FIXED_IS_FLOAT
void ClearBounds( bvec3_t mins, bvec3_t maxs ) {
	mins[0] = mins[1] = mins[2] = BFIXED(99999,0);
	maxs[0] = maxs[1] = maxs[2] = -BFIXED(99999,0);
}
#endif

void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs ) {
	if ( v[0] < mins[0] ) {
		mins[0] = v[0];
	}
	if ( v[0] > maxs[0]) {
		maxs[0] = v[0];
	}

	if ( v[1] < mins[1] ) {
		mins[1] = v[1];
	}
	if ( v[1] > maxs[1]) {
		maxs[1] = v[1];
	}

	if ( v[2] < mins[2] ) {
		mins[2] = v[2];
	}
	if ( v[2] > maxs[2]) {
		maxs[2] = v[2];
	}
}

#ifndef FIXED_IS_FLOAT

void AddPointToBounds( const bvec3_t v, bvec3_t mins, bvec3_t maxs ) {
	if ( v[0] < mins[0] ) {
		mins[0] = v[0];
	}
	if ( v[0] > maxs[0]) {
		maxs[0] = v[0];
	}

	if ( v[1] < mins[1] ) {
		mins[1] = v[1];
	}
	if ( v[1] > maxs[1]) {
		maxs[1] = v[1];
	}

	if ( v[2] < mins[2] ) {
		mins[2] = v[2];
	}
	if ( v[2] > maxs[2]) {
		maxs[2] = v[2];
	}
}
#endif

bfixed VectorNormalizeB2A(const bvec3_t bv, avec3_t av)
{
	lfixed	length, lvec[3];
	lvec[0]=MAKE_LFIXED(bv[0]);lvec[1]=MAKE_LFIXED(bv[1]);lvec[2]=MAKE_LFIXED(bv[2]);
	length = FIXED_VEC3LEN(lvec);
	if (FIXED_NOT_ZERO(length))
	{
		av[0] = MAKE_AFIXED(lvec[0]/length);
		av[1] = MAKE_AFIXED(lvec[1]/length);
		av[2] = MAKE_AFIXED(lvec[2]/length);
	} else {
		VectorClear( av );
	}
	return MAKE_BFIXED(length);
}

afixed VectorNormalizeA2B(const avec3_t av, bvec3_t bv)
{
	lfixed	length, lvec[3];
	lvec[0]=MAKE_LFIXED(av[0]);lvec[1]=MAKE_LFIXED(av[1]);lvec[2]=MAKE_LFIXED(av[2]);
	length = FIXED_VEC3LEN(lvec);
	if (FIXED_NOT_ZERO(length))
	{
		bv[0] = MAKE_BFIXED(lvec[0]/length);
		bv[1] = MAKE_BFIXED(lvec[1]/length);
		bv[2] = MAKE_BFIXED(lvec[2]/length);
	} else {
		VectorClear( bv );
	}
	return MAKE_AFIXED(length);
}


lfixed VectorNormalizeLong2( const vec3_t v, vec3_t out) {
	lfixed	length, lvec[3];
	lvec[0]=MAKE_LFIXED(v[0]);lvec[1]=MAKE_LFIXED(v[1]);lvec[2]=MAKE_LFIXED(v[2]);
	length = FIXED_VEC3LEN(lvec);
	if (FIXED_NOT_ZERO(length))
	{
		out[0] = MAKE_GFIXED(lvec[0]/length);
		out[1] = MAKE_GFIXED(lvec[1]/length);
		out[2] = MAKE_GFIXED(lvec[2]/length);
	} else {
		VectorClear( out );
	}
	
	return length;
}


#ifndef FIXED_IS_FLOAT

lfixed VectorNormalizeLong2( const avec3_t v, avec3_t out) {
	lfixed	length,lvec[3];
	lvec[0]=MAKE_LFIXED(v[0]);lvec[1]=MAKE_LFIXED(v[1]);lvec[2]=MAKE_LFIXED(v[2]);
	length = FIXED_VEC3LEN(lvec);

	if (FIXED_NOT_ZERO(length))
	{
		out[0] = MAKE_AFIXED(lvec[0]/length);
		out[1] = MAKE_AFIXED(lvec[1]/length);
		out[2] = MAKE_AFIXED(lvec[2]/length);
	} else {
		VectorClear( out );
	}

	return length;
}

lfixed VectorNormalizeLong2( const bvec3_t v, bvec3_t out) {
	lfixed	length,lvec[3];
	lvec[0]=MAKE_LFIXED(v[0]);lvec[1]=MAKE_LFIXED(v[1]);lvec[2]=MAKE_LFIXED(v[2]);
	length = FIXED_VEC3LEN(lvec);
	if (FIXED_NOT_ZERO(length))
	{
		out[0] = MAKE_BFIXED(lvec[0]/length);
		out[1] = MAKE_BFIXED(lvec[1]/length);
		out[2] = MAKE_BFIXED(lvec[2]/length);
	} else {
		VectorClear( out );
	}
		
	return length;
}





#endif

int Q_log2( int val ) {
	int answer;

	answer = 0;
	while ( ( val>>=1 ) != 0 ) {
		answer++;
	}
	return answer;
}



/*
=================
PlaneTypeForNormal
=================
*/
/*
int	PlaneTypeForNormal (bvec3_t normal) {
	if ( normal[0] == GFIXED_1 )
		return PLANE_X;
	if ( normal[1] == GFIXED_1 )
		return PLANE_Y;
	if ( normal[2] == GFIXED_1 )
		return PLANE_Z;
	
	return PLANE_NON_AXIAL;
}
*/


/*
================
MatrixMultiply
================
*/

void MatrixMultiply(bmat_33 &in1, bmat_33 &in2, bmat_33 &out) 
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}

#ifndef FIXED_IS_FLOAT


/*
================
MatrixMultiply
================
*/

void MatrixMultiply(amat_33 &in1, amat_33 &in2, amat_33 &out) 
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}



void MatrixMultiply(bmat_33 &in1, amat_33 &in2, bmat_33 &out) 
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}

#endif

void AngleVectors( const avec3_t angles, avec3_t forward, avec3_t right, avec3_t up) 
{
	afixed		angle;
	static afixed		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * (AFIXED_PI * AFIXED(2,0) / AFIXED(360,0));
	sy = FIXED_SIN(angle);
	cy = FIXED_COS(angle);
	angle = angles[PITCH] * (AFIXED_PI * AFIXED(2,0) / AFIXED(360,0));
	sp = FIXED_SIN(angle);
	cp = FIXED_COS(angle);
	angle = angles[ROLL] * (AFIXED_PI * AFIXED(2,0) / AFIXED(360,0));
	sr = FIXED_SIN(angle);
	cr = FIXED_COS(angle);

	if (forward)
	{
		forward[0] = (cp*cy);
		forward[1] = (cp*sy);
		forward[2] = -(sp);
	}
	if (right)
	{
		right[0] = (((-AFIXED_1)*sr*sp*cy+(-AFIXED_1)*cr*-sy));
		right[1] = (((-AFIXED_1)*sr*sp*sy+(-AFIXED_1)*cr*cy));
		right[2] = (-AFIXED_1*sr*cp);
	}
	if (up)
	{
		up[0] = ((cr*sp*cy+-sr*-sy));
		up[1] = ((cr*sp*sy+-sr*cy));
		up[2] = (cr*cp);
	}
}

void AngleVectors( const avec3_t angles, bvec3_t forward, bvec3_t right, bvec3_t up) 
{
	afixed		angle;
	static afixed		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * (AFIXED_PI * AFIXED(2,0) / AFIXED(360,0));
	sy = FIXED_SIN(angle);
	cy = FIXED_COS(angle);
	angle = angles[PITCH] * (AFIXED_PI * AFIXED(2,0) / AFIXED(360,0));
	sp = FIXED_SIN(angle);
	cp = FIXED_COS(angle);
	angle = angles[ROLL] * (AFIXED_PI * AFIXED(2,0) / AFIXED(360,0));
	sr = FIXED_SIN(angle);
	cr = FIXED_COS(angle);

	if (forward)
	{
		forward[0] = MAKE_BFIXED(cp*cy);
		forward[1] = MAKE_BFIXED(cp*sy);
		forward[2] = -MAKE_BFIXED(sp);
	}
	if (right)
	{
		right[0] = MAKE_BFIXED(((-AFIXED_1)*sr*sp*cy+(-AFIXED_1)*cr*-sy));
		right[1] = MAKE_BFIXED(((-AFIXED_1)*sr*sp*sy+(-AFIXED_1)*cr*cy));
		right[2] = MAKE_BFIXED(-AFIXED_1*sr*cp);
	}
	if (up)
	{
		up[0] = MAKE_BFIXED((cr*sp*cy+-sr*-sy));
		up[1] = MAKE_BFIXED((cr*sp*sy+-sr*cy));
		up[2] = MAKE_BFIXED(cr*cp);
	}
}



void PerpendicularVector( bvec3_t dst, const bvec3_t src )
{
	int	pos;
	int i;
	bfixed minelem = BFIXED_1;
	avec3_t tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for ( pos = -1, i = 0; i < 3; i++ )
	{
		if ( pos==-1 || FIXED_ABS( src[i] ) < minelem )
		{
			pos = i;
			minelem = FIXED_ABS( src[i] );
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = AFIXED_0;
	tempvec[pos] = AFIXED_1;

	/*
	** project the point onto the plane defined by src
	*/
	avec3_t asrc;
	VectorNormalizeB2A(src,asrc);

	avec3_t adst;
	ProjectPointOnPlane( adst, tempvec, asrc );

	/*
	** normalize the result
	*/
	VectorNormalizeA2B( adst, dst );
}

#ifndef FIXED_IS_FLOAT

void PerpendicularVector( avec3_t dst, const avec3_t src )
{
	int	pos;
	int i;
	afixed minelem = AFIXED_1;
	avec3_t tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for ( pos = -1, i = 0; i < 3; i++ )
	{
		if ( pos==-1 || FIXED_ABS( src[i] ) < minelem )
		{
			pos = i;
			minelem = FIXED_ABS( src[i] );
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = AFIXED_0;
	tempvec[pos] = AFIXED_1;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlane( dst, tempvec, src );

	/*
	** normalize the result
	*/
	VectorNormalize( dst );
}

#endif



