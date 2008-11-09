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
#ifndef _WIN32
#include"float.h"
#endif

angles_t ang_zero( GFIXED_0, GFIXED_0, GFIXED_0);

void toAngles( mat3_t &src, angles_t &dst ) {
	lfixed		theta;
	lfixed		cp;
	lfixed		sp;

	sp = MAKE_LFIXED(src[ 0 ][ 2 ]);

	// cap off our sin value so that we don't get any NANs
	if ( sp > LFIXED_1 ) {
		sp = LFIXED_1;
	} else if ( sp < -LFIXED_1 ) {
		sp = -LFIXED_1;
	}

	theta = -FIXED_ASIN( sp );
	cp = FIXED_COS( theta );

	if ( cp > LFIXED(8192,0) * MAKE_LFIXED(FLT_EPSILON) ) {
		dst.pitch	= MAKE_GFIXED(theta * LFIXED(180,0) / LFIXED_PI);
		dst.yaw		= MAKE_GFIXED(FIXED_ATAN2( MAKE_LFIXED(src[ 0 ][ 1 ]), MAKE_LFIXED(src[ 0 ][ 0 ]) ) * LFIXED(180,0) / LFIXED_PI);
		dst.roll	= MAKE_GFIXED(FIXED_ATAN2( MAKE_LFIXED(src[ 1 ][ 2 ]), MAKE_LFIXED(src[ 2 ][ 2 ]) ) * LFIXED(180,0) / LFIXED_PI);
	} else {
		dst.pitch	= MAKE_GFIXED(theta * LFIXED(180,0) / LFIXED_PI);
		dst.yaw		= MAKE_GFIXED(-FIXED_ATAN2( MAKE_LFIXED(src[ 1 ][ 0 ]), MAKE_LFIXED(src[ 1 ][ 1 ]) ) * LFIXED(180,0) / LFIXED_PI);
		dst.roll	= GFIXED_0;
	}
}

void toAngles( quat_t &src, angles_t &dst ) {
	mat3_t temp;

	toMatrix( src, temp );
	toAngles( temp, dst );
}

void toAngles( idVec3_t &src, angles_t &dst ) {
	dst.pitch	= src[ 0 ];
	dst.yaw		= src[ 1 ];
	dst.roll	= src[ 2 ];
}

void angles_t::toVectors( idVec3_t *forward, idVec3_t *right, idVec3_t *up ) {
	gfixed			angle;
	static gfixed	sr, sp, sy, cr, cp, cy; // static to help MS compiler fp bugs
	
	angle = yaw * ( GFIXED_PI * GFIXED(2,0) / GFIXED(360,0) );
	sy = FIXED_SIN( angle );
	cy = FIXED_COS( angle );

	angle = pitch * ( GFIXED_PI * GFIXED(2,0) / GFIXED(360,0) );
	sp = FIXED_SIN( angle );
	cp = FIXED_COS( angle );

	angle = roll * ( GFIXED_PI * GFIXED(2,0) / GFIXED(360,0) );
	sr = FIXED_SIN( angle );
	cr = FIXED_COS( angle );

	if ( forward ) {
		forward->set( cp * cy, cp * sy, -sp );
	}

	if ( right ) {
		right->set( -sr * sp * cy + cr * sy, -sr * sp * sy + -cr * cy, -sr * cp );
	}

	if ( up ) {
		up->set( cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp );
	}
}

idVec3_t angles_t::toForward( void ) {
	gfixed			angle;
	static gfixed	sp, sy, cp, cy; // static to help MS compiler fp bugs
	
	angle = yaw * ( GFIXED_PI * GFIXED(2,0) / GFIXED(360,0) );
	sy = FIXED_SIN( angle );
	cy = FIXED_COS( angle );

	angle = pitch * ( GFIXED_PI * GFIXED(2,0) / GFIXED(360,0) );
	sp = FIXED_SIN( angle );
	cp = FIXED_COS( angle );

	return idVec3_t( cp * cy, cp * sy, -sp );
}

/*
=================
Normalize360

returns angles normalized to the range [0 <= angle < 360]
=================
*/
angles_t& angles_t::Normalize360( void ) {
	//pitch	= (GFIXED(360,0) / GFIXED(65536,0)) * MAKE_GFIXED(FIXED_TO_INT( pitch	* ( GFIXED(65536,0) / GFIXED(360,0) ) ) & 65535 );
	//yaw		= (GFIXED(360,0) / GFIXED(65536,0)) * MAKE_GFIXED(FIXED_TO_INT( yaw		* ( GFIXED(65536,0) / GFIXED(360,0) ) ) & 65535 );
	//roll	= (GFIXED(360,0) / GFIXED(65536,0)) * MAKE_GFIXED(FIXED_TO_INT( roll		* ( GFIXED(65536,0) / GFIXED(360,0) ) ) & 65535 );
	pitch=FIXED_MOD_CLAMP(pitch,GFIXED(360,0));
	yaw=FIXED_MOD_CLAMP(yaw,GFIXED(360,0));
	roll=FIXED_MOD_CLAMP(roll,GFIXED(360,0));

	return *this;
}


/*
=================
Normalize180

returns angles normalized to the range [-180 < angle <= 180]
=================
*/
angles_t& angles_t::Normalize180( void ) {
	Normalize360();

	if ( pitch > GFIXED(180,0) ) {
		pitch -= GFIXED(360,0);
	}
	
	if ( yaw > GFIXED(180,0) ) {
		yaw  -= GFIXED(360,0);
	}

	if ( roll > GFIXED(180,0) ) {
		roll -= GFIXED(360,0);
	}
	return *this;
}
