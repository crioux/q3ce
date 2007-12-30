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
#ifndef __MATH_QUATERNION_H__
#define __MATH_QUATERNION_H__

#include <assert.h>
#include <math.h>
#include "../qcommon/fixed.h"

class idVec3_t;
class angles_t;
class mat3_t;

class quat_t {
public:
	gfixed			x;
	gfixed			y;
	gfixed			z;
	gfixed			w;

					quat_t();
					quat_t( gfixed x, gfixed y, gfixed z, gfixed w );

	friend void		toQuat( idVec3_t &src, quat_t &dst );
	friend void		toQuat( angles_t &src, quat_t &dst );
	friend void		toQuat( mat3_t &src, quat_t &dst );

	gfixed			*vec4( void );
			
	gfixed			operator[]( int index ) const;
	gfixed			&operator[]( int index );

	void 			set( gfixed x, gfixed y, gfixed z, gfixed w );

	void			operator=( quat_t a );

	friend quat_t	operator+( quat_t a, quat_t b );
	quat_t			&operator+=( quat_t a );

	friend quat_t	operator-( quat_t a, quat_t b );
	quat_t			&operator-=( quat_t a );

	friend quat_t	operator*( quat_t a, gfixed b );
	friend quat_t	operator*( gfixed a, quat_t b );
	quat_t			&operator*=( gfixed a );

	friend int		operator==(	quat_t a, quat_t b );
	friend int		operator!=(	quat_t a, quat_t b );

	gfixed			Length( void );
	quat_t			&Normalize( void );

	quat_t			operator-();
};

inline quat_t::quat_t() {
}

inline quat_t::quat_t( gfixed x, gfixed y, gfixed z, gfixed w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

inline gfixed *quat_t::vec4( void ) {
	return &x;
}

inline gfixed quat_t::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index < 4 ) );
	return ( &x )[ index ];
}

inline gfixed& quat_t::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index < 4 ) );
	return ( &x )[ index ];
}

inline void quat_t::set( gfixed x, gfixed y, gfixed z, gfixed w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

inline void quat_t::operator=( quat_t a ) {
	x = a.x;
	y = a.y;
	z = a.z;
	w = a.w;
}

inline quat_t operator+( quat_t a, quat_t b ) {
	return quat_t( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w );
}

inline quat_t& quat_t::operator+=( quat_t a ) {
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;

	return *this;
}

inline quat_t operator-( quat_t a, quat_t b ) {
	return quat_t( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w );
}

inline quat_t& quat_t::operator-=( quat_t a ) {
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;

	return *this;
}

inline quat_t operator*( quat_t a, gfixed b ) {
	return quat_t( a.x * b, a.y * b, a.z * b, a.w * b );
}

inline quat_t operator*( gfixed a, quat_t b ) {
	return b * a;
}

inline quat_t& quat_t::operator*=( gfixed a ) {
	x *= a;
	y *= a;
	z *= a;
	w *= a;

	return *this;
}

inline int operator==( quat_t a, quat_t b ) {
	return ( ( a.x == b.x ) && ( a.y == b.y ) && ( a.z == b.z ) && ( a.w == b.w ) );
}

inline int operator!=( quat_t a, quat_t b ) {
	return ( ( a.x != b.x ) || ( a.y != b.y ) || ( a.z != b.z ) && ( a.w != b.w ) );
}

inline gfixed quat_t::Length( void ) {
	gfixed vec[4];
	vec[0]=x;
	vec[1]=y;
	vec[2]=z;
	vec[3]=w;
	return FIXED_VEC4LEN(vec);
}

inline quat_t& quat_t::Normalize( void ) {
	gfixed length;
	gfixed ilength;

	length = this->Length();
	if ( FIXED_NOT_ZERO(length) ) {
		ilength = GFIXED_1 / length;
		x *= ilength;
		y *= ilength;
		z *= ilength;
		w *= ilength;
	}
		
	return *this;
}

inline quat_t quat_t::operator-() {
	return quat_t( -x, -y, -z, -w );
}

#endif /* !__MATH_QUATERNION_H__ */
