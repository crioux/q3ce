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
#include"common_pch.h"

// always use bbox vs. bbox collision and never capsule vs. bbox or vice versa
//#define ALWAYS_BBOX_VS_BBOX
// always use capsule vs. capsule collision and never capsule vs. bbox or vice versa
//#define ALWAYS_CAPSULE_VS_CAPSULE

//#define CAPSULE_DEBUG

/*
===============================================================================

BASIC MATH

===============================================================================
*/

/*
================
RotatePoint
================
*/
void RotatePoint(bvec3_t point, const avec3_t matrix[3]) {
	bvec3_t tvec;

	VectorCopy(point, tvec);
	point[0] = FIXED_VEC3DOT_R(matrix[0], tvec);
	point[1] = FIXED_VEC3DOT_R(matrix[1], tvec);
	point[2] = FIXED_VEC3DOT_R(matrix[2], tvec);
}
#ifndef FIXED_IS_FLOAT
void RotatePoint(avec3_t point, const avec3_t matrix[3]) {
	avec3_t tvec;

	VectorCopy(point, tvec);
	point[0] = FIXED_VEC3DOT_R(matrix[0], tvec);
	point[1] = FIXED_VEC3DOT_R(matrix[1], tvec);
	point[2] = FIXED_VEC3DOT_R(matrix[2], tvec);
}
#endif


/*
================
TransposeMatrix
================
*/
void TransposeMatrix(const avec3_t matrix[3], avec3_t transpose[3]) {
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

/*
================
CreateRotationMatrix
================
*/
void CreateRotationMatrix(const avec3_t angles, avec3_t matrix[3]) {
	AngleVectors(angles, matrix[0], matrix[1], matrix[2]);
	VectorInverse(matrix[1]);
}

/*
================
CM_ProjectPointOntoVector
================
*/
void CM_ProjectPointOntoVector( bvec3_t point, bvec3_t vStart, avec3_t vDir, bvec3_t vProj )
{
	bvec3_t pVec;

	VectorSubtract( point, vStart, pVec );
	// project onto the directional vector for this segment
	FIXED_VEC3MA_R( vStart, FIXED_VEC3DOT( pVec, vDir ), vDir, vProj );
}

/*
================
CM_DistanceFromLineSquared
================
*/
bfixed CM_DistanceFromLineSquared(bvec3_t p, bvec3_t lp1, bvec3_t lp2, avec3_t dir) {
	bvec3_t proj, t;
	int j;

	CM_ProjectPointOntoVector(p, lp1, dir, proj);
	for (j = 0; j < 3; j++) 
		if ((proj[j] > lp1[j] && proj[j] > lp2[j]) ||
			(proj[j] < lp1[j] && proj[j] < lp2[j]))
			break;
	if (j < 3) {
		if (FIXED_ABS(proj[j] - lp1[j]) < FIXED_ABS(proj[j] - lp2[j]))
			VectorSubtract(p, lp1, t);
		else
			VectorSubtract(p, lp2, t);
		return FIXED_VEC3LEN_SQ(t);
	}
	VectorSubtract(p, proj, t);
	return FIXED_VEC3LEN_SQ(t);
}

/*
================
CM_VectorDistanceSquared
================
*/
bfixed CM_VectorDistanceSquared(bvec3_t p1, bvec3_t p2) {
	bvec3_t dir;

	VectorSubtract(p2, p1, dir);
	return FIXED_VEC3LEN_SQ(dir);
}

/*
===============================================================================

POSITION TESTING

===============================================================================
*/

/*
================
CM_TestBoxInBrush
================
*/
void CM_TestBoxInBrush( traceWork_t *tw, cbrush_t *brush ) {
	int			i;
	cplane_t	*plane;
	bfixed		dist;
	bfixed		d1;
	cbrushside_t	*side;
	bfixed		t;
	bvec3_t		startp;

	if (!brush->numsides) {
		return;
	}

	// special test for axial
	if ( tw->bounds[0][0] > brush->bounds[1][0]
		|| tw->bounds[0][1] > brush->bounds[1][1]
		|| tw->bounds[0][2] > brush->bounds[1][2]
		|| tw->bounds[1][0] < brush->bounds[0][0]
		|| tw->bounds[1][1] < brush->bounds[0][1]
		|| tw->bounds[1][2] < brush->bounds[0][2]
		) {
		return;
	}

   if ( tw->sphere.use ) {
		// the first six planes are the axial planes, so we only
		// need to test the remainder
		for ( i = 6 ; i < brush->numsides ; i++ ) {
			side = brush->sides + i;
			plane = side->plane;

			// adjust the plane distance apropriately for radius
			dist = plane->dist + tw->sphere.radius;
			// find the closest point on the capsule to the plane
			t = FIXED_VEC3DOT_R( plane->normal, tw->sphere.offset );
			if ( t > BFIXED_0 )
			{
				VectorSubtract( tw->start, tw->sphere.offset, startp );
			}
			else
			{
				VectorAdd( tw->start, tw->sphere.offset, startp );
			}
			d1 = FIXED_VEC3DOT( startp, plane->normal ) - dist;
			// if completely in front of face, no intersection
			if ( d1 > BFIXED_0 ) {
				return;
			}
		}
	} else {
		// the first six planes are the axial planes, so we only
		// need to test the remainder
		for ( i = 6 ; i < brush->numsides ; i++ ) {
			side = brush->sides + i;
			plane = side->plane;

			// adjust the plane distance apropriately for mins/maxs
			dist = plane->dist - FIXED_VEC3DOT( tw->offsets[ plane->signbits ], plane->normal );

			d1 = FIXED_VEC3DOT( tw->start, plane->normal ) - dist;

			// if completely in front of face, no intersection
			if ( d1 > BFIXED_0 ) {
				return;
			}
		}
	}

	// inside this brush
	tw->trace.startsolid = tw->trace.allsolid = qtrue;
	tw->trace.fraction = GFIXED_0;
	tw->trace.contents = brush->contents;
}



/*
================
CM_TestInLeaf
================
*/
void CM_TestInLeaf( traceWork_t *tw, cLeaf_t *leaf ) {
	int			k;
	int			brushnum;
	cbrush_t	*b;
	cPatch_t	*patch;

	// test box position against all brushes in the leaf
	for (k=0 ; k<leaf->numLeafBrushes ; k++) {
		brushnum = cm.leafbrushes[leaf->firstLeafBrush+k];
		b = &cm.brushes[brushnum];
		if (b->checkcount == cm.checkcount) {
			continue;	// already checked this brush in another leaf
		}
		b->checkcount = cm.checkcount;

		if ( !(b->contents & tw->contents)) {
			continue;
		}
		
		CM_TestBoxInBrush( tw, b );
		if ( tw->trace.allsolid ) {
			return;
		}
	}

	// test against all patches
#ifdef BSPC
	if (1) {
#else
	if ( !cm_noCurves->integer ) {
#endif //BSPC
		for ( k = 0 ; k < leaf->numLeafSurfaces ; k++ ) {
			patch = cm.surfaces[ cm.leafsurfaces[ leaf->firstLeafSurface + k ] ];
			if ( !patch ) {
				continue;
			}
			if ( patch->checkcount == cm.checkcount ) {
				continue;	// already checked this brush in another leaf
			}
			patch->checkcount = cm.checkcount;

			if ( !(patch->contents & tw->contents)) {
				continue;
			}
			
			if ( CM_PositionTestInPatchCollide( tw, patch->pc ) ) {
				tw->trace.startsolid = tw->trace.allsolid = qtrue;
				tw->trace.fraction = GFIXED_0;
				tw->trace.contents = patch->contents;
				return;
			}
		}
	}
}

/*
==================
CM_TestCapsuleInCapsule

capsule inside capsule check
==================
*/
void CM_TestCapsuleInCapsule( traceWork_t *tw, clipHandle_t model ) {
	int i;
	bvec3_t mins, maxs;
	bvec3_t top, bottom;
	bvec3_t p1, p2, tmp;
	bvec3_t offset, symetricSize[2];
	bfixed radius, halfwidth, halfheight, offs, r;

	CM_ModelBounds(model, mins, maxs);

	VectorAdd(tw->start, tw->sphere.offset, top);
	VectorSubtract(tw->start, tw->sphere.offset, bottom);
	for ( i = 0 ; i < 3 ; i++ ) {
		offset[i] = FIXED_DIVPOW2(( mins[i] + maxs[i] ),1);
		symetricSize[0][i] = mins[i] - offset[i];
		symetricSize[1][i] = maxs[i] - offset[i];
	}
	halfwidth = symetricSize[ 1 ][ 0 ];
	halfheight = symetricSize[ 1 ][ 2 ];
	radius = ( halfwidth > halfheight ) ? halfheight : halfwidth;
	offs = halfheight - radius;

	r = Square(tw->sphere.radius + radius);
	// check if any of the spheres overlap
	VectorCopy(offset, p1);
	p1[2] += offs;
	VectorSubtract(p1, top, tmp);
	if ( FIXED_VEC3LEN_SQ(tmp) < r ) {
		tw->trace.startsolid = tw->trace.allsolid = qtrue;
		tw->trace.fraction = GFIXED_0;
	}
	VectorSubtract(p1, bottom, tmp);
	if ( FIXED_VEC3LEN_SQ(tmp) < r ) {
		tw->trace.startsolid = tw->trace.allsolid = qtrue;
		tw->trace.fraction = GFIXED_0;
	}
	VectorCopy(offset, p2);
	p2[2] -= offs;
	VectorSubtract(p2, top, tmp);
	if ( FIXED_VEC3LEN_SQ(tmp) < r ) {
		tw->trace.startsolid = tw->trace.allsolid = qtrue;
		tw->trace.fraction = GFIXED_0;
	}
	VectorSubtract(p2, bottom, tmp);
	if ( FIXED_VEC3LEN_SQ(tmp) < r ) {
		tw->trace.startsolid = tw->trace.allsolid = qtrue;
		tw->trace.fraction = GFIXED_0;
	}
	// if between cylinder up and lower bounds
	if ( (top[2] >= p1[2] && top[2] <= p2[2]) ||
		(bottom[2] >= p1[2] && bottom[2] <= p2[2]) ) {
		// 2d coordinates
		top[2] = p1[2] = BFIXED_0;
		// if the cylinders overlap
		VectorSubtract(top, p1, tmp);
		if ( FIXED_VEC3LEN_SQ(tmp) < r ) {
			tw->trace.startsolid = tw->trace.allsolid = qtrue;
			tw->trace.fraction = GFIXED_0;
		}
	}
}

/*
==================
CM_TestBoundingBoxInCapsule

bounding box inside capsule check
==================
*/
void CM_TestBoundingBoxInCapsule( traceWork_t *tw, clipHandle_t model ) {
	bvec3_t mins, maxs, offset, size[2];
	clipHandle_t h;
	cmodel_t *cmod;
	int i;

	// mins maxs of the capsule
	CM_ModelBounds(model, mins, maxs);

	// offset for capsule center
	for ( i = 0 ; i < 3 ; i++ ) {
		offset[i] = FIXED_DIVPOW2(( mins[i] + maxs[i] ),1);
		size[0][i] = mins[i] - offset[i];
		size[1][i] = maxs[i] - offset[i];
		tw->start[i] -= offset[i];
		tw->end[i] -= offset[i];
	}

	// replace the bounding box with the capsule
	tw->sphere.use = qtrue;
	tw->sphere.radius = ( size[1][0] > size[1][2] ) ? size[1][2]: size[1][0];
	tw->sphere.halfheight = size[1][2];
	VectorSet( tw->sphere.offset, BFIXED_0, BFIXED_0, size[1][2] - tw->sphere.radius );

	// replace the capsule with the bounding box
	h = CM_TempBoxModel(tw->size[0], tw->size[1], qfalse);
	// calculate collision
	cmod = CM_ClipHandleToModel( h );
	CM_TestInLeaf( tw, &cmod->leaf );
}

/*
==================
CM_PositionTest
==================
*/
#define	MAX_POSITION_LEAFS	1024
void CM_PositionTest( traceWork_t *tw ) {
	int		leafs[MAX_POSITION_LEAFS];
	int		i;
	leafList_t	ll;

	// identify the leafs we are touching
	VectorAdd( tw->start, tw->size[0], ll.bounds[0] );
	VectorAdd( tw->start, tw->size[1], ll.bounds[1] );

	for (i=0 ; i<3 ; i++) {
		ll.bounds[0][i] -= BFIXED_1;
		ll.bounds[1][i] += BFIXED_1;
	}

	ll.count = 0;
	ll.maxcount = MAX_POSITION_LEAFS;
	ll.list = leafs;
	ll.storeLeafs = CM_StoreLeafs;
	ll.lastLeaf = 0;
	ll.overflowed = qfalse;

	cm.checkcount++;

	CM_BoxLeafnums_r( &ll, 0 );


	cm.checkcount++;

	// test the contents of the leafs
	for (i=0 ; i < ll.count ; i++) {
		CM_TestInLeaf( tw, &cm.leafs[leafs[i]] );
		if ( tw->trace.allsolid ) {
			break;
		}
	}
}

/*
===============================================================================

TRACING

===============================================================================
*/


/*
================
CM_TraceThroughPatch
================
*/

void CM_TraceThroughPatch( traceWork_t *tw, cPatch_t *patch ) {
	gfixed		oldFrac;

	c_patch_traces++;

	oldFrac = tw->trace.fraction;

	CM_TraceThroughPatchCollide( tw, patch->pc );

	if ( tw->trace.fraction < oldFrac ) {
		tw->trace.surfaceFlags = patch->surfaceFlags;
		tw->trace.contents = patch->contents;
	}
}

/*
================
CM_TraceThroughBrush
================
*/
void CM_TraceThroughBrush( traceWork_t *tw, cbrush_t *brush ) {
	int			i;
	cplane_t	*plane, *clipplane;
	bfixed		dist;
	gfixed		enterFrac, leaveFrac;
	bfixed		d1, d2;
	qboolean	getout, startout;
	gfixed		f;
	cbrushside_t	*side, *leadside;
	bfixed		t;
	bvec3_t		startp;
	bvec3_t		endp;

	enterFrac = -GFIXED_1;
	leaveFrac = GFIXED_1;
	clipplane = NULL;

	if ( !brush->numsides ) {
		return;
	}

	c_brush_traces++;

	getout = qfalse;
	startout = qfalse;

	leadside = NULL;

	if ( tw->sphere.use ) {
		//
		// compare the trace against all planes of the brush
		// find the latest time the trace crosses a plane towards the interior
		// and the earliest time the trace crosses a plane towards the exterior
		//
		for (i = 0; i < brush->numsides; i++) {
			side = brush->sides + i;
			plane = side->plane;

			// adjust the plane distance apropriately for radius
			dist = plane->dist + tw->sphere.radius;

			// find the closest point on the capsule to the plane
			t = FIXED_VEC3DOT_R( plane->normal, tw->sphere.offset );
			if ( t > BFIXED_0 )
			{
				VectorSubtract( tw->start, tw->sphere.offset, startp );
				VectorSubtract( tw->end, tw->sphere.offset, endp );
			}
			else
			{
				VectorAdd( tw->start, tw->sphere.offset, startp );
				VectorAdd( tw->end, tw->sphere.offset, endp );
			}

			d1 = FIXED_VEC3DOT( startp, plane->normal ) - dist;
			d2 = FIXED_VEC3DOT( endp, plane->normal ) - dist;

			if (d2 > BFIXED_0) {
				getout = qtrue;	// endpoint is not in solid
			}
			if (d1 > BFIXED_0) {
				startout = qtrue;
			}

			// if completely in front of face, no intersection with the entire brush
			if (d1 > BFIXED_0 && ( d2 >= SURFACE_CLIP_EPSILON || d2 >= d1 )  ) {
				return;
			}

			// if it doesn't cross the plane, the plane isn't relevent
			if (d1 <= BFIXED_0 && d2 <= BFIXED_0 ) {
				continue;
			}

			// crosses face
			if (d1 > d2) {	// enter
				f = FIXED_RATIO_G(d1-SURFACE_CLIP_EPSILON,d1-d2);
				if ( f < GFIXED_0 ) {
					f = GFIXED_0;
				}
				if (f > enterFrac) {
					enterFrac = f;
					clipplane = plane;
					leadside = side;
				}
			} else {	// leave
				f = FIXED_RATIO_G(d1+SURFACE_CLIP_EPSILON,d1-d2);
				if ( f > GFIXED_1 ) {
					f = GFIXED_1;
				}
				if (f < leaveFrac) {
					leaveFrac = f;
				}
			}
		}
	} else {
		//
		// compare the trace against all planes of the brush
		// find the latest time the trace crosses a plane towards the interior
		// and the earliest time the trace crosses a plane towards the exterior
		//
		for (i = 0; i < brush->numsides; i++) {
			side = brush->sides + i;
			plane = side->plane;

			// adjust the plane distance apropriately for mins/maxs
			dist = plane->dist - FIXED_VEC3DOT( tw->offsets[ plane->signbits ], plane->normal );

			d1 = FIXED_VEC3DOT( tw->start, plane->normal ) - dist;
			d2 = FIXED_VEC3DOT( tw->end, plane->normal ) - dist;

			if (d2 > BFIXED_0) {
				getout = qtrue;	// endpoint is not in solid
			}
			if (d1 > BFIXED_0) {
				startout = qtrue;
			}

			// if completely in front of face, no intersection with the entire brush
			if (d1 > BFIXED_0 && ( d2 >= SURFACE_CLIP_EPSILON || d2 >= d1 )  ) {
				return;
			}

			// if it doesn't cross the plane, the plane isn't relevent
			if (d1 <= BFIXED_0 && d2 <= BFIXED_0 ) {
				continue;
			}

			// crosses face
			if (d1 > d2) {	// enter
				f = FIXED_RATIO_G(d1-SURFACE_CLIP_EPSILON,d1-d2);
				if ( f < GFIXED_0 ) {
					f = GFIXED_0;
				}
				if (f > enterFrac) {
					enterFrac = f;
					clipplane = plane;
					leadside = side;
				}
			} else {	// leave
				f = FIXED_RATIO_G(d1+SURFACE_CLIP_EPSILON,d1-d2);
				if ( f > GFIXED_1 ) {
					f = GFIXED_1;
				}
				if (f < leaveFrac) {
					leaveFrac = f;
				}
			}
		}
	}

	//
	// all planes have been checked, and the trace was not
	// completely outside the brush
	//
	if (!startout) {	// original point was inside brush
		tw->trace.startsolid = qtrue;
		if (!getout) {
			tw->trace.allsolid = qtrue;
			tw->trace.fraction = GFIXED_0;
			tw->trace.contents = brush->contents;
		}
		return;
	}
	
	if (enterFrac < leaveFrac) {
		if (enterFrac > -GFIXED_1 && enterFrac < tw->trace.fraction) {
			if (enterFrac < GFIXED_0) {
				enterFrac = GFIXED_0;
			}
			tw->trace.fraction = enterFrac;
			tw->trace.plane = *clipplane;
			tw->trace.surfaceFlags = leadside->surfaceFlags;
			tw->trace.contents = brush->contents;
		}
	}
}

/*
================
CM_TraceThroughLeaf
================
*/
void CM_TraceThroughLeaf( traceWork_t *tw, cLeaf_t *leaf ) {
	int			k;
	int			brushnum;
	cbrush_t	*b;
	cPatch_t	*patch;

	// trace line against all brushes in the leaf
	for ( k = 0 ; k < leaf->numLeafBrushes ; k++ ) {
		brushnum = cm.leafbrushes[leaf->firstLeafBrush+k];

		b = &cm.brushes[brushnum];
		if ( b->checkcount == cm.checkcount ) {
			continue;	// already checked this brush in another leaf
		}
		b->checkcount = cm.checkcount;

		if ( !(b->contents & tw->contents) ) {
			continue;
		}

		CM_TraceThroughBrush( tw, b );
		if ( FIXED_IS_ZERO(tw->trace.fraction) ) {
			return;
		}
	}

	// trace line against all patches in the leaf
#ifdef BSPC
	if (1) {
#else
	if ( !cm_noCurves->integer ) {
#endif
		for ( k = 0 ; k < leaf->numLeafSurfaces ; k++ ) {
			patch = cm.surfaces[ cm.leafsurfaces[ leaf->firstLeafSurface + k ] ];
			if ( !patch ) {
				continue;
			}
			if ( patch->checkcount == cm.checkcount ) {
				continue;	// already checked this patch in another leaf
			}
			patch->checkcount = cm.checkcount;

			if ( !(patch->contents & tw->contents) ) {
				continue;
			}
			
			CM_TraceThroughPatch( tw, patch );
			if ( FIXED_IS_ZERO(tw->trace.fraction) ) {
				return;
			}
		}
	}
}

#define RADIUS_EPSILON		BFIXED_1

/*
================
CM_TraceThroughSphere

get the first intersection of the ray with the sphere
================
*/
void CM_TraceThroughSphere( traceWork_t *tw, bvec3_t origin, bfixed radius, bvec3_t start, bvec3_t end ) {
	bfixed l1, l2;
	bfixed scale;
	bfixed length;
	gfixed fraction;
	bfixed a, b, c, d, sqrtd;
	bvec3_t v1, intersection;
	avec3_t adir;
	bvec3_t bdir;

	// if inside the sphere
	VectorSubtract(start, origin, bdir);
	l1 = FIXED_VEC3LEN_SQ(bdir);
	if (l1 < Square(radius)) {
		tw->trace.fraction = GFIXED_0;
		tw->trace.startsolid = qtrue;
		// test for allsolid
		VectorSubtract(end, origin, bdir);
		l1 = FIXED_VEC3LEN_SQ(bdir);
		if (l1 < Square(radius)) {
			tw->trace.allsolid = qtrue;
		}
		return;
	}
	//
	VectorSubtract(end, start, bdir);
	length = VectorNormalizeB2A(bdir,adir);
	//
	l1 = CM_DistanceFromLineSquared(origin, start, end, adir);
	VectorSubtract(end, origin, v1);
	l2 = FIXED_VEC3LEN_SQ(v1);
	// if no intersection with the sphere and the end point is at least an epsilon away
	if (l1 >= Square(radius) && l2 > Square(radius+SURFACE_CLIP_EPSILON)) {
		return;
	}
	//
	//	| origin - (start + t * dir) | = radius
	//	a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//	b = 2 * (dir[0] * (start[0] - origin[0]) + dir[1] * (start[1] - origin[1]) + dir[2] * (start[2] - origin[2]));
	//	c = (start[0] - origin[0])^2 + (start[1] - origin[1])^2 + (start[2] - origin[2])^2 - radius^2;
	//
	VectorSubtract(start, origin, v1);
	// dir is normalized so a = 1
	a = BFIXED_1;//adir[0] * adir[0] + adir[1] * adir[1] + adir[2] * adir[2];
	b = FIXED_MULPOW2((adir[0] * v1[0] + adir[1] * v1[1] + adir[2] * v1[2]),1);
	c = v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2] - (radius+RADIUS_EPSILON) * (radius+RADIUS_EPSILON);

	d = b * b - BFIXED(4,0) * c;// * a;
	if (d > BFIXED_0) {
		sqrtd = FIXED_SQRT(d);
		// = (- b + sqrtd) * BFIXED(0,5); // / (BFIXED(2,0) * a);
		fraction = MAKE_GFIXED(FIXED_DIVPOW2((- b - sqrtd),1)); // / (BFIXED(2,0) * a);
		//
		if (fraction < GFIXED_0) {
			fraction = GFIXED_0;
		}
		else {
			fraction /= MAKE_GFIXED(length);
		}
		if ( fraction < tw->trace.fraction ) {
			tw->trace.fraction = fraction;
			VectorSubtract(end, start, bdir);
			FIXED_VEC3MA(start, fraction, bdir, intersection);
			VectorSubtract(intersection, origin, bdir);
			#ifdef CAPSULE_DEBUG
				l2 = FIXED_VEC3LEN(bdir);
				if (l2 < radius) {
					int bah = 1;
				}
			#endif
			scale = (BFIXED_1 / (radius+RADIUS_EPSILON));
			adir[0]=MAKE_AFIXED(bdir[0]*scale);
			adir[1]=MAKE_AFIXED(bdir[1]*scale);
			adir[2]=MAKE_AFIXED(bdir[2]*scale);
			//FIXED_VEC3SCALE(bdir, scale, bdir);
			VectorCopy(adir, tw->trace.plane.normal);
			VectorAdd( tw->modelOrigin, intersection, intersection);
			tw->trace.plane.dist = FIXED_VEC3DOT_R(tw->trace.plane.normal, intersection);
			tw->trace.contents = CONTENTS_BODY;
		}
	}
	else if (d == BFIXED_0) {
		//t1 = (- b ) / 2;
		// slide along the sphere
	}
	// no intersection at all
}

/*
================
CM_TraceThroughVerticalCylinder

get the first intersection of the ray with the cylinder
the cylinder extends halfheight above and below the origin
================
*/
void CM_TraceThroughVerticalCylinder( traceWork_t *tw, bvec3_t origin, bfixed radius, bfixed halfheight, bvec3_t start, bvec3_t end) {
	bfixed scale, l1, l2;
	bfixed length;
	gfixed fraction;
	bfixed a, b, c, d, sqrtd;
	bvec3_t v1, start2d, end2d, org2d, intersection;
	bvec3_t bdir;
	avec3_t adir;

	// 2d coordinates
	VectorSet(start2d, start[0], start[1], BFIXED_0);
	VectorSet(end2d, end[0], end[1], BFIXED_0);
	VectorSet(org2d, origin[0], origin[1], BFIXED_0);
	// if between lower and upper cylinder bounds
	if (start[2] <= origin[2] + halfheight &&
				start[2] >= origin[2] - halfheight) {
		// if inside the cylinder
		VectorSubtract(start2d, org2d, bdir);
		l1 = FIXED_VEC3LEN_SQ(bdir);
		if (l1 < Square(radius)) {
			tw->trace.fraction = GFIXED_0;
			tw->trace.startsolid = qtrue;
			VectorSubtract(end2d, org2d, bdir);
			l1 = FIXED_VEC3LEN_SQ(bdir);
			if (l1 < Square(radius)) {
				tw->trace.allsolid = qtrue;
			}
			return;
		}
	}
	//
	VectorSubtract(end2d, start2d, bdir);
	length = VectorNormalizeB2A(bdir,adir);
	//
	l1 = CM_DistanceFromLineSquared(org2d, start2d, end2d, adir);
	VectorSubtract(end2d, org2d, v1);
	l2 = FIXED_VEC3LEN_SQ(v1);
	// if no intersection with the cylinder and the end point is at least an epsilon away
	if (l1 >= Square(radius) && l2 > Square(radius+SURFACE_CLIP_EPSILON)) {
		return;
	}
	//
	//
	// (start[0] - origin[0] - t * dir[0]) ^ 2 + (start[1] - origin[1] - t * dir[1]) ^ 2 = radius ^ 2
	// (v1[0] + t * dir[0]) ^ 2 + (v1[1] + t * dir[1]) ^ 2 = radius ^ 2;
	// v1[0] ^ 2 + 2 * v1[0] * t * dir[0] + (t * dir[0]) ^ 2 +
	//						v1[1] ^ 2 + 2 * v1[1] * t * dir[1] + (t * dir[1]) ^ 2 = radius ^ 2
	// t ^ 2 * (dir[0] ^ 2 + dir[1] ^ 2) + t * (2 * v1[0] * dir[0] + 2 * v1[1] * dir[1]) +
	//						v1[0] ^ 2 + v1[1] ^ 2 - radius ^ 2 = 0
	//
	VectorSubtract(start, origin, v1);
	// dir is normalized so we can use a = 1
	a = BFIXED_1;// * (dir[0] * dir[0] + dir[1] * dir[1]);
	b = FIXED_MULPOW2((v1[0] * adir[0] + v1[1] * adir[1]),1);
	c = v1[0] * v1[0] + v1[1] * v1[1] - (radius+RADIUS_EPSILON) * (radius+RADIUS_EPSILON);

	d = b * b - BFIXED(4,0) * c;// * a;
	if (d > BFIXED_0) {
		sqrtd = FIXED_SQRT(d);
		// = (- b + sqrtd) * BFIXED(0,5);// / (BFIXED(2,0) * a);
		fraction = MAKE_GFIXED(FIXED_DIVPOW2((- b - sqrtd),1));// / (BFIXED(2,0) * a);
		//
		if (fraction < GFIXED_0) {
			fraction = GFIXED_0;
		}
		else {
			fraction /= MAKE_GFIXED(length);
		}
		if ( fraction < tw->trace.fraction ) {
			VectorSubtract(end, start, bdir);
			FIXED_VEC3MA(start, fraction, bdir, intersection);
			// if the intersection is between the cylinder lower and upper bound
			if (intersection[2] <= origin[2] + halfheight &&
						intersection[2] >= origin[2] - halfheight) {
				//
				tw->trace.fraction = fraction;
				VectorSubtract(intersection, origin, bdir);
				bdir[2] = BFIXED_0;
				#ifdef CAPSULE_DEBUG
					l2 = FIXED_VEC3LEN(bdir);
					if (l2 <= radius) {
						int bah = 1;
					}
				#endif
				scale = BFIXED_1 / (radius+RADIUS_EPSILON);
				//FIXED_VEC3SCALE(dir, scale, dir);
				adir[0]=MAKE_AFIXED(bdir[0]*scale);
				adir[1]=MAKE_AFIXED(bdir[1]*scale);
				adir[2]=MAKE_AFIXED(bdir[2]*scale);
				VectorCopy(adir, tw->trace.plane.normal);
				VectorAdd( tw->modelOrigin, intersection, intersection);
				tw->trace.plane.dist = FIXED_VEC3DOT_R(tw->trace.plane.normal, intersection);
				tw->trace.contents = CONTENTS_BODY;
			}
		}
	}
	else if (d == BFIXED_0) {
		//t[0] = (- b ) / 2 * a;
		// slide along the cylinder
	}
	// no intersection at all
}

/*
================
CM_TraceCapsuleThroughCapsule

capsule vs. capsule collision (not rotated)
================
*/
void CM_TraceCapsuleThroughCapsule( traceWork_t *tw, clipHandle_t model ) {
	int i;
	bvec3_t mins, maxs;
	bvec3_t top, bottom, starttop, startbottom, endtop, endbottom;
	bvec3_t offset, symetricSize[2];
	bfixed radius, halfwidth, halfheight, offs, h;

	CM_ModelBounds(model, mins, maxs);
	// test trace bounds vs. capsule bounds
	if ( tw->bounds[0][0] > maxs[0] + RADIUS_EPSILON
		|| tw->bounds[0][1] > maxs[1] + RADIUS_EPSILON
		|| tw->bounds[0][2] > maxs[2] + RADIUS_EPSILON
		|| tw->bounds[1][0] < mins[0] - RADIUS_EPSILON
		|| tw->bounds[1][1] < mins[1] - RADIUS_EPSILON
		|| tw->bounds[1][2] < mins[2] - RADIUS_EPSILON
		) {
		return;
	}
	// top origin and bottom origin of each sphere at start and end of trace
	VectorAdd(tw->start, tw->sphere.offset, starttop);
	VectorSubtract(tw->start, tw->sphere.offset, startbottom);
	VectorAdd(tw->end, tw->sphere.offset, endtop);
	VectorSubtract(tw->end, tw->sphere.offset, endbottom);

	// calculate top and bottom of the capsule spheres to collide with
	for ( i = 0 ; i < 3 ; i++ ) {
		offset[i] = FIXED_DIVPOW2(( mins[i] + maxs[i] ),1);
		symetricSize[0][i] = mins[i] - offset[i];
		symetricSize[1][i] = maxs[i] - offset[i];
	}
	halfwidth = symetricSize[ 1 ][ 0 ];
	halfheight = symetricSize[ 1 ][ 2 ];
	radius = ( halfwidth > halfheight ) ? halfheight : halfwidth;
	offs = halfheight - radius;
	VectorCopy(offset, top);
	top[2] += offs;
	VectorCopy(offset, bottom);
	bottom[2] -= offs;
	// expand radius of spheres
	radius += tw->sphere.radius;
	// if there is horizontal movement
	if ( tw->start[0] != tw->end[0] || tw->start[1] != tw->end[1] ) {
		// height of the expanded cylinder is the height of both cylinders minus the radius of both spheres
		h = halfheight + tw->sphere.halfheight - radius;
		// if the cylinder has a height
		if ( h > BFIXED_0 ) {
			// test for collisions between the cylinders
			CM_TraceThroughVerticalCylinder(tw, offset, radius, h, tw->start, tw->end);
		}
	}
	// test for collision between the spheres
	CM_TraceThroughSphere(tw, top, radius, startbottom, endbottom);
	CM_TraceThroughSphere(tw, bottom, radius, starttop, endtop);
}

/*
================
CM_TraceBoundingBoxThroughCapsule

bounding box vs. capsule collision
================
*/
void CM_TraceBoundingBoxThroughCapsule( traceWork_t *tw, clipHandle_t model ) {
	bvec3_t mins, maxs, offset, size[2];
	clipHandle_t h;
	cmodel_t *cmod;
	int i;

	// mins maxs of the capsule
	CM_ModelBounds(model, mins, maxs);

	// offset for capsule center
	for ( i = 0 ; i < 3 ; i++ ) {
		offset[i] = FIXED_DIVPOW2(( mins[i] + maxs[i] ),1);
		size[0][i] = mins[i] - offset[i];
		size[1][i] = maxs[i] - offset[i];
		tw->start[i] -= offset[i];
		tw->end[i] -= offset[i];
	}

	// replace the bounding box with the capsule
	tw->sphere.use = qtrue;
	tw->sphere.radius = ( size[1][0] > size[1][2] ) ? size[1][2]: size[1][0];
	tw->sphere.halfheight = size[1][2];
	VectorSet( tw->sphere.offset, BFIXED_0, BFIXED_0, size[1][2] - tw->sphere.radius );

	// replace the capsule with the bounding box
	h = CM_TempBoxModel(tw->size[0], tw->size[1], qfalse);
	// calculate collision
	cmod = CM_ClipHandleToModel( h );
	CM_TraceThroughLeaf( tw, &cmod->leaf );
}

//=========================================================================================

/*
==================
CM_TraceThroughTree

Traverse all the contacted leafs from the start to the end position.
If the trace is a point, they will be exactly in order, but for larger
trace volumes it is possible to hit something in a later leaf with
a smaller intercept fraction.
==================
*/
void CM_TraceThroughTree( traceWork_t *tw, int num, gfixed p1f, gfixed p2f, bvec3_t p1, bvec3_t p2) {
	cNode_t		*node;
	cplane_t	*plane;
	bfixed		t1, t2, offset;
	gfixed		frac, frac2;
	bvec3_t		mid;
	int			side;
	gfixed		midf;

	if (tw->trace.fraction <= p1f) {
		return;		// already hit something nearer
	}

	// if < 0, we are in a leaf node
	if (num < 0) {
		CM_TraceThroughLeaf( tw, &cm.leafs[-1-num] );
		return;
	}

	//
	// find the point distances to the seperating plane
	// and the offset for the size of the box
	//
	node = cm.nodes + num;
	plane = node->plane;

	// adjust the plane distance apropriately for mins/maxs
	if ( plane->type < 3 ) {
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
		offset = tw->extents[plane->type];
	} else {
		t1 = FIXED_VEC3DOT_R (plane->normal, p1) - plane->dist;
		t2 = FIXED_VEC3DOT_R (plane->normal, p2) - plane->dist;
		if ( tw->isPoint ) {
			offset = BFIXED_0;
		} else {
#if 0 // bk010201 - DEAD
			// an axial brush right behind a slanted bsp plane
			// will poke through when expanded, so adjust
			// by sqrt(3)
			offset = FIXED_ABS(tw->extents[0]*plane->normal[0]) +
				FIXED_ABS(tw->extents[1]*plane->normal[1]) +
				FIXED_ABS(tw->extents[2]*plane->normal[2]);

			offset *= 2;
			offset = tw->maxOffset;
#endif
			// this is silly
			offset = BFIXED(2048,0);
		}
	}

	// see which sides we need to consider
	if ( t1 >= offset +BFIXED_1 && t2 >= offset + BFIXED_1 ) {
		CM_TraceThroughTree( tw, node->children[0], p1f, p2f, p1, p2 );
		return;
	}
	if ( t1 < -offset - BFIXED_1 && t2 < -offset - BFIXED_1 ) {
		CM_TraceThroughTree( tw, node->children[1], p1f, p2f, p1, p2 );
		return;
	}

	// put the crosspoint SURFACE_CLIP_EPSILON pixels on the near side
	if ( t1 < t2 ) {
		side = 1;
		frac2 = FIXED_RATIO_G(t1 + offset + SURFACE_CLIP_EPSILON,t1-t2);
		frac = FIXED_RATIO_G(t1 - offset + SURFACE_CLIP_EPSILON,t1-t2);
	} else if (t1 > t2) {
		side = 0;
		frac2 = FIXED_RATIO_G(t1 - offset - SURFACE_CLIP_EPSILON,t1-t2);
		frac = FIXED_RATIO_G(t1 + offset + SURFACE_CLIP_EPSILON,t1-t2);
	} else {
		side = 0;
		frac = GFIXED_1;
		frac2 = GFIXED_0;
	}

	// move up to the node
	if ( frac < GFIXED_0 ) {
		frac = GFIXED_0;
	}
	if ( frac > GFIXED_1 ) {
		frac = GFIXED_1;
	}
		
	midf = p1f + (p2f - p1f)*frac;

	mid[0] = p1[0] + frac*(p2[0] - p1[0]);
	mid[1] = p1[1] + frac*(p2[1] - p1[1]);
	mid[2] = p1[2] + frac*(p2[2] - p1[2]);

	CM_TraceThroughTree( tw, node->children[side], p1f, midf, p1, mid );


	// go past the node
	if ( frac2 < GFIXED_0 ) {
		frac2 = GFIXED_0;
	}
	if ( frac2 > GFIXED_1 ) {
		frac2 = GFIXED_1;
	}
		
	midf = p1f + (p2f - p1f)*frac2;

	mid[0] = p1[0] + frac2*(p2[0] - p1[0]);
	mid[1] = p1[1] + frac2*(p2[1] - p1[1]);
	mid[2] = p1[2] + frac2*(p2[2] - p1[2]);

	CM_TraceThroughTree( tw, node->children[side^1], midf, p2f, mid, p2 );
}


//======================================================================


/*
==================
CM_Trace
==================
*/
void CM_Trace( trace_t *results, const bvec3_t start, const bvec3_t end, const bvec3_t mins, const bvec3_t maxs,
						  clipHandle_t model, const bvec3_t origin, int brushmask, int capsule, sphere_t *sphere ) {
	int			i;
	traceWork_t	tw;
	bvec3_t		offset;
	cmodel_t	*cmod;

	cmod = CM_ClipHandleToModel( model );

	cm.checkcount++;		// for multi-check avoidance

	c_traces++;				// for statistics, may be zeroed

	// fill in a default trace
	Com_Memset( &tw, 0, sizeof(tw) );
	tw.trace.fraction = GFIXED_1;	// assume it goes the entire distance until shown otherwise
	VectorCopy(origin, tw.modelOrigin);

	if (!cm.numNodes) {
		*results = tw.trace;

		return;	// map not loaded, shouldn't happen
	}

	// allow NULL to be passed in for 0,0,0
	if ( !mins ) {
		mins = bvec3_origin;
	}
	if ( !maxs ) {
		maxs = bvec3_origin;
	}

	// set basic parms
	tw.contents = brushmask;

	// adjust so that mins and maxs are always symetric, which
	// avoids some complications with plane expanding of rotated
	// bmodels
	for ( i = 0 ; i < 3 ; i++ ) {
		offset[i] = FIXED_DIVPOW2( mins[i] + maxs[i],1 );
		tw.size[0][i] = mins[i] - offset[i];
		tw.size[1][i] = maxs[i] - offset[i];
		tw.start[i] = start[i] + offset[i];
		tw.end[i] = end[i] + offset[i];
	}

	// if a sphere is already specified
	if ( sphere ) {
		tw.sphere = *sphere;
	}
	else {
		tw.sphere.use = capsule;
		tw.sphere.radius = ( tw.size[1][0] > tw.size[1][2] ) ? tw.size[1][2]: tw.size[1][0];
		tw.sphere.halfheight = tw.size[1][2];
		VectorSet( tw.sphere.offset, BFIXED_0, BFIXED_0, tw.size[1][2] - tw.sphere.radius );
	}

	tw.maxOffset = tw.size[1][0] + tw.size[1][1] + tw.size[1][2];

	// tw.offsets[signbits] = vector to apropriate corner from origin
	tw.offsets[0][0] = tw.size[0][0];
	tw.offsets[0][1] = tw.size[0][1];
	tw.offsets[0][2] = tw.size[0][2];

	tw.offsets[1][0] = tw.size[1][0];
	tw.offsets[1][1] = tw.size[0][1];
	tw.offsets[1][2] = tw.size[0][2];

	tw.offsets[2][0] = tw.size[0][0];
	tw.offsets[2][1] = tw.size[1][1];
	tw.offsets[2][2] = tw.size[0][2];

	tw.offsets[3][0] = tw.size[1][0];
	tw.offsets[3][1] = tw.size[1][1];
	tw.offsets[3][2] = tw.size[0][2];

	tw.offsets[4][0] = tw.size[0][0];
	tw.offsets[4][1] = tw.size[0][1];
	tw.offsets[4][2] = tw.size[1][2];

	tw.offsets[5][0] = tw.size[1][0];
	tw.offsets[5][1] = tw.size[0][1];
	tw.offsets[5][2] = tw.size[1][2];

	tw.offsets[6][0] = tw.size[0][0];
	tw.offsets[6][1] = tw.size[1][1];
	tw.offsets[6][2] = tw.size[1][2];

	tw.offsets[7][0] = tw.size[1][0];
	tw.offsets[7][1] = tw.size[1][1];
	tw.offsets[7][2] = tw.size[1][2];

	//
	// calculate bounds
	//
	if ( tw.sphere.use ) {
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( tw.start[i] < tw.end[i] ) {
				tw.bounds[0][i] = tw.start[i] - FIXED_ABS(tw.sphere.offset[i]) - tw.sphere.radius;
				tw.bounds[1][i] = tw.end[i] + FIXED_ABS(tw.sphere.offset[i]) + tw.sphere.radius;
			} else {
				tw.bounds[0][i] = tw.end[i] - FIXED_ABS(tw.sphere.offset[i]) - tw.sphere.radius;
				tw.bounds[1][i] = tw.start[i] + FIXED_ABS(tw.sphere.offset[i]) + tw.sphere.radius;
			}
		}
	}
	else {
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( tw.start[i] < tw.end[i] ) {
				tw.bounds[0][i] = tw.start[i] + tw.size[0][i];
				tw.bounds[1][i] = tw.end[i] + tw.size[1][i];
			} else {
				tw.bounds[0][i] = tw.end[i] + tw.size[0][i];
				tw.bounds[1][i] = tw.start[i] + tw.size[1][i];
			}
		}
	}

	//
	// check for position test special case
	//
	if (start[0] == end[0] && start[1] == end[1] && start[2] == end[2]) {
		if ( model ) {
#ifdef ALWAYS_BBOX_VS_BBOX // bk010201 - FIXME - compile time flag?
			if ( model == BOX_MODEL_HANDLE || model == CAPSULE_MODEL_HANDLE) {
				tw.sphere.use = qfalse;
				CM_TestInLeaf( &tw, &cmod->leaf );
			}
			else
#elif defined(ALWAYS_CAPSULE_VS_CAPSULE)
			if ( model == BOX_MODEL_HANDLE || model == CAPSULE_MODEL_HANDLE) {
				CM_TestCapsuleInCapsule( &tw, model );
			}
			else
#endif
			if ( model == CAPSULE_MODEL_HANDLE ) {
				if ( tw.sphere.use ) {
					CM_TestCapsuleInCapsule( &tw, model );
				}
				else {
					CM_TestBoundingBoxInCapsule( &tw, model );
				}
			}
			else {
				CM_TestInLeaf( &tw, &cmod->leaf );
			}
		} else {
			CM_PositionTest( &tw );
		}
	} else {
		//
		// check for point special case
		//
		if ( tw.size[0][0] == BFIXED_0 && tw.size[0][1] == BFIXED_0 && tw.size[0][2] == BFIXED_0 ) {
			tw.isPoint = qtrue;
			VectorClear( tw.extents );
		} else {
			tw.isPoint = qfalse;
			tw.extents[0] = tw.size[1][0];
			tw.extents[1] = tw.size[1][1];
			tw.extents[2] = tw.size[1][2];
		}

		//
		// general sweeping through world
		//
		if ( model ) {
#ifdef ALWAYS_BBOX_VS_BBOX
			if ( model == BOX_MODEL_HANDLE || model == CAPSULE_MODEL_HANDLE) {
				tw.sphere.use = qfalse;
				CM_TraceThroughLeaf( &tw, &cmod->leaf );
			}
			else
#elif defined(ALWAYS_CAPSULE_VS_CAPSULE)
			if ( model == BOX_MODEL_HANDLE || model == CAPSULE_MODEL_HANDLE) {
				CM_TraceCapsuleThroughCapsule( &tw, model );
			}
			else
#endif
			if ( model == CAPSULE_MODEL_HANDLE ) {
				if ( tw.sphere.use ) {
					CM_TraceCapsuleThroughCapsule( &tw, model );
				}
				else {
					CM_TraceBoundingBoxThroughCapsule( &tw, model );
				}
			}
			else {
				CM_TraceThroughLeaf( &tw, &cmod->leaf );
			}
		} else {
			CM_TraceThroughTree( &tw, 0, GFIXED_0, GFIXED_1, tw.start, tw.end );
		}
	}

	// generate endpos from the original, unmodified start/end
	if ( tw.trace.fraction == GFIXED_1 ) {
		VectorCopy (end, tw.trace.endpos);
	} else {
		for ( i=0 ; i<3 ; i++ ) {
			tw.trace.endpos[i] = start[i] + (tw.trace.fraction * (end[i] - start[i]));
		}
	}

        // If allsolid is set (was entirely inside something solid), the plane is not valid.
        // If fraction == BFIXED_1, we never hit anything, and thus the plane is not valid.
        // Otherwise, the normal on the plane should have unit length
        assert(tw.trace.allsolid ||
               tw.trace.fraction == GFIXED_1 ||
               FIXED_VEC3LEN_SQ(tw.trace.plane.normal) > AFIXED(0,9999));
	*results = tw.trace;
}

/*
==================
CM_BoxTrace
==================
*/
void CM_BoxTrace( trace_t *results, const bvec3_t start, const bvec3_t end,
						  const bvec3_t mins, const bvec3_t maxs,
						  clipHandle_t model, int brushmask, int capsule ) {
	CM_Trace( results, start, end, mins, maxs, model, bvec3_origin, brushmask, capsule, NULL );
}

/*
==================
CM_TransformedBoxTrace

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
void CM_TransformedBoxTrace( trace_t *results, const bvec3_t start, const bvec3_t end,
						  const bvec3_t _mins, const bvec3_t _maxs,
						  clipHandle_t model, int brushmask,
						  const bvec3_t origin, const avec3_t angles, int capsule ) {
	trace_t		trace;
	bvec3_t		start_l, end_l;
	qboolean	rotated;
	bvec3_t		offset;
	bvec3_t		symetricSize[2];
	avec3_t		matrix[3], transpose[3];
	int			i;
	bfixed		halfwidth;
	bfixed		halfheight;
	bfixed		t;
	sphere_t	sphere;
	const bfixed *mins,*maxs;

	if(_mins==NULL) {
		mins=bvec3_origin;
	} else {
		mins=_mins;
	}
	if(_maxs==NULL) {
		maxs=bvec3_origin;
	} else {
		maxs=_maxs;
	}
	
	// adjust so that mins and maxs are always symetric, which
	// avoids some complications with plane expanding of rotated
	// bmodels
	for ( i = 0 ; i < 3 ; i++ ) {
		offset[i] = FIXED_DIVPOW2(( mins[i] + maxs[i] ),1);
		symetricSize[0][i] = mins[i] - offset[i];
		symetricSize[1][i] = maxs[i] - offset[i];
		start_l[i] = start[i] + offset[i];
		end_l[i] = end[i] + offset[i];
	}

	// subtract origin offset
	VectorSubtract( start_l, origin, start_l );
	VectorSubtract( end_l, origin, end_l );

	// rotate start and end into the models frame of reference
	if ( model != BOX_MODEL_HANDLE && 
		(FIXED_NOT_ZERO(angles[0]) || FIXED_NOT_ZERO(angles[1]) || FIXED_NOT_ZERO(angles[2])) ) {
		rotated = qtrue;
	} else {
		rotated = qfalse;
	}

	halfwidth = symetricSize[ 1 ][ 0 ];
	halfheight = symetricSize[ 1 ][ 2 ];

	sphere.use = capsule;
	sphere.radius = ( halfwidth > halfheight ) ? halfheight : halfwidth;
	sphere.halfheight = halfheight;
	t = halfheight - sphere.radius;

	if (rotated) {
		// rotation on trace line (start-end) instead of rotating the bmodel
		// NOTE: This is still incorrect for bounding boxes because the actual bounding
		//		 box that is swept through the model is not rotated. We cannot rotate
		//		 the bounding box or the bmodel because that would make all the brush
		//		 bevels invalid.
		//		 However this is correct for capsules since a capsule itself is rotated too.
		CreateRotationMatrix(angles, matrix);
		RotatePoint(start_l, matrix);
		RotatePoint(end_l, matrix);
		// rotated sphere offset for capsule
		sphere.offset[0] = matrix[0][ 2 ] * t;
		sphere.offset[1] = -matrix[1][ 2 ] * t;
		sphere.offset[2] = matrix[2][ 2 ] * t;
	}
	else {
		VectorSet( sphere.offset, BFIXED_0, BFIXED_0, t );
	}

	// sweep the box through the model
	CM_Trace( &trace, start_l, end_l, symetricSize[0], symetricSize[1], model, origin, brushmask, capsule, &sphere );

	// if the bmodel was rotated and there was a collision
	if ( rotated && trace.fraction != GFIXED_1 ) {
		// rotation of bmodel collision plane
		TransposeMatrix(matrix, transpose);
		RotatePoint(trace.plane.normal, transpose);
	}

	// re-calculate the end position of the trace because the trace.endpos
	// calculated by CM_Trace could be rotated and have an offset
	trace.endpos[0] = start[0] + trace.fraction * (end[0] - start[0]);
	trace.endpos[1] = start[1] + trace.fraction * (end[1] - start[1]);
	trace.endpos[2] = start[2] + trace.fraction * (end[2] - start[2]);

	*results = trace;
}
