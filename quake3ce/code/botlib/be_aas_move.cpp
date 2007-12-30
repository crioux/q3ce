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

/*****************************************************************************
 * name:		be_aas_move.c
 *
 * desc:		AAS
 *
 * $Archive: /MissionPack/code/botlib/be_aas_move.c $
 *
 *****************************************************************************/

#include"botlib_pch.h"


extern botlib_import_t botimport;

aas_settings_t aassettings;

//#define AAS_MOVE_DEBUG

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_DropToFloor(bvec3_t origin, bvec3_t mins, bvec3_t maxs)
{
	bvec3_t end;
	bsp_trace_t trace;

	VectorCopy(origin, end);
	end[2] -= BFIXED(100,0);
	trace = AAS_Trace(origin, mins, maxs, end, 0, CONTENTS_SOLID);
	if (trace.startsolid) return qfalse;
	VectorCopy(trace.endpos, origin);
	return qtrue;
} //end of the function AAS_DropToFloor
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_InitSettings(void)
{
	aassettings.phys_gravitydirection[0]	= BFIXED_0;
	aassettings.phys_gravitydirection[1]	= BFIXED_0;
	aassettings.phys_gravitydirection[2]	= -BFIXED_1;
	aassettings.phys_friction				= MAKE_BFIXED(LibVarValue("phys_friction", "6"));
	aassettings.phys_stopspeed				= MAKE_BFIXED(LibVarValue("phys_stopspeed", "100"));
	aassettings.phys_gravity				= MAKE_BFIXED(LibVarValue("phys_gravity", "800"));
	aassettings.phys_waterfriction			= MAKE_BFIXED(LibVarValue("phys_waterfriction", "1"));
	aassettings.phys_watergravity			= MAKE_BFIXED(LibVarValue("phys_watergravity", "400"));
	aassettings.phys_maxvelocity			= MAKE_BFIXED(LibVarValue("phys_maxvelocity", "320"));
	aassettings.phys_maxwalkvelocity		= MAKE_BFIXED(LibVarValue("phys_maxwalkvelocity", "320"));
	aassettings.phys_maxcrouchvelocity		= MAKE_BFIXED(LibVarValue("phys_maxcrouchvelocity", "100"));
	aassettings.phys_maxswimvelocity		= MAKE_BFIXED(LibVarValue("phys_maxswimvelocity", "150"));
	aassettings.phys_walkaccelerate			= MAKE_BFIXED(LibVarValue("phys_walkaccelerate", "10"));
	aassettings.phys_airaccelerate			= MAKE_BFIXED(LibVarValue("phys_airaccelerate", "1"));
	aassettings.phys_swimaccelerate			= MAKE_BFIXED(LibVarValue("phys_swimaccelerate", "4"));
	aassettings.phys_maxstep				= MAKE_BFIXED(LibVarValue("phys_maxstep", "19"));
	aassettings.phys_maxsteepness			= MAKE_BFIXED(LibVarValue("phys_maxsteepness", "0.7"));
	aassettings.phys_maxwaterjump			= MAKE_BFIXED(LibVarValue("phys_maxwaterjump", "18"));
	aassettings.phys_maxbarrier				= MAKE_BFIXED(LibVarValue("phys_maxbarrier", "33"));
	aassettings.phys_jumpvel				= MAKE_BFIXED(LibVarValue("phys_jumpvel", "270"));
	aassettings.phys_falldelta5				= MAKE_BFIXED(LibVarValue("phys_falldelta5", "40"));
	aassettings.phys_falldelta10			= MAKE_BFIXED(LibVarValue("phys_falldelta10", "60"));
	aassettings.rs_waterjump				= MAKE_BFIXED(LibVarValue("rs_waterjump", "400"));
	aassettings.rs_teleport					= MAKE_BFIXED(LibVarValue("rs_teleport", "50"));
	aassettings.rs_barrierjump				= MAKE_BFIXED(LibVarValue("rs_barrierjump", "100"));
	aassettings.rs_startcrouch				= MAKE_BFIXED(LibVarValue("rs_startcrouch", "300"));
	aassettings.rs_startgrapple				= MAKE_BFIXED(LibVarValue("rs_startgrapple", "500"));
	aassettings.rs_startwalkoffledge		= MAKE_BFIXED(LibVarValue("rs_startwalkoffledge", "70"));
	aassettings.rs_startjump				= MAKE_BFIXED(LibVarValue("rs_startjump", "300"));
	aassettings.rs_rocketjump				= MAKE_BFIXED(LibVarValue("rs_rocketjump", "500"));
	aassettings.rs_bfgjump					= MAKE_BFIXED(LibVarValue("rs_bfgjump", "500"));
	aassettings.rs_jumppad					= MAKE_BFIXED(LibVarValue("rs_jumppad", "250"));
	aassettings.rs_aircontrolledjumppad		= MAKE_BFIXED(LibVarValue("rs_aircontrolledjumppad", "300"));
	aassettings.rs_funcbob					= MAKE_BFIXED(LibVarValue("rs_funcbob", "300"));
	aassettings.rs_startelevator			= MAKE_BFIXED(LibVarValue("rs_startelevator", "50"));
	aassettings.rs_falldamage5				= MAKE_BFIXED(LibVarValue("rs_falldamage5", "300"));
	aassettings.rs_falldamage10				= MAKE_BFIXED(LibVarValue("rs_falldamage10", "500"));
	aassettings.rs_maxfallheight			= MAKE_BFIXED(LibVarValue("rs_maxfallheight", "0"));
	aassettings.rs_maxjumpfallheight		= MAKE_BFIXED(LibVarValue("rs_maxjumpfallheight", "450"));
} //end of the function AAS_InitSettings
//===========================================================================
// returns qtrue if the bot is against a ladder
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_AgainstLadder(bvec3_t origin)
{
	int areanum, i, facenum, side;
	bvec3_t org;
	aas_plane_t *plane;
	aas_face_t *face;
	aas_area_t *area;

	VectorCopy(origin, org);
	areanum = AAS_PointAreaNum(org);
	if (!areanum)
	{
		org[0] += BFIXED_1;
		areanum = AAS_PointAreaNum(org);
		if (!areanum)
		{
			org[1] += BFIXED_1;
			areanum = AAS_PointAreaNum(org);
			if (!areanum)
			{
				org[0] -= BFIXED(2,0);
				areanum = AAS_PointAreaNum(org);
				if (!areanum)
				{
					org[1] -= BFIXED(2,0);
					areanum = AAS_PointAreaNum(org);
				} //end if
			} //end if
		} //end if
	} //end if
	//if in solid... wrrr shouldn't happen
	if (!areanum) return qfalse;
	//if not in a ladder area
	if (!(aasworld.areasettings[areanum].areaflags & AREA_LADDER)) return qfalse;
	//if a crouch only area
	if (!(aasworld.areasettings[areanum].presencetype & PRESENCE_NORMAL)) return qfalse;
	//
	area = &aasworld.areas[areanum];
	for (i = 0; i < area->numfaces; i++)
	{
		facenum = aasworld.faceindex[area->firstface + i];
		side = facenum < 0;
		face = &aasworld.faces[abs(facenum)];
		//if the face isn't a ladder face
		if (!(face->faceflags & FACE_LADDER)) continue;
		//get the plane the face is in
		plane = &aasworld.planes[face->planenum ^ side];
		//if the origin is pretty close to the plane
		if (abs(FIXED_TO_INT(FIXED_VEC3DOT_R(plane->normal, origin) - plane->dist)) < 3)
		{
			if (AAS_PointInsideFace(abs(facenum), origin, BFIXED(0,1))) return qtrue;
		} //end if
	} //end for
	return qfalse;
} //end of the function AAS_AgainstLadder
//===========================================================================
// returns qtrue if the bot is on the ground
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_OnGround(bvec3_t origin, int presencetype, int passent)
{
	aas_trace_t trace;
	bvec3_t end;
	avec3_t up = {AFIXED_0, AFIXED_0, AFIXED_1};
	aas_plane_t *plane;

	VectorCopy(origin, end);
	end[2] -= BFIXED(10,0);

	trace = AAS_TraceClientBBox(origin, end, presencetype, passent);

	//if in solid
	if (trace.startsolid) return qfalse;
	//if nothing hit at all
	if (trace.fraction >= GFIXED_1) return qfalse;
	//if too far from the hit plane
	if (origin[2] - trace.endpos[2] > BFIXED(10,0)) return qfalse;
	//check if the plane isn't too steep
	plane = AAS_PlaneFromNum(trace.planenum);
	if (FIXED_VEC3DOT(plane->normal, up) < MAKE_AFIXED(aassettings.phys_maxsteepness)) return qfalse;
	//the bot is on the ground
	return qtrue;
} //end of the function AAS_OnGround
//===========================================================================
// returns qtrue if a bot at the given position is swimming
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_Swimming(bvec3_t origin)
{
	bvec3_t testorg;

	VectorCopy(origin, testorg);
	testorg[2] -= BFIXED(2,0);
	if (AAS_PointContents(testorg) & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER)) return qtrue;
	return qfalse;
} //end of the function AAS_Swimming
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
static avec3_t VEC_UP			= {AFIXED_0, -AFIXED_1,  AFIXED_0};
static avec3_t MOVEDIR_UP		= {AFIXED_0,  AFIXED_0,  AFIXED_1};
static avec3_t VEC_DOWN		= {AFIXED_0, -AFIXED(2,0),  AFIXED_0};
static avec3_t MOVEDIR_DOWN	= {AFIXED_0,  AFIXED_0, -AFIXED_1};

void AAS_SetMovedir(avec3_t angles, avec3_t movedir)
{
	if (VectorCompare(angles, VEC_UP))
	{
		VectorCopy(MOVEDIR_UP, movedir);
	} //end if
	else if (VectorCompare(angles, VEC_DOWN))
	{
		VectorCopy(MOVEDIR_DOWN, movedir);
	} //end else if
	else
	{
		AngleVectors(angles, movedir, NULL, NULL);
	} //end else
} //end of the function AAS_SetMovedir
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_JumpReachRunStart(aas_reachability_t *reach, bvec3_t runstart)
{
	bvec3_t tmp, start, cmdmove;
	aas_clientmove_t move;
	avec3_t hordir;

	//
	tmp[0] = reach->start[0] - reach->end[0];
	tmp[1] = reach->start[1] - reach->end[1];
	tmp[2] = BFIXED_0;
	VectorNormalizeB2A(tmp,hordir);
	//start point
	VectorCopy(reach->start, start);
	start[2] += BFIXED_1;
	//get command movement
	FIXED_VEC3SCALE_R(hordir, BFIXED(400,0), cmdmove);
	//
	AAS_PredictClientMovement(&move, -1, start, PRESENCE_NORMAL, qtrue,
								bvec3_origin, cmdmove, 1, 2, GFIXED(0,1),
								PSE_ENTERWATER|PSE_ENTERSLIME|PSE_ENTERLAVA|
								PSE_HITGROUNDDAMAGE|PSE_GAP, 0, qfalse);
	VectorCopy(move.endpos, runstart);
	//don't enter slime or lava and don't fall from too high
	if (move.stopevent & (PSE_ENTERSLIME|PSE_ENTERLAVA|PSE_HITGROUNDDAMAGE))
	{
		VectorCopy(start, runstart);
	} //end if
} //end of the function AAS_JumpReachRunStart
//===========================================================================
// returns the Z velocity when rocket jumping at the origin
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bfixed AAS_WeaponJumpZVelocity(bvec3_t origin, bfixed radiusdamage)
{
	bvec3_t kvel, v, start, end, tmp;
	avec3_t forward, right, dir;
	avec3_t viewangles;
	bfixed	mass, knockback, points;
	bvec3_t rocketoffset = {BFIXED(8,0), BFIXED(8,0), -BFIXED(8,0)};
	bvec3_t botmins = {-BFIXED(16,0), -BFIXED(16,0), -BFIXED(24,0)};
	bvec3_t botmaxs = {BFIXED(16,0), BFIXED(16,0), BFIXED(32,0)};
	bsp_trace_t bsptrace;

	//look down (90 degrees)
	viewangles[PITCH] = AFIXED(90,0);
	viewangles[YAW] = AFIXED_0;
	viewangles[ROLL] = AFIXED_0;
	//get the start point shooting from
	VectorCopy(origin, start);
	start[2] += BFIXED(8,0); //view offset Z
	AngleVectors(viewangles, forward, right, NULL);
	start[0] += forward[0] * rocketoffset[0] + right[0] * rocketoffset[1];
	start[1] += forward[1] * rocketoffset[0] + right[1] * rocketoffset[1];
	start[2] += forward[2] * rocketoffset[0] + right[2] * rocketoffset[1] + rocketoffset[2];
	//end point of the trace
	FIXED_VEC3MA_R(start, BFIXED(500,0), forward, end);
	//trace a line to get the impact point
	bsptrace = AAS_Trace(start, NULL, NULL, end, 1, CONTENTS_SOLID);
	//calculate the damage the bot will get from the rocket impact
	VectorAdd(botmins, botmaxs, v);
	FIXED_VEC3MA(origin, BFIXED(0,5), v, v);
	VectorSubtract(bsptrace.endpos, v, v);
	//
	points = radiusdamage - BFIXED(0,5) * FIXED_VEC3LEN(v);
	if (points < BFIXED_0) points = BFIXED_0;
	//the owner of the rocket gets half the damage
	points *= BFIXED(0,5);
	//mass of the bot (p_client.c: PutClientInServer)
	mass = BFIXED(200,0);
	//knockback is the same as the damage points
	knockback = points;
	//direction of the damage (from trace.endpos to bot origin)
	VectorSubtract(origin, bsptrace.endpos, tmp);
	VectorNormalizeB2A(tmp,dir);
	//damage velocity
	FIXED_VEC3SCALE_R(dir, BFIXED(1600,0) * knockback / mass, kvel);	//the rocket jump hack...
	
	//rocket impact velocity + jump velocity
	return kvel[2] + aassettings.phys_jumpvel;
} //end of the function AAS_WeaponJumpZVelocity
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bfixed AAS_RocketJumpZVelocity(bvec3_t origin)
{
	//rocket radius damage is 120 (p_weapon.c: Weapon_RocketLauncher_Fire)
	return AAS_WeaponJumpZVelocity(origin, BFIXED(120,0));
} //end of the function AAS_RocketJumpZVelocity
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bfixed AAS_BFGJumpZVelocity(bvec3_t origin)
{
	//bfg radius damage is 1000 (p_weapon.c: weapon_bfg_fire)
	return AAS_WeaponJumpZVelocity(origin, BFIXED(120,0));
} //end of the function AAS_BFGJumpZVelocity
//===========================================================================
// applies ground friction to the given velocity
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_Accelerate(bvec3_t velocity, gfixed frametime, bvec3_t wishdir, bfixed wishspeed, bfixed accel)
{
	// q2 style
	int			i;
	bfixed		addspeed, accelspeed, currentspeed;

	currentspeed = FIXED_VEC3DOT(velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= BFIXED_0) {
		return;
	}
	accelspeed = accel*frametime*wishspeed;
	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}
	
	for (i=0 ; i<3 ; i++) {
		velocity[i] += accelspeed*wishdir[i];	
	}
} //end of the function AAS_Accelerate
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_AirControl(bvec3_t start, bvec3_t end, bvec3_t velocity, bvec3_t cmdmove)
{
	bvec3_t dir;

	VectorSubtract(end, start, dir);
} //end of the function AAS_AirControl
//===========================================================================
// applies ground friction to the given velocity
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_ApplyFriction(bvec3_t vel, bfixed friction, bfixed stopspeed,
													gfixed frametime)
{
	bfixed speed, control, newspeed;

	//horizontal speed
	speed = FIXED_VEC2LEN(vel);
	if (FIXED_NOT_ZERO(speed))
	{
		control = speed < stopspeed ? stopspeed : speed;
		newspeed = speed - frametime * control * friction;
		if (newspeed < BFIXED_0) newspeed = BFIXED_0;
		newspeed /= speed;
		vel[0] *= newspeed;
		vel[1] *= newspeed;
	} //end if
} //end of the function AAS_ApplyFriction
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_ClipToBBox(aas_trace_t *trace, bvec3_t start, bvec3_t end, int presencetype, bvec3_t mins, bvec3_t maxs)
{
	int i, j, side;
	bfixed front, back, planedist;
	gfixed frac;
	bvec3_t bboxmins, bboxmaxs, absmins, absmaxs, dir, mid;

	AAS_PresenceTypeBoundingBox(presencetype, bboxmins, bboxmaxs);
	VectorSubtract(mins, bboxmaxs, absmins);
	VectorSubtract(maxs, bboxmins, absmaxs);
	//
	VectorCopy(end, trace->endpos);
	trace->fraction = GFIXED_1;
	for (i = 0; i < 3; i++)
	{
		if (start[i] < absmins[i] && end[i] < absmins[i]) return qfalse;
		if (start[i] > absmaxs[i] && end[i] > absmaxs[i]) return qfalse;
	} //end for
	//check bounding box collision
	VectorSubtract(end, start, dir);
	frac = GFIXED_1;
	for (i = 0; i < 3; i++)
	{
		//get plane to test collision with for the current axis direction
		if (dir[i] > BFIXED_0) planedist = absmins[i];
		else planedist = absmaxs[i];
		//calculate collision fraction
		front = start[i] - planedist;
		back = end[i] - planedist;
		if(FIXED_IS_ZERO(front-back))
		{
			frac= GFIXED_0;
		}
		else
		{
			frac = FIXED_RATIO_G(front,(front-back));
		}
		//check if between bounding planes of next axis
		side = i + 1;
		if (side > 2) side = 0;
		mid[side] = start[side] + dir[side] * frac;
		if (mid[side] > absmins[side] && mid[side] < absmaxs[side])
		{
			//check if between bounding planes of next axis
			side++;
			if (side > 2) side = 0;
			mid[side] = start[side] + dir[side] * frac;
			if (mid[side] > absmins[side] && mid[side] < absmaxs[side])
			{
				mid[i] = planedist;
				break;
			} //end if
		} //end if
	} //end for
	//if there was a collision
	if (i != 3)
	{
		trace->startsolid = qfalse;
		trace->fraction = frac;
		trace->ent = 0;
		trace->planenum = 0;
		trace->area = 0;
		trace->lastarea = 0;
		//trace endpos
		for (j = 0; j < 3; j++) trace->endpos[j] = start[j] + dir[j] * frac;
		return qtrue;
	} //end if
	return qfalse;
} //end of the function AAS_ClipToBBox
//===========================================================================
// predicts the movement
// assumes regular bounding box sizes
// NOTE: out of water jumping is not included
// NOTE: grappling hook is not included
//
// Parameter:			origin			: origin to start with
//						presencetype	: presence type to start with
//						velocity		: velocity to start with
//						cmdmove			: client command movement
//						cmdframes		: number of frame cmdmove is valid
//						maxframes		: maximum number of predicted frames
//						frametime		: duration of one predicted frame
//						stopevent		: events that stop the prediction
//						stopareanum		: stop as soon as entered this area
// Returns:				aas_clientmove_t
// Changes Globals:		-
//===========================================================================
int AAS_ClientMovementPrediction(struct aas_clientmove_s *move,
								int entnum, bvec3_t origin,
								int presencetype, int onground,
								bvec3_t velocity, bvec3_t cmdmove,
								int cmdframes,
								int maxframes, gfixed frametime,
								int stopevent, int stopareanum,
								bvec3_t mins, bvec3_t maxs, int visualize)
{
	bfixed phys_friction, phys_stopspeed, phys_gravity, phys_waterfriction;
	bfixed phys_watergravity;
	bfixed phys_walkaccelerate, phys_airaccelerate, phys_swimaccelerate;
	bfixed phys_maxwalkvelocity, phys_maxcrouchvelocity, phys_maxswimvelocity;
	bfixed phys_maxstep, phys_maxsteepness, phys_jumpvel, friction;
	bfixed gravity, delta, maxvel, wishspeed, accelerate;
	//bfixed velchange, newvel;
	int n, i, j, pc, step, swimming, ax, crouch, event, jump_frame, areanum;
	int areas[20], numareas;
	bvec3_t points[20];
	bvec3_t org, end, feet, start, stepend, lastorg, wishdir;
	bvec3_t frame_test_vel, old_frame_test_vel, left_test_vel;
	avec3_t up = {AFIXED_0, AFIXED_0, AFIXED_1};
	aas_plane_t *plane, *plane2;
	aas_trace_t trace, steptrace;
	
	if (frametime <= GFIXED_0) frametime = GFIXED(0,1);
	//
	phys_friction = aassettings.phys_friction;
	phys_stopspeed = aassettings.phys_stopspeed;
	phys_gravity = aassettings.phys_gravity;
	phys_waterfriction = aassettings.phys_waterfriction;
	phys_watergravity = aassettings.phys_watergravity;
	phys_maxwalkvelocity = aassettings.phys_maxwalkvelocity;// * frametime;
	phys_maxcrouchvelocity = aassettings.phys_maxcrouchvelocity;// * frametime;
	phys_maxswimvelocity = aassettings.phys_maxswimvelocity;// * frametime;
	phys_walkaccelerate = aassettings.phys_walkaccelerate;
	phys_airaccelerate = aassettings.phys_airaccelerate;
	phys_swimaccelerate = aassettings.phys_swimaccelerate;
	phys_maxstep = aassettings.phys_maxstep;
	phys_maxsteepness = aassettings.phys_maxsteepness;
	phys_jumpvel = aassettings.phys_jumpvel * frametime;
	//
	Com_Memset(move, 0, sizeof(aas_clientmove_t));
	Com_Memset(&trace, 0, sizeof(aas_trace_t));
	//start at the current origin
	VectorCopy(origin, org);
	org[2] += BFIXED(0,25);
	//velocity to test for the first frame
	FIXED_VEC3SCALE(velocity, frametime, frame_test_vel);
	//
	jump_frame = -1;
	//predict a maximum of 'maxframes' ahead
	for (n = 0; n < maxframes; n++)
	{
		swimming = AAS_Swimming(org);
		//get gravity depending on swimming or not
		gravity = swimming ? phys_watergravity : phys_gravity;
		//apply gravity at the START of the frame
		frame_test_vel[2] = frame_test_vel[2] - (gravity * GFIXED(0,1) * frametime);
		//if on the ground or swimming
		if (onground || swimming)
		{
			friction = swimming ? phys_friction : phys_waterfriction;
			//apply friction
			FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, frame_test_vel);
			AAS_ApplyFriction(frame_test_vel, friction, phys_stopspeed, frametime);
			FIXED_VEC3SCALE(frame_test_vel, frametime, frame_test_vel);
		} //end if
		crouch = qfalse;
		//apply command movement
		if (n < cmdframes)
		{
			ax = 0;
			maxvel = phys_maxwalkvelocity;
			accelerate = phys_airaccelerate;
			VectorCopy(cmdmove, wishdir);
			if (onground)
			{
				if (cmdmove[2] < -BFIXED(300,0))
				{
					crouch = qtrue;
					maxvel = phys_maxcrouchvelocity;
				} //end if
				//if not swimming and upmove is positive then jump
				if (!swimming && cmdmove[2] > BFIXED_1)
				{
					//jump velocity minus the gravity for one frame + 5 for safety
					frame_test_vel[2] = phys_jumpvel - (gravity * (GFIXED(0,1) * frametime)) + BFIXED(5,0);
					jump_frame = n;
					//jumping so air accelerate
					accelerate = phys_airaccelerate;
				} //end if
				else
				{
					accelerate = phys_walkaccelerate;
				} //end else
				ax = 2;
			} //end if
			if (swimming)
			{
				maxvel = phys_maxswimvelocity;
				accelerate = phys_swimaccelerate;
				ax = 3;
			} //end if
			else
			{
				wishdir[2] = BFIXED_0;
			} //end else
			//
			wishspeed = VectorNormalize(wishdir);
			if (wishspeed > maxvel) wishspeed = maxvel;
			FIXED_VEC3SCALE(frame_test_vel,GFIXED_1/frametime, frame_test_vel);
			AAS_Accelerate(frame_test_vel, frametime, wishdir, wishspeed, accelerate);
			FIXED_VEC3SCALE(frame_test_vel, frametime, frame_test_vel);
			/*
			for (i = 0; i < ax; i++)
			{
				velchange = (cmdmove[i] * frametime) - frame_test_vel[i];
				if (velchange > phys_maxacceleration) velchange = phys_maxacceleration;
				else if (velchange < -phys_maxacceleration) velchange = -phys_maxacceleration;
				newvel = frame_test_vel[i] + velchange;
				//
				if (frame_test_vel[i] <= maxvel && newvel > maxvel) frame_test_vel[i] = maxvel;
				else if (frame_test_vel[i] >= -maxvel && newvel < -maxvel) frame_test_vel[i] = -maxvel;
				else frame_test_vel[i] = newvel;
			} //end for
			*/
		} //end if
		if (crouch)
		{
			presencetype = PRESENCE_CROUCH;
		} //end if
		else if (presencetype == PRESENCE_CROUCH)
		{
			if (AAS_PointPresenceType(org) & PRESENCE_NORMAL)
			{
				presencetype = PRESENCE_NORMAL;
			} //end if
		} //end else
		//save the current origin
		VectorCopy(org, lastorg);
		//move linear during one frame
		VectorCopy(frame_test_vel, left_test_vel);
		j = 0;
		do
		{
			VectorAdd(org, left_test_vel, end);
			//trace a bounding box
			trace = AAS_TraceClientBBox(org, end, presencetype, entnum);
			//
//#ifdef AAS_MOVE_DEBUG
			if (visualize)
			{
				if (trace.startsolid) botimport.Print(PRT_MESSAGE, "PredictMovement: start solid\n");
				AAS_DebugLine(org, trace.endpos, LINECOLOR_RED);
			} //end if
//#endif //AAS_MOVE_DEBUG
			//
			if (stopevent & (PSE_ENTERAREA|PSE_TOUCHJUMPPAD|PSE_TOUCHTELEPORTER|PSE_TOUCHCLUSTERPORTAL))
			{
				numareas = AAS_TraceAreas(org, trace.endpos, areas, points, 20);
				for (i = 0; i < numareas; i++)
				{
					if (stopevent & PSE_ENTERAREA)
					{
						if (areas[i] == stopareanum)
						{
							VectorCopy(points[i], move->endpos);
							FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
							move->endarea = areas[i];
							move->trace = trace;
							move->stopevent = PSE_ENTERAREA;
							move->presencetype = presencetype;
							move->endcontents = 0;
							move->time = MAKE_GFIXED(n) * frametime;
							move->frames = n;
							return qtrue;
						} //end if
					} //end if
					//NOTE: if not the first frame
					if ((stopevent & PSE_TOUCHJUMPPAD) && n)
					{
						if (aasworld.areasettings[areas[i]].contents & AREACONTENTS_JUMPPAD)
						{
							VectorCopy(points[i], move->endpos);
							FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
							move->endarea = areas[i];
							move->trace = trace;
							move->stopevent = PSE_TOUCHJUMPPAD;
							move->presencetype = presencetype;
							move->endcontents = 0;
							move->time = MAKE_GFIXED(n) * frametime;
							move->frames = n;
							return qtrue;
						} //end if
					} //end if
					if (stopevent & PSE_TOUCHTELEPORTER)
					{
						if (aasworld.areasettings[areas[i]].contents & AREACONTENTS_TELEPORTER)
						{
							VectorCopy(points[i], move->endpos);
							move->endarea = areas[i];
							FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
							move->trace = trace;
							move->stopevent = PSE_TOUCHTELEPORTER;
							move->presencetype = presencetype;
							move->endcontents = 0;
							move->time = MAKE_GFIXED(n) * frametime;
							move->frames = n;
							return qtrue;
						} //end if
					} //end if
					if (stopevent & PSE_TOUCHCLUSTERPORTAL)
					{
						if (aasworld.areasettings[areas[i]].contents & AREACONTENTS_CLUSTERPORTAL)
						{
							VectorCopy(points[i], move->endpos);
							move->endarea = areas[i];
							FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
							move->trace = trace;
							move->stopevent = PSE_TOUCHCLUSTERPORTAL;
							move->presencetype = presencetype;
							move->endcontents = 0;
							move->time = MAKE_GFIXED(n) * frametime;
							move->frames = n;
							return qtrue;
						} //end if
					} //end if
				} //end for
			} //end if
			//
			if (stopevent & PSE_HITBOUNDINGBOX)
			{
				if (AAS_ClipToBBox(&trace, org, trace.endpos, presencetype, mins, maxs))
				{
					VectorCopy(trace.endpos, move->endpos);
					move->endarea = AAS_PointAreaNum(move->endpos);
					FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
					move->trace = trace;
					move->stopevent = PSE_HITBOUNDINGBOX;
					move->presencetype = presencetype;
					move->endcontents = 0;
					move->time = MAKE_GFIXED(n) * frametime;
					move->frames = n;
					return qtrue;
				} //end if
			} //end if
			//move the entity to the trace end point
			VectorCopy(trace.endpos, org);
			//if there was a collision
			if (trace.fraction < GFIXED_1)
			{
				//get the plane the bounding box collided with
				plane = AAS_PlaneFromNum(trace.planenum);
				//
				if (stopevent & PSE_HITGROUNDAREA)
				{
					if (FIXED_VEC3DOT(plane->normal, up) > MAKE_AFIXED(phys_maxsteepness))
					{
						VectorCopy(org, start);
						start[2] += BFIXED(0,5);
						if (AAS_PointAreaNum(start) == stopareanum)
						{
							VectorCopy(start, move->endpos);
							move->endarea = stopareanum;
							FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
							move->trace = trace;
							move->stopevent = PSE_HITGROUNDAREA;
							move->presencetype = presencetype;
							move->endcontents = 0;
							move->time = MAKE_GFIXED(n) * frametime;
							move->frames = n;
							return qtrue;
						} //end if
					} //end if
				} //end if
				//assume there's no step
				step = qfalse;
				//if it is a vertical plane and the bot didn't jump recently
				if (plane->normal[2] == AFIXED_0 && (jump_frame < 0 || n - jump_frame > 2))
				{
					//check for a step
					FIXED_VEC3MA_R(org, -BFIXED(0,25), plane->normal, start);
					VectorCopy(start, stepend);
					start[2] += phys_maxstep;
					steptrace = AAS_TraceClientBBox(start, stepend, presencetype, entnum);
					//
					if (!steptrace.startsolid)
					{
						plane2 = AAS_PlaneFromNum(steptrace.planenum);
						if (FIXED_VEC3DOT(plane2->normal, up) > MAKE_AFIXED(phys_maxsteepness))
						{
							VectorSubtract(end, steptrace.endpos, left_test_vel);
							left_test_vel[2] = BFIXED_0;
							frame_test_vel[2] = BFIXED_0;
//#ifdef AAS_MOVE_DEBUG
							if (visualize)
							{
								if (steptrace.endpos[2] - org[2] > BFIXED(0,125))
								{
									VectorCopy(org, start);
									start[2] = steptrace.endpos[2];
									AAS_DebugLine(org, start, LINECOLOR_BLUE);
								} //end if
							} //end if
//#endif //AAS_MOVE_DEBUG
							org[2] = steptrace.endpos[2];
							step = qtrue;
						} //end if
					} //end if
				} //end if
				//
				if (!step)
				{
					//velocity left to test for this frame is the projection
					//of the current test velocity into the hit plane 
					FIXED_VEC3MA_R(left_test_vel, -FIXED_VEC3DOT(left_test_vel, plane->normal),
										plane->normal, left_test_vel);
					//store the old velocity for landing check
					VectorCopy(frame_test_vel, old_frame_test_vel);
					//test velocity for the next frame is the projection
					//of the velocity of the current frame into the hit plane 
					FIXED_VEC3MA_R(frame_test_vel, -FIXED_VEC3DOT(frame_test_vel, plane->normal),
										plane->normal, frame_test_vel);
					//check for a landing on an almost horizontal floor
					if (FIXED_VEC3DOT(plane->normal, up) > MAKE_AFIXED(phys_maxsteepness))
					{
						onground = qtrue;
					} //end if
					if (stopevent & PSE_HITGROUNDDAMAGE)
					{
						delta = BFIXED_0;
						if (old_frame_test_vel[2] < BFIXED_0 &&
								frame_test_vel[2] > old_frame_test_vel[2] &&
								!onground)
						{
							delta = old_frame_test_vel[2];
						} //end if
						else if (onground)
						{
							delta = frame_test_vel[2] - old_frame_test_vel[2];
						} //end else
						if (FIXED_NOT_ZERO(delta))
						{
							delta = delta * BFIXED(10,0);
							delta = delta * delta * BFIXED(0,0001);
							if (swimming) delta = BFIXED_0;
							// never take falling damage if completely underwater
							/*
							if (ent->waterlevel == 3) return;
							if (ent->waterlevel == 2) delta *= BFIXED(0,25);
							if (ent->waterlevel == 1) delta *= BFIXED(0,5);
							*/
							if (delta > BFIXED(40,0))
							{
								VectorCopy(org, move->endpos);
								move->endarea = AAS_PointAreaNum(org);
								VectorCopy(frame_test_vel, move->velocity);
								move->trace = trace;
								move->stopevent = PSE_HITGROUNDDAMAGE;
								move->presencetype = presencetype;
								move->endcontents = 0;
								move->time = MAKE_GFIXED(n) * frametime;
								move->frames = n;
								return qtrue;
							} //end if
						} //end if
					} //end if
				} //end if
			} //end if
			//extra check to prevent endless loop
			if (++j > 20) return qfalse;
		//while there is a plane hit
		} while(trace.fraction < GFIXED_1);
		//if going down
		if (frame_test_vel[2] <= BFIXED(10,0))
		{
			//check for a liquid at the feet of the bot
			VectorCopy(org, feet);
			feet[2] -= BFIXED(22,0);
			pc = AAS_PointContents(feet);
			//get event from pc
			event = PSE_NONE;
			if (pc & CONTENTS_LAVA) event |= PSE_ENTERLAVA;
			if (pc & CONTENTS_SLIME) event |= PSE_ENTERSLIME;
			if (pc & CONTENTS_WATER) event |= PSE_ENTERWATER;
			//
			areanum = AAS_PointAreaNum(org);
			if (aasworld.areasettings[areanum].contents & AREACONTENTS_LAVA)
				event |= PSE_ENTERLAVA;
			if (aasworld.areasettings[areanum].contents & AREACONTENTS_SLIME)
				event |= PSE_ENTERSLIME;
			if (aasworld.areasettings[areanum].contents & AREACONTENTS_WATER)
				event |= PSE_ENTERWATER;
			//if in lava or slime
			if (event & stopevent)
			{
				VectorCopy(org, move->endpos);
				move->endarea = areanum;
				FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
				move->stopevent = event & stopevent;
				move->presencetype = presencetype;
				move->endcontents = pc;
				move->time = MAKE_GFIXED(n) * frametime;
				move->frames = n;
				return qtrue;
			} //end if
		} //end if
		//
		onground = AAS_OnGround(org, presencetype, entnum);
		//if onground and on the ground for at least one whole frame
		if (onground)
		{
			if (stopevent & PSE_HITGROUND)
			{
				VectorCopy(org, move->endpos);
				move->endarea = AAS_PointAreaNum(org);
				FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
				move->trace = trace;
				move->stopevent = PSE_HITGROUND;
				move->presencetype = presencetype;
				move->endcontents = 0;
				move->time = MAKE_GFIXED(n) * frametime;
				move->frames = n;
				return qtrue;
			} //end if
		} //end if
		else if (stopevent & PSE_LEAVEGROUND)
		{
			VectorCopy(org, move->endpos);
			move->endarea = AAS_PointAreaNum(org);
			FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
			move->trace = trace;
			move->stopevent = PSE_LEAVEGROUND;
			move->presencetype = presencetype;
			move->endcontents = 0;
			move->time = MAKE_GFIXED(n) * frametime;
			move->frames = n;
			return qtrue;
		} //end else if
		else if (stopevent & PSE_GAP)
		{
			aas_trace_t gaptrace;

			VectorCopy(org, start);
			VectorCopy(start, end);
			end[2] -= BFIXED(48,0) + aassettings.phys_maxbarrier;
			gaptrace = AAS_TraceClientBBox(start, end, PRESENCE_CROUCH, -1);
			//if solid is found the bot cannot walk any further and will not fall into a gap
			if (!gaptrace.startsolid)
			{
				//if it is a gap (lower than one step height)
				if (gaptrace.endpos[2] < org[2] - aassettings.phys_maxstep - BFIXED_1)
				{
					if (!(AAS_PointContents(end) & CONTENTS_WATER))
					{
						VectorCopy(lastorg, move->endpos);
						move->endarea = AAS_PointAreaNum(lastorg);
						FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
						move->trace = trace;
						move->stopevent = PSE_GAP;
						move->presencetype = presencetype;
						move->endcontents = 0;
						move->time = MAKE_GFIXED(n) * frametime;
						move->frames = n;
						return qtrue;
					} //end if
				} //end if
			} //end if
		} //end else if
	} //end for
	//
	VectorCopy(org, move->endpos);
	move->endarea = AAS_PointAreaNum(org);
	FIXED_VEC3SCALE(frame_test_vel, GFIXED_1/frametime, move->velocity);
	move->stopevent = PSE_NONE;
	move->presencetype = presencetype;
	move->endcontents = 0;
	move->time = MAKE_GFIXED(n) * frametime;
	move->frames = n;
	//
	return qtrue;
} //end of the function AAS_ClientMovementPrediction
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_PredictClientMovement(struct aas_clientmove_s *move,
								int entnum, bvec3_t origin,
								int presencetype, int onground,
								bvec3_t velocity, bvec3_t cmdmove,
								int cmdframes,
								int maxframes, gfixed frametime,
								int stopevent, int stopareanum, int visualize)
{
	bvec3_t mins, maxs;
	return AAS_ClientMovementPrediction(move, entnum, origin, presencetype, onground,
										velocity, cmdmove, cmdframes, maxframes,
										frametime, stopevent, stopareanum,
										mins, maxs, visualize);
} //end of the function AAS_PredictClientMovement
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_ClientMovementHitBBox(struct aas_clientmove_s *move,
								int entnum, bvec3_t origin,
								int presencetype, int onground,
								bvec3_t velocity, bvec3_t cmdmove,
								int cmdframes,
								int maxframes, gfixed frametime,
								bvec3_t mins, bvec3_t maxs, int visualize)
{
	return AAS_ClientMovementPrediction(move, entnum, origin, presencetype, onground,
										velocity, cmdmove, cmdframes, maxframes,
										frametime, PSE_HITBOUNDINGBOX, 0,
										mins, maxs, visualize);
} //end of the function AAS_ClientMovementHitBBox
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_TestMovementPrediction(int entnum, bvec3_t origin, avec3_t dir)
{
	bvec3_t velocity, cmdmove;
	aas_clientmove_t move;

	VectorClear(velocity);
	if (!AAS_Swimming(origin)) dir[2] = AFIXED_0;
	VectorNormalize(dir);
	FIXED_VEC3SCALE_R(dir, BFIXED(400,0), cmdmove);
	cmdmove[2] = BFIXED(224,0);
	AAS_ClearShownDebugLines();
	AAS_PredictClientMovement(&move, entnum, origin, PRESENCE_NORMAL, qtrue,
									velocity, cmdmove, 13, 13, GFIXED(0,1), PSE_HITGROUND, 0, qtrue);//PSE_LEAVEGROUND);
	if (move.stopevent & PSE_LEAVEGROUND)
	{
		botimport.Print(PRT_MESSAGE, "leave ground\n");
	} //end if
} //end of the function TestMovementPrediction
//===========================================================================
// calculates the horizontal velocity needed to perform a jump from start
// to end
//
// Parameter:			zvel	: z velocity for jump
//						start	: start position of jump
//						end		: end position of jump
//						*speed	: returned speed for jump
// Returns:				qfalse if too high or too far from start to end
// Changes Globals:		-
//===========================================================================
int AAS_HorizontalVelocityForJump(bfixed zvel, bvec3_t start, bvec3_t end, bfixed *velocity)
{
	bfixed phys_gravity, phys_maxvelocity;
	bfixed maxjump, height2fall, t, top;
	bvec3_t dir;

	phys_gravity = aassettings.phys_gravity;
	phys_maxvelocity = aassettings.phys_maxvelocity;

	//maximum height a player can jump with the given initial z velocity
	maxjump = BFIXED(0,5) * phys_gravity * (zvel / phys_gravity) * (zvel / phys_gravity);
	//top of the parabolic jump
	top = start[2] + maxjump;
	//height the bot will fall from the top
	height2fall = top - end[2];
	//if the goal is to high to jump to
	if (height2fall < BFIXED_0)
	{
		*velocity = phys_maxvelocity;
		return 0;
	} //end if
	//time a player takes to fall the height
	t = FIXED_SQRT(height2fall / (BFIXED(0,5) * phys_gravity));
  	//direction from start to end
	VectorSubtract(end, start, dir);
	//
	if ( (t + zvel / phys_gravity) == BFIXED_0 ) {
		*velocity = phys_maxvelocity;
		return 0;
	}
	//calculate horizontal speed
	*velocity = FIXED_VEC2LEN(dir) / (t + zvel / phys_gravity);
	//the horizontal speed must be lower than the max speed
	if (*velocity > phys_maxvelocity)
	{
		*velocity = phys_maxvelocity;
		return 0;
	} //end if
	return 1;
} //end of the function AAS_HorizontalVelocityForJump
