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
// bg_local.h -- local definitions for the bg (both games) files

#define	MIN_WALK_NORMAL	AFIXED(0,7)		// can't walk on very steep slopes

#define	STEPSIZE		18

#define	JUMP_VELOCITY	270

#define	TIMER_LAND		130
#define	TIMER_GESTURE	(34*66+50)

#define	OVERCLIP		BFIXED(1,001)

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct {
	avec3_t		forward, right, up;
	gfixed		frametime;

	int			msec;

	qboolean	walking;
	qboolean	groundPlane;
	trace_t		groundTrace;

	bfixed		impactSpeed;

	bvec3_t		previous_origin;
	bvec3_t		previous_velocity;
	int			previous_waterlevel;
} pml_t;

extern	pmove_t		*pm;
extern	pml_t		pml;

// movement parameters
extern	bfixed	pm_stopspeed;
extern	bfixed	pm_duckScale;
extern	bfixed	pm_swimScale;
extern	bfixed	pm_wadeScale;

extern	bfixed	pm_accelerate;
extern	bfixed	pm_airaccelerate;
extern	bfixed	pm_wateraccelerate;
extern	bfixed	pm_flyaccelerate;

extern	bfixed	pm_friction;
extern	bfixed	pm_waterfriction;
extern	bfixed	pm_flightfriction;

extern	int		c_pmove;

void PM_ClipVelocity( bvec3_t in, avec3_t normal, bvec3_t out, bfixed overbounce );
#ifndef FIXED_IS_FLOAT
void PM_ClipVelocity( avec3_t in, avec3_t normal, avec3_t out, bfixed overbounce );
#endif
void PM_AddTouchEnt( int entityNum );
void PM_AddEvent( int newEvent );

qboolean	PM_SlideMove( qboolean gravity );
void		PM_StepSlideMove( qboolean gravity );


