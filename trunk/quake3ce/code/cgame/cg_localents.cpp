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

// cg_localents.c -- every frame, generate renderer commands for locally
// processed entities, like smoke puffs, gibs, shells, etc.

#include"cgame_pch.h"

#define	MAX_LOCAL_ENTITIES	512
localEntity_t	cg_localEntities[MAX_LOCAL_ENTITIES];
localEntity_t	cg_activeLocalEntities;		// double linked list
localEntity_t	*cg_freeLocalEntities;		// single linked list

/*
===================
CG_InitLocalEntities

This is called at startup and for tournement restarts
===================
*/
void	CG_InitLocalEntities( void ) {
	int		i;

	memset( cg_localEntities, 0, sizeof( cg_localEntities ) );
	cg_activeLocalEntities.next = &cg_activeLocalEntities;
	cg_activeLocalEntities.prev = &cg_activeLocalEntities;
	cg_freeLocalEntities = cg_localEntities;
	for ( i = 0 ; i < MAX_LOCAL_ENTITIES - 1 ; i++ ) {
		cg_localEntities[i].next = &cg_localEntities[i+1];
	}
}


/*
==================
CG_FreeLocalEntity
==================
*/
void CG_FreeLocalEntity( localEntity_t *le ) {
	if ( !le->prev ) {
		CG_Error( "CG_FreeLocalEntity: not active" );
	}

	// remove from the doubly linked active list
	le->prev->next = le->next;
	le->next->prev = le->prev;

	// the free list is only singly linked
	le->next = cg_freeLocalEntities;
	cg_freeLocalEntities = le;
}

/*
===================
CG_AllocLocalEntity

Will allways succeed, even if it requires freeing an old active entity
===================
*/
localEntity_t	*CG_AllocLocalEntity( void ) {
	localEntity_t	*le;

	if ( !cg_freeLocalEntities ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		CG_FreeLocalEntity( cg_activeLocalEntities.prev );
	}

	le = cg_freeLocalEntities;
	cg_freeLocalEntities = cg_freeLocalEntities->next;

	memset( le, 0, sizeof( *le ) );

	// link into the active list
	le->next = cg_activeLocalEntities.next;
	le->prev = &cg_activeLocalEntities;
	cg_activeLocalEntities.next->prev = le;
	cg_activeLocalEntities.next = le;
	return le;
}


/*
====================================================================================

FRAGMENT PROCESSING

A fragment localentity interacts with the environment in some way (hitting walls),
or generates more localentities along a trail.

====================================================================================
*/

/*
================
CG_BloodTrail

Leave expanding blood puffs behind gibs
================
*/
void CG_BloodTrail( localEntity_t *le ) {
	int		t;
	int		t2;
	int		step;
	bvec3_t	newOrigin;
	localEntity_t	*blood;

	step = 150;
	t = step * ( (cg.time - cg.frametime + step ) / step );
	t2 = step * ( cg.time / step );

	for ( ; t <= t2; t += step ) {
		BG_EvaluateTrajectory( &le->pos, t, newOrigin );

		blood = CG_SmokePuff( newOrigin, bvec3_origin, 
					  BFIXED(20,0),		// radius
					  GFIXED_1, GFIXED_1, GFIXED_1, GFIXED_1,	// color
					  GFIXED(2000,0),		// trailTime
					  t,		// startTime
					  0,		// fadeInTime
					  0,		// flags
					  cgs.media.bloodTrailShader );
		// use the optimized version
		blood->leType = LE_FALL_SCALE_FADE;
		// drop a total of 40 units over its lifetime
		blood->pos.trDelta[2] = BFIXED(40,0);
	}
}


/*
================
CG_FragmentBounceMark
================
*/
void CG_FragmentBounceMark( localEntity_t *le, trace_t *trace ) {
	int			radius;

	if ( le->leMarkType == LEMT_BLOOD ) {

		radius = 16+(rand()&31);
		CG_ImpactMark( cgs.media.bloodMarkShader, trace->endpos, trace->plane.normal, random_a()*AFIXED(360,0),
			GFIXED_1,GFIXED_1,GFIXED_1,GFIXED_1, qtrue, MAKE_BFIXED(radius), qfalse );
	} else if ( le->leMarkType == LEMT_BURN ) {

		radius = 8 + (rand()&15);
		CG_ImpactMark( cgs.media.burnMarkShader, trace->endpos, trace->plane.normal, random_a()*AFIXED(360,0),
			GFIXED_1,GFIXED_1,GFIXED_1,GFIXED_1, qtrue, MAKE_BFIXED(radius), qfalse );
	}


	// don't allow a fragment to make multiple marks, or they
	// pile up while settling
	le->leMarkType = LEMT_NONE;
}

/*
================
CG_FragmentBounceSound
================
*/
void CG_FragmentBounceSound( localEntity_t *le, trace_t *trace ) {
	if ( le->leBounceSoundType == LEBS_BLOOD ) {
		// half the gibs will make splat sounds
		if ( rand() & 1 ) {
			int r = rand()&3;
			sfxHandle_t	s;

			if ( r == 0 ) {
				s = cgs.media.gibBounce1Sound;
			} else if ( r == 1 ) {
				s = cgs.media.gibBounce2Sound;
			} else {
				s = cgs.media.gibBounce3Sound;
			}
			_CG_trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );
		}
	} else if ( le->leBounceSoundType == LEBS_BRASS ) {

	}

	// don't allow a fragment to make multiple bounce sounds,
	// or it gets too noisy as they settle
	le->leBounceSoundType = LEBS_NONE;
}


/*
================
CG_ReflectVelocity
================
*/
void CG_ReflectVelocity( localEntity_t *le, trace_t *trace ) {
	bvec3_t	velocity;
	bfixed	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = cg.time - cg.frametime + FIXED_TO_INT(MAKE_GFIXED(cg.frametime) * trace->fraction);
	BG_EvaluateTrajectoryDelta( &le->pos, hitTime, velocity );
	dot = FIXED_VEC3DOT( velocity, trace->plane.normal );
	FIXED_VEC3MA_R( velocity, -BFIXED(2,0)*dot, trace->plane.normal, le->pos.trDelta );

	FIXED_VEC3SCALE( le->pos.trDelta, le->bounceFactor, le->pos.trDelta );

	VectorCopy( trace->endpos, le->pos.trBase );
	le->pos.trTime = cg.time;


	// check for stop, making sure that even on low FPS systems it doesn't bobble
	if ( trace->allsolid || 
		( trace->plane.normal[2] > AFIXED_0 && 
		( le->pos.trDelta[2] < BFIXED(40,0) || le->pos.trDelta[2] < -MAKE_BFIXED(cg.frametime) * le->pos.trDelta[2] ) ) ) {
		le->pos.trType = TR_STATIONARY;
	} else {

	}
}

/*
================
CG_AddFragment
================
*/
void CG_AddFragment( localEntity_t *le ) {
	bvec3_t	newOrigin;
	trace_t	trace;

	if ( le->pos.trType == TR_STATIONARY ) {
		// sink into the ground if near the removal time
		int		t;
		bfixed	oldZ;
		
		t = le->endTime - cg.time;
		if ( t < SINK_TIME ) {
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			VectorCopy( le->refEntity.origin, le->refEntity.lightingOrigin );
			le->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
			oldZ = le->refEntity.origin[2];
			le->refEntity.origin[2] -= FIXED_MULPOW2(( BFIXED_1 - FIXED_INT32RATIO_B(t,SINK_TIME) ),4);
			_CG_trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.origin[2] = oldZ;
		} else {
			_CG_trap_R_AddRefEntityToScene( &le->refEntity );
		}

		return;
	}

	// calculate new position
	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

	// trace a line from previous position to new position
	CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, CONTENTS_SOLID );
	if ( trace.fraction == GFIXED_1 ) {
		// still in free fall
		VectorCopy( newOrigin, le->refEntity.origin );

		if ( le->leFlags & LEF_TUMBLE ) {
			avec3_t angles;

			BG_EvaluateTrajectory( &le->angles, cg.time, angles );
			AnglesToAxis( angles, le->refEntity.axis );
		}

		_CG_trap_R_AddRefEntityToScene( &le->refEntity );

		// add a blood trail
		if ( le->leBounceSoundType == LEBS_BLOOD ) {
			CG_BloodTrail( le );
		}

		return;
	}

	// if it is in a nodrop zone, remove it
	// this keeps gibs from waiting at the bottom of pits of death
	// and floating levels
	if ( _CG_trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FreeLocalEntity( le );
		return;
	}

	// leave a mark
	CG_FragmentBounceMark( le, &trace );

	// do a bouncy sound
	CG_FragmentBounceSound( le, &trace );

	// reflect the velocity on the trace plane
	CG_ReflectVelocity( le, &trace );

	_CG_trap_R_AddRefEntityToScene( &le->refEntity );
}

/*
=====================================================================

TRIVIAL LOCAL ENTITIES

These only do simple scaling or modulation before passing to the renderer
=====================================================================
*/

/*
====================
CG_AddFadeRGB
====================
*/
void CG_AddFadeRGB( localEntity_t *le ) {
	refEntity_t *re;
	gfixed c;

	re = &le->refEntity;

	c = MAKE_GFIXED( le->endTime - cg.time ) * le->lifeRate;
	c *= GFIXED(255,0);

	re->shaderRGBA[0] = FIXED_TO_INT(le->color[0] * c);
	re->shaderRGBA[1] = FIXED_TO_INT(le->color[1] * c);
	re->shaderRGBA[2] = FIXED_TO_INT(le->color[2] * c);
	re->shaderRGBA[3] = FIXED_TO_INT(le->color[3] * c);

	_CG_trap_R_AddRefEntityToScene( re );
}

/*
==================
CG_AddMoveScaleFade
==================
*/
static void CG_AddMoveScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	gfixed		c;
	bvec3_t		delta;
	bfixed		len;

	re = &le->refEntity;

	if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
		// fade / grow time
		c = GFIXED_1 - MAKE_GFIXED( le->fadeInTime - cg.time ) / MAKE_GFIXED( le->fadeInTime - le->startTime );
	}
	else {
		// fade / grow time
		c = MAKE_GFIXED( le->endTime - cg.time ) * le->lifeRate;
	}

	re->shaderRGBA[3] = FIXED_TO_INT( GFIXED(255,0) * c * le->color[3] );

	if ( !( le->leFlags & LEF_PUFF_DONT_SCALE ) ) {
		re->radius = (le->radius * ( GFIXED_1 - c )) + BFIXED(8,0);
	}

	BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = FIXED_VEC3LEN( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	_CG_trap_R_AddRefEntityToScene( re );
}


/*
===================
CG_AddScaleFade

For rocket smokes that hang in place, fade out, and are
removed if the view passes through them.
There are often many of these, so it needs to be simple.
===================
*/
static void CG_AddScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	gfixed		c;
	bvec3_t		delta;
	bfixed		len;

	re = &le->refEntity;

	// fade / grow time
	c = MAKE_GFIXED( le->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = FIXED_TO_INT(GFIXED(255,0) * c * le->color[3]);
	re->radius = (le->radius * ( GFIXED_1 - c )) + BFIXED(8,0);

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = FIXED_VEC3LEN( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	_CG_trap_R_AddRefEntityToScene( re );
}


/*
=================
CG_AddFallScaleFade

This is just an optimized CG_AddMoveScaleFade
For blood mists that drift down, fade out, and are
removed if the view passes through them.
There are often 100+ of these, so it needs to be simple.
=================
*/
static void CG_AddFallScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	gfixed		c;
	bvec3_t		delta;
	bfixed		len;

	re = &le->refEntity;

	// fade time
	c = MAKE_GFIXED( le->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = FIXED_TO_INT(GFIXED(255,0) * c * le->color[3]);

	re->origin[2] = le->pos.trBase[2] - MAKE_BFIXED( GFIXED_1 - c ) * le->pos.trDelta[2];

	re->radius = le->radius * MAKE_BFIXED( GFIXED_1 - c ) + BFIXED(16,0);

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = FIXED_VEC3LEN( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	_CG_trap_R_AddRefEntityToScene( re );
}



/*
================
CG_AddExplosion
================
*/
static void CG_AddExplosion( localEntity_t *ex ) {
	refEntity_t	*ent;

	ent = &ex->refEntity;

	// add the entity
	_CG_trap_R_AddRefEntityToScene(ent);

	// add the dlight
	if ( FIXED_NOT_ZERO(ex->light) ) {
		gfixed		light;

		light = FIXED_INT32RATIO_G(( cg.time - ex->startTime ),( ex->endTime - ex->startTime ));
		if ( light < GFIXED(0,5) ) {
			light = GFIXED_1;
		} else {
			light = GFIXED_1 - ( light - GFIXED(0,5) ) * GFIXED(2,0);
		}
		light = ex->light * light;
		_CG_trap_R_AddLightToScene(ent->origin, light, ex->lightColor[0], ex->lightColor[1], ex->lightColor[2] );
	}
}

/*
================
CG_AddSpriteExplosion
================
*/
static void CG_AddSpriteExplosion( localEntity_t *le ) {
	refEntity_t	re;
	gfixed c;

	re = le->refEntity;

	c = FIXED_INT32RATIO_G(( le->endTime - cg.time ),( le->endTime - le->startTime ));
	if ( c > GFIXED_1 ) {
		c = GFIXED_1;	// can happen during connection problems
	}

	re.shaderRGBA[0] = 255;
	re.shaderRGBA[1] = 255;
	re.shaderRGBA[2] = 255;
	re.shaderRGBA[3] = FIXED_TO_INT(GFIXED(255,0) * c * GFIXED(0,33));

	re.reType = RT_SPRITE;
	re.radius = BFIXED(42,0) * MAKE_BFIXED( GFIXED_1 - c ) + BFIXED(30,0);

	_CG_trap_R_AddRefEntityToScene( &re );

	// add the dlight
	if ( FIXED_NOT_ZERO(le->light) ) {
		gfixed		light;

		light = FIXED_INT32RATIO_G(( cg.time - le->startTime ),( le->endTime - le->startTime ));
		if ( light < GFIXED(0,5) ) {
			light = GFIXED_1;
		} else {
			light = GFIXED_1 - ( light - GFIXED(0,5) ) * GFIXED(2,0);
		}
		light = le->light * light;
		_CG_trap_R_AddLightToScene(re.origin, light, le->lightColor[0], le->lightColor[1], le->lightColor[2] );
	}
}


#ifdef MISSIONPACK
/*
====================
CG_AddKamikaze
====================
*/
void CG_AddKamikaze( localEntity_t *le ) {
	refEntity_t	*re;
	refEntity_t shockwave;
	gfixed		c;
	avec3_t		test, axis[3];
	int			t;

	re = &le->refEntity;

	t = cg.time - le->startTime;
	VectorClear( test );
	AnglesToAxis( test, axis );

	if (t > KAMI_SHOCKWAVE_STARTTIME && t < KAMI_SHOCKWAVE_ENDTIME) {

		if (!(le->leFlags & LEF_SOUND1)) {
//			_CG_trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.kamikazeExplodeSound );
			_CG_trap_S_StartLocalSound(cgs.media.kamikazeExplodeSound, CHAN_AUTO);
			le->leFlags |= LEF_SOUND1;
		}
		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.kamikazeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = MAKE_GFIXED(t - KAMI_SHOCKWAVE_STARTTIME) / MAKE_GFIXED(KAMI_SHOCKWAVE_ENDTIME - KAMI_SHOCKWAVE_STARTTIME);
		FIXED_VEC3SCALE( axis[0], c * KAMI_SHOCKWAVE_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		FIXED_VEC3SCALE( axis[1], c * KAMI_SHOCKWAVE_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		FIXED_VEC3SCALE( axis[2], c * KAMI_SHOCKWAVE_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qtrue;

		if (t > KAMI_SHOCKWAVEFADE_STARTTIME) {
			c = MAKE_GFIXED(t - KAMI_SHOCKWAVEFADE_STARTTIME) / MAKE_GFIXED(KAMI_SHOCKWAVE_ENDTIME - KAMI_SHOCKWAVEFADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		_CG_trap_R_AddRefEntityToScene( &shockwave );
	}

	if (t > KAMI_EXPLODE_STARTTIME && t < KAMI_IMPLODE_ENDTIME) {
		// explosion and implosion
		c = ( le->endTime - cg.time ) * le->lifeRate;
		c *= 0xff;
		re->shaderRGBA[0] = le->color[0] * c;
		re->shaderRGBA[1] = le->color[1] * c;
		re->shaderRGBA[2] = le->color[2] * c;
		re->shaderRGBA[3] = le->color[3] * c;

		if( t < KAMI_IMPLODE_STARTTIME ) {
			c = MAKE_GFIXED(t - KAMI_EXPLODE_STARTTIME) / MAKE_GFIXED(KAMI_IMPLODE_STARTTIME - KAMI_EXPLODE_STARTTIME);
		}
		else {
			if (!(le->leFlags & LEF_SOUND2)) {
//				_CG_trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.kamikazeImplodeSound );
				_CG_trap_S_StartLocalSound(cgs.media.kamikazeImplodeSound, CHAN_AUTO);
				le->leFlags |= LEF_SOUND2;
			}
			c = MAKE_GFIXED(KAMI_IMPLODE_ENDTIME - t) / MAKE_GFIXED (KAMI_IMPLODE_ENDTIME - KAMI_IMPLODE_STARTTIME);
		}
		FIXED_VEC3SCALE( axis[0], c * KAMI_BOOMSPHERE_MAXRADIUS / KAMI_BOOMSPHEREMODEL_RADIUS, re->axis[0] );
		FIXED_VEC3SCALE( axis[1], c * KAMI_BOOMSPHERE_MAXRADIUS / KAMI_BOOMSPHEREMODEL_RADIUS, re->axis[1] );
		FIXED_VEC3SCALE( axis[2], c * KAMI_BOOMSPHERE_MAXRADIUS / KAMI_BOOMSPHEREMODEL_RADIUS, re->axis[2] );
		re->nonNormalizedAxes = qtrue;

		_CG_trap_R_AddRefEntityToScene( re );
		// add the dlight
		_CG_trap_R_AddLightToScene( re->origin, c * GFIXED(1000,0), GFIXED_1, GFIXED_1, c );
	}

	if (t > KAMI_SHOCKWAVE2_STARTTIME && t < KAMI_SHOCKWAVE2_ENDTIME) {
		// 2nd kamikaze shockwave
		if (le->angles.trBase[0] == 0 &&
			le->angles.trBase[1] == 0 &&
			le->angles.trBase[2] == 0) {
			le->angles.trBase[0] = random() * 360;
			le->angles.trBase[1] = random() * 360;
			le->angles.trBase[2] = random() * 360;
		}
		else {
			c = 0;
		}
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.kamikazeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		test[0] = le->angles.trBase[0];
		test[1] = le->angles.trBase[1];
		test[2] = le->angles.trBase[2];
		AnglesToAxis( test, axis );

		c = MAKE_GFIXED(t - KAMI_SHOCKWAVE2_STARTTIME) / MAKE_GFIXED(KAMI_SHOCKWAVE2_ENDTIME - KAMI_SHOCKWAVE2_STARTTIME);
		FIXED_VEC3SCALE( axis[0], c * KAMI_SHOCKWAVE2_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		FIXED_VEC3SCALE( axis[1], c * KAMI_SHOCKWAVE2_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		FIXED_VEC3SCALE( axis[2], c * KAMI_SHOCKWAVE2_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qtrue;

		if (t > KAMI_SHOCKWAVE2FADE_STARTTIME) {
			c = MAKE_GFIXED(t - KAMI_SHOCKWAVE2FADE_STARTTIME) / MAKE_GFIXED(KAMI_SHOCKWAVE2_ENDTIME - KAMI_SHOCKWAVE2FADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		_CG_trap_R_AddRefEntityToScene( &shockwave );
	}
}

/*
===================
CG_AddInvulnerabilityImpact
===================
*/
void CG_AddInvulnerabilityImpact( localEntity_t *le ) {
	_CG_trap_R_AddRefEntityToScene( &le->refEntity );
}

/*
===================
CG_AddInvulnerabilityJuiced
===================
*/
void CG_AddInvulnerabilityJuiced( localEntity_t *le ) {
	int t;

	t = cg.time - le->startTime;
	if ( t > 3000 ) {
		le->refEntity.axis[0][0] = MAKE_GFIXED GFIXED_1 + GFIXED(0,3) * (t - 3000) / 2000;
		le->refEntity.axis[1][1] = MAKE_GFIXED GFIXED_1 + GFIXED(0,3) * (t - 3000) / 2000;
		le->refEntity.axis[2][2] = MAKE_GFIXED GFIXED(0,7) + GFIXED(0,3) * (2000 - (t - 3000)) / 2000;
	}
	if ( t > 5000 ) {
		le->endTime = 0;
		CG_GibPlayer( le->refEntity.origin );
	}
	else {
		_CG_trap_R_AddRefEntityToScene( &le->refEntity );
	}
}

/*
===================
CG_AddRefEntity
===================
*/
void CG_AddRefEntity( localEntity_t *le ) {
	if (le->endTime < cg.time) {
		CG_FreeLocalEntity( le );
		return;
	}
	_CG_trap_R_AddRefEntityToScene( &le->refEntity );
}

#endif
/*
===================
CG_AddScorePlum
===================
*/
#define NUMBER_SIZE		8

void CG_AddScorePlum( localEntity_t *le ) {
	refEntity_t	*re;
	bvec3_t		origin, delta, dir, vec;
	avec3_t		up = {AFIXED_0, AFIXED_0, AFIXED_1};
	gfixed		c;
	bfixed		len;
	int			i, score, digits[10], numdigits, negative;

	re = &le->refEntity;

	c = MAKE_GFIXED( le->endTime - cg.time ) * le->lifeRate;

	score = FIXED_TO_INT(le->radius);
	if (score < 0) {
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0x11;
		re->shaderRGBA[2] = 0x11;
	}
	else {
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		if (score >= 50) {
			re->shaderRGBA[1] = 0;
		} else if (score >= 20) {
			re->shaderRGBA[0] = re->shaderRGBA[1] = 0;
		} else if (score >= 10) {
			re->shaderRGBA[2] = 0;
		} else if (score >= 2) {
			re->shaderRGBA[0] = re->shaderRGBA[2] = 0;
		}

	}
	if (c < GFIXED(0,25))
		re->shaderRGBA[3] = FIXED_TO_INT(GFIXED(255,0) * GFIXED(4,0) * c);
	else
		re->shaderRGBA[3] = 255;

	re->radius = MAKE_BFIXED(NUMBER_SIZE / 2);

	VectorCopy(le->pos.trBase, origin);
	origin[2] += MAKE_BFIXED(GFIXED(110,0) - c * GFIXED(100,0));

	VectorSubtract(cg.refdef.vieworg, origin, dir);
	CrossProduct(dir, up, vec);
	VectorNormalize(vec);

	FIXED_VEC3MA(origin, MAKE_BFIXED(-GFIXED(10,0) + GFIXED(20,0) * FIXED_SIN(c * GFIXED(2,0) * GFIXED_PI)), vec, origin);

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( origin, cg.refdef.vieworg, delta );
	len = FIXED_VEC3LEN( delta );
	if ( len < BFIXED(20,0) ) {
		CG_FreeLocalEntity( le );
		return;
	}

	negative = qfalse;
	if (score < 0) {
		negative = qtrue;
		score = -score;
	}

	for (numdigits = 0; !(numdigits && !score); numdigits++) {
		digits[numdigits] = score % 10;
		score = score / 10;
	}

	if (negative) {
		digits[numdigits] = 10;
		numdigits++;
	}

	for (i = 0; i < numdigits; i++) {
		FIXED_VEC3MA(origin, MAKE_BFIXED(((MAKE_GFIXED(numdigits) / GFIXED(2,0)) - MAKE_GFIXED(i)) * MAKE_GFIXED(NUMBER_SIZE)), vec, re->origin);
		re->customShader = cgs.media.numberShaders[digits[numdigits-1-i]];
		_CG_trap_R_AddRefEntityToScene( re );
	}
}




//==============================================================================

/*
===================
CG_AddLocalEntities

===================
*/
void CG_AddLocalEntities( void ) {
	localEntity_t	*le, *next;

	// walk the list backwards, so any new local entities generated
	// (trails, marks, etc) will be present this frame
	le = cg_activeLocalEntities.prev;
	for ( ; le != &cg_activeLocalEntities ; le = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = le->prev;

		if ( cg.time >= le->endTime ) {
			CG_FreeLocalEntity( le );
			continue;
		}
		switch ( le->leType ) {
		default:
			CG_Error( "Bad leType: %i", le->leType );
			break;

		case LE_MARK:
			break;

		case LE_SPRITE_EXPLOSION:
			CG_AddSpriteExplosion( le );
			break;

		case LE_EXPLOSION:
			CG_AddExplosion( le );
			break;

		case LE_FRAGMENT:			// gibs and brass
			CG_AddFragment( le );
			break;

		case LE_MOVE_SCALE_FADE:		// water bubbles
			CG_AddMoveScaleFade( le );
			break;

		case LE_FADE_RGB:				// teleporters, railtrails
			CG_AddFadeRGB( le );
			break;

		case LE_FALL_SCALE_FADE: // gib blood trails
			CG_AddFallScaleFade( le );
			break;

		case LE_SCALE_FADE:		// rocket trails
			CG_AddScaleFade( le );
			break;

		case LE_SCOREPLUM:
			CG_AddScorePlum( le );
			break;

#ifdef MISSIONPACK
		case LE_KAMIKAZE:
			CG_AddKamikaze( le );
			break;
		case LE_INVULIMPACT:
			CG_AddInvulnerabilityImpact( le );
			break;
		case LE_INVULJUICED:
			CG_AddInvulnerabilityJuiced( le );
			break;
		case LE_SHOWREFENTITY:
			CG_AddRefEntity( le );
			break;
#endif
		}
	}
}




