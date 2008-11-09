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
// ui_players.c

#include"ui_pch.h"


#define UI_TIMER_GESTURE		2300
#define UI_TIMER_JUMP			1000
#define UI_TIMER_LAND			130
#define UI_TIMER_WEAPON_SWITCH	300
#define UI_TIMER_ATTACK			500
#define	UI_TIMER_MUZZLE_FLASH	20
#define	UI_TIMER_WEAPON_DELAY	250

#define JUMP_HEIGHT				56

#define SWINGSPEED				AFIXED(0,3)

#define SPIN_SPEED				AFIXED(0,9)
#define COAST_TIME				1000


static int			dp_realtime;
static gfixed		jumpHeight;


/*
===============
UI_PlayerInfo_SetWeapon
===============
*/
static void UI_PlayerInfo_SetWeapon( playerInfo_t *pi, weapon_t weaponNum ) {
	gitem_t *	item;
	char		path[MAX_QPATH];

	pi->currentWeapon = weaponNum;
tryagain:
	pi->realWeapon = weaponNum;
	pi->weaponModel = 0;
	pi->barrelModel = 0;
	pi->flashModel = 0;

	if ( weaponNum == WP_NONE ) {
		return;
	}

	for ( item = bg_itemlist + 1; item->classname ; item++ ) {
		if ( item->giType != IT_WEAPON ) {
			continue;
		}
		if ( item->giTag == weaponNum ) {
			break;
		}
	}

	if ( item->classname ) {
		pi->weaponModel = _UI_trap_R_RegisterModel( item->world_model[0] );
	}

	if( pi->weaponModel == 0 ) {
		if( weaponNum == WP_MACHINEGUN ) {
			weaponNum = WP_NONE;
			goto tryagain;
		}
		weaponNum = WP_MACHINEGUN;
		goto tryagain;
	}

	if ( weaponNum == WP_MACHINEGUN || weaponNum == WP_GAUNTLET || weaponNum == WP_BFG ) {
		strcpy( path, item->world_model[0] );
		COM_StripExtension( path, path );
		strcat( path, "_barrel.md3" );
		pi->barrelModel = _UI_trap_R_RegisterModel( path );
	}

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path );
	strcat( path, "_flash.md3" );
	pi->flashModel = _UI_trap_R_RegisterModel( path );

	switch( weaponNum ) {
	case WP_GAUNTLET:
		MAKERGB( pi->flashDlightColor, GFIXED(0,6), GFIXED(0,6), GFIXED_1 );
		break;

	case WP_MACHINEGUN:
		MAKERGB( pi->flashDlightColor, GFIXED_1, GFIXED_1, GFIXED_0 );
		break;

	case WP_SHOTGUN:
		MAKERGB( pi->flashDlightColor, GFIXED_1, GFIXED_1, GFIXED_0 );
		break;

	case WP_GRENADE_LAUNCHER:
		MAKERGB( pi->flashDlightColor, GFIXED_1, GFIXED(0,7), GFIXED(0,5) );
		break;

	case WP_ROCKET_LAUNCHER:
		MAKERGB( pi->flashDlightColor, GFIXED_1, GFIXED(0,75), GFIXED_0 );
		break;

	case WP_LIGHTNING:
		MAKERGB( pi->flashDlightColor, GFIXED(0,6), GFIXED(0,6), GFIXED_1 );
		break;

	case WP_RAILGUN:
		MAKERGB( pi->flashDlightColor, GFIXED_1, GFIXED(0,5), GFIXED_0 );
		break;

	case WP_PLASMAGUN:
		MAKERGB( pi->flashDlightColor, GFIXED(0,6), GFIXED(0,6), GFIXED_1 );
		break;

	case WP_BFG:
		MAKERGB( pi->flashDlightColor, GFIXED_1, GFIXED(0,7), GFIXED_1 );
		break;

	case WP_GRAPPLING_HOOK:
		MAKERGB( pi->flashDlightColor, GFIXED(0,6), GFIXED(0,6), GFIXED_1 );
		break;

	default:
		MAKERGB( pi->flashDlightColor, GFIXED_1, GFIXED_1, GFIXED_1 );
		break;
	}
}


/*
===============
UI_ForceLegsAnim
===============
*/
static void UI_ForceLegsAnim( playerInfo_t *pi, int anim ) {
	pi->legsAnim = ( ( pi->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

	if ( anim == LEGS_JUMP ) {
		pi->legsAnimationTimer = UI_TIMER_JUMP;
	}
}


/*
===============
UI_SetLegsAnim
===============
*/
static void UI_SetLegsAnim( playerInfo_t *pi, int anim ) {
	if ( pi->pendingLegsAnim ) {
		anim = pi->pendingLegsAnim;
		pi->pendingLegsAnim = 0;
	}
	UI_ForceLegsAnim( pi, anim );
}


/*
===============
UI_ForceTorsoAnim
===============
*/
static void UI_ForceTorsoAnim( playerInfo_t *pi, int anim ) {
	pi->torsoAnim = ( ( pi->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

	if ( anim == TORSO_GESTURE ) {
		pi->torsoAnimationTimer = UI_TIMER_GESTURE;
	}

	if ( anim == TORSO_ATTACK || anim == TORSO_ATTACK2 ) {
		pi->torsoAnimationTimer = UI_TIMER_ATTACK;
	}
}


/*
===============
UI_SetTorsoAnim
===============
*/
static void UI_SetTorsoAnim( playerInfo_t *pi, int anim ) {
	if ( pi->pendingTorsoAnim ) {
		anim = pi->pendingTorsoAnim;
		pi->pendingTorsoAnim = 0;
	}

	UI_ForceTorsoAnim( pi, anim );
}


/*
===============
UI_TorsoSequencing
===============
*/
static void UI_TorsoSequencing( playerInfo_t *pi ) {
	int		currentAnim;

	currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

	if ( pi->weapon != pi->currentWeapon ) {
		if ( currentAnim != TORSO_DROP ) {
			pi->torsoAnimationTimer = UI_TIMER_WEAPON_SWITCH;
			UI_ForceTorsoAnim( pi, TORSO_DROP );
		}
	}

	if ( pi->torsoAnimationTimer > 0 ) {
		return;
	}

	if( currentAnim == TORSO_GESTURE ) {
		UI_SetTorsoAnim( pi, TORSO_STAND );
		return;
	}

	if( currentAnim == TORSO_ATTACK || currentAnim == TORSO_ATTACK2 ) {
		UI_SetTorsoAnim( pi, TORSO_STAND );
		return;
	}

	if ( currentAnim == TORSO_DROP ) {
		UI_PlayerInfo_SetWeapon( pi, pi->weapon );
		pi->torsoAnimationTimer = UI_TIMER_WEAPON_SWITCH;
		UI_ForceTorsoAnim( pi, TORSO_RAISE );
		return;
	}

	if ( currentAnim == TORSO_RAISE ) {
		UI_SetTorsoAnim( pi, TORSO_STAND );
		return;
	}
}


/*
===============
UI_LegsSequencing
===============
*/
static void UI_LegsSequencing( playerInfo_t *pi ) {
	int		currentAnim;

	currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;

	if ( pi->legsAnimationTimer > 0 ) {
		if ( currentAnim == LEGS_JUMP ) {
			jumpHeight = GFIXED(JUMP_HEIGHT,0) * FIXED_SIN( GFIXED_PI *  FIXED_INT32RATIO_G( UI_TIMER_JUMP - pi->legsAnimationTimer,UI_TIMER_JUMP) );
		}
		return;
	}

	if ( currentAnim == LEGS_JUMP ) {
		UI_ForceLegsAnim( pi, LEGS_LAND );
		pi->legsAnimationTimer = UI_TIMER_LAND;
		jumpHeight = GFIXED_0;
		return;
	}

	if ( currentAnim == LEGS_LAND ) {
		UI_SetLegsAnim( pi, LEGS_IDLE );
		return;
	}
}


/*
======================
UI_PositionEntityOnTag
======================
*/
static void UI_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							clipHandle_t parentModel, const char *tagName ) {
	int				i;
	orientation_t	lerped;
	
	// lerp the tag
	_UI_trap_CM_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		GFIXED_1 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		FIXED_VEC3MA_R( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// cast away const because of compiler problems
	MatrixMultiply( lerped.axis, ((refEntity_t*)parent)->axis, entity->axis );
	entity->backlerp = parent->backlerp;
}


/*
======================
UI_PositionRotatedEntityOnTag
======================
*/
static void UI_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							clipHandle_t parentModel, const char *tagName ) {
	int				i;
	orientation_t	lerped;
	avec3_t			tempAxis[3];

	// lerp the tag
	_UI_trap_CM_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		GFIXED_1 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		FIXED_VEC3MA_R( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// cast away const because of compiler problems
	MatrixMultiply( entity->axis, ((refEntity_t *)parent)->axis, tempAxis );
	MatrixMultiply( lerped.axis, tempAxis, entity->axis );
}


/*
===============
UI_SetLerpFrameAnimation
===============
*/
static void UI_SetLerpFrameAnimation( playerInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_ANIMATIONS ) {
		_UI_trap_Error( va("Bad animation number: %i", newAnimation) );
	}

	anim = &ci->animations[ newAnimation ];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;
}


/*
===============
UI_RunLerpFrame
===============
*/
static void UI_RunLerpFrame( playerInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
	int			f;
	animation_t	*anim;

	// see if the animation sequence is switching
	if ( newAnimation != lf->animationNumber || !lf->animation ) {
		UI_SetLerpFrameAnimation( ci, lf, newAnimation );
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( dp_realtime >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if ( dp_realtime < lf->animationTime ) {
			lf->frameTime = lf->animationTime;		// initial lerp
		} else {
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
		if ( f >= anim->numFrames ) {
			f -= anim->numFrames;
			if ( anim->loopFrames ) {
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			} else {
				f = anim->numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = dp_realtime;
			}
		}
		lf->frame = anim->firstFrame + f;
		if ( dp_realtime > lf->frameTime ) {
			lf->frameTime = dp_realtime;
		}
	}

	if ( lf->frameTime > dp_realtime + 200 ) {
		lf->frameTime = dp_realtime;
	}

	if ( lf->oldFrameTime > dp_realtime ) {
		lf->oldFrameTime = dp_realtime;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = GFIXED_0;
	} else {
		lf->backlerp = GFIXED_1 - FIXED_INT32RATIO_G( dp_realtime - lf->oldFrameTime, lf->frameTime - lf->oldFrameTime );
	}
}


/*
===============
UI_PlayerAnimation
===============
*/
static void UI_PlayerAnimation( playerInfo_t *pi, int *legsOld, int *legs, gfixed *legsBackLerp,
						int *torsoOld, int *torso, gfixed *torsoBackLerp ) {

	// legs animation
	pi->legsAnimationTimer -= uis.frametime;
	if ( pi->legsAnimationTimer < 0 ) {
		pi->legsAnimationTimer = 0;
	}

	UI_LegsSequencing( pi );

	if ( pi->legs.yawing && ( pi->legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_IDLE ) {
		UI_RunLerpFrame( pi, &pi->legs, LEGS_TURN );
	} else {
		UI_RunLerpFrame( pi, &pi->legs, pi->legsAnim );
	}
	*legsOld = pi->legs.oldFrame;
	*legs = pi->legs.frame;
	*legsBackLerp = pi->legs.backlerp;

	// torso animation
	pi->torsoAnimationTimer -= uis.frametime;
	if ( pi->torsoAnimationTimer < 0 ) {
		pi->torsoAnimationTimer = 0;
	}

	UI_TorsoSequencing( pi );

	UI_RunLerpFrame( pi, &pi->torso, pi->torsoAnim );
	*torsoOld = pi->torso.oldFrame;
	*torso = pi->torso.frame;
	*torsoBackLerp = pi->torso.backlerp;
}


/*
==================
UI_SwingAngles
==================
*/
static void UI_SwingAngles( afixed destination, afixed swingTolerance, afixed clampTolerance,
					afixed speed, afixed *angle, qboolean *swinging ) {
	afixed	swing;
	gfixed	move;
	afixed	scale;

	if ( !*swinging ) {
		// see if a swing should be started
		swing = AngleSubtract( *angle, destination );
		if ( swing > swingTolerance || swing < -swingTolerance ) {
			*swinging = qtrue;
		}
	}

	if ( !*swinging ) {
		return;
	}
	
	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract( destination, *angle );
	scale = FIXED_ABS( swing );
	if ( scale < swingTolerance * AFIXED(0,5) ) {
		scale = AFIXED(0,5);
	} else if ( scale < swingTolerance ) {
		scale = AFIXED_1;
	} else {
		scale = AFIXED(2,0);
	}

	// swing towards the destination angle
	if ( swing >= AFIXED_0 ) {
		move = MAKE_GFIXED(uis.frametime) * MAKE_GFIXED(scale) * MAKE_GFIXED(speed);
		if ( move >= MAKE_GFIXED(swing) ) {
			move = MAKE_GFIXED(swing);
			*swinging = qfalse;
		}
		*angle = AngleModB2A( MAKE_BFIXED(*angle) + MAKE_BFIXED(move) );
	} else if ( swing < AFIXED_0 ) {
		move = MAKE_GFIXED(uis.frametime) * MAKE_GFIXED(scale) * MAKE_GFIXED(-speed);
		if ( move <= MAKE_GFIXED(swing) ) {
			move = MAKE_GFIXED(swing);
			*swinging = qfalse;
		}
		*angle = AngleModB2A( MAKE_BFIXED(*angle) + MAKE_BFIXED(move) );
	}

	// clamp to no more than tolerance
	swing = AngleSubtract( destination, *angle );
	if ( swing > clampTolerance ) {
		*angle = AngleMod( destination - (clampTolerance - AFIXED_1) );
	} else if ( swing < -clampTolerance ) {
		*angle = AngleMod( destination + (clampTolerance - AFIXED_1) );
	}
}


/*
======================
UI_MovedirAdjustment
======================
*/
static afixed UI_MovedirAdjustment( playerInfo_t *pi ) {
	avec3_t		relativeAngles;
	avec3_t		moveVector;

	VectorSubtract( pi->viewAngles, pi->moveAngles, relativeAngles );
	AngleVectors( relativeAngles, moveVector, NULL, NULL );
	if ( FIXED_ABS( moveVector[0] ) < AFIXED(0,01) ) {
		moveVector[0] = AFIXED_0;
	}
	if ( FIXED_ABS( moveVector[1] ) < AFIXED(0,01) ) {
		moveVector[1] = AFIXED_0;
	}

	if ( moveVector[1] == AFIXED_0 && moveVector[0] > AFIXED_0 ) {
		return AFIXED_0;
	}
	if ( moveVector[1] < AFIXED_0 && moveVector[0] > AFIXED_0 ) {
		return AFIXED(22,0);
	}
	if ( moveVector[1] < AFIXED_0 && moveVector[0] == AFIXED_0 ) {
		return AFIXED(45,0);
	}
	if ( moveVector[1] < AFIXED_0 && moveVector[0] < AFIXED_0 ) {
		return -AFIXED(22,0);
	}
	if ( moveVector[1] == AFIXED_0 && moveVector[0] < AFIXED_0 ) {
		return AFIXED_0;
	}
	if ( moveVector[1] > AFIXED_0 && moveVector[0] < AFIXED_0 ) {
		return AFIXED(22,0);
	}
	if ( moveVector[1] > AFIXED_0 && moveVector[0] == AFIXED_0 ) {
		return  -AFIXED(45,0);
	}

	return -AFIXED(22,0);
}


/*
===============
UI_PlayerAngles
===============
*/
static void UI_PlayerAngles( playerInfo_t *pi, avec3_t legs[3], avec3_t torso[3], avec3_t head[3] ) {
	avec3_t		legsAngles, torsoAngles, headAngles;
	afixed		dest;
	afixed		adjust;

	VectorCopy( pi->viewAngles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ( ( pi->legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_IDLE 
		|| ( pi->torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_STAND  ) {
		// if not standing still, always point all in the same direction
		pi->torso.yawing = qtrue;	// always center
		pi->torso.pitching = qtrue;	// always center
		pi->legs.yawing = qtrue;	// always center
	}

	// adjust legs for movement dir
	adjust = UI_MovedirAdjustment( pi );
	legsAngles[YAW] = headAngles[YAW] + adjust;
	torsoAngles[YAW] = headAngles[YAW] + AFIXED(0,25) * adjust;


	// torso
	UI_SwingAngles( torsoAngles[YAW], AFIXED(25,0), AFIXED(90,0), SWINGSPEED, &pi->torso.yawAngle, &pi->torso.yawing );
	UI_SwingAngles( legsAngles[YAW], AFIXED(40,0), AFIXED(90,0), SWINGSPEED, &pi->legs.yawAngle, &pi->legs.yawing );

	torsoAngles[YAW] = pi->torso.yawAngle;
	legsAngles[YAW] = pi->legs.yawAngle;

	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > AFIXED(180 ,0)) {
		dest = (-AFIXED(360 ,0)+ headAngles[PITCH]) * AFIXED(0,75);
	} else {
		dest = headAngles[PITCH] * AFIXED(0,75);
	}
	UI_SwingAngles( dest, AFIXED(15,0), AFIXED(30,0), AFIXED(0,1), &pi->torso.pitchAngle, &pi->torso.pitching );
	torsoAngles[PITCH] = pi->torso.pitchAngle;

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
	AnglesToAxis( legsAngles, legs );
	AnglesToAxis( torsoAngles, torso );
	AnglesToAxis( headAngles, head );
}


/*
===============
UI_PlayerFloatSprite
===============
*/
static void UI_PlayerFloatSprite( playerInfo_t *pi, bvec3_t origin, qhandle_t shader ) {
	refEntity_t		ent;

	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( origin, ent.origin );
	ent.origin[2] += BFIXED(48,0);
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = BFIXED(10,0);
	ent.renderfx = 0;
	_UI_trap_R_AddRefEntityToScene( &ent );
}


/*
======================
UI_MachinegunSpinAngle
======================
*/
afixed	UI_MachinegunSpinAngle( playerInfo_t *pi ) {
	int		delta;
	afixed	angle;
	afixed	speed;
	int		torsoAnim;

	delta = dp_realtime - pi->barrelTime;
	if ( pi->barrelSpinning ) {
		angle = pi->barrelAngle + MAKE_AFIXED(delta) * SPIN_SPEED;
	} else {
		if ( delta > COAST_TIME) {
			delta = COAST_TIME;
		}

		speed = FIXED_DIVPOW2( (SPIN_SPEED + FIXED_INT32RATIO_A( COAST_TIME - delta , COAST_TIME)) ,1);
		angle = pi->barrelAngle + MAKE_AFIXED(delta) * speed;
	}

	torsoAnim = pi->torsoAnim  & ~ANIM_TOGGLEBIT;
	if( torsoAnim == TORSO_ATTACK2 ) {
		torsoAnim = TORSO_ATTACK;
	}
	if ( pi->barrelSpinning == !(torsoAnim == TORSO_ATTACK) ) {
		pi->barrelTime = dp_realtime;
		pi->barrelAngle = AngleMod( angle );
		pi->barrelSpinning = !!(torsoAnim == TORSO_ATTACK);
	}

	return angle;
}


/*
===============
UI_DrawPlayer
===============
*/
void UI_DrawPlayer( gfixed x, gfixed y, gfixed w, gfixed h, playerInfo_t *pi, int time ) {
	refdef_t		refdef;
	refEntity_t		legs;
	refEntity_t		torso;
	refEntity_t		head;
	refEntity_t		gun;
	refEntity_t		barrel;
	refEntity_t		flash;
	bvec3_t			origin;
	int				renderfx;
	bvec3_t			mins = {-BFIXED(16,0), -BFIXED(16,0), -BFIXED(24,0)};
	bvec3_t			maxs = {BFIXED(16,0), BFIXED(16,0), BFIXED(32,0)};
	bfixed			len;
	gfixed			xx;

	if ( !pi->legsModel || !pi->torsoModel || !pi->headModel || !pi->animations[0].numFrames ) {
		return;
	}

	dp_realtime = time;

	if ( pi->pendingWeapon != -1 && dp_realtime > pi->weaponTimer ) {
		pi->weapon = pi->pendingWeapon;
		pi->lastWeapon = pi->pendingWeapon;
		pi->pendingWeapon = (weapon_t) -1;
		pi->weaponTimer = 0;
		if( pi->currentWeapon != pi->weapon ) {
			_UI_trap_S_StartLocalSound( weaponChangeSound, CHAN_LOCAL );
		}
	}

	UI_AdjustFrom640( &x, &y, &w, &h );

	y -= jumpHeight;

	memset( &refdef, 0, sizeof( refdef ) );
	memset( &legs, 0, sizeof(legs) );
	memset( &torso, 0, sizeof(torso) );
	memset( &head, 0, sizeof(head) );

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.x = FIXED_TO_INT(x);
	refdef.y = FIXED_TO_INT(y);
	refdef.width = FIXED_TO_INT(w);
	refdef.height = FIXED_TO_INT(h);

	refdef.fov_x = (MAKE_AFIXED(MAKE_BFIXED(refdef.width) / BFIXED(640,0))) * AFIXED(90,0);
	xx = MAKE_GFIXED(refdef.width) / MAKE_GFIXED(FIXED_TAN( refdef.fov_x / AFIXED(360 ,0) * AFIXED_PI ));
	refdef.fov_y = MAKE_AFIXED(FIXED_ATAN2( MAKE_GFIXED(refdef.height), xx ));
	refdef.fov_y *= ( AFIXED(360,0) / AFIXED_PI );

	// calculate distance so the player nearly fills the box
	len = BFIXED(0,7) * ( maxs[2] - mins[2] );		
	origin[0] = len / MAKE_BFIXED(FIXED_TAN( DEG2RAD_A(refdef.fov_x) * AFIXED(0,5) ));
	origin[1] = BFIXED(0,5) * ( mins[1] + maxs[1] );
	origin[2] = -BFIXED(0,5) * ( mins[2] + maxs[2] );

	refdef.time = dp_realtime;

	_UI_trap_R_ClearScene();

	// get the rotation information
	UI_PlayerAngles( pi, legs.axis, torso.axis, head.axis );
	
	// get the animation state (after rotation, to allow feet shuffle)
	UI_PlayerAnimation( pi, &legs.oldframe, &legs.frame, &legs.backlerp,
		 &torso.oldframe, &torso.frame, &torso.backlerp );

	renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;

	//
	// add the legs
	//
	legs.hModel = pi->legsModel;
	legs.customSkin = pi->legsSkin;

	VectorCopy( origin, legs.origin );

	VectorCopy( origin, legs.lightingOrigin );
	legs.renderfx = renderfx;
	VectorCopy (legs.origin, legs.oldorigin);

	_UI_trap_R_AddRefEntityToScene( &legs );

	if (!legs.hModel) {
		return;
	}

	//
	// add the torso
	//
	torso.hModel = pi->torsoModel;
	if (!torso.hModel) {
		return;
	}

	torso.customSkin = pi->torsoSkin;

	VectorCopy( origin, torso.lightingOrigin );

	UI_PositionRotatedEntityOnTag( &torso, &legs, pi->legsModel, "tag_torso");

	torso.renderfx = renderfx;

	_UI_trap_R_AddRefEntityToScene( &torso );

	//
	// add the head
	//
	head.hModel = pi->headModel;
	if (!head.hModel) {
		return;
	}
	head.customSkin = pi->headSkin;

	VectorCopy( origin, head.lightingOrigin );

	UI_PositionRotatedEntityOnTag( &head, &torso, pi->torsoModel, "tag_head");

	head.renderfx = renderfx;

	_UI_trap_R_AddRefEntityToScene( &head );

	//
	// add the gun
	//
	if ( pi->currentWeapon != WP_NONE ) {
		memset( &gun, 0, sizeof(gun) );
		gun.hModel = pi->weaponModel;
		VectorCopy( origin, gun.lightingOrigin );
		UI_PositionEntityOnTag( &gun, &torso, pi->torsoModel, "tag_weapon");
		gun.renderfx = renderfx;
		_UI_trap_R_AddRefEntityToScene( &gun );
	}

	//
	// add the spinning barrel
	//
	if ( pi->realWeapon == WP_MACHINEGUN || pi->realWeapon == WP_GAUNTLET || pi->realWeapon == WP_BFG ) {
		avec3_t	angles;

		memset( &barrel, 0, sizeof(barrel) );
		VectorCopy( origin, barrel.lightingOrigin );
		barrel.renderfx = renderfx;

		barrel.hModel = pi->barrelModel;
		angles[YAW] = AFIXED_0;
		angles[PITCH] = AFIXED_0;
		angles[ROLL] = UI_MachinegunSpinAngle( pi );
		if( pi->realWeapon == WP_GAUNTLET || pi->realWeapon == WP_BFG ) {
			angles[PITCH] = angles[ROLL];
			angles[ROLL] = AFIXED_0;
		}
		AnglesToAxis( angles, barrel.axis );

		UI_PositionRotatedEntityOnTag( &barrel, &gun, pi->weaponModel, "tag_barrel");

		_UI_trap_R_AddRefEntityToScene( &barrel );
	}

	//
	// add muzzle flash
	//
	if ( dp_realtime <= pi->muzzleFlashTime ) {
		if ( pi->flashModel ) {
			memset( &flash, 0, sizeof(flash) );
			flash.hModel = pi->flashModel;
			VectorCopy( origin, flash.lightingOrigin );
			UI_PositionEntityOnTag( &flash, &gun, pi->weaponModel, "tag_flash");
			flash.renderfx = renderfx;
			_UI_trap_R_AddRefEntityToScene( &flash );
		}

		// make a dlight for the flash
		if ( FIXED_NOT_ZERO(pi->flashDlightColor[0]) || FIXED_NOT_ZERO(pi->flashDlightColor[1]) || FIXED_NOT_ZERO(pi->flashDlightColor[2]) ) {
			_UI_trap_R_AddLightToScene( flash.origin, MAKE_GFIXED(200 + (rand()&31)), pi->flashDlightColor[0],
				pi->flashDlightColor[1], pi->flashDlightColor[2] );
		}
	}

	//
	// add the chat icon
	//
	if ( pi->chat ) {
		UI_PlayerFloatSprite( pi, origin, _UI_trap_R_RegisterShaderNoMip( "sprites/balloon3" ) );
	}

	//
	// add an accent light
	//
	origin[0] -= BFIXED(100,0);	// + = behind, - = in front
	origin[1] += BFIXED(100,0);	// + = left, - = right
	origin[2] += BFIXED(100,0);	// + = above, - = below
	_UI_trap_R_AddLightToScene( origin, GFIXED(500,0), GFIXED_1, GFIXED_1, GFIXED_1 );

	origin[0] -= BFIXED(100,0);
	origin[1] -= BFIXED(100,0);
	origin[2] -= BFIXED(100,0);
	_UI_trap_R_AddLightToScene( origin, GFIXED(500,0), GFIXED_1, GFIXED_0, GFIXED_0 );

	_UI_trap_R_RenderScene( &refdef );
}


/*
==========================
UI_RegisterClientSkin
==========================
*/
static qboolean UI_RegisterClientSkin( playerInfo_t *pi, const char *modelName, const char *skinName ) {
	char		filename[MAX_QPATH];

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower_%s.skin", modelName, skinName );
	pi->legsSkin = _UI_trap_R_RegisterSkin( filename );

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper_%s.skin", modelName, skinName );
	pi->torsoSkin = _UI_trap_R_RegisterSkin( filename );

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/head_%s.skin", modelName, skinName );
	pi->headSkin = _UI_trap_R_RegisterSkin( filename );

	if ( !pi->legsSkin || !pi->torsoSkin || !pi->headSkin ) {
		return qfalse;
	}

	return qtrue;
}


/*
======================
UI_ParseAnimationFile
======================
*/
static qboolean UI_ParseAnimationFile( const char *filename, animation_t *animations ) {
	const char		*text_p, *prev;
	int			len;
	int			i;
	const char		*token;
	gfixed		fps;
	int			skip;
	char		text[20000];
	fileHandle_t	f;

	memset( animations, 0, sizeof( animation_t ) * MAX_ANIMATIONS );

	// load the file
	len = _UI_trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		return qfalse;
	}
	if ( len >= ( sizeof( text ) - 1 ) ) {
		Com_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	_UI_trap_FS_Read( text, len, f );
	text[len] = 0;
	_UI_trap_FS_FCloseFile( f );

	// parse the text
	text_p = text;
	skip = 0;	// quite the compiler warning

	// read optional parameters
	while ( 1 ) {
		prev = text_p;	// so we can unget
		token = COM_Parse( (const char **)&text_p );
		if ( !token ) {
			break;
		}
		if ( !Q_stricmp( token, "footsteps" ) ) {
			token = COM_Parse( &text_p );
			if ( !token ) {
				break;
			}
			continue;
		} else if ( !Q_stricmp( token, "headoffset" ) ) {
			for ( i = 0 ; i < 3 ; i++ ) {
				token = COM_Parse( &text_p );
				if ( !token ) {
					break;
				}
			}
			continue;
		} else if ( !Q_stricmp( token, "sex" ) ) {
			token = COM_Parse( &text_p );
			if ( !token ) {
				break;
			}
			continue;
		}

		// if it is a number, start parsing animations
		if ( token[0] >= '0' && token[0] <= '9' ) {
			text_p = prev;	// unget the token
			break;
		}

		Com_Printf( "unknown token '%s' is %s\n", token, filename );
	}

	// read information for each frame
	for ( i = 0 ; i < MAX_ANIMATIONS ; i++ ) {

		token = COM_Parse( &text_p );
		if ( !token ) {
			break;
		}
		animations[i].firstFrame = atoi( token );
		// leg only frames are adjusted to not count the upper body only frames
		if ( i == LEGS_WALKCR ) {
			skip = animations[LEGS_WALKCR].firstFrame - animations[TORSO_GESTURE].firstFrame;
		}
		if ( i >= LEGS_WALKCR ) {
			animations[i].firstFrame -= skip;
		}

		token = COM_Parse( &text_p );
		if ( !token ) {
			break;
		}
		animations[i].numFrames = atoi( token );

		token = COM_Parse( &text_p );
		if ( !token ) {
			break;
		}
		animations[i].loopFrames = atoi( token );

		token = COM_Parse( &text_p );
		if ( !token ) {
			break;
		}
		fps = MAKE_GFIXED(atof( token ));
		if ( fps == GFIXED_0 ) {
			fps = GFIXED_1;
		}
		animations[i].frameLerp = FIXED_TO_INT( GFIXED(1000 ,0)/ fps );
		animations[i].initialLerp = FIXED_TO_INT(GFIXED(1000 ,0)/ fps );
	}

	if ( i != MAX_ANIMATIONS ) {
		Com_Printf( "Error parsing animation file: %s", filename );
		return qfalse;
	}

	return qtrue;
}


/*
==========================
UI_RegisterClientModelname
==========================
*/
qboolean UI_RegisterClientModelname( playerInfo_t *pi, const char *modelSkinName ) {
	char		modelName[MAX_QPATH];
	char		skinName[MAX_QPATH];
	char		filename[MAX_QPATH];
	char		*slash;

	pi->torsoModel = 0;
	pi->headModel = 0;

	if ( !modelSkinName[0] ) {
		return qfalse;
	}

	Q_strncpyz( modelName, modelSkinName, sizeof( modelName ) );

	slash = strchr( modelName, '/' );
	if ( !slash ) {
		// modelName did not include a skin name
		Q_strncpyz( skinName, "default", sizeof( skinName ) );
	} else {
		Q_strncpyz( skinName, slash + 1, sizeof( skinName ) );
		// truncate modelName
		*slash = 0;
	}

	// load cmodels before models so filecache works

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower.md3", modelName );
	pi->legsModel = _UI_trap_R_RegisterModel( filename );
	if ( !pi->legsModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper.md3", modelName );
	pi->torsoModel = _UI_trap_R_RegisterModel( filename );
	if ( !pi->torsoModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/head.md3", modelName );
	pi->headModel = _UI_trap_R_RegisterModel( filename );
	if ( !pi->headModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	// if any skins failed to load, fall back to default
	if ( !UI_RegisterClientSkin( pi, modelName, skinName ) ) {
		if ( !UI_RegisterClientSkin( pi, modelName, "default" ) ) {
			Com_Printf( "Failed to load skin file: %s : %s\n", modelName, skinName );
			return qfalse;
		}
	}

	// load the animations
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/animation.cfg", modelName );
	if ( !UI_ParseAnimationFile( filename, pi->animations ) ) {
		Com_Printf( "Failed to load animation file %s\n", filename );
		return qfalse;
	}

	return qtrue;
}


/*
===============
UI_PlayerInfo_SetModel
===============
*/
void UI_PlayerInfo_SetModel( playerInfo_t *pi, const char *model ) {
	memset( pi, 0, sizeof(*pi) );
	UI_RegisterClientModelname( pi, model );
	pi->weapon = WP_MACHINEGUN;
	pi->currentWeapon = pi->weapon;
	pi->lastWeapon = pi->weapon;
	pi->pendingWeapon = (weapon_t)-1;
	pi->weaponTimer = 0;
	pi->chat = qfalse;
	pi->newModel = qtrue;
	UI_PlayerInfo_SetWeapon( pi, pi->weapon );
}


/*
===============
UI_PlayerInfo_SetInfo
===============
*/
void UI_PlayerInfo_SetInfo( playerInfo_t *pi, int legsAnim, int torsoAnim, avec3_t viewAngles, avec3_t moveAngles, weapon_t weaponNumber, qboolean chat ) {
	int			currentAnim;
	weapon_t	weaponNum;

	pi->chat = chat;

	// view angles
	VectorCopy( viewAngles, pi->viewAngles );

	// move angles
	VectorCopy( moveAngles, pi->moveAngles );

	if ( pi->newModel ) {
		pi->newModel = qfalse;

		jumpHeight = GFIXED_0;
		pi->pendingLegsAnim = 0;
		UI_ForceLegsAnim( pi, legsAnim );
		pi->legs.yawAngle = viewAngles[YAW];
		pi->legs.yawing = qfalse;

		pi->pendingTorsoAnim = 0;
		UI_ForceTorsoAnim( pi, torsoAnim );
		pi->torso.yawAngle = viewAngles[YAW];
		pi->torso.yawing = qfalse;

		if ( weaponNumber != -1 ) {
			pi->weapon = weaponNumber;
			pi->currentWeapon = weaponNumber;
			pi->lastWeapon = weaponNumber;
			pi->pendingWeapon = (weapon_t)-1;
			pi->weaponTimer = 0;
			UI_PlayerInfo_SetWeapon( pi, pi->weapon );
		}

		return;
	}

	// weapon
	if ( weaponNumber == -1 ) {
		pi->pendingWeapon = (weapon_t)-1;
		pi->weaponTimer = 0;
	}
	else if ( weaponNumber != WP_NONE ) {
		pi->pendingWeapon = weaponNumber;
		pi->weaponTimer = dp_realtime + UI_TIMER_WEAPON_DELAY;
	}
	weaponNum = pi->lastWeapon;
	pi->weapon = weaponNum;

	if ( torsoAnim == BOTH_DEATH1 || legsAnim == BOTH_DEATH1 ) {
		torsoAnim = legsAnim = BOTH_DEATH1;
		pi->weapon = pi->currentWeapon = WP_NONE;
		UI_PlayerInfo_SetWeapon( pi, pi->weapon );

		jumpHeight = GFIXED_0;
		pi->pendingLegsAnim = 0;
		UI_ForceLegsAnim( pi, legsAnim );

		pi->pendingTorsoAnim = 0;
		UI_ForceTorsoAnim( pi, torsoAnim );

		return;
	}

	// leg animation
	currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;
	if ( legsAnim != LEGS_JUMP && ( currentAnim == LEGS_JUMP || currentAnim == LEGS_LAND ) ) {
		pi->pendingLegsAnim = legsAnim;
	}
	else if ( legsAnim != currentAnim ) {
		jumpHeight = GFIXED_0;
		pi->pendingLegsAnim = 0;
		UI_ForceLegsAnim( pi, legsAnim );
	}

	// torso animation
	if ( torsoAnim == TORSO_STAND || torsoAnim == TORSO_STAND2 ) {
		if ( weaponNum == WP_NONE || weaponNum == WP_GAUNTLET ) {
			torsoAnim = TORSO_STAND2;
		}
		else {
			torsoAnim = TORSO_STAND;
		}
	}

	if ( torsoAnim == TORSO_ATTACK || torsoAnim == TORSO_ATTACK2 ) {
		if ( weaponNum == WP_NONE || weaponNum == WP_GAUNTLET ) {
			torsoAnim = TORSO_ATTACK2;
		}
		else {
			torsoAnim = TORSO_ATTACK;
		}
		pi->muzzleFlashTime = dp_realtime + UI_TIMER_MUZZLE_FLASH;
		//FIXME play firing sound here
	}

	currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

	if ( weaponNum != pi->currentWeapon || currentAnim == TORSO_RAISE || currentAnim == TORSO_DROP ) {
		pi->pendingTorsoAnim = torsoAnim;
	}
	else if ( ( currentAnim == TORSO_GESTURE || currentAnim == TORSO_ATTACK ) && ( torsoAnim != currentAnim ) ) {
		pi->pendingTorsoAnim = torsoAnim;
	}
	else if ( torsoAnim != currentAnim ) {
		pi->pendingTorsoAnim = 0;
		UI_ForceTorsoAnim( pi, torsoAnim );
	}
}
