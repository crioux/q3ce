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
#include"splines_pch.h"


#define LERP_DELTA GFIXED(0,000001)			//1e-6

idVec3_t vec_zero( GFIXED_0, GFIXED_0, GFIXED_0 );

Bounds	boundsZero;

gfixed idVec3_t::toYaw( void ) {
	gfixed yaw;
	
	if ( FIXED_IS_ZERO(y) && FIXED_IS_ZERO(x) ) {
		yaw = GFIXED_0;
	} else {
		yaw = FIXED_ATAN2( y, x ) * GFIXED(180,0) / GFIXED_PI;
		if ( yaw < GFIXED_0 ) {
			yaw += GFIXED(360,0);
		}
	}

	return yaw;
}

gfixed idVec3_t::toPitch( void ) {
	gfixed	forward;
	gfixed	pitch;
	
	if ( FIXED_IS_ZERO(x) && FIXED_IS_ZERO(y) ) {
		if ( z > GFIXED_0 ) {
			pitch = GFIXED(90,0);
		} else {
			pitch = GFIXED(270,0);
		}
	} else {
		gfixed vec[2];
		vec[0]=x;
		vec[1]=y;
		forward = FIXED_VEC2LEN(vec);
		pitch = FIXED_ATAN2( z, forward ) * GFIXED(180,0) / GFIXED_PI;
		if ( pitch < GFIXED_0 ) {
			pitch += GFIXED(360,0);
		}
	}

	return pitch;
}

/*
angles_t idVec3_t::toAngles( void ) {
	gfixed forward;
	gfixed yaw;
	gfixed pitch;
	
	if ( ( x == 0 ) && ( y == 0 ) ) {
		yaw = 0;
		if ( z > 0 ) {
			pitch = 90;
		} else {
			pitch = 270;
		}
	} else {
		yaw = atan2( y, x ) * 180 / GFIXED_PI;
		if ( yaw < 0 ) {
			yaw += 360;
		}

		forward = ( gfixed )LFIXED_SQRT( x * x + y * y );
		pitch = atan2( z, forward ) * 180 / GFIXED_PI;
		if ( pitch < 0 ) {
			pitch += 360;
		}
	}

	return angles_t( -pitch, yaw, 0 );
}
*/

idVec3_t LerpVector( idVec3_t &w1, idVec3_t &w2, const gfixed t ) {
	gfixed omega, cosom, sinom, scale0, scale1;

	cosom = w1 * w2;
	if ( ( GFIXED_1 - cosom ) > LERP_DELTA ) {
		omega = FIXED_ACOS( cosom );
		sinom = FIXED_SIN( omega );
		scale0 = FIXED_SIN( ( GFIXED_1 - t ) * omega ) / sinom;
		scale1 = FIXED_SIN( t * omega ) / sinom;
	} else {
		scale0 = GFIXED_1 - t;
		scale1 = t;
	}

	return ( w1 * scale0 + w2 * scale1 );
}

/*
=============
idVec3_t::string

This is just a convenience function
for printing vectors
=============
*/
char *idVec3_t::string( void ) {
	static	int		index = 0;
	static	char	str[ 8 ][ 36 ];
	char	*s;

	// use an array so that multiple toString's won't collide
	s = str[ index ];
	index = (index + 1)&7;

	sprintf( s, "%.2f %.2f %.2f", x, y, z );

	return s;
}
