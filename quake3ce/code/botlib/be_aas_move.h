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
 * name:		be_aas_move.h
 *
 * desc:		AAS
 *
 * $Archive: /source/code/botlib/be_aas_move.h $
 *
 *****************************************************************************/

#ifdef AASINTERN
extern aas_settings_t aassettings;
#endif //AASINTERN

//movement prediction
int AAS_PredictClientMovement(struct aas_clientmove_s *move,
							int entnum, bvec3_t origin,
							int presencetype, int onground,
							bvec3_t velocity, bvec3_t cmdmove,
							int cmdframes,
							int maxframes, gfixed frametime,
							int stopevent, int stopareanum, int visualize);
//predict movement until bounding box is hit
int AAS_ClientMovementHitBBox(struct aas_clientmove_s *move,
								int entnum, bvec3_t origin,
								int presencetype, int onground,
								bvec3_t velocity, bvec3_t cmdmove,
								int cmdframes,
								int maxframes, gfixed frametime,
								bvec3_t mins, bvec3_t maxs, int visualize);
//returns true if on the ground at the given origin
int AAS_OnGround(bvec3_t origin, int presencetype, int passent);
//returns true if swimming at the given origin
int AAS_Swimming(bvec3_t origin);
//returns the jump reachability run start point
void AAS_JumpReachRunStart(struct aas_reachability_s *reach, bvec3_t runstart);
//returns true if against a ladder at the given origin
int AAS_AgainstLadder(bvec3_t origin);
//rocket jump Z velocity when rocket-jumping at origin
bfixed AAS_RocketJumpZVelocity(bvec3_t origin);
//bfg jump Z velocity when bfg-jumping at origin
bfixed AAS_BFGJumpZVelocity(bvec3_t origin);
//calculates the horizontal velocity needed for a jump and returns true this velocity could be calculated
int AAS_HorizontalVelocityForJump(bfixed zvel, bvec3_t start, bvec3_t end, bfixed *velocity);
//
void AAS_SetMovedir(avec3_t angles, bvec3_t movedir);
//
int AAS_DropToFloor(bvec3_t origin, bvec3_t mins, bvec3_t maxs);
//
void AAS_InitSettings(void);
