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
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering
#include"cgame_pch.h"


/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like 
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.

Note that none of the model testing features update while the game is paused, so
it may be convenient to test with deathmatch set to 1 so that bringing down the
console doesn't pause the game.

=============================================================================
*/

/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f (void) {
	avec3_t		angles;

	memset( &cg.testModelEntity, 0, sizeof(cg.testModelEntity) );
	if ( _CG_trap_Argc() < 2 ) {
		return;
	}

	Q_strncpyz (cg.testModelName, CG_Argv( 1 ), MAX_QPATH );
	cg.testModelEntity.hModel = _CG_trap_R_RegisterModel( cg.testModelName );

	if ( _CG_trap_Argc() == 3 ) {
		cg.testModelEntity.backlerp = MAKE_GFIXED(atof( CG_Argv( 2 ) ));
		cg.testModelEntity.frame = 1;
		cg.testModelEntity.oldframe = 0;
	}
	if (! cg.testModelEntity.hModel ) {
		CG_Printf( "Can't register model\n" );
		return;
	}

	FIXED_VEC3MA_R( cg.refdef.vieworg, BFIXED(100,0), cg.refdef.viewaxis[0], cg.testModelEntity.origin );

	angles[PITCH] = AFIXED_0;
	angles[YAW] = AFIXED(180,0) + cg.refdefViewAngles[1];
	angles[ROLL] = AFIXED_0;

	AnglesToAxis( angles, cg.testModelEntity.axis );
	cg.testGun = qfalse;
}

/*
=================
CG_TestGun_f

Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f (void) {
	CG_TestModel_f();
	cg.testGun = qtrue;
	cg.testModelEntity.renderfx = RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;
}


void CG_TestModelNextFrame_f (void) {
	cg.testModelEntity.frame++;
	CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelPrevFrame_f (void) {
	cg.testModelEntity.frame--;
	if ( cg.testModelEntity.frame < 0 ) {
		cg.testModelEntity.frame = 0;
	}
	CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelNextSkin_f (void) {
	cg.testModelEntity.skinNum++;
	CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_TestModelPrevSkin_f (void) {
	cg.testModelEntity.skinNum--;
	if ( cg.testModelEntity.skinNum < 0 ) {
		cg.testModelEntity.skinNum = 0;
	}
	CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

static void CG_AddTestModel (void) {
	int		i;

	// re-register the model, because the level may have changed
	cg.testModelEntity.hModel = _CG_trap_R_RegisterModel( cg.testModelName );
	if (! cg.testModelEntity.hModel ) {
		CG_Printf ("Can't register model\n");
		return;
	}

	// if testing a gun, set the origin reletive to the view origin
	if ( cg.testGun ) {
		VectorCopy( cg.refdef.vieworg, cg.testModelEntity.origin );
		VectorCopy( cg.refdef.viewaxis[0], cg.testModelEntity.axis[0] );
		VectorCopy( cg.refdef.viewaxis[1], cg.testModelEntity.axis[1] );
		VectorCopy( cg.refdef.viewaxis[2], cg.testModelEntity.axis[2] );

		// allow the position to be adjusted
		for (i=0 ; i<3 ; i++) {
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * MAKE_BFIXED(cg_gun_x.value);
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * MAKE_BFIXED(cg_gun_y.value);
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * MAKE_BFIXED(cg_gun_z.value);
		}
	}

	_CG_trap_R_AddRefEntityToScene( &cg.testModelEntity );
}



//============================================================================


/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect (void) {
	int		size;

	// the intermission should allways be full screen
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		size = 100;
	} else {
		// bound normal viewsize
		if (cg_viewsize.integer < 30) {
			_CG_trap_Cvar_Set ("cg_viewsize","30");
			size = 30;
		} else if (cg_viewsize.integer > 100) {
			_CG_trap_Cvar_Set ("cg_viewsize","100");
			size = 100;
		} else {
			size = cg_viewsize.integer;
		}

	}
	cg.refdef.width = cgs.glconfig.vidWidth*size/100;
	cg.refdef.width &= ~1;

	cg.refdef.height = cgs.glconfig.vidHeight*size/100;
	cg.refdef.height &= ~1;

	cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width)/2;
	cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height)/2;
}

//==============================================================================


/*
===============
CG_OffsetThirdPersonView

===============
*/
#define	FOCUS_DISTANCE	512
static void CG_OffsetThirdPersonView( void ) {
	avec3_t		forward, right, up;
	bvec3_t		view;
	avec3_t		focusAngles;
	trace_t		trace;
	static bvec3_t	mins = { -BFIXED(4,0), -BFIXED(4,0), -BFIXED(4,0) };
	static bvec3_t	maxs = { BFIXED(4,0), BFIXED(4,0), BFIXED(4,0) };
	bvec3_t		focusPoint;
	bfixed		focusDist;
	lfixed		forwardScale, sideScale;

	cg.refdef.vieworg[2] += MAKE_BFIXED(cg.predictedPlayerState.viewheight);

	VectorCopy( cg.refdefViewAngles, focusAngles );

	// if dead, look at killer
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		focusAngles[YAW] = MAKE_AFIXED(cg.predictedPlayerState.stats[STAT_DEAD_YAW]);
		cg.refdefViewAngles[YAW] = MAKE_AFIXED(cg.predictedPlayerState.stats[STAT_DEAD_YAW]);
	}

	if ( focusAngles[PITCH] > AFIXED(45,0) ) {
		focusAngles[PITCH] = AFIXED(45,0);		// don't go too far overhead
	}
	AngleVectors( focusAngles, forward, NULL, NULL );

	FIXED_VEC3MA_R( cg.refdef.vieworg, BFIXED(FOCUS_DISTANCE,0), forward, focusPoint );

	VectorCopy( cg.refdef.vieworg, view );

	view[2] += BFIXED(8,0);

	cg.refdefViewAngles[PITCH] *= AFIXED(0,5);

	AngleVectors( cg.refdefViewAngles, forward, right, up );

	forwardScale = FIXED_COS( cg_thirdPersonAngle.value) / LFIXED(180,0) * LFIXED_PI ;
	sideScale = FIXED_SIN( cg_thirdPersonAngle.value) / LFIXED(180,0) * LFIXED_PI ;
	FIXED_VEC3MA_R( view, -MAKE_BFIXED(cg_thirdPersonRange.value * forwardScale), forward, view );
	FIXED_VEC3MA_R( view, -MAKE_BFIXED(cg_thirdPersonRange.value * sideScale), right, view );

	// trace a ray from the origin to the viewpoint to make sure the view isn't
	// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything

	if (!cg_cameraMode.integer) {
		CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );

		if ( trace.fraction != GFIXED_1 ) {
			VectorCopy( trace.endpos, view );
			view[2] += MAKE_BFIXED((GFIXED_1 - trace.fraction) * GFIXED(32,0));
			// try another trace to this position, because a tunnel may have the ceiling
			// close enogh that this is poking out

			CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );
			VectorCopy( trace.endpos, view );
		}
	}


	VectorCopy( view, cg.refdef.vieworg );

	// select pitch to look at focus point from vieword
	VectorSubtract( focusPoint, cg.refdef.vieworg, focusPoint );
	focusDist = FIXED_VEC2LEN(focusPoint);
	if ( focusDist < BFIXED_1 ) {
		focusDist = BFIXED_1;	// should never happen
	}
	cg.refdefViewAngles[PITCH] = -AFIXED(180,0) / AFIXED_PI * MAKE_AFIXED(FIXED_ATAN2( focusPoint[2], focusDist ));
	cg.refdefViewAngles[YAW] -= MAKE_AFIXED(cg_thirdPersonAngle.value);
}


// this causes a compiler bug on mac MrC compiler
static void CG_StepOffset( void ) {
	int		timeDelta;
	
	// smooth out stair climbing
	timeDelta = cg.time - cg.stepTime;
	if ( timeDelta < STEP_TIME ) {
		cg.refdef.vieworg[2] -= cg.stepChange * FIXED_INT32RATIO_B(STEP_TIME - timeDelta,STEP_TIME);
	}
}

/*
===============
CG_OffsetFirstPersonView

===============
*/
static void CG_OffsetFirstPersonView( void ) {
	bfixed			*origin;
	afixed			*angles;
	bfixed			bob;
	afixed			ratio;
	afixed			adelta;
	int				delta;
	afixed			speed;
	bfixed			bf;
	bvec3_t			predictedVelocity;
	int				timeDelta;
	
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		return;
	}

	origin = cg.refdef.vieworg;
	angles = cg.refdefViewAngles;

	// if dead, fix the angle and don't add any kick
	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
		angles[ROLL] = AFIXED(40,0);
		angles[PITCH] = -AFIXED(15,0);
		angles[YAW] = MAKE_AFIXED(cg.snap->ps.stats[STAT_DEAD_YAW]);
		origin[2] += MAKE_BFIXED(cg.predictedPlayerState.viewheight);
		return;
	}

	// add angles based on weapon kick
	VectorAdd (angles, cg.kick_angles, angles);

	// add angles based on damage kick
	if ( !cg.damageTime ) {
		int timeval=cg.time - cg.damageTime;
		if ( timeval < DAMAGE_DEFLECT_TIME) {
			ratio = FIXED_INT32RATIO_A(timeval,DAMAGE_DEFLECT_TIME);
			angles[PITCH] += ratio * cg.v_dmg_pitch;
			angles[ROLL] += ratio * cg.v_dmg_roll;
		} else {
			ratio = AFIXED_1 - FIXED_INT32RATIO_A( (timeval - DAMAGE_DEFLECT_TIME ),DAMAGE_RETURN_TIME );
			if ( ratio > AFIXED_0 ) {
				angles[PITCH] += ratio * cg.v_dmg_pitch;
				angles[ROLL] += ratio * cg.v_dmg_roll;
			}
		}
	}

	// add pitch based on fall kick
#if 0
	ratio = FIXED_INT32RATIO_A( cg.time - cg.landTime ,  FALL_TIME );
	if (ratio < 0)
		ratio = 0;
	angles[PITCH] += ratio * cg.fall_value;
#endif

	// add angles based on velocity
	VectorCopy( cg.predictedPlayerState.velocity, predictedVelocity );

	adelta = MAKE_AFIXED(FIXED_VEC3DOT ( predictedVelocity, cg.refdef.viewaxis[0]));
	angles[PITCH] += adelta * MAKE_AFIXED(cg_runpitch.value);
	
	adelta = MAKE_AFIXED(FIXED_VEC3DOT ( predictedVelocity, cg.refdef.viewaxis[1]));
	angles[ROLL] -= adelta * MAKE_AFIXED(cg_runroll.value);

	// add angles based on bob

	// make sure the bob is visible even at low speeds
	speed = MAKE_AFIXED(cg.xyspeed > GFIXED(200,0) ? cg.xyspeed : GFIXED(200,0));

	adelta = MAKE_AFIXED(cg.bobfracsin * MAKE_GFIXED(cg_bobpitch.value)) * speed;
	if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
		adelta *= AFIXED(3,0);		// crouching
	angles[PITCH] += adelta;
	adelta = MAKE_AFIXED(cg.bobfracsin * MAKE_GFIXED(cg_bobroll.value)) * speed;
	if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
		adelta *= AFIXED(3,0);		// crouching accentuates roll
	if (cg.bobcycle & 1)
		adelta = -adelta;
	angles[ROLL] += adelta;

//===================================

	// add view height
	origin[2] += MAKE_BFIXED(cg.predictedPlayerState.viewheight);

	// smooth out duck height changes
	timeDelta = cg.time - cg.duckTime;
	if ( timeDelta < DUCK_TIME) {
		cg.refdef.vieworg[2] -= cg.duckChange 
			* FIXED_INT32RATIO_B(DUCK_TIME - timeDelta, DUCK_TIME);
	}

	// add bob height
	bob = MAKE_BFIXED(cg.bobfracsin * cg.xyspeed * MAKE_GFIXED(cg_bobup.value));
	if (bob > BFIXED(6,0)) {
		bob = BFIXED(6,0);
	}

	origin[2] += bob;


	// add fall height
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		bf = FIXED_INT32RATIO_B( delta , LAND_DEFLECT_TIME);
		cg.refdef.vieworg[2] += cg.landChange * bf;
	} else if ( delta < (LAND_DEFLECT_TIME + LAND_RETURN_TIME) ) {
		delta -= LAND_DEFLECT_TIME;
		bf = BFIXED_1 - FIXED_INT32RATIO_B( delta , LAND_RETURN_TIME);
		cg.refdef.vieworg[2] += cg.landChange * bf;
	}

	// add step offset
	CG_StepOffset();

	// add kick offset

	VectorAdd (origin, cg.kick_origin, origin);

	// pivot the eye based on a neck length
#if 0
	{
#define	NECK_LENGTH		8
	bvec3_t			forward, up;
 
	cg.refdef.vieworg[2] -= NECK_LENGTH;
	AngleVectors( cg.refdefViewAngles, forward, NULL, up );
	FIXED_VEC3MA( cg.refdef.vieworg, GFIXED(3,0), forward, cg.refdef.vieworg );
	FIXED_VEC3MA( cg.refdef.vieworg, GFIXED(NECK_LENGTH,0), up, cg.refdef.vieworg );
	}
#endif
}

//======================================================================

void CG_ZoomDown_f( void ) { 
	if ( cg.zoomed ) {
		return;
	}
	cg.zoomed = qtrue;
	cg.zoomTime = cg.time;
}

void CG_ZoomUp_f( void ) { 
	if ( !cg.zoomed ) {
		return;
	}
	cg.zoomed = qfalse;
	cg.zoomTime = cg.time;
}


/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	GFIXED(0,4)

static int CG_CalcFov( void ) {
	gfixed	x;
	gfixed	phase;
	gfixed	v;
	int		contents;
	afixed	fov_x, fov_y;
	afixed	zoomFov;
	gfixed	f;
	int		inwater;

	if ( cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		// if in intermission, use a gfixed value
		fov_x = AFIXED(90,0);
	} else {
		// user selectable
		if ( cgs.dmflags & DF_FIXED_FOV ) {
			// dmflag to prevent wide fov for all clients
			fov_x = AFIXED(90,0);
		} else {
			fov_x = MAKE_AFIXED(cg_fov.value);
			if ( fov_x < AFIXED_1 ) {
				fov_x = AFIXED_1;
			} else if ( fov_x > AFIXED(160,0) ) {
				fov_x = AFIXED(160,0);
			}
		}

		// account for zooms
		zoomFov = MAKE_AFIXED(cg_zoomFov.value);
		if ( zoomFov < AFIXED_1 ) {
			zoomFov = AFIXED_1;
		} else if ( zoomFov > AFIXED(160,0) ) {
			zoomFov = AFIXED(160,0);
		}

		f = FIXED_INT32RATIO_G(cg.time - cg.zoomTime,ZOOM_TIME);	
		if ( cg.zoomed ) {
			if ( f > GFIXED_1 ) {
				fov_x = zoomFov;
			} else {
				fov_x = fov_x + MAKE_AFIXED(f * MAKE_GFIXED( zoomFov - fov_x ));
			}
		} else {
			if ( f > GFIXED_1 ) {
				fov_x = fov_x;
			} else {
				fov_x = zoomFov + MAKE_AFIXED(f * MAKE_GFIXED( fov_x - zoomFov ));
			}
		}
	}

	x = MAKE_GFIXED(cg.refdef.width) / MAKE_GFIXED(FIXED_TAN( fov_x / AFIXED(360,0) * AFIXED_PI ));
	fov_y = MAKE_AFIXED(FIXED_ATAN2( MAKE_GFIXED(cg.refdef.height), x ));
	fov_y = fov_y / AFIXED_PI * AFIXED(360,0) ;

	// warp if underwater
	contents = CG_PointContents( cg.refdef.vieworg, -1 );
	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ){
		phase = FIXED_MULPOW2(MSECTIME_G(cg.time) * WAVE_FREQUENCY * GFIXED_PI,1);
		v = GFIXED(WAVE_AMPLITUDE,0) * FIXED_SIN( phase );
		fov_x += MAKE_AFIXED(v);
		fov_y -= MAKE_AFIXED(v);
		inwater = qtrue;
	}
	else {
		inwater = qfalse;
	}


	// set it
	cg.refdef.fov_x = fov_x;
	cg.refdef.fov_y = fov_y;

	if ( !cg.zoomed ) {
		cg.zoomSensitivity = GFIXED_1;
	} else {
		cg.zoomSensitivity = MAKE_GFIXED(cg.refdef.fov_y / AFIXED(75,0));
	}

	return inwater;
}



/*
===============
CG_DamageBlendBlob

===============
*/
static void CG_DamageBlendBlob( void ) {
	int			t;
	int			maxTime;
	refEntity_t		ent;

	if ( FIXED_NOT_ZERO(cg.damageValue) ) {
		return;
	}

	maxTime = DAMAGE_TIME;
	t = cg.time - cg.damageTime;
	if ( t <= 0 || t >= maxTime ) {
		return;
	}


	memset( &ent, 0, sizeof( ent ) );
	ent.reType = RT_SPRITE;
	ent.renderfx = RF_FIRST_PERSON;

	FIXED_VEC3MA_R( cg.refdef.vieworg, BFIXED(8,0), cg.refdef.viewaxis[0], ent.origin );
	FIXED_VEC3MA_R( ent.origin, cg.damageX * -BFIXED(8,0), cg.refdef.viewaxis[1], ent.origin );
	FIXED_VEC3MA_R( ent.origin, cg.damageY * BFIXED(8,0), cg.refdef.viewaxis[2], ent.origin );

	ent.radius = MAKE_BFIXED(cg.damageValue * GFIXED(3,0));
	ent.customShader = cgs.media.viewBloodShader;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = FIXED_TO_INT(GFIXED(200,0) * ( GFIXED_1 - FIXED_INT32RATIO_G(t,maxTime)) );
	_CG_trap_R_AddRefEntityToScene( &ent );
}


/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void ) {
	playerState_t	*ps;

	memset( &cg.refdef, 0, sizeof( cg.refdef ) );

	// strings for in game rendering
	// Q_strncpyz( cg.refdef.text[0], "Park Ranger", sizeof(cg.refdef.text[0]) );
	// Q_strncpyz( cg.refdef.text[1], "19", sizeof(cg.refdef.text[1]) );

	// calculate size of 3D view
	CG_CalcVrect();

	ps = &cg.predictedPlayerState;
/*
	if (cg.cameraMode) {
		bvec3_t origin;
		avec3_t angles;
		if (_CG_trap_getCameraInfo(cg.time, &origin, &angles)) {
			VectorCopy(origin, cg.refdef.vieworg);
			angles[ROLL] = 0;
			VectorCopy(angles, cg.refdefViewAngles);
			AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
			return CG_CalcFov();
		} else {
			cg.cameraMode = qfalse;
		}
	}
*/
	// intermission view
	if ( ps->pm_type == PM_INTERMISSION ) {
		VectorCopy( ps->origin, cg.refdef.vieworg );
		VectorCopy( ps->viewangles, cg.refdefViewAngles );
		AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
		return CG_CalcFov();
	}

	cg.bobcycle = ( ps->bobCycle & 128 ) >> 7;
	cg.bobfracsin = FIXED_ABS( FIXED_SIN( FIXED_INT32RATIO_G( ps->bobCycle & 127, 127) * GFIXED_PI ) );
	cg.xyspeed = MAKE_GFIXED(FIXED_VEC2LEN(ps->velocity));


	VectorCopy( ps->origin, cg.refdef.vieworg );
	VectorCopy( ps->viewangles, cg.refdefViewAngles );

	if (cg_cameraOrbit.integer) {
		if (cg.time > cg.nextOrbitTime) {
			cg.nextOrbitTime = cg.time + cg_cameraOrbitDelay.integer;
			cg_thirdPersonAngle.value += cg_cameraOrbit.value;
		}
	}
	// add error decay
	if ( cg_errorDecay.value > LFIXED_0 ) {
		int		t;
		bfixed	f;

		t = cg.time - cg.predictedErrorTime;
		f = FIXED_INT32RATIO_B((cg_errorDecay.integer - t) , cg_errorDecay.integer);
		if ( f > BFIXED_0 && f < BFIXED_1 ) {
			FIXED_VEC3MA( cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg );
		} else {
			cg.predictedErrorTime = 0;
		}
	}

	if ( cg.renderingThirdPerson ) {
		// back away from character
		CG_OffsetThirdPersonView();
	} else {
		// offset for local bobbing and kicks
		CG_OffsetFirstPersonView();
	}

	// position eye reletive to origin
	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

	if ( cg.hyperspace ) {
		cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
	}

	// field of view
	return CG_CalcFov();
}


/*
=====================
CG_PowerupTimerSounds
=====================
*/
static void CG_PowerupTimerSounds( void ) {
	int		i;
	int		t;

	// powerup timers going away
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		t = cg.snap->ps.powerups[i];
		if ( t <= cg.time ) {
			continue;
		}
		if ( t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME ) {
			continue;
		}
		if ( ( t - cg.time ) / POWERUP_BLINK_TIME != ( t - cg.oldTime ) / POWERUP_BLINK_TIME ) {
			_CG_trap_S_StartSound( NULL, cg.snap->ps.clientNum, CHAN_ITEM, cgs.media.wearOffSound );
		}
	}
}

/*
=====================
CG_AddBufferedSound
=====================
*/
void CG_AddBufferedSound( sfxHandle_t sfx ) {
	if ( !sfx )
		return;
	cg.soundBuffer[cg.soundBufferIn] = sfx;
	cg.soundBufferIn = (cg.soundBufferIn + 1) % MAX_SOUNDBUFFER;
	if (cg.soundBufferIn == cg.soundBufferOut) {
		cg.soundBufferOut++;
	}
}

/*
=====================
CG_PlayBufferedSounds
=====================
*/
static void CG_PlayBufferedSounds( void ) {
	if ( cg.soundTime < cg.time ) {
		if (cg.soundBufferOut != cg.soundBufferIn && cg.soundBuffer[cg.soundBufferOut]) {
			_CG_trap_S_StartLocalSound(cg.soundBuffer[cg.soundBufferOut], CHAN_ANNOUNCER);
			cg.soundBuffer[cg.soundBufferOut] = 0;
			cg.soundBufferOut = (cg.soundBufferOut + 1) % MAX_SOUNDBUFFER;
			cg.soundTime = cg.time + 750;
		}
	}
}

//=========================================================================

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	int		inwater;

	cg.time = serverTime;
	cg.demoPlayback = demoPlayback;

	// update cvars
	CG_UpdateCvars();

	// if we are only updating the screen as a loading
	// pacifier, don't even try to read snapshots
	if ( cg.infoScreenText[0] != 0 ) {
		CG_DrawInformation();
		return;
	}

	// any looped sounds will be respecified as entities
	// are added to the render list
	_CG_trap_S_ClearLoopingSounds(qfalse);

	// clear all the render lists
	_CG_trap_R_ClearScene();

	// set up cg.snap and possibly cg.nextSnap
	CG_ProcessSnapshots();

	// if we haven't received any snapshots yet, all
	// we can draw is the information screen
	if ( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
		CG_DrawInformation();
		return;
	}

	// let the client system know what our weapon and zoom settings are
	_CG_trap_SetUserCmdValue( cg.weaponSelect, cg.zoomSensitivity );

	// this counter will be bumped for every valid scene we generate
	cg.clientFrame++;

	// update cg.predictedPlayerState
	CG_PredictPlayerState();

	// decide on third person view
	cg.renderingThirdPerson = cg_thirdPerson.integer || (cg.snap->ps.stats[STAT_HEALTH] <= 0);

	// build cg.refdef
	inwater = CG_CalcViewValues();

	// first person blend blobs, done after AnglesToAxis
	if ( !cg.renderingThirdPerson ) {
		CG_DamageBlendBlob();
	}

	// build the render lists
	if ( !cg.hyperspace ) {
		CG_AddPacketEntities();			// adter calcViewValues, so predicted player state is correct
		CG_AddMarks();
		CG_AddParticles ();
		CG_AddLocalEntities();
	}
	CG_AddViewWeapon( &cg.predictedPlayerState );

	// add buffered sounds
	CG_PlayBufferedSounds();

	// play buffered voice chats
	//CG_PlayBufferedVoiceChats();

	// finish up the rest of the refdef
	if ( cg.testModelEntity.hModel ) {
		CG_AddTestModel();
	}
	cg.refdef.time = cg.time;
	memcpy( cg.refdef.areamask, cg.snap->areamask, sizeof( cg.refdef.areamask ) );

	// warning sounds when powerup is wearing off
	CG_PowerupTimerSounds();

	// update audio positions
	_CG_trap_S_Respatialize( cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater );

	// make sure the lagometerSample and frame timing isn't done twice when in stereo
	if ( stereoView != STEREO_RIGHT ) {
		cg.frametime = cg.time - cg.oldTime;
		if ( cg.frametime < 0 ) {
			cg.frametime = 0;
		}
		cg.oldTime = cg.time;
		CG_AddLagometerFrameInfo();
	}
	if (cg_timescale.value != cg_timescaleFadeEnd.value) {
		if (cg_timescale.value < cg_timescaleFadeEnd.value) {
			cg_timescale.value += cg_timescaleFadeSpeed.value * MSECTIME_L(cg.frametime);
			if (cg_timescale.value > cg_timescaleFadeEnd.value)
				cg_timescale.value = cg_timescaleFadeEnd.value;
		}
		else {
			cg_timescale.value -= cg_timescaleFadeSpeed.value * MSECTIME_L(cg.frametime);
			if (cg_timescale.value < cg_timescaleFadeEnd.value)
				cg_timescale.value = cg_timescaleFadeEnd.value;
		}
		if (FIXED_NOT_ZERO(cg_timescaleFadeSpeed.value)) {
			_CG_trap_Cvar_Set("timescale", va("%f", FIXED_TO_DOUBLE(cg_timescale.value)));
		}
	}

	// actually issue the rendering calls
	CG_DrawActive( stereoView );

	if ( cg_stats.integer ) {
		CG_Printf( "cg.clientFrame:%i\n", cg.clientFrame );
	}


}

