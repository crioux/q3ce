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
#ifndef __MATH_VECTOR_H__
#define __MATH_VECTOR_H__

#if defined(_WIN32)
#pragma warning(disable : 4244)
#endif

#include <math.h>
#include <assert.h>
#include "../qcommon/fixed.h"

//#define FIXED_VEC3DOT(a,b)			((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
//#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
//#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
//#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
//#define VectorCopy(a,b)			((b).x=(a).x,(b).y=(a).y,(b).z=(a).z])

//#define	FIXED_VEC3SCALE(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	__VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))
//#define CrossProduct(a,b,c)		((c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1],(c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2],(c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0])

#define DotProduct4(x,y)		((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2]+(x)[3]*(y)[3])
#define VectorSubtract4(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2],(c)[3]=(a)[3]-(b)[3])
#define VectorAdd4(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2],(c)[3]=(a)[3]+(b)[3])
#define VectorCopy4(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define	VectorScale4(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s),(o)[3]=(v)[3]*(s))
#define	VectorMA4(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s),(o)[3]=(v)[3]+(b)[3]*(s))


//#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
//#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define Vector4Copy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])


//#include "util_heap.h"

#ifndef EQUAL_EPSILON
#define EQUAL_EPSILON	GFIXED(0,001)
#endif


#ifndef ID_INLINE
#ifdef _WIN32
#define ID_INLINE __inline 
#else
#define ID_INLINE inline
#endif
#endif

// if this is defined, vec3 will take four elements, which may allow
// easier SIMD optimizations
//#define	FAT_VEC3
//#ifdef __ppc__
//#pragma align(16)
//#endif

class angles_t;



//class idVec3_t  : public idHeap<idVec3_t> {
class idVec3_t {
public:	
#ifndef	FAT_VEC3
	    gfixed x,y,z;
#else
	    gfixed x,y,z,dist;
#endif

#ifndef	FAT_VEC3
					idVec3_t() {};
#else
					idVec3_t() {dist = GFIXED_0;};
#endif
					idVec3_t( const gfixed x, const gfixed y, const gfixed z );

					operator gfixed *();

	gfixed			operator[]( const int index ) const;
	gfixed			&operator[]( const int index );

	void 			set( const gfixed x, const gfixed y, const gfixed z );

	idVec3_t			operator-() const;

	idVec3_t			&operator=( const idVec3_t &a );

	gfixed			operator*( const idVec3_t &a ) const;
	idVec3_t			operator*( const gfixed a ) const;
	friend idVec3_t	operator*( gfixed a, idVec3_t b );

	idVec3_t			operator+( const idVec3_t &a ) const;
	idVec3_t			operator-( const idVec3_t &a ) const;
	
	idVec3_t			&operator+=( const idVec3_t &a );
	idVec3_t			&operator-=( const idVec3_t &a );
	idVec3_t			&operator*=( const gfixed a );

	int				operator==(	const idVec3_t &a ) const;
	int				operator!=(	const idVec3_t &a ) const;

	idVec3_t			Cross( const idVec3_t &a ) const;
	idVec3_t			&Cross( const idVec3_t &a, const idVec3_t &b );

	gfixed			Length( void ) const;
	gfixed			Normalize( void );

	void			Zero( void );
	void			Snap( void );
	void			SnapTowards( const idVec3_t &to );

	gfixed			toYaw( void );
	gfixed			toPitch( void );
	angles_t		toAngles( void );
	friend idVec3_t	LerpVector( const idVec3_t &w1, const idVec3_t &w2, const gfixed t );

	char			*string( void );
};

extern idVec3_t vec_zero;

ID_INLINE idVec3_t::idVec3_t( const gfixed x, const gfixed y, const gfixed z ) {
	this->x = x;
	this->y = y;
	this->z = z;
#ifdef	FAT_VEC3
	this->dist = GFIXED_0;
#endif
}

ID_INLINE gfixed idVec3_t::operator[]( const int index ) const {
	return ( &x )[ index ];
}

ID_INLINE gfixed &idVec3_t::operator[]( const int index ) {
	return ( &x )[ index ];
}

ID_INLINE idVec3_t::operator gfixed *( void ) {
	return &x;
}

ID_INLINE idVec3_t idVec3_t::operator-() const {
	return idVec3_t( -x, -y, -z );
}
	
ID_INLINE idVec3_t &idVec3_t::operator=( const idVec3_t &a ) { 
	x = a.x;
	y = a.y;
	z = a.z;
	
	return *this;
}

ID_INLINE void idVec3_t::set( const gfixed x, const gfixed y, const gfixed z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}

ID_INLINE idVec3_t idVec3_t::operator-( const idVec3_t &a ) const {
	return idVec3_t( x - a.x, y - a.y, z - a.z );
}

ID_INLINE gfixed idVec3_t::operator*( const idVec3_t &a ) const {
	return x * a.x + y * a.y + z * a.z;
}

ID_INLINE idVec3_t idVec3_t::operator*( const gfixed a ) const {
	return idVec3_t( x * a, y * a, z * a );
}

ID_INLINE idVec3_t operator*( const gfixed a, const idVec3_t b ) {
	return idVec3_t( b.x * a, b.y * a, b.z * a );
}

ID_INLINE idVec3_t idVec3_t::operator+( const idVec3_t &a ) const {
	return idVec3_t( x + a.x, y + a.y, z + a.z );
}

ID_INLINE idVec3_t &idVec3_t::operator+=( const idVec3_t &a ) {
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

ID_INLINE idVec3_t &idVec3_t::operator-=( const idVec3_t &a ) {
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

ID_INLINE idVec3_t &idVec3_t::operator*=( const gfixed a ) {
	x *= a;
	y *= a;
	z *= a;

	return *this;
}

ID_INLINE int idVec3_t::operator==( const idVec3_t &a ) const {
	if ( FIXED_ABS( x - a.x ) > EQUAL_EPSILON ) {
		return false;
	}
			
	if ( FIXED_ABS( y - a.y ) > EQUAL_EPSILON ) {
		return false;
	}

	if ( FIXED_ABS( z - a.z ) > EQUAL_EPSILON ) {
		return false;
	}

	return true;
}

ID_INLINE int idVec3_t::operator!=( const idVec3_t &a ) const {
	if ( FIXED_ABS( x - a.x ) > EQUAL_EPSILON ) {
		return true;
	}
			
	if ( FIXED_ABS( y - a.y ) > EQUAL_EPSILON ) {
		return true;
	}

	if ( FIXED_ABS( z - a.z ) > EQUAL_EPSILON ) {
		return true;
	}

	return false;
}

ID_INLINE idVec3_t idVec3_t::Cross( const idVec3_t &a ) const {
	return idVec3_t( y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x );
}

ID_INLINE idVec3_t &idVec3_t::Cross( const idVec3_t &a, const idVec3_t &b ) {
	x = a.y * b.z - a.z * b.y;
	y = a.z * b.x - a.x * b.z;
	z = a.x * b.y - a.y * b.x;

	return *this;
}

ID_INLINE gfixed idVec3_t::Length( void ) const {
	gfixed vec[3];
	vec[0]=x;
	vec[1]=y;
	vec[2]=z;
	return FIXED_VEC3LEN(vec);
}

ID_INLINE gfixed idVec3_t::Normalize( void ) {
	gfixed length;
	gfixed ilength;

	length = this->Length();
	if ( FIXED_NOT_ZERO(length) ) {
		ilength = GFIXED_1 / length;
		x *= ilength;
		y *= ilength;
		z *= ilength;
	}
		
	return length;
}

ID_INLINE void idVec3_t::Zero( void ) {
	x = GFIXED_0;
	y = GFIXED_0;
	z = GFIXED_0;
}

ID_INLINE void idVec3_t::Snap( void ) {
	x = FIXED_SNAP( x );
	y = FIXED_SNAP( y );
	z = FIXED_SNAP( z );
}

/*
======================
SnapTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating 
into a wall.
======================
*/
ID_INLINE void idVec3_t::SnapTowards( const idVec3_t &to ) {
	if ( to.x <= x ) {
		x = FIXED_SNAP( x );
	} else {
		x = FIXED_SNAP( x ) + GFIXED_1;
	}

	if ( to.y <= y ) {
		y = FIXED_SNAP( y );
	} else {
		y = FIXED_SNAP( y ) + GFIXED_1;
	}

	if ( to.z <= z ) {
		z = FIXED_SNAP( z ) ;
	} else {
		z = FIXED_SNAP( z ) + GFIXED_1;
	}
}

//===============================================================

class Bounds {
public:
	idVec3_t	b[2];

			Bounds();
			Bounds( const idVec3_t &mins, const idVec3_t &maxs );

	void	Clear();
	void	Zero();
	gfixed	Radius();		// radius from origin, not from center
	idVec3_t	Center();
	void	AddPoint( const idVec3_t &v );
	void	AddBounds( const Bounds &bb );
	bool	IsCleared();
	bool	ContainsPoint( const idVec3_t &p );
	bool	IntersectsBounds( const Bounds &b2 );	// touching is NOT intersecting
};

extern Bounds	boundsZero;

ID_INLINE Bounds::Bounds(){
}

ID_INLINE bool Bounds::IsCleared() {
	return b[0][0] > b[1][0];
}

ID_INLINE bool Bounds::ContainsPoint( const idVec3_t &p ) {
	if ( p[0] < b[0][0] || p[1] < b[0][1] || p[2] < b[0][2]
		|| p[0] > b[1][0] || p[1] > b[1][1] || p[2] > b[1][2] ) {
		return false;
	}
	return true;
}

ID_INLINE bool Bounds::IntersectsBounds( const Bounds &b2 ) {
	if ( b2.b[1][0] < b[0][0] || b2.b[1][1] < b[0][1] || b2.b[1][2] < b[0][2]
		|| b2.b[0][0] > b[1][0] || b2.b[0][1] > b[1][1] || b2.b[0][2] > b[1][2] ) {
		return false;
	}
	return true;
}

ID_INLINE Bounds::Bounds( const idVec3_t &mins, const idVec3_t &maxs ) {
	b[0] = mins;
	b[1] = maxs;
}

ID_INLINE idVec3_t Bounds::Center() {
	return idVec3_t( ( b[1][0] + b[0][0] ) * GFIXED(0,5), ( b[1][1] + b[0][1] ) * GFIXED(0,5), ( b[1][2] + b[0][2] ) * GFIXED(0,5) );
}

ID_INLINE void Bounds::Clear() {
	b[0][0] = b[0][1] = b[0][2] = MAKE_GFIXED(99999);
	b[1][0] = b[1][1] = b[1][2] = MAKE_GFIXED(-99999);
}

ID_INLINE void Bounds::Zero() {
	b[0][0] = b[0][1] = b[0][2] =
	b[1][0] = b[1][1] = b[1][2] = MAKE_GFIXED(0);
}

ID_INLINE void Bounds::AddPoint( const idVec3_t &v ) {
	if ( v[0] < b[0][0]) {
		b[0][0] = v[0];
	}
	if ( v[0] > b[1][0]) {
		b[1][0] = v[0];
	}
	if ( v[1] < b[0][1] ) {
		b[0][1] = v[1];
	}
	if ( v[1] > b[1][1]) {
		b[1][1] = v[1];
	}
	if ( v[2] < b[0][2] ) {
		b[0][2] = v[2];
	}
	if ( v[2] > b[1][2]) {
		b[1][2] = v[2];
	}
}


ID_INLINE void Bounds::AddBounds( const Bounds &bb ) {
	if ( bb.b[0][0] < b[0][0]) {
		b[0][0] = bb.b[0][0];
	}
	if ( bb.b[0][1] < b[0][1]) {
		b[0][1] = bb.b[0][1];
	}
	if ( bb.b[0][2] < b[0][2]) {
		b[0][2] = bb.b[0][2];
	}

	if ( bb.b[1][0] > b[1][0]) {
		b[1][0] = bb.b[1][0];
	}
	if ( bb.b[1][1] > b[1][1]) {
		b[1][1] = bb.b[1][1];
	}
	if ( bb.b[1][2] > b[1][2]) {
		b[1][2] = bb.b[1][2];
	}
}

ID_INLINE gfixed Bounds::Radius( ) {
	int		i;
	gfixed	total;
	gfixed	a, aa;

	total = GFIXED_0;
	for (i=0 ; i<3 ; i++) {
		a = FIXED_ABS( b[0][i] );
		aa = FIXED_ABS( b[1][i] );
		if ( aa > a ) {
			a = aa;
		}
		total += a * a;
	}

	return FIXED_SQRT( total );
}

//===============================================================


class idVec2_t {
public:
	gfixed			x;
	gfixed			y;

					operator gfixed *();
	gfixed			operator[]( int index ) const;
	gfixed			&operator[]( int index );
};

ID_INLINE gfixed idVec2_t::operator[]( int index ) const {
	return ( &x )[ index ];
}

ID_INLINE gfixed& idVec2_t::operator[]( int index ) {
	return ( &x )[ index ];
}

ID_INLINE idVec2_t::operator gfixed *( void ) {
	return &x;
}

class mvec4_t : public idVec3_t {
public:
#ifndef	FAT_VEC3
	gfixed			dist;
#endif
	mvec4_t();
	~mvec4_t() {};
	
	mvec4_t( gfixed x, gfixed y, gfixed z, gfixed dist );
	gfixed			operator[]( int index ) const;
	gfixed			&operator[]( int index );
};

ID_INLINE mvec4_t::mvec4_t() {}
ID_INLINE mvec4_t::mvec4_t( gfixed x, gfixed y, gfixed z, gfixed dist ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->dist = dist;
}

ID_INLINE gfixed mvec4_t::operator[]( int index ) const {
	return ( &x )[ index ];
}

ID_INLINE gfixed& mvec4_t::operator[]( int index ) {
	return ( &x )[ index ];
}


class idVec5_t : public idVec3_t {
public:
	gfixed			s;
	gfixed			t;
	gfixed			operator[]( int index ) const;
	gfixed			&operator[]( int index );
};


ID_INLINE gfixed idVec5_t::operator[]( int index ) const {
	return ( &x )[ index ];
}

ID_INLINE gfixed& idVec5_t::operator[]( int index ) {
	return ( &x )[ index ];
}

#endif /* !__MATH_VECTOR_H__ */
