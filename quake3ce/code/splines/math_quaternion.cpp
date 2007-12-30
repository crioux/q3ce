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


void toQuat( idVec3_t &src, quat_t &dst ) {
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
	dst.w = GFIXED_0;
}

void toQuat( angles_t &src, quat_t &dst ) {
	mat3_t temp;

	toMatrix( src, temp );
	toQuat( temp, dst );
}

void toQuat( mat3_t &src, quat_t &dst ) {
	gfixed		trace;
	gfixed		s;
	int     	i;
	int			j;
	int			k;

	static int 	next[ 3 ] = { 1, 2, 0 };

	trace = src[ 0 ][ 0 ] + src[ 1 ][ 1 ] + src[ 2 ][ 2 ];
	if ( trace > GFIXED_0 ) {
		s = FIXED_SQRT( trace + GFIXED_1 );
		dst.w = s * GFIXED(0,5);
		s = GFIXED(0,5) / s;
    
		dst.x = ( src[ 2 ][ 1 ] - src[ 1 ][ 2 ] ) * s;
		dst.y = ( src[ 0 ][ 2 ] - src[ 2 ][ 0 ] ) * s;
		dst.z = ( src[ 1 ][ 0 ] - src[ 0 ][ 1 ] ) * s;
	} else {
		i = 0;
		if ( src[ 1 ][ 1 ] > src[ 0 ][ 0 ] ) {
			i = 1;
		}
		if ( src[ 2 ][ 2 ] > src[ i ][ i ] ) {
			i = 2;
		}

		j = next[ i ];  
		k = next[ j ];
    
		s = FIXED_SQRT( ( src[ i ][ i ] - ( src[ j ][ j ] + src[ k ][ k ] ) ) + GFIXED_1 );
		dst[ i ] = s * GFIXED(0,5);
    
		s = GFIXED(0,5) / s;
    
		dst.w		= ( src[ k ][ j ] - src[ j ][ k ] ) * s;
		dst[ j ]	= ( src[ j ][ i ] + src[ i ][ j ] ) * s;
		dst[ k ]	= ( src[ k ][ i ] + src[ i ][ k ] ) * s;
	}
}
