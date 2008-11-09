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
// cg_weapons.c -- events and effects dealing with weapons
#include"cgame_pch.h"

/*
==========================
CG_MachineGunEjectBrass
==========================
*/
static void CG_MachineGunEjectBrass( centity_t *cent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	bvec3_t			velocity, xvelocity;
	bvec3_t			offset, xoffset;
	bfixed			waterScale = BFIXED_1;
	avec3_t			v[3];

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = BFIXED_0;
	velocity[1] = -BFIXED(50,0) + (BFIXED(40,0) * crandom_b());
	velocity[2] = BFIXED(100,0) + (BFIXED(50,0) * crandom_b());

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + (rand() % ( cg_brassTime.integer / 4 ) );

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = BFIXED(8,0);
	offset[1] = -BFIXED(4,0);
	offset[2] = BFIXED(24,0);

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, re->origin );

	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = BFIXED(0,10);
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	FIXED_VEC3SCALE( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( aaxisDefault, re->axis );
	re->hModel = cgs.media.machinegunBrassModel;

	le->bounceFactor = BFIXED(0,4) * MAKE_BFIXED(waterScale);

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = MAKE_AFIXED(rand()&31);
	le->angles.trBase[1] = MAKE_AFIXED(rand()&31);
	le->angles.trBase[2] = MAKE_AFIXED(rand()&31);
	le->angles.trDelta[0] = AFIXED(2,0);
	le->angles.trDelta[1] = AFIXED_1;
	le->angles.trDelta[2] = AFIXED_0;

	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass( centity_t *cent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	bvec3_t			velocity, xvelocity;
	bvec3_t			offset, xoffset;
	avec3_t			v[3];
	int				i;

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	for ( i = 0; i < 2; i++ ) {
		bfixed	waterScale = BFIXED_1;

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		velocity[0] = BFIXED(60,0) + BFIXED(60,0) * crandom_b();
		if ( i == 0 ) {
			velocity[1] = BFIXED(40,0) + BFIXED(10,0) * crandom_b();
		} else {
			velocity[1] = -BFIXED(40,0) + BFIXED(10,0) * crandom_b();
		}
		velocity[2] = BFIXED(100,0) + BFIXED(50,0) * crandom_b();

		le->leType = LE_FRAGMENT;
		le->startTime = cg.time;
		le->endTime = le->startTime + cg_brassTime.integer*3 + (rand() % cg_brassTime.integer);

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		AnglesToAxis( cent->lerpAngles, v );

		offset[0] = BFIXED(8,0);
		offset[1] = BFIXED_0;
		offset[2] = BFIXED(24,0);

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
		VectorAdd( cent->lerpOrigin, xoffset, re->origin );
		VectorCopy( re->origin, le->pos.trBase );
		if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
			waterScale = BFIXED(0,10);
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		FIXED_VEC3SCALE( xvelocity, waterScale, le->pos.trDelta );

		AxisCopy( aaxisDefault, re->axis );
		re->hModel = cgs.media.shotgunBrassModel;
		le->bounceFactor = BFIXED(0,3);

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = MAKE_AFIXED(rand()&31);
		le->angles.trBase[1] = MAKE_AFIXED(rand()&31);
		le->angles.trBase[2] = MAKE_AFIXED(rand()&31);
		le->angles.trDelta[0] = AFIXED_1;
		le->angles.trDelta[1] = AFIXED(0,5);
		le->angles.trDelta[2] = AFIXED_0;

		le->leFlags = LEF_TUMBLE;
		le->leBounceSoundType = LEBS_BRASS;
		le->leMarkType = LEMT_NONE;
	}
}


#ifdef MISSIONPACK
/*
==========================
CG_NailgunEjectBrass
==========================
*/
static void CG_NailgunEjectBrass( centity_t *cent ) {
	localEntity_t	*smoke;
	bvec3_t			origin;
	bvec3_t			v[3];
	bvec3_t			offset;
	bvec3_t			xoffset;
	bvec3_t			up;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 0;
	offset[1] = -12;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, origin );

	VectorSet( up, GFIXED_0, GFIXED_0, GFIXED(64,0) );

	smoke = CG_SmokePuff( origin, up, 32, 1, 1, 1, GFIXED(0,33), 700, cg.time, 0, 0, cgs.media.smokePuffShader );
	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}
#endif


/*
==========================
CG_RailTrail
==========================
*/
void CG_RailTrail (clientInfo_t *ci, bvec3_t start, bvec3_t end) {
	avec3_t axis[36];
	bvec3_t move, move2, next_move, vec;
	avec3_t avec,temp;
	int len;
	int    i, j, skip;
 
	localEntity_t *le;
	refEntity_t   *re;
 
#define RADIUS   4
#define ROTATION 1
#define SPACING  5
 
	start[2] -= BFIXED(4,0);
	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = FIXED_TO_INT(VectorNormalizeB2A(vec,avec));
	PerpendicularVector(temp, avec);
	for (i = 0 ; i < 36; i++) {
		RotatePointAroundVector(axis[i], avec, temp, MAKE_AFIXED(i * 10));//banshee GFIXED(2,4) was 10
	}
 
	le = CG_AllocLocalEntity();
	re = &le->refEntity;
 
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + FIXED_TO_INT(cg_railTrailTime.value);
	le->lifeRate = FIXED_INT32RATIO_G(1,(le->endTime - le->startTime));
 
	re->shaderTime = MSECTIME_G(cg.time);
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;
 
	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);
 
	re->shaderRGBA[0] = FIXED_TO_INT(ci->color1[0] * GFIXED(255,0));
    re->shaderRGBA[1] = FIXED_TO_INT(ci->color1[1] * GFIXED(255,0));
    re->shaderRGBA[2] = FIXED_TO_INT(ci->color1[2] * GFIXED(255,0));
    re->shaderRGBA[3] = 255;

	le->color[0] = ci->color1[0] * GFIXED(0,75);
	le->color[1] = ci->color1[1] * GFIXED(0,75);
	le->color[2] = ci->color1[2] * GFIXED(0,75);
	le->color[3] = GFIXED_1;

	AxisClear( re->axis );
 
	FIXED_VEC3MA(move, BFIXED(20,0), vec, move);
	VectorCopy(move, next_move);
	FIXED_VEC3SCALE (vec, BFIXED(SPACING,0), vec);

	if (cg_oldRail.integer != 0) {
		// nudge down a bit so it isn't exactly in center
		re->origin[2] -= BFIXED(8,0);
		re->oldorigin[2] -= BFIXED(8,0);
		return;
	}
	skip = -1;
 
	j = 18;
    for (i = 0; i < len; i += SPACING) {
		if (i != skip) {
			skip = i + SPACING;
			le = CG_AllocLocalEntity();
            re = &le->refEntity;
            le->leFlags = LEF_PUFF_DONT_SCALE;
			le->leType = LE_MOVE_SCALE_FADE;
            le->startTime = cg.time;
            le->endTime = cg.time + (i>>1) + 600;
            le->lifeRate = FIXED_INT32RATIO_G(1,(le->endTime - le->startTime));

            re->shaderTime = MSECTIME_G(cg.time);
            re->reType = RT_SPRITE;
            re->radius = BFIXED(1,1);
			re->customShader = cgs.media.railRingsShader;

            re->shaderRGBA[0] = FIXED_TO_INT(ci->color2[0] * GFIXED(255,0));
            re->shaderRGBA[1] = FIXED_TO_INT(ci->color2[1] * GFIXED(255,0));
            re->shaderRGBA[2] = FIXED_TO_INT(ci->color2[2] * GFIXED(255,0));
            re->shaderRGBA[3] = 255;

            le->color[0] = ci->color2[0] * GFIXED(0,75);
            le->color[1] = ci->color2[1] * GFIXED(0,75);
            le->color[2] = ci->color2[2] * GFIXED(0,75);
            le->color[3] = GFIXED_1;

            le->pos.trType = TR_LINEAR;
            le->pos.trTime = cg.time;

			VectorCopy( move, move2);
            FIXED_VEC3MA_R(move2, BFIXED(RADIUS,0) , axis[j], move2);
            VectorCopy(move2, le->pos.trBase);

            le->pos.trDelta[0] = axis[j][0]*BFIXED(6,0);
            le->pos.trDelta[1] = axis[j][1]*BFIXED(6,0);
            le->pos.trDelta[2] = axis[j][2]*BFIXED(6,0);
		}

        VectorAdd (move, vec, move);

        j = j + ROTATION < 36 ? j + ROTATION : (j + ROTATION) % 36;
	}
}

/*
==========================
CG_RocketTrail
==========================
*/
static void CG_RocketTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	bvec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	bvec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = BFIXED_0;
	up[1] = BFIXED_0;
	up[2] = BFIXED_0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, BFIXED(8,0) );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up, 
					  wi->trailRadius, 
					  GFIXED_1, GFIXED_1, GFIXED_1, GFIXED(0,33),
					  wi->wiTrailTime, 
					  t,
					  0,
					  0, 
					  cgs.media.smokePuffShader );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}

#ifdef MISSIONPACK
/*
==========================
CG_NailTrail
==========================
*/
static void CG_NailTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	bvec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	bvec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up, 
					  wi->trailRadius, 
					  1, 1, 1, GFIXED(0,33),
					  wi->wiTrailTime, 
					  t,
					  0,
					  0, 
					  cgs.media.nailPuffShader );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}
#endif

/*
==========================
CG_NailTrail
==========================
*/
static void CG_PlasmaTrail( centity_t *cent, const weaponInfo_t *wi ) {
	localEntity_t	*le;
	refEntity_t		*re;
	entityState_t	*es;
	bvec3_t			velocity, xvelocity, origin;
	bvec3_t			offset, xoffset;
	avec3_t			v[3];
	int				t, startTime, step;

	bfixed	waterScale = BFIXED_1;

	if ( cg_noProjectileTrail.integer || cg_oldPlasma.integer ) {
		return;
	}

	step = 50;

	es = &cent->currentState;
	startTime = cent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = BFIXED(60,0) - BFIXED(120,0) * crandom_b();
	velocity[1] = BFIXED(40,0) - BFIXED(80,0) * crandom_b();
	velocity[2] = BFIXED(100,0) - BFIXED(200,0) * crandom_b();

	le->leType = LE_MOVE_SCALE_FADE;
	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;

	le->startTime = cg.time;
	le->endTime = le->startTime + 600;

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = BFIXED(2,0);
	offset[1] = BFIXED(2,0);
	offset[2] = BFIXED(2,0);

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

	VectorAdd( origin, xoffset, re->origin );
	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = BFIXED(0,10);
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	FIXED_VEC3SCALE( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( aaxisDefault, re->axis );
    re->shaderTime = MSECTIME_G(cg.time);
    re->reType = RT_SPRITE;
    re->radius = BFIXED(0,25);
	re->customShader = cgs.media.railRingsShader;
	le->bounceFactor = BFIXED(0,3);

    re->shaderRGBA[0] = FIXED_TO_INT(wi->flashDlightColor[0] * GFIXED(63,0));
    re->shaderRGBA[1] = FIXED_TO_INT(wi->flashDlightColor[1] * GFIXED(63,0));
    re->shaderRGBA[2] = FIXED_TO_INT(wi->flashDlightColor[2] * GFIXED(63,0));
    re->shaderRGBA[3] = 63;

    le->color[0] = wi->flashDlightColor[0] * GFIXED(0,2);
    le->color[1] = wi->flashDlightColor[1] * GFIXED(0,2);
    le->color[2] = wi->flashDlightColor[2] * GFIXED(0,2);
    le->color[3] = GFIXED(0,25);

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = MAKE_AFIXED(rand()&31);
	le->angles.trBase[1] = MAKE_AFIXED(rand()&31);
	le->angles.trBase[2] = MAKE_AFIXED(rand()&31);
	le->angles.trDelta[0] = AFIXED_1;
	le->angles.trDelta[1] = AFIXED(0,5);
	le->angles.trDelta[2] = AFIXED_0;

}
/*
==========================
CG_GrappleTrail
==========================
*/
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi ) {
	bvec3_t	origin;
	entityState_t	*es;
	avec3_t			forward, up;
	refEntity_t		beam;

	es = &ent->currentState;

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	ent->trailTime = cg.time;

	memset( &beam, 0, sizeof( beam ) );
	//FIXME adjust for muzzle position
	VectorCopy ( cg_entities[ ent->currentState.otherEntityNum ].lerpOrigin, beam.origin );
	beam.origin[2] += BFIXED(26,0);
	AngleVectors( cg_entities[ ent->currentState.otherEntityNum ].lerpAngles, forward, NULL, up );
	FIXED_VEC3MA_R( beam.origin, -BFIXED(6,0), up, beam.origin );
	VectorCopy( origin, beam.oldorigin );

	if (Distance( beam.origin, beam.oldorigin ) < BFIXED(64,0) )
		return; // Don't draw if close

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;

	AxisClear( beam.axis );
	beam.shaderRGBA[0] = 0xff;
	beam.shaderRGBA[1] = 0xff;
	beam.shaderRGBA[2] = 0xff;
	beam.shaderRGBA[3] = 0xff;
	_CG_trap_R_AddRefEntityToScene( &beam );
}

/*
==========================
CG_GrenadeTrail
==========================
*/
static void CG_GrenadeTrail( centity_t *ent, const weaponInfo_t *wi ) {
	CG_RocketTrail( ent, wi );
}


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon( int weaponNum ) {
	weaponInfo_t	*weaponInfo;
	gitem_t			*item, *ammo;
	char			path[MAX_QPATH];
	bvec3_t			mins, maxs;
	int				i;

	weaponInfo = &cg_weapons[weaponNum];

	if ( weaponNum == 0 ) {
		return;
	}

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			weaponInfo->item = item;
			break;
		}
	}
	if ( !item->classname ) {
		CG_Error( "Couldn't find weapon %i", weaponNum );
	}
	CG_RegisterItemVisuals( item - bg_itemlist );

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = _CG_trap_R_RegisterModel( item->world_model[0] );

	// calc midpoint for rotation
	_CG_trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + BFIXED(0,5) * ( maxs[i] - mins[i] );
	}

	weaponInfo->weaponIcon = _CG_trap_R_RegisterShader( item->icon );
	weaponInfo->ammoIcon = _CG_trap_R_RegisterShader( item->icon );

	for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ ) {
		if ( ammo->giType == IT_AMMO && ammo->giTag == weaponNum ) {
			break;
		}
	}
	if ( ammo->classname && ammo->world_model[0] ) {
		weaponInfo->ammoModel = _CG_trap_R_RegisterModel( ammo->world_model[0] );
	}

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path );
	strcat( path, "_flash.md3" );
	weaponInfo->flashModel = _CG_trap_R_RegisterModel( path );

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path );
	strcat( path, "_barrel.md3" );
	weaponInfo->barrelModel = _CG_trap_R_RegisterModel( path );

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path );
	strcat( path, "_hand.md3" );
	weaponInfo->handsModel = _CG_trap_R_RegisterModel( path );

	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = _CG_trap_R_RegisterModel( "models/weapons2/shotgun/shotgun_hand.md3" );
	}

	weaponInfo->loopFireSound = qfalse;

	switch ( weaponNum ) {
	case WP_GAUNTLET:
		MAKERGB( weaponInfo->flashDlightColor, GFIXED(0,6), GFIXED(0,6), GFIXED_1 );
		weaponInfo->firingSound = _CG_trap_S_RegisterSound( "sound/weapons/melee/fstrun.wav", qfalse );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/melee/fstatck.wav", qfalse );
		break;

	case WP_LIGHTNING:
		MAKERGB( weaponInfo->flashDlightColor, GFIXED(0,6), GFIXED(0,6), GFIXED_1 );
		weaponInfo->readySound = _CG_trap_S_RegisterSound( "sound/weapons/melee/fsthum.wav", qfalse );
		weaponInfo->firingSound = _CG_trap_S_RegisterSound( "sound/weapons/lightning/lg_hum.wav", qfalse );

		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/lightning/lg_fire.wav", qfalse );
		cgs.media.lightningShader = _CG_trap_R_RegisterShader( "lightningBoltNew");
		cgs.media.lightningExplosionModel = _CG_trap_R_RegisterModel( "models/weaphits/crackle.md3" );
		cgs.media.sfx_lghit1 = _CG_trap_S_RegisterSound( "sound/weapons/lightning/lg_hit.wav", qfalse );
		cgs.media.sfx_lghit2 = _CG_trap_S_RegisterSound( "sound/weapons/lightning/lg_hit2.wav", qfalse );
		cgs.media.sfx_lghit3 = _CG_trap_S_RegisterSound( "sound/weapons/lightning/lg_hit3.wav", qfalse );

		break;

	case WP_GRAPPLING_HOOK:
		MAKERGB( weaponInfo->flashDlightColor, GFIXED(0,6), GFIXED(0,6), GFIXED_1 );
		weaponInfo->missileModel = _CG_trap_R_RegisterModel( "models/ammo/rocket/rocket.md3" );
		weaponInfo->missileTrailFunc = CG_GrappleTrail;
		weaponInfo->missileDlight = GFIXED(200,0);
		weaponInfo->wiTrailTime = GFIXED(2000,0);
		weaponInfo->trailRadius = BFIXED(64,0);
		MAKERGB( weaponInfo->missileDlightColor, GFIXED_1, GFIXED(0,75), GFIXED_0 );
		weaponInfo->readySound = _CG_trap_S_RegisterSound( "sound/weapons/melee/fsthum.wav", qfalse );
		weaponInfo->firingSound = _CG_trap_S_RegisterSound( "sound/weapons/melee/fstrun.wav", qfalse );
		break;

#ifdef MISSIONPACK
	case WP_CHAINGUN:
		weaponInfo->firingSound = _CG_trap_S_RegisterSound( "sound/weapons/vulcan/wvulfire.wav", qfalse );
		weaponInfo->loopFireSound = qtrue;
		MAKERGB( weaponInfo->flashDlightColor, GFIXED_1, GFIXED_1, GFIXED_0 );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf1b.wav", qfalse );
		weaponInfo->flashSound[1] = _CG_trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf2b.wav", qfalse );
		weaponInfo->flashSound[2] = _CG_trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf3b.wav", qfalse );
		weaponInfo->flashSound[3] = _CG_trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = _CG_trap_R_RegisterShader( "bulletExplosion" );
		break;
#endif

	case WP_MACHINEGUN:
		MAKERGB( weaponInfo->flashDlightColor, GFIXED_1, GFIXED_1, GFIXED_0 );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/machinegun/machgf1b.wav", qfalse );
		weaponInfo->flashSound[1] = _CG_trap_S_RegisterSound( "sound/weapons/machinegun/machgf2b.wav", qfalse );
		weaponInfo->flashSound[2] = _CG_trap_S_RegisterSound( "sound/weapons/machinegun/machgf3b.wav", qfalse );
		weaponInfo->flashSound[3] = _CG_trap_S_RegisterSound( "sound/weapons/machinegun/machgf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = _CG_trap_R_RegisterShader( "bulletExplosion" );
		break;

	case WP_SHOTGUN:
		MAKERGB( weaponInfo->flashDlightColor, GFIXED_1, GFIXED_1, GFIXED_0 );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/shotgun/sshotf1b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
		break;

	case WP_ROCKET_LAUNCHER:
		weaponInfo->missileModel = _CG_trap_R_RegisterModel( "models/ammo/rocket/rocket.md3" );
		weaponInfo->missileSound = _CG_trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
		weaponInfo->missileTrailFunc = CG_RocketTrail;
		weaponInfo->missileDlight = GFIXED(200,0);
		weaponInfo->wiTrailTime = GFIXED(2000,0);
		weaponInfo->trailRadius = BFIXED(64,0);
		
		MAKERGB( weaponInfo->missileDlightColor, GFIXED_1, GFIXED(0,75), GFIXED_0 );
		MAKERGB( weaponInfo->flashDlightColor, GFIXED_1, GFIXED(0,75), GFIXED_0 );

		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
		cgs.media.rocketExplosionShader = _CG_trap_R_RegisterShader( "rocketExplosion" );
		break;

#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		weaponInfo->missileModel = _CG_trap_R_RegisterModel( "models/weaphits/proxmine.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = GFIXED(700,0);
		weaponInfo->trailRadius = BFIXED(32,0);
		MAKERGB( weaponInfo->flashDlightColor, GFIXED_1, GFIXED(0,70), GFIXED_0 );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/proxmine/wstbfire.wav", qfalse );
		cgs.media.grenadeExplosionShader = _CG_trap_R_RegisterShader( "grenadeExplosion" );
		break;
#endif

	case WP_GRENADE_LAUNCHER:
		weaponInfo->missileModel = _CG_trap_R_RegisterModel( "models/ammo/grenade1.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = GFIXED(700,0);
		weaponInfo->trailRadius = BFIXED(32,0);
		MAKERGB( weaponInfo->flashDlightColor, GFIXED_1, GFIXED(0,70), GFIXED_0 );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/grenade/grenlf1a.wav", qfalse );
		cgs.media.grenadeExplosionShader = _CG_trap_R_RegisterShader( "grenadeExplosion" );
		break;

#ifdef MISSIONPACK
	case WP_NAILGUN:
		weaponInfo->ejectBrassFunc = CG_NailgunEjectBrass;
		weaponInfo->missileTrailFunc = CG_NailTrail;
//		weaponInfo->missileSound = _CG_trap_S_RegisterSound( "sound/weapons/nailgun/wnalflit.wav", qfalse );
		weaponInfo->trailRadius = BFIXED(16,0);
		weaponInfo->wiTrailTime = GFIXED(250,0);
		weaponInfo->missileModel = _CG_trap_R_RegisterModel( "models/weaphits/nail.md3" );
		MAKERGB( weaponInfo->flashDlightColor, GFIXED_1, GFIXED(0,75), GFIXED_0 );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/nailgun/wnalfire.wav", qfalse );
		break;
#endif

	case WP_PLASMAGUN:
//		weaponInfo->missileModel = cgs.media.invulnerabilityPowerupModel;
		weaponInfo->missileTrailFunc = CG_PlasmaTrail;
		weaponInfo->missileSound = _CG_trap_S_RegisterSound( "sound/weapons/plasma/lasfly.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, GFIXED(0,6), GFIXED(0,6), GFIXED_1 );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/plasma/hyprbf1a.wav", qfalse );
		cgs.media.plasmaExplosionShader = _CG_trap_R_RegisterShader( "plasmaExplosion" );
		cgs.media.railRingsShader = _CG_trap_R_RegisterShader( "railDisc" );
		break;

	case WP_RAILGUN:
		weaponInfo->readySound = _CG_trap_S_RegisterSound( "sound/weapons/railgun/rg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, GFIXED_1, GFIXED(0,5), GFIXED_0 );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/railgun/railgf1a.wav", qfalse );
		cgs.media.railExplosionShader = _CG_trap_R_RegisterShader( "railExplosion" );
		cgs.media.railRingsShader = _CG_trap_R_RegisterShader( "railDisc" );
		cgs.media.railCoreShader = _CG_trap_R_RegisterShader( "railCore" );
		break;

	case WP_BFG:
		weaponInfo->readySound = _CG_trap_S_RegisterSound( "sound/weapons/bfg/bfg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, GFIXED_1, GFIXED(0,7), GFIXED_1 );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/bfg/bfg_fire.wav", qfalse );
		cgs.media.bfgExplosionShader = _CG_trap_R_RegisterShader( "bfgExplosion" );
		weaponInfo->missileModel = _CG_trap_R_RegisterModel( "models/weaphits/bfg.md3" );
		weaponInfo->missileSound = _CG_trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
		break;

	 default:
		MAKERGB( weaponInfo->flashDlightColor, GFIXED_1, GFIXED_1, GFIXED_1 );
		weaponInfo->flashSound[0] = _CG_trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
		break;
	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;

	if ( itemNum < 0 || itemNum >= bg_numItems ) {
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = _CG_trap_R_RegisterModel( item->world_model[0] );

	itemInfo->icon = _CG_trap_R_RegisterShader( item->icon );

	if ( item->giType == IT_WEAPON ) {
		CG_RegisterWeapon( item->giTag );
	}

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_POWERUP || item->giType == IT_HEALTH || 
		item->giType == IT_ARMOR || item->giType == IT_HOLDABLE ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = _CG_trap_R_RegisterModel( item->world_model[1] );
		}
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
=================
CG_MapTorsoToWeaponFrame

=================
*/
static int CG_MapTorsoToWeaponFrame( clientInfo_t *ci, int frame ) {

	// change weapon
	if ( frame >= ci->animations[TORSO_DROP].firstFrame 
		&& frame < ci->animations[TORSO_DROP].firstFrame + 9 ) {
		return frame - ci->animations[TORSO_DROP].firstFrame + 6;
	}

	// stand attack
	if ( frame >= ci->animations[TORSO_ATTACK].firstFrame 
		&& frame < ci->animations[TORSO_ATTACK].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK].firstFrame;
	}

	// stand attack 2
	if ( frame >= ci->animations[TORSO_ATTACK2].firstFrame 
		&& frame < ci->animations[TORSO_ATTACK2].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK2].firstFrame;
	}
	
	return 0;
}


/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition( bvec3_t origin, avec3_t angles ) {
	gfixed	scale;
	int		delta;
	gfixed	fracsin;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdefViewAngles, angles );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += MAKE_AFIXED(scale * cg.bobfracsin * GFIXED(0,005));
	angles[YAW] += MAKE_AFIXED(scale * cg.bobfracsin * GFIXED(0,01));
	angles[PITCH] += MAKE_AFIXED(cg.xyspeed * cg.bobfracsin * GFIXED(0,005));

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin[2] += FIXED_DIVPOW2(cg.landChange,2) * FIXED_INT32RATIO_B(delta,LAND_DEFLECT_TIME);
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin[2] += FIXED_DIVPOW2(cg.landChange,2) * FIXED_INT32RATIO_B(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta,LAND_RETURN_TIME);
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*GFIXED(0,25) * MAKE_GFIXED(delta) / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*GFIXED(0,25) * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + GFIXED(40,0);
	fracsin = FIXED_SIN( MSECTIME_G(cg.time) );
	angles[ROLL] += MAKE_AFIXED(scale * fracsin * GFIXED(0,01));
	angles[YAW] += MAKE_AFIXED(scale * fracsin * GFIXED(0,01));
	angles[PITCH] += MAKE_AFIXED(scale * fracsin * GFIXED(0,01));
}


/*
===============
CG_LightningBolt

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
The cent should be the non-predicted cent if it is from the player,
so the endpoint will reflect the simulated strike (lagging the predicted
angle)
===============
*/
static void CG_LightningBolt( centity_t *cent, bvec3_t origin ) {
	trace_t  trace;
	refEntity_t  beam;
	avec3_t   forward;
	bvec3_t   muzzlePoint, endPoint;

	if (cent->currentState.weapon != WP_LIGHTNING) {
		return;
	}

	memset( &beam, 0, sizeof( beam ) );

	// CPMA  "true" lightning
	if ((cent->currentState.number == cg.predictedPlayerState.clientNum) && (cg_trueLightning.value != LFIXED_0)) {
		avec3_t angle;
		int i;

		for (i = 0; i < 3; i++) {
			afixed a = cent->lerpAngles[i] - cg.refdefViewAngles[i];
			if (a > AFIXED(180,0)) {
				a -= AFIXED(360,0);
			}
			if (a < -AFIXED(180,0)) {
				a += AFIXED(360,0);
			}

			angle[i] = cg.refdefViewAngles[i] + a * (AFIXED_1 - MAKE_AFIXED(cg_trueLightning.value));
			if (angle[i] < AFIXED_0) {
				angle[i] += AFIXED(360,0);
			}
			if (angle[i] > AFIXED(360,0)) {
				angle[i] -= AFIXED(360,0);
			}
		}

		AngleVectors(angle, forward, NULL, NULL );
		VectorCopy(cent->lerpOrigin, muzzlePoint );
//		VectorCopy(cg.refdef.vieworg, muzzlePoint );
	} else {
		// !CPMA
		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		VectorCopy(cent->lerpOrigin, muzzlePoint );
	}

	// FIXME: crouch
	muzzlePoint[2] += BFIXED(DEFAULT_VIEWHEIGHT,0);

	FIXED_VEC3MA_R( muzzlePoint, BFIXED(14,0), forward, muzzlePoint );

	// project forward by the lightning range
	FIXED_VEC3MA_R( muzzlePoint, BFIXED(LIGHTNING_RANGE,0), forward, endPoint );

	// see if it hit a wall
	CG_Trace( &trace, muzzlePoint, bvec3_origin, bvec3_origin, endPoint, 
		cent->currentState.number, MASK_SHOT );

	// this is the endpoint
	VectorCopy( trace.endpos, beam.oldorigin );

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy( origin, beam.origin );

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	_CG_trap_R_AddRefEntityToScene( &beam );

	// add the impact flare if it hit something
	if ( trace.fraction < GFIXED_1 ) {
		avec3_t	angles;
		bvec3_t	dir;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		memset( &beam, 0, sizeof( beam ) );
		beam.hModel = cgs.media.lightningExplosionModel;

		FIXED_VEC3MA( trace.endpos, -BFIXED(16,0), dir, beam.origin );

		// make a random orientation
		angles[0] = MAKE_AFIXED(rand() % 360);
		angles[1] = MAKE_AFIXED(rand() % 360);
		angles[2] = MAKE_AFIXED(rand() % 360);
		AnglesToAxis( angles, beam.axis );
		_CG_trap_R_AddRefEntityToScene( &beam );
	}
}
/*

static void CG_LightningBolt( centity_t *cent, bvec3_t origin ) {
	trace_t		trace;
	refEntity_t		beam;
	bvec3_t			forward;
	bvec3_t			muzzlePoint, endPoint;

	if ( cent->currentState.weapon != WP_LIGHTNING ) {
		return;
	}

	memset( &beam, 0, sizeof( beam ) );

	// find muzzle point for this frame
	VectorCopy( cent->lerpOrigin, muzzlePoint );
	AngleVectors( cent->lerpAngles, forward, NULL, NULL );

	// FIXME: crouch
	muzzlePoint[2] += DEFAULT_VIEWHEIGHT;

	FIXED_VEC3MA( muzzlePoint, GFIXED(14,0), forward, muzzlePoint );

	// project forward by the lightning range
	FIXED_VEC3MA( muzzlePoint, GFIXED(LIGHTNING_RANGE,0), forward, endPoint );

	// see if it hit a wall
	CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, 
		cent->currentState.number, MASK_SHOT );

	// this is the endpoint
	VectorCopy( trace.endpos, beam.oldorigin );

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy( origin, beam.origin );

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	_CG_trap_R_AddRefEntityToScene( &beam );

	// add the impact flare if it hit something
	if ( trace.fraction < GFIXED_1 ) {
		bvec3_t	angles;
		bvec3_t	dir;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		memset( &beam, 0, sizeof( beam ) );
		beam.hModel = cgs.media.lightningExplosionModel;

		FIXED_VEC3MA( trace.endpos, -GFIXED(16,0), dir, beam.origin );

		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;
		AnglesToAxis( angles, beam.axis );
		_CG_trap_R_AddRefEntityToScene( &beam );
	}
}
*/

/*
===============
CG_SpawnRailTrail

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
===============
*/
static void CG_SpawnRailTrail( centity_t *cent, bvec3_t origin ) {
	clientInfo_t	*ci;

	if ( cent->currentState.weapon != WP_RAILGUN ) {
		return;
	}
	if ( !cent->pe.railgunFlash ) {
		return;
	}
	cent->pe.railgunFlash = qtrue;
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	CG_RailTrail( ci, origin, cent->pe.railgunImpact );
}


/*
======================
CG_MachinegunSpinAngle
======================
*/
#define		SPIN_SPEED	BFIXED(0,9)
#define		COAST_TIME	1000
static afixed	CG_MachinegunSpinAngle( centity_t *cent ) {
	int		delta;
	afixed	angle;
	bfixed	speed;

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = AngleModB2A(MAKE_BFIXED(cent->pe.barrelAngle) + MAKE_BFIXED(delta) * SPIN_SPEED);
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = FIXED_DIVPOW2(( SPIN_SPEED + FIXED_INT32RATIO_B( COAST_TIME - delta, COAST_TIME ) ),1);
		angle = AngleModB2A(MAKE_BFIXED(cent->pe.barrelAngle) + MAKE_BFIXED(delta) * speed);
	}

	if ( cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING) ) {
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = angle;
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);
#ifdef MISSIONPACK
		if ( cent->currentState.weapon == WP_CHAINGUN && !cent->pe.barrelSpinning ) {
			_CG_trap_S_StartSound( NULL, cent->currentState.number, CHAN_WEAPON, _CG_trap_S_RegisterSound( "sound/weapons/vulcan/wvulwind.wav", qfalse ) );
		}
#endif
	}

	return angle;
}


/*
========================
CG_AddWeaponWithPowerups
========================
*/
static void CG_AddWeaponWithPowerups( refEntity_t *gun, int powerups ) {
	// add powerup effects
	if ( powerups & ( 1 << PW_INVIS ) ) {
		gun->customShader = cgs.media.invisShader;
		_CG_trap_R_AddRefEntityToScene( gun );
	} else {
		_CG_trap_R_AddRefEntityToScene( gun );

		if ( powerups & ( 1 << PW_BATTLESUIT ) ) {
			gun->customShader = cgs.media.battleWeaponShader;
			_CG_trap_R_AddRefEntityToScene( gun );
		}
		if ( powerups & ( 1 << PW_QUAD ) ) {
			gun->customShader = cgs.media.quadWeaponShader;
			_CG_trap_R_AddRefEntityToScene( gun );
		}
	}
}


/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team ) {
	refEntity_t	gun;
	refEntity_t	barrel;
	refEntity_t	flash;
	avec3_t		angles;
	weapon_t	weaponNum;
	weaponInfo_t	*weapon;
	centity_t	*nonPredictedCent;
//	int	col;

	weaponNum = (weapon_t)cent->currentState.weapon;

	CG_RegisterWeapon( weaponNum );
	weapon = &cg_weapons[weaponNum];

	// add the weapon
	memset( &gun, 0, sizeof( gun ) );
	VectorCopy( parent->lightingOrigin, gun.lightingOrigin );
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;

	// set custom shading for railgun refire rate
	if ( ps ) {
		if ( cg.predictedPlayerState.weapon == WP_RAILGUN 
			&& cg.predictedPlayerState.weaponstate == WEAPON_FIRING ) {
			gfixed	f;

			f = FIXED_INT32RATIO_G(cg.predictedPlayerState.weaponTime,1500);
			gun.shaderRGBA[1] = 0;
			gun.shaderRGBA[0] = 
			gun.shaderRGBA[2] = FIXED_INT32SCALE(255,( GFIXED_1 - f ));
		} else {
			gun.shaderRGBA[0] = 255;
			gun.shaderRGBA[1] = 255;
			gun.shaderRGBA[2] = 255;
			gun.shaderRGBA[3] = 255;
		}
	}

	gun.hModel = weapon->weaponModel;
	if (!gun.hModel) {
		return;
	}

	if ( !ps ) {
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
		if ( ( cent->currentState.eFlags & EF_FIRING ) && weapon->firingSound ) {
			// lightning gun and guantlet make a different sound when fire is held down
			_CG_trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, bvec3_origin, weapon->firingSound );
			cent->pe.lightningFiring = qtrue;
		} else if ( weapon->readySound ) {
			_CG_trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, bvec3_origin, weapon->readySound );
		}
	}

	CG_PositionEntityOnTag( &gun, parent, parent->hModel, "tag_weapon");

	CG_AddWeaponWithPowerups( &gun, cent->currentState.powerups );

	// add the spinning barrel
	if ( weapon->barrelModel ) {
		memset( &barrel, 0, sizeof( barrel ) );
		VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = parent->renderfx;

		barrel.hModel = weapon->barrelModel;
		angles[YAW] = AFIXED_0;
		angles[PITCH] = AFIXED_0;
		angles[ROLL] = CG_MachinegunSpinAngle( cent );
		AnglesToAxis( angles, barrel.axis );

		CG_PositionRotatedEntityOnTag( &barrel, &gun, weapon->weaponModel, "tag_barrel" );

		CG_AddWeaponWithPowerups( &barrel, cent->currentState.powerups );
	}

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
		nonPredictedCent = cent;
	}

	// add the flash
	if ( ( weaponNum == WP_LIGHTNING || weaponNum == WP_GAUNTLET || weaponNum == WP_GRAPPLING_HOOK )
		&& ( nonPredictedCent->currentState.eFlags & EF_FIRING ) ) 
	{
		// continuous flash
	} else {
		// impulse flash
		if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME && !cent->pe.railgunFlash ) {
			return;
		}
	}

	memset( &flash, 0, sizeof( flash ) );
	VectorCopy( parent->lightingOrigin, flash.lightingOrigin );
	flash.shadowPlane = parent->shadowPlane;
	flash.renderfx = parent->renderfx;

	flash.hModel = weapon->flashModel;
	if (!flash.hModel) {
		return;
	}
	angles[YAW] = AFIXED_0;
	angles[PITCH] = AFIXED_0;
	angles[ROLL] = crandom_a() * AFIXED(10,0);
	AnglesToAxis( angles, flash.axis );

	// colorize the railgun blast
	if ( weaponNum == WP_RAILGUN ) {
		clientInfo_t	*ci;

		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		flash.shaderRGBA[0] = FIXED_INT32SCALE(255, ci->color1[0]);
		flash.shaderRGBA[1] = FIXED_INT32SCALE(255, ci->color1[1]);
		flash.shaderRGBA[2] = FIXED_INT32SCALE(255, ci->color1[2]);
	}

	CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->weaponModel, "tag_flash");
	_CG_trap_R_AddRefEntityToScene( &flash );

	if ( ps || cg.renderingThirdPerson ||
		cent->currentState.number != cg.predictedPlayerState.clientNum ) {
		// add lightning bolt
		CG_LightningBolt( nonPredictedCent, flash.origin );

		// add rail trail
		CG_SpawnRailTrail( cent, flash.origin );

		if ( FIXED_NOT_ZERO(weapon->flashDlightColor[0]) || FIXED_NOT_ZERO(weapon->flashDlightColor[1]) || FIXED_NOT_ZERO(weapon->flashDlightColor[2]) ) {
			_CG_trap_R_AddLightToScene( flash.origin, MAKE_GFIXED(300 + (rand()&31)), weapon->flashDlightColor[0],
				weapon->flashDlightColor[1], weapon->flashDlightColor[2] );
		}
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t	hand;
	centity_t	*cent;
	clientInfo_t	*ci;
	bfixed		fovOffset;
	avec3_t		angles;
	weaponInfo_t	*weapon;

	if ( ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	// no gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if ( cg.renderingThirdPerson ) {
		return;
	}


	// allow the gun to be completely removed
	if ( !cg_drawGun.integer ) {
		bvec3_t		origin;

		if ( cg.predictedPlayerState.eFlags & EF_FIRING ) {
			// special hack for lightning gun...
			VectorCopy( cg.refdef.vieworg, origin );
			FIXED_VEC3MA_R( origin, -BFIXED(8,0), cg.refdef.viewaxis[2], origin );
			CG_LightningBolt( &cg_entities[ps->clientNum], origin );
		}
		return;
	}

	// don't draw if testing a gun model
	if ( cg.testGun ) {
		return;
	}

	// drop gun lower at higher fov
	if ( cg_fov.integer > 90 ) {
		fovOffset = -BFIXED(0,2) * MAKE_BFIXED( cg_fov.integer - 90 );
	} else {
		fovOffset = BFIXED_0;
	}

	cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];
	CG_RegisterWeapon( ps->weapon );
	weapon = &cg_weapons[ ps->weapon ];

	memset (&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition( hand.origin, angles );

	FIXED_VEC3MA_R( hand.origin, MAKE_BFIXED(cg_gun_x.value), cg.refdef.viewaxis[0], hand.origin );
	FIXED_VEC3MA_R( hand.origin, MAKE_BFIXED(cg_gun_y.value), cg.refdef.viewaxis[1], hand.origin );
	FIXED_VEC3MA_R( hand.origin, MAKE_BFIXED(cg_gun_z.value)+fovOffset, cg.refdef.viewaxis[2], hand.origin );

	AnglesToAxis( angles, hand.axis );

	// map torso animations to weapon animations
	if ( cg_gun_frame.integer ) {
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = GFIXED_0;
	} else {
		// get clientinfo for animation map
		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		hand.frame = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.frame );
		hand.oldframe = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.oldFrame );
		hand.backlerp = cent->pe.torso.backlerp;
	}

	hand.hModel = weapon->handsModel;
	hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

	// add everything onto the hand
	CG_AddPlayerWeapon( &hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM] );
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===================
CG_DrawWeaponSelect
===================
*/
void CG_DrawWeaponSelect( void ) {
	int		i;
	int		bits;
	int		count;
	int		x, y, w;
	const char	*name;
	gfixed	*color;

	// don't display if dead
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	color = CG_FadeColor( cg.weaponSelectTime, WEAPON_SELECT_TIME );
	if ( !color ) {
		return;
	}
	_CG_trap_R_SetColor( color );

	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;

	// count the number of weapons owned
	bits = cg.snap->ps.stats[ STAT_WEAPONS ];
	count = 0;
	for ( i = 1 ; i < 16 ; i++ ) {
		if ( bits & ( 1 << i ) ) {
			count++;
		}
	}

	x = 320 - count * 20;
	y = 380;

	for ( i = 1 ; i < 16 ; i++ ) {
		if ( !( bits & ( 1 << i ) ) ) {
			continue;
		}

		CG_RegisterWeapon( i );

		// draw weapon icon
		CG_DrawPic( MAKE_GFIXED(x), MAKE_GFIXED(y), GFIXED(32,0), GFIXED(32,0), cg_weapons[i].weaponIcon );

		// draw selection marker
		if ( i == cg.weaponSelect ) {
			CG_DrawPic( MAKE_GFIXED(x-4), MAKE_GFIXED(y-4), GFIXED(40,0), GFIXED(40,0), cgs.media.selectShader );
		}

		// no ammo cross on top
		if ( !cg.snap->ps.ammo[ i ] ) {
			CG_DrawPic( MAKE_GFIXED(x), MAKE_GFIXED(y), GFIXED(32,0), GFIXED(32,0), cgs.media.noammoShader );
		}

		x += 40;
	}

	// draw the selected name
	if ( cg_weapons[ cg.weaponSelect ].item ) {
		name = cg_weapons[ cg.weaponSelect ].item->pickup_name;
		if ( name ) {
			w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;
			x = ( SCREEN_WIDTH - w ) / 2;
			CG_DrawBigStringColor(x, y - 22, name, color);
		}
	}

	_CG_trap_R_SetColor( NULL );
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable( int i ) {
	if ( !cg.snap->ps.ammo[i] ) {
		return qfalse;
	}
	if ( ! (cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ) ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	int		i;
	int		original;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for ( i = 0 ; i < 16 ; i++ ) {
		cg.weaponSelect++;
		if ( cg.weaponSelect == 16 ) {
			cg.weaponSelect = 0;
		}
		if ( cg.weaponSelect == WP_GAUNTLET ) {
			continue;		// never cycle to gauntlet
		}
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			break;
		}
	}
	if ( i == 16 ) {
		cg.weaponSelect = original;
	}
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	int		i;
	int		original;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for ( i = 0 ; i < 16 ; i++ ) {
		cg.weaponSelect--;
		if ( cg.weaponSelect == -1 ) {
			cg.weaponSelect = 15;
		}
		if ( cg.weaponSelect == WP_GAUNTLET ) {
			continue;		// never cycle to gauntlet
		}
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			break;
		}
	}
	if ( i == 16 ) {
		cg.weaponSelect = original;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	num = atoi( CG_Argv( 1 ) );

	if ( num < 1 || num > 15 ) {
		return;
	}

	cg.weaponSelectTime = cg.time;

	if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) ) {
		return;		// don't have the weapon
	}

	cg.weaponSelect = num;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange( void ) {
	int		i;

	cg.weaponSelectTime = cg.time;

	for ( i = 15 ; i > 0 ; i-- ) {
		if ( CG_WeaponSelectable( i ) ) {
			cg.weaponSelect = i;
			break;
		}
	}
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent ) {
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	ent = &cent->currentState;
	if ( ent->weapon == WP_NONE ) {
		return;
	}
	if ( ent->weapon >= WP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS" );
		return;
	}
	weap = &cg_weapons[ ent->weapon ];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

	// lightning gun only does this this on initial press
	if ( ent->weapon == WP_LIGHTNING ) {
		if ( cent->pe.lightningFiring ) {
			return;
		}
	}

	// play quad sound if needed
	if ( cent->currentState.powerups & ( 1 << PW_QUAD ) ) {
		_CG_trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound );
	}

	// play a sound
	for ( c = 0 ; c < 4 ; c++ ) {
		if ( !weap->flashSound[c] ) {
			break;
		}
	}
	if ( c > 0 ) {
		c = rand() % c;
		if ( weap->flashSound[c] )
		{
			_CG_trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->flashSound[c] );
		}
	}

	// do brass ejection
	if ( weap->ejectBrassFunc && cg_brassTime.integer > 0 ) {
		weap->ejectBrassFunc( cent );
	}
}


/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall( int weapon, int clientNum, bvec3_t origin, avec3_t dir, impactSound_t soundType ) {
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	bfixed			radius;
	gfixed			light;
	vec3_t			lightColor;
	localEntity_t	*le;
	int				r;
	qboolean		alphaFade;
	qboolean		isSprite;
	int				duration;
	bvec3_t			sprOrg;
	bvec3_t			sprVel;

	mark = 0;
	radius = BFIXED(32,0);
	sfx = 0;
	mod = 0;
	shader = 0;
	light = GFIXED_0;
	lightColor[0] = GFIXED_1;
	lightColor[1] = GFIXED_1;
	lightColor[2] = GFIXED_0;

	// set defaults
	isSprite = qfalse;
	duration = 600;

	switch ( weapon ) {
	default:
#ifdef MISSIONPACK
	case WP_NAILGUN:
		if( soundType == IMPACTSOUND_FLESH ) {
			sfx = cgs.media.sfx_nghitflesh;
		} else if( soundType == IMPACTSOUND_METAL ) {
			sfx = cgs.media.sfx_nghitmetal;
		} else {
			sfx = cgs.media.sfx_nghit;
		}
		mark = cgs.media.holeMarkShader;
		radius = BFIXED(12,0);
		break;
#endif
	case WP_LIGHTNING:
		// no explosion at LG impact, it is added with the beam
		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_lghit2;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_lghit1;
		} else {
			sfx = cgs.media.sfx_lghit3;
		}
		mark = cgs.media.holeMarkShader;
		radius = BFIXED(12,0);
		break;
#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_proxexp;
		mark = cgs.media.burnMarkShader;
		radius = BFIXED(64,0);
		light = 300;
		isSprite = qtrue;
		break;
#endif
	case WP_GRENADE_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = BFIXED(64,0);
		light = GFIXED(300,0);
		isSprite = qtrue;
		break;
	case WP_ROCKET_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = BFIXED(64,0);
		light = GFIXED(300,0);
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = GFIXED_1;
		lightColor[1] = GFIXED(0,75);
		lightColor[2] = GFIXED_0;
		if (cg_oldRocket.integer == 0) {
			// explosion sprite animation
			FIXED_VEC3MA_R( origin, BFIXED(24,0), dir, sprOrg );
			FIXED_VEC3SCALE_R( dir, BFIXED(64,0), sprVel );

			CG_ParticleExplosion( "explode1", sprOrg, sprVel, 1400, 20, 30 );
		}
		break;
	case WP_RAILGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.railExplosionShader;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = BFIXED(24,0);
		break;
	case WP_PLASMAGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.plasmaExplosionShader;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = BFIXED(16,0);
		break;
	case WP_BFG:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.bfgExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = BFIXED(32,0);
		isSprite = qtrue;
		break;
	case WP_SHOTGUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;
		sfx = 0;
		radius = BFIXED(4,0);
		break;

#ifdef MISSIONPACK
	case WP_CHAINGUN:
		mod = cgs.media.bulletFlashModel;
		if( soundType == IMPACTSOUND_FLESH ) {
			sfx = cgs.media.sfx_chghitflesh;
		} else if( soundType == IMPACTSOUND_METAL ) {
			sfx = cgs.media.sfx_chghitmetal;
		} else {
			sfx = cgs.media.sfx_chghit;
		}
		mark = cgs.media.bulletMarkShader;

		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

		radius = 8;
		break;
#endif

	case WP_MACHINEGUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;

		r = rand() & 3;
		if ( r == 0 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 1 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

		radius = BFIXED(8,0);
		break;
	}

	if ( sfx ) {
		_CG_trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	//
	// create the explosion
	//
	if ( mod ) {
		le = CG_MakeExplosion( origin, dir, 
							   mod,	shader,
							   duration, isSprite );
		le->light = light;
		VectorCopy( lightColor, le->lightColor );
		if ( weapon == WP_RAILGUN ) {
			// colorize with client color
			VectorCopy( cgs.clientinfo[clientNum].color1, le->color );
		}
	}

	//
	// impact mark
	//
	alphaFade = (mark == cgs.media.energyMarkShader);	// plasma fades alpha, all others fade color
	if ( weapon == WP_RAILGUN ) {
		gfixed	*color;

		// colorize with client color
		color = cgs.clientinfo[clientNum].color2;
		CG_ImpactMark( mark, origin, dir, random_a()*AFIXED(360,0), color[0],color[1], color[2],GFIXED_1, alphaFade, radius, qfalse );
	} else {
		CG_ImpactMark( mark, origin, dir, random_a()*AFIXED(360,0), GFIXED_1,GFIXED_1,GFIXED_1,GFIXED_1, alphaFade, radius, qfalse );
	}
}


/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer( int weapon, bvec3_t origin, avec3_t dir, int entityNum ) {
	CG_Bleed( origin, entityNum );

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	switch ( weapon ) {
	case WP_GRENADE_LAUNCHER:
	case WP_ROCKET_LAUNCHER:
#ifdef MISSIONPACK
	case WP_NAILGUN:
	case WP_CHAINGUN:
	case WP_PROX_LAUNCHER:
#endif
		CG_MissileHitWall( weapon, 0, origin, dir, IMPACTSOUND_FLESH );
		break;
	default:
		break;
	}
}



/*
============================================================================

SHOTGUN TRACING

============================================================================
*/

/*
================
CG_ShotgunPellet
================
*/
static void CG_ShotgunPellet( bvec3_t start, bvec3_t end, int skipNum ) {
	trace_t		tr;
	int sourceContentType, destContentType;

	CG_Trace( &tr, start, NULL, NULL, end, skipNum, MASK_SHOT );

	sourceContentType = _CG_trap_CM_PointContents( start, 0 );
	destContentType = _CG_trap_CM_PointContents( tr.endpos, 0 );

	// FIXME: should probably move this cruft into CG_BubbleTrail
	if ( sourceContentType == destContentType ) {
		if ( sourceContentType & CONTENTS_WATER ) {
			CG_BubbleTrail( start, tr.endpos, BFIXED(32,0) );
		}
	} else if ( sourceContentType & CONTENTS_WATER ) {
		trace_t trace;

		_CG_trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( start, trace.endpos, BFIXED(32,0) );
	} else if ( destContentType & CONTENTS_WATER ) {
		trace_t trace;

		_CG_trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( tr.endpos, trace.endpos, BFIXED(32,0) );
	}

	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	if ( cg_entities[tr.entityNum].currentState.eType == ET_PLAYER ) {
		CG_MissileHitPlayer( WP_SHOTGUN, tr.endpos, tr.plane.normal, tr.entityNum );
	} else {
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}
		if ( tr.surfaceFlags & SURF_METALSTEPS ) {
			CG_MissileHitWall( WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL );
		} else {
			CG_MissileHitWall( WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT );
		}
	}
}

/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes
================
*/
static void CG_ShotgunPattern( bvec3_t origin, bvec3_t origin2, int seed, int otherEntNum ) {
	int			i;
	bfixed		r, u;
	bvec3_t		end;
	bvec3_t		forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_SHOTGUN_COUNT ; i++ ) {
		r = Q_bcrandom( &seed ) * BFIXED(DEFAULT_SHOTGUN_SPREAD * 16,0);
		u = Q_bcrandom( &seed ) * BFIXED(DEFAULT_SHOTGUN_SPREAD * 16,0);
		FIXED_VEC3MA( origin, BFIXED(8192 * 16,0), forward, end);
		FIXED_VEC3MA (end, r, right, end);
		FIXED_VEC3MA (end, u, up, end);

		CG_ShotgunPellet( origin, end, otherEntNum );
	}
}

/*
==============
CG_ShotgunFire
==============
*/
void CG_ShotgunFire( entityState_t *es ) {
	bvec3_t	v;
	int		contents;

	VectorSubtract( es->origin2, es->pos.trBase, v );
	VectorNormalize( v );
	FIXED_VEC3SCALE( v, BFIXED(32,0), v );
	VectorAdd( es->pos.trBase, v, v );
	if ( cgs.glconfig.hardwareType != GLHW_RAGEPRO ) {
		// ragepro can't alpha fade, so don't even bother with smoke
		bvec3_t			up;

		contents = _CG_trap_CM_PointContents( es->pos.trBase, 0 );
		if ( !( contents & CONTENTS_WATER ) ) {
			VectorSet( up, BFIXED_0, BFIXED_0, BFIXED(8,0) );
			CG_SmokePuff( v, up, BFIXED(32,0), GFIXED_1, GFIXED_1, GFIXED_1, GFIXED(0,33), GFIXED(900,0), cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
		}
	}
	CG_ShotgunPattern( es->pos.trBase, es->origin2, es->eventParm, es->otherEntityNum );
}

/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
void CG_Tracer( bvec3_t source, bvec3_t dest ) {
	bvec3_t		forward, right;
	polyVert_t	verts[4];
	bvec3_t		line;
	bfixed		len, begin, end;
	bvec3_t		start, finish;
	bvec3_t		midpoint;

	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );

	// start at least a little ways from the muzzle
	if ( len < BFIXED(100,0) ) {
		return;
	}
	begin = BFIXED(50,0) + random_b() * len - BFIXED(60,0);
	end = begin + MAKE_BFIXED(cg_tracerLength.value);
	if ( end > len ) {
		end = len;
	}
	FIXED_VEC3MA( source, begin, forward, start );
	FIXED_VEC3MA( source, end, forward, finish );

	line[0] = FIXED_VEC3DOT( forward, cg.refdef.viewaxis[1] );
	line[1] = FIXED_VEC3DOT( forward, cg.refdef.viewaxis[2] );

	FIXED_VEC3SCALE_R( cg.refdef.viewaxis[1], line[1], right );
	FIXED_VEC3MA_R( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	FIXED_VEC3MA( finish, MAKE_BFIXED(cg_tracerWidth.value), right, verts[0].xyz );
	verts[0].st[0] = GFIXED_0;
	verts[0].st[1] = GFIXED_1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	FIXED_VEC3MA( finish, -MAKE_BFIXED(cg_tracerWidth.value), right, verts[1].xyz );
	verts[1].st[0] = GFIXED_1;
	verts[1].st[1] = GFIXED_0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	FIXED_VEC3MA( start, -MAKE_BFIXED(cg_tracerWidth.value), right, verts[2].xyz );
	verts[2].st[0] = GFIXED_1;
	verts[2].st[1] = GFIXED_1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	FIXED_VEC3MA( start, MAKE_BFIXED(cg_tracerWidth.value), right, verts[3].xyz );
	verts[3].st[0] = GFIXED_0;
	verts[3].st[1] = GFIXED_0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	_CG_trap_R_AddPolyToScene( cgs.media.tracerShader, 4, verts );

	midpoint[0] = ( start[0] + finish[0] ) * BFIXED(0,5);
	midpoint[1] = ( start[1] + finish[1] ) * BFIXED(0,5);
	midpoint[2] = ( start[2] + finish[2] ) * BFIXED(0,5);

	// add the tracer sound
	_CG_trap_S_StartSound( midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound );

}


/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean	CG_CalcMuzzlePoint( int entityNum, bvec3_t muzzle ) {
	avec3_t		forward;
	centity_t	*cent;
	int			anim;

	if ( entityNum == cg.snap->ps.clientNum ) {
		VectorCopy( cg.snap->ps.origin, muzzle );
		muzzle[2] += MAKE_BFIXED(cg.snap->ps.viewheight);
		AngleVectors( cg.snap->ps.viewangles, forward, NULL, NULL );
		FIXED_VEC3MA_R( muzzle, BFIXED(14,0), forward, muzzle );
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if ( !cent->currentValid ) {
		return qfalse;
	}

	VectorCopy( cent->currentState.pos.trBase, muzzle );

	AngleVectors( cent->currentState.apos.trBase, forward, NULL, NULL );
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
		muzzle[2] += BFIXED(CROUCH_VIEWHEIGHT,0);
	} else {
		muzzle[2] += BFIXED(DEFAULT_VIEWHEIGHT,0);
	}

	FIXED_VEC3MA_R( muzzle, BFIXED(14,0), forward, muzzle );

	return qtrue;

}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
void CG_Bullet( bvec3_t end, int sourceEntityNum, avec3_t normal, qboolean flesh, int fleshEntityNum ) {
	trace_t trace;
	int sourceContentType, destContentType;
	bvec3_t		start;

	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if ( sourceEntityNum >= 0 && cg_tracerChance.value > LFIXED_0 ) {
		if ( CG_CalcMuzzlePoint( sourceEntityNum, start ) ) {
			sourceContentType = _CG_trap_CM_PointContents( start, 0 );
			destContentType = _CG_trap_CM_PointContents( end, 0 );

			// do a complete bubble trail if necessary
			if ( ( sourceContentType == destContentType ) && ( sourceContentType & CONTENTS_WATER ) ) {
				CG_BubbleTrail( start, end, BFIXED(32,0) );
			}
			// bubble trail from water into air
			else if ( ( sourceContentType & CONTENTS_WATER ) ) {
				_CG_trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
				CG_BubbleTrail( start, trace.endpos, BFIXED(32,0) );
			}
			// bubble trail from air into water
			else if ( ( destContentType & CONTENTS_WATER ) ) {
				_CG_trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
				CG_BubbleTrail( trace.endpos, end, BFIXED(32,0) );
			}

			// draw a tracer
			if ( random() < MAKE_GFIXED(cg_tracerChance.value) ) {
				CG_Tracer( start, end );
			}
		}
	}

	// impact splash and mark
	if ( flesh ) {
		CG_Bleed( end, fleshEntityNum );
	} else {
		CG_MissileHitWall( WP_MACHINEGUN, 0, end, normal, IMPACTSOUND_DEFAULT );
	}

}
