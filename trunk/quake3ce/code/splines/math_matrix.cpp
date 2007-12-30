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


mat3_t mat3_default( idVec3_t( GFIXED_1, GFIXED_0, GFIXED_0 ), idVec3_t( GFIXED_0, GFIXED_1, GFIXED_0 ), idVec3_t( GFIXED_0, GFIXED_0, GFIXED_1 ) );

void toMatrix( quat_t const &src, mat3_t &dst ) {
	gfixed	wx, wy, wz;
	gfixed	xx, yy, yz;
	gfixed	xy, xz, zz;
	gfixed	x2, y2, z2;

	x2 = src.x + src.x;
	y2 = src.y + src.y;
	z2 = src.z + src.z;

	xx = src.x * x2;
	xy = src.x * y2;
	xz = src.x * z2;

	yy = src.y * y2;
	yz = src.y * z2;
	zz = src.z * z2;

	wx = src.w * x2;
	wy = src.w * y2;
	wz = src.w * z2;

	dst[ 0 ][ 0 ] = GFIXED_1 - ( yy + zz );
	dst[ 0 ][ 1 ] = xy - wz;
	dst[ 0 ][ 2 ] = xz + wy;

	dst[ 1 ][ 0 ] = xy + wz;
	dst[ 1 ][ 1 ] = GFIXED_1 - ( xx + zz );
	dst[ 1 ][ 2 ] = yz - wx;

	dst[ 2 ][ 0 ] = xz - wy;
	dst[ 2 ][ 1 ] = yz + wx;
	dst[ 2 ][ 2 ] = GFIXED_1 - ( xx + yy );
}

void toMatrix( angles_t const &src, mat3_t &dst ) {
	gfixed			angle;
	static gfixed	sr, sp, sy, cr, cp, cy; // static to help MS compiler fp bugs
		
	angle = src.yaw * ( GFIXED_PI * GFIXED(2,0) / GFIXED(360,0) );
	sy = FIXED_SIN( angle );
	cy = FIXED_COS( angle );

	angle = src.pitch * ( GFIXED_PI * GFIXED(2,0) / GFIXED(360,0) );
	sp = FIXED_SIN( angle );
	cp = FIXED_COS( angle );

	angle = src.roll * ( GFIXED_PI * GFIXED(2,0) / GFIXED(360,0) );
	sr = FIXED_SIN( angle );
	cr = FIXED_COS( angle );

	dst[ 0 ].set( cp * cy, cp * sy, -sp );
	dst[ 1 ].set( sr * sp * cy + cr * -sy, sr * sp * sy + cr * cy, sr * cp );
	dst[ 2 ].set( cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp );
}

void toMatrix( idVec3_t const &src, mat3_t &dst ) {
        angles_t sup = src;
        toMatrix(sup, dst);
}

void mat3_t::ProjectVector( const idVec3_t &src, idVec3_t &dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
}

void mat3_t::UnprojectVector( const idVec3_t &src, idVec3_t &dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z;
}

void mat3_t::Transpose( mat3_t &matrix ) {
	int	i;
	int	j;
   
	for( i = 0; i < 3; i++ ) {
		for( j = 0; j < 3; j++ ) {
			matrix[ i ][ j ] = mat[ j ][ i ];
        }
	}
}

void mat3_t::Transpose( void ) {
	gfixed	temp;
	int		i;
	int		j;
   
	for( i = 0; i < 3; i++ ) {
		for( j = i + 1; j < 3; j++ ) {
			temp = mat[ i ][ j ];
			mat[ i ][ j ] = mat[ j ][ i ];
			mat[ j ][ i ] = temp;
        }
	}
}

mat3_t mat3_t::Inverse( void ) const {
	mat3_t inv( *this );

	inv.Transpose();

	return inv;
}

void mat3_t::Clear( void ) {
	mat[0].set( GFIXED_1, GFIXED_0, GFIXED_0 );
	mat[1].set( GFIXED_0, GFIXED_1, GFIXED_0 );
	mat[2].set( GFIXED_0, GFIXED_0, GFIXED_1 );
}
