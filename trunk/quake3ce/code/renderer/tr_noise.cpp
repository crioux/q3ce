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
// tr_noise.c
#include"renderer_pch.h"

#define NOISE_SIZE 256
#define NOISE_MASK ( NOISE_SIZE - 1 )

#define VAL( a ) s_noise_perm[ ( a ) & ( NOISE_MASK )]
#define INDEX( x, y, z, t ) VAL( x + VAL( y + VAL( z + VAL( t ) ) ) )

static gfixed s_noise_table[NOISE_SIZE];
static int s_noise_perm[NOISE_SIZE];

#define LERP( a, b, w ) ( a * ( GFIXED_1 - w ) + b * w )

static gfixed GetNoiseValue( int x, int y, int z, int t )
{
	int index = INDEX( ( int ) x, ( int ) y, ( int ) z, ( int ) t );

	return s_noise_table[index];
}

void R_NoiseInit( void )
{
	int i;

	srand( 1001 );

	for ( i = 0; i < NOISE_SIZE; i++ )
	{
		s_noise_table[i] = ( crandom() * GFIXED(2,0) - GFIXED_1 );
		s_noise_perm[i] = FIXED_TO_INT( crandom() *GFIXED(255,0) );
	}
}

gfixed R_NoiseGet4f( gfixed x, gfixed y, gfixed z, gfixed t )
{
	int i;
	int ix, iy, iz, it;
	gfixed fx, fy, fz, ft;
	gfixed front[4];
	gfixed back[4];
	gfixed fvalue, bvalue, value[2], finalvalue;

	ix = FIXED_TO_INT(FIXED_FLOOR( x ));
	fx = x - MAKE_GFIXED(ix);
	iy = FIXED_TO_INT(FIXED_FLOOR( y ));
	fy = y - MAKE_GFIXED(iy);
	iz = FIXED_TO_INT(FIXED_FLOOR( z ));
	fz = z - MAKE_GFIXED(iz);
	it = FIXED_TO_INT(FIXED_FLOOR( t ));
	ft = t - MAKE_GFIXED(it);

	for ( i = 0; i < 2; i++ )
	{
		front[0] = GetNoiseValue( ix, iy, iz, it + i );
		front[1] = GetNoiseValue( ix+1, iy, iz, it + i );
		front[2] = GetNoiseValue( ix, iy+1, iz, it + i );
		front[3] = GetNoiseValue( ix+1, iy+1, iz, it + i );

		back[0] = GetNoiseValue( ix, iy, iz + 1, it + i );
		back[1] = GetNoiseValue( ix+1, iy, iz + 1, it + i );
		back[2] = GetNoiseValue( ix, iy+1, iz + 1, it + i );
		back[3] = GetNoiseValue( ix+1, iy+1, iz + 1, it + i );

		fvalue = LERP( LERP( front[0], front[1], fx ), LERP( front[2], front[3], fx ), fy );
		bvalue = LERP( LERP( back[0], back[1], fx ), LERP( back[2], back[3], fx ), fy );

		value[i] = LERP( fvalue, bvalue, fz );
	}

	finalvalue = LERP( value[0], value[1], ft );

	return finalvalue;
}
