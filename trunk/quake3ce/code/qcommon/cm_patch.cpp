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


/*

This file does not reference any globals, and has these entry points:

void CM_ClearLevelPatches( void );
struct patchCollide_s	*CM_GeneratePatchCollide( int width, int height, const bvec3_t *points );
void CM_TraceThroughPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
qboolean CM_PositionTestInPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
void CM_DrawDebugSurface( void (*drawPoly)(int color, int numPoints, flaot *points) );


WARNING: this may misbehave with meshes that have rows or columns that only
degenerate a few triangles.  Completely degenerate rows and columns are handled
properly.
*/

/*
#define	MAX_FACETS			1024
#define	MAX_PATCH_PLANES	2048

typedef struct {
	gfixed	plane[4];
	int		signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
} patchPlane_t;

typedef struct {
	int			surfacePlane;
	int			numBorders;		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
	int			borderPlanes[4+6+16];
	int			borderInward[4+6+16];
	qboolean	borderNoAdjust[4+6+16];
} facet_t;

typedef struct patchCollide_s {
	bvec3_t	bounds[2];
	int		numPlanes;			// surface planes plus edge planes
	patchPlane_t	*planes;
	int		numFacets;
	facet_t	*facets;
} patchCollide_t;


#define	MAX_GRID_SIZE	129

typedef struct {
	int			width;
	int			height;
	qboolean	wrapWidth;
	qboolean	wrapHeight;
	bvec3_t	points[MAX_GRID_SIZE][MAX_GRID_SIZE];	// [width][height]
} cGrid_t;

#define	SUBDIVIDE_DISTANCE	16	//4	// never more than this units away from curve
#define	PLANE_TRI_EPSILON	BFIXED(0,1)
#define	WRAP_POINT_EPSILON	BFIXED(0,1)
*/

int	c_totalPatchBlocks;
int	c_totalPatchSurfaces;
int	c_totalPatchEdges;

static const patchCollide_t	*debugPatchCollide;
static const facet_t		*debugFacet;
static qboolean		debugBlock;
static bvec3_t		debugBlockPoints[4];

/*
=================
CM_ClearLevelPatches
=================
*/
void CM_ClearLevelPatches( void ) {
	debugPatchCollide = NULL;
	debugFacet = NULL;
}

/*
=================
CM_SignbitsForNormal
=================
*/
static int CM_SignbitsForNormal( avec3_t normal ) {
	int	bits, j;

	bits = 0;
	for (j=0 ; j<3 ; j++) {
		if ( normal[j] < AFIXED_0 ) {
			bits |= 1<<j;
		}
	}
	return bits;
}

/*
=====================
CM_PlaneFromPoints

Returns false if the triangle is degenrate.
The normal will point out of the clock for clockwise ordered points
=====================
*/
static qboolean CM_PlaneFromPoints( planeDef_t & plane, bvec3_t a, bvec3_t b, bvec3_t c ) {
	bvec3_t	d1, d2, tmp;
	VectorSubtract( b, a, d1 );
	VectorSubtract( c, a, d2 );
	CrossProduct( d2, d1, tmp );
	
	if ( FIXED_IS_ZERO(VectorNormalizeB2A( tmp, plane.normal ))) {
		return qfalse;
	}

	plane.dist = FIXED_VEC3DOT( a, plane.normal );
	return qtrue;
}


/*
================================================================================

GRID SUBDIVISION

================================================================================
*/

/*
=================
CM_NeedsSubdivision

Returns true if the given quadratic curve is not flat enough for our
collision detection purposes
=================
*/
static qboolean	CM_NeedsSubdivision( bvec3_t a, bvec3_t b, bvec3_t c ) {
	bvec3_t		cmid;
	bvec3_t		lmid;
	bvec3_t		delta;
	bfixed		dist;
	int			i;

	// calculate the linear midpoint
	for ( i = 0 ; i < 3 ; i++ ) {
		lmid[i] = FIXED_DIVPOW2(a[i] + c[i],1);
	}

	// calculate the exact curve midpoint
	for ( i = 0 ; i < 3 ; i++ ) {
		cmid[i] = FIXED_DIVPOW2((FIXED_DIVPOW2(a[i] + b[i],1) + FIXED_DIVPOW2(b[i] + c[i], 1)),1);
	}

	// see if the curve is far enough away from the linear mid
	VectorSubtract( cmid, lmid, delta );
	dist = FIXED_VEC3LEN( delta );
	
	return dist >= BFIXED(SUBDIVIDE_DISTANCE,0);
}

/*
===============
CM_Subdivide

a, b, and c are control points.
the subdivided sequence will be: a, out1, out2, out3, c
===============
*/
static void CM_Subdivide( bvec3_t a, bvec3_t b, bvec3_t c, bvec3_t out1, bvec3_t out2, bvec3_t out3 ) {
	int		i;

	for ( i = 0 ; i < 3 ; i++ ) {
		out1[i] = FIXED_DIVPOW2(a[i] + b[i],1);
		out3[i] = FIXED_DIVPOW2(b[i] + c[i],1);
		out2[i] = FIXED_DIVPOW2(out1[i] + out3[i],1);
	}
}

/*
=================
CM_TransposeGrid

Swaps the rows and columns in place
=================
*/
static void CM_TransposeGrid( cGrid_t *grid ) {
	int			i, j, l;
	bvec3_t		temp;
	qboolean	tempWrap;

	if ( grid->width > grid->height ) {
		for ( i = 0 ; i < grid->height ; i++ ) {
			for ( j = i + 1 ; j < grid->width ; j++ ) {
				if ( j < grid->height ) {
					// swap the value
					VectorCopy( grid->points[i][j], temp );
					VectorCopy( grid->points[j][i], grid->points[i][j] );
					VectorCopy( temp, grid->points[j][i] );
				} else {
					// just copy
					VectorCopy( grid->points[j][i], grid->points[i][j] );
				}
			}
		}
	} else {
		for ( i = 0 ; i < grid->width ; i++ ) {
			for ( j = i + 1 ; j < grid->height ; j++ ) {
				if ( j < grid->width ) {
					// swap the value
					VectorCopy( grid->points[j][i], temp );
					VectorCopy( grid->points[i][j], grid->points[j][i] );
					VectorCopy( temp, grid->points[i][j] );
				} else {
					// just copy
					VectorCopy( grid->points[i][j], grid->points[j][i] );
				}
			}
		}
	}

	l = grid->width;
	grid->width = grid->height;
	grid->height = l;

	tempWrap = grid->wrapWidth;
	grid->wrapWidth = grid->wrapHeight;
	grid->wrapHeight = tempWrap;
}

/*
===================
CM_SetGridWrapWidth

If the left and right columns are exactly equal, set grid->wrapWidth qtrue
===================
*/
static void CM_SetGridWrapWidth( cGrid_t *grid ) {
	int		i, j;
	bfixed	d;

	for ( i = 0 ; i < grid->height ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			d = grid->points[0][i][j] - grid->points[grid->width-1][i][j];
			if ( d < -WRAP_POINT_EPSILON || d > WRAP_POINT_EPSILON ) {
				break;
			}
		}
		if ( j != 3 ) {
			break;
		}
	}
	if ( i == grid->height ) {
		grid->wrapWidth = qtrue;
	} else {
		grid->wrapWidth = qfalse;
	}
}

/*
=================
CM_SubdivideGridColumns

Adds columns as necessary to the grid until
all the aproximating points are within SUBDIVIDE_DISTANCE
from the true curve
=================
*/
static void CM_SubdivideGridColumns( cGrid_t *grid ) {
	int		i, j, k;

	for ( i = 0 ; i < grid->width - 2 ;  ) {
		// grid->points[i][x] is an interpolating control point
		// grid->points[i+1][x] is an aproximating control point
		// grid->points[i+2][x] is an interpolating control point

		//
		// first see if we can collapse the aproximating collumn away
		//
		for ( j = 0 ; j < grid->height ; j++ ) {
			if ( CM_NeedsSubdivision( grid->points[i][j], grid->points[i+1][j], grid->points[i+2][j] ) ) {
				break;
			}
		}
		if ( j == grid->height ) {
			// all of the points were close enough to the linear midpoints
			// that we can collapse the entire column away
			for ( j = 0 ; j < grid->height ; j++ ) {
				// remove the column
				for ( k = i + 2 ; k < grid->width ; k++ ) {
					VectorCopy( grid->points[k][j], grid->points[k-1][j] );
				}
			}

			grid->width--;

			// go to the next curve segment
			i++;
			continue;
		}

		//
		// we need to subdivide the curve
		//
		for ( j = 0 ; j < grid->height ; j++ ) {
			bvec3_t	prev, mid, next;

			// save the control points now
			VectorCopy( grid->points[i][j], prev );
			VectorCopy( grid->points[i+1][j], mid );
			VectorCopy( grid->points[i+2][j], next );

			// make room for two additional columns in the grid
			// columns i+1 will be replaced, column i+2 will become i+4
			// i+1, i+2, and i+3 will be generated
			for ( k = grid->width - 1 ; k > i + 1 ; k-- ) {
				VectorCopy( grid->points[k][j], grid->points[k+2][j] );
			}

			// generate the subdivided points
			CM_Subdivide( prev, mid, next, grid->points[i+1][j], grid->points[i+2][j], grid->points[i+3][j] );
		}

		grid->width += 2;

		// the new aproximating point at i+1 may need to be removed
		// or subdivided farther, so don't advance i
	}
}

/*
======================
CM_ComparePoints
======================
*/
#define	POINT_EPSILON	BFIXED(0,1)
static qboolean CM_ComparePoints( bfixed *a, bfixed *b ) {
	bfixed		d;

	d = a[0] - b[0];
	if ( d < -POINT_EPSILON || d > POINT_EPSILON ) {
		return qfalse;
	}
	d = a[1] - b[1];
	if ( d < -POINT_EPSILON || d > POINT_EPSILON ) {
		return qfalse;
	}
	d = a[2] - b[2];
	if ( d < -POINT_EPSILON || d > POINT_EPSILON ) {
		return qfalse;
	}
	return qtrue;
}

/*
=================
CM_RemoveDegenerateColumns

If there are any identical columns, remove them
=================
*/
static void CM_RemoveDegenerateColumns( cGrid_t *grid ) {
	int		i, j, k;

	for ( i = 0 ; i < grid->width - 1 ; i++ ) {
		for ( j = 0 ; j < grid->height ; j++ ) {
			if ( !CM_ComparePoints( grid->points[i][j], grid->points[i+1][j] ) ) {
				break;
			}
		}

		if ( j != grid->height ) {
			continue;	// not degenerate
		}

		for ( j = 0 ; j < grid->height ; j++ ) {
			// remove the column
			for ( k = i + 2 ; k < grid->width ; k++ ) {
				VectorCopy( grid->points[k][j], grid->points[k-1][j] );
			}
		}
		grid->width--;

		// check against the next column
		i--;
	}
}

/*
================================================================================

PATCH COLLIDE GENERATION

================================================================================
*/

static	int				numPlanes;
static	patchPlane_t	planes[MAX_PATCH_PLANES];

static	int				numFacets;
static	facet_t			facets[MAX_PATCH_PLANES]; //maybe MAX_FACETS ??

#define	NORMAL_EPSILON	 AFIXED(0,0001)	
#define	DIST_EPSILON	 BFIXED(0,02)

/*
==================
CM_PlaneEqual
==================
*/
int CM_PlaneEqual(patchPlane_t *p, planeDef_t &plane, int *flipped) {
	planeDef_t invplane;

	if (
	   FIXED_ABS(p->pd.normal[0] - plane.normal[0]) < NORMAL_EPSILON
	&& FIXED_ABS(p->pd.normal[1] - plane.normal[1]) < NORMAL_EPSILON
	&& FIXED_ABS(p->pd.normal[2] - plane.normal[2]) < NORMAL_EPSILON
	&& FIXED_ABS(p->pd.dist - plane.dist) < DIST_EPSILON )
	{
		*flipped = qfalse;
		return qtrue;
	}

	invplane.normal[0] = -plane.normal[0];
	invplane.normal[1] = -plane.normal[1];
	invplane.normal[2] = -plane.normal[2];
	invplane.dist = -plane.dist;

	if (
	   FIXED_ABS(p->pd.normal[0] - plane.normal[0]) < NORMAL_EPSILON
	&& FIXED_ABS(p->pd.normal[1] - plane.normal[1]) < NORMAL_EPSILON
	&& FIXED_ABS(p->pd.normal[2] - plane.normal[2]) < NORMAL_EPSILON
	&& FIXED_ABS(p->pd.dist - plane.dist) < DIST_EPSILON )
	{
		*flipped = qtrue;
		return qtrue;
	}

	return qfalse;
}

/*
==================
CM_SnapVector
==================
*/
void CM_SnapVector(avec3_t normal) {
	int		i;

	for (i=0 ; i<3 ; i++)
	{
		if ( FIXED_ABS(normal[i] - AFIXED_1) < NORMAL_EPSILON )
		{
			VectorClear (normal);
			normal[i] = AFIXED_1;
			break;
		}
		if ( FIXED_ABS(normal[i] - -AFIXED_1) < NORMAL_EPSILON )
		{
			VectorClear (normal);
			normal[i] = -AFIXED_1;
			break;
		}
	}
}

/*
==================
CM_FindPlane2
==================
*/
int CM_FindPlane2(planeDef_t &plane, int *flipped) {
	int i;

	// see if the points are close enough to an existing plane
	for ( i = 0 ; i < numPlanes ; i++ ) {
		if (CM_PlaneEqual(&planes[i], plane, flipped)) return i;
	}

	// add a new plane
	if ( numPlanes == MAX_PATCH_PLANES ) {
		Com_Error( ERR_DROP, "MAX_PATCH_PLANES" );
	}

	planes[numPlanes].pd=plane;
	planes[numPlanes].signbits = CM_SignbitsForNormal( plane.normal );

	numPlanes++;

	*flipped = qfalse;

	return numPlanes-1;
}

/*
==================
CM_FindPlane
==================
*/
static int CM_FindPlane( bfixed *p1, bfixed *p2, bfixed *p3 ) {
	planeDef_t plane;
	int		i;
	bfixed	d;

	if ( !CM_PlaneFromPoints( plane, p1, p2, p3 ) ) {
		return -1;
	}

	// see if the points are close enough to an existing plane
	for ( i = 0 ; i < numPlanes ; i++ ) {
		if ( FIXED_VEC3DOT( plane.normal, planes[i].pd.normal ) < AFIXED_0 ) {
			continue;	// allow backwards planes?
		}

		d = FIXED_VEC3DOT( p1, planes[i].pd.normal ) - planes[i].pd.dist;
		if ( d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON ) {
			continue;
		}

		d = FIXED_VEC3DOT( p2, planes[i].pd.normal ) - planes[i].pd.dist;
		if ( d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON ) {
			continue;
		}

		d = FIXED_VEC3DOT( p3, planes[i].pd.normal ) - planes[i].pd.dist;
		if ( d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON ) {
			continue;
		}

		// found it
		return i;
	}

	// add a new plane
	if ( numPlanes == MAX_PATCH_PLANES ) {
		Com_Error( ERR_DROP, "MAX_PATCH_PLANES" );
	}

	planes[numPlanes].pd=plane;
	planes[numPlanes].signbits = CM_SignbitsForNormal( plane.normal );

	numPlanes++;

	return numPlanes-1;
}

/*
==================
CM_PointOnPlaneSide
==================
*/
static int CM_PointOnPlaneSide(bfixed *p, int planeNum ) {
	bfixed	d;

	if ( planeNum == -1 ) {
		return SIDE_ON;
	}

	d = FIXED_VEC3DOT( p, planes[ planeNum ].pd.normal ) - planes[ planeNum ].pd.dist;

	if ( d > PLANE_TRI_EPSILON ) {
		return SIDE_FRONT;
	}

	if ( d < -PLANE_TRI_EPSILON ) {
		return SIDE_BACK;
	}

	return SIDE_ON;
}

/*
==================
CM_GridPlane
==================
*/
static int	CM_GridPlane( int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], int i, int j, int tri ) {
	int		p;

	p = gridPlanes[i][j][tri];
	if ( p != -1 ) {
		return p;
	}
	p = gridPlanes[i][j][!tri];
	if ( p != -1 ) {
		return p;
	}

	// should never happen
	Com_Printf( "WARNING: CM_GridPlane unresolvable\n" );
	return -1;
}

/*
==================
CM_EdgePlaneNum
==================
*/
static int CM_EdgePlaneNum( cGrid_t *grid, int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], int i, int j, int k ) {
	bfixed	*p1, *p2;
	bvec3_t		up;
	int			p;

	switch ( k ) {
	case 0:	// top border
		p1 = grid->points[i][j];
		p2 = grid->points[i+1][j];
		p = CM_GridPlane( gridPlanes, i, j, 0 );
		FIXED_VEC3MA_R( p1, BFIXED(4,0), planes[ p ].pd.normal, up );
		return CM_FindPlane( p1, p2, up );

	case 2:	// bottom border
		p1 = grid->points[i][j+1];
		p2 = grid->points[i+1][j+1];
		p = CM_GridPlane( gridPlanes, i, j, 1 );
		FIXED_VEC3MA_R( p1, BFIXED(4,0), planes[ p ].pd.normal, up );
		return CM_FindPlane( p2, p1, up );

	case 3: // left border
		p1 = grid->points[i][j];
		p2 = grid->points[i][j+1];
		p = CM_GridPlane( gridPlanes, i, j, 1 );
		FIXED_VEC3MA_R( p1, BFIXED(4,0), planes[ p ].pd.normal, up );
		return CM_FindPlane( p2, p1, up );

	case 1:	// right border
		p1 = grid->points[i+1][j];
		p2 = grid->points[i+1][j+1];
		p = CM_GridPlane( gridPlanes, i, j, 0 );
		FIXED_VEC3MA_R( p1, BFIXED(4,0), planes[ p ].pd.normal, up );
		return CM_FindPlane( p1, p2, up );

	case 4:	// diagonal out of triangle 0
		p1 = grid->points[i+1][j+1];
		p2 = grid->points[i][j];
		p = CM_GridPlane( gridPlanes, i, j, 0 );
		FIXED_VEC3MA_R( p1, BFIXED(4,0), planes[ p ].pd.normal, up );
		return CM_FindPlane( p1, p2, up );

	case 5:	// diagonal out of triangle 1
		p1 = grid->points[i][j];
		p2 = grid->points[i+1][j+1];
		p = CM_GridPlane( gridPlanes, i, j, 1 );
		FIXED_VEC3MA_R( p1, BFIXED(4,0), planes[ p ].pd.normal, up );
		return CM_FindPlane( p1, p2, up );

	}

	Com_Error( ERR_DROP, "CM_EdgePlaneNum: bad k" );
	return -1;
}

/*
===================
CM_SetBorderInward
===================
*/
static void CM_SetBorderInward( facet_t *facet, cGrid_t *grid, int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2],
						  int i, int j, int which ) {
	int		k, l;
	bfixed	*points[4];
	int		numPoints;

	switch ( which ) {
	case -1:
		points[0] = grid->points[i][j];
		points[1] = grid->points[i+1][j];
		points[2] = grid->points[i+1][j+1];
		points[3] = grid->points[i][j+1];
		numPoints = 4;
		break;
	case 0:
		points[0] = grid->points[i][j];
		points[1] = grid->points[i+1][j];
		points[2] = grid->points[i+1][j+1];
		numPoints = 3;
		break;
	case 1:
		points[0] = grid->points[i+1][j+1];
		points[1] = grid->points[i][j+1];
		points[2] = grid->points[i][j];
		numPoints = 3;
		break;
	default:
		Com_Error( ERR_FATAL, "CM_SetBorderInward: bad parameter" );
		numPoints = 0;
		break;
	}

	for ( k = 0 ; k < facet->numBorders ; k++ ) {
		int		front, back;

		front = 0;
		back = 0;

		for ( l = 0 ; l < numPoints ; l++ ) {
			int		side;

			side = CM_PointOnPlaneSide( points[l], facet->borderPlanes[k] );
			if ( side == SIDE_FRONT ) {
				front++;
			} if ( side == SIDE_BACK ) {
				back++;
			}
		}

		if ( front && !back ) {
			facet->borderInward[k] = qtrue;
		} else if ( back && !front ) {
			facet->borderInward[k] = qfalse;
		} else if ( !front && !back ) {
			// flat side border
			facet->borderPlanes[k] = -1;
		} else {
			// bisecting side border
			Com_DPrintf( "WARNING: CM_SetBorderInward: mixed plane sides\n" );
			facet->borderInward[k] = qfalse;
			if ( !debugBlock ) {
				debugBlock = qtrue;
				VectorCopy( grid->points[i][j], debugBlockPoints[0] );
				VectorCopy( grid->points[i+1][j], debugBlockPoints[1] );
				VectorCopy( grid->points[i+1][j+1], debugBlockPoints[2] );
				VectorCopy( grid->points[i][j+1], debugBlockPoints[3] );
			}
		}
	}
}

/*
==================
CM_ValidateFacet

If the facet isn't bounded by its borders, we screwed up.
==================
*/
static qboolean CM_ValidateFacet( facet_t *facet ) {
	planeDef_t	plane;
	int			j;
	winding_t	*w;
	bvec3_t		bounds[2];

	if ( facet->surfacePlane == -1 ) {
		return qfalse;
	}

	plane=planes[ facet->surfacePlane ].pd;
	w = BaseWindingForPlane( plane.normal,  plane.dist );
	for ( j = 0 ; j < facet->numBorders && w ; j++ ) {
		if ( facet->borderPlanes[j] == -1 ) {
			return qfalse;
		}
		plane=planes[ facet->borderPlanes[j] ].pd;
		if ( !facet->borderInward[j] ) {
			VectorSubtract( avec3_origin, plane.normal, plane.normal );
			plane.dist = -plane.dist;
		}
		ChopWindingInPlace( &w, plane.normal, plane.dist, BFIXED(0,1) );
	}

	if ( !w ) {
		return qfalse;		// winding was completely chopped away
	}

	// see if the facet is unreasonably large
	WindingBounds( w, bounds[0], bounds[1] );
	FreeWinding( w );
	
	for ( j = 0 ; j < 3 ; j++ ) {
		if ( bounds[1][j] - bounds[0][j] > BFIXED(MAX_MAP_BOUNDS,0) ) {
			return qfalse;		// we must be missing a plane
		}
		if ( bounds[0][j] >= BFIXED(MAX_MAP_BOUNDS,0) ) {
			return qfalse;
		}
		if ( bounds[1][j] <= -BFIXED(MAX_MAP_BOUNDS,0) ) {
			return qfalse;
		}
	}
	return qtrue;		// winding is fine
}

/*
==================
CM_AddFacetBevels
==================
*/
void CM_AddFacetBevels( facet_t *facet ) {

	int i, j, k, l;
	int axis, dir, order, flipped;
	planeDef_t plane,newplane;
	bfixed d;
	winding_t *w, *w2;
	bvec3_t mins, maxs;
	avec3_t vec, vec2;

	plane=planes[ facet->surfacePlane ].pd;

	w = BaseWindingForPlane( plane.normal,  plane.dist );
	for ( j = 0 ; j < facet->numBorders && w ; j++ ) {
		if (facet->borderPlanes[j] == facet->surfacePlane) continue;
		plane=planes[ facet->borderPlanes[j] ].pd;

		if ( !facet->borderInward[j] ) {
			VectorSubtract( avec3_origin, plane.normal, plane.normal );
			plane.dist = -plane.dist;
		}

		ChopWindingInPlace( &w, plane.normal, plane.dist, BFIXED(0,1) );
	}
	if ( !w ) {
		return;
	}

	WindingBounds(w, mins, maxs);

	// add the axial planes
	order = 0;
	for ( axis = 0 ; axis < 3 ; axis++ )
	{
		for ( dir = -1 ; dir <= 1 ; dir += 2, order++ )
		{
			VectorClear(plane.normal);
			plane.normal[axis] = MAKE_AFIXED(dir);
			if (dir == 1) {
				plane.dist = maxs[axis];
			}
			else {
				plane.dist = -mins[axis];
			}
			//if it's the surface plane
			if (CM_PlaneEqual(&planes[facet->surfacePlane], plane, &flipped)) {
				continue;
			}
			// see if the plane is allready present
			for ( i = 0 ; i < facet->numBorders ; i++ ) {
				if (CM_PlaneEqual(&planes[facet->borderPlanes[i]], plane, &flipped))
					break;
			}

			if ( i == facet->numBorders ) {
				if (facet->numBorders > 4 + 6 + 16) Com_Printf("ERROR: too many bevels\n");
				facet->borderPlanes[facet->numBorders] = CM_FindPlane2(plane, &flipped);
				facet->borderNoAdjust[facet->numBorders] = 0;
				facet->borderInward[facet->numBorders] = flipped;
				facet->numBorders++;
			}
		}
	}
	//
	// add the edge bevels
	//
	// test the non-axial plane edges
	for ( j = 0 ; j < w->numpoints ; j++ )
	{
		k = (j+1)%w->numpoints;
		bvec3_t tmp;
		VectorSubtract (w->p[j], w->p[k], tmp);
		//if it's a degenerate edge
		if (VectorNormalizeB2A(tmp,vec) < BFIXED(0,5))
			continue;
		CM_SnapVector(vec);
		for ( k = 0; k < 3 ; k++ )
			if ( vec[k] == -AFIXED_1 || vec[k] == AFIXED_1 )
				break;	// axial
		if ( k < 3 )
			continue;	// only test non-axial edges

		// try the six possible slanted axials from this edge
		for ( axis = 0 ; axis < 3 ; axis++ )
		{
			for ( dir = -1 ; dir <= 1 ; dir += 2 )
			{
				// construct a plane
				VectorClear (vec2);
				vec2[axis] = MAKE_AFIXED(dir);
				CrossProduct (vec, vec2, plane.normal);
				if (VectorNormalize(plane.normal) < AFIXED(0,5))
					continue;
				plane.dist = FIXED_VEC3DOT (w->p[j], plane.normal);

				// if all the points of the facet winding are
				// behind this plane, it is a proper edge bevel
				for ( l = 0 ; l < w->numpoints ; l++ )
				{
					d = FIXED_VEC3DOT(w->p[l], plane.normal) - plane.dist;
					if (d > BFIXED(0,1))
						break;	// point in front
				}
				if ( l < w->numpoints )
					continue;

				//if it's the surface plane
				if (CM_PlaneEqual(&planes[facet->surfacePlane], plane, &flipped)) {
					continue;
				}
				// see if the plane is allready present
				for ( i = 0 ; i < facet->numBorders ; i++ ) {
					if (CM_PlaneEqual(&planes[facet->borderPlanes[i]], plane, &flipped)) {
							break;
					}
				}

				if ( i == facet->numBorders ) {
					if (facet->numBorders > 4 + 6 + 16) Com_Printf("ERROR: too many bevels\n");
					facet->borderPlanes[facet->numBorders] = CM_FindPlane2(plane, &flipped);

					for ( k = 0 ; k < facet->numBorders ; k++ ) {
						if (facet->borderPlanes[facet->numBorders] ==
							facet->borderPlanes[k]) Com_Printf("WARNING: bevel plane already used\n");
					}

					facet->borderNoAdjust[facet->numBorders] = 0;
					facet->borderInward[facet->numBorders] = flipped;
					//
					w2 = CopyWinding(w);
					newplane=planes[facet->borderPlanes[facet->numBorders]].pd;
					if (!facet->borderInward[facet->numBorders])
					{
						VectorNegate(newplane.normal, newplane.normal);
						newplane.dist = -newplane.dist;
					} //end if
					ChopWindingInPlace( &w2, newplane.normal, newplane.dist, BFIXED(0,1) );
					if (!w2) {
						Com_DPrintf("WARNING: CM_AddFacetBevels... invalid bevel\n");
						continue;
					}
					else {
						FreeWinding(w2);
					}
					//
					facet->numBorders++;
					//already got a bevel
//					break;
				}
			}
		}
	}
	FreeWinding( w );

#ifndef BSPC
	//add opposite plane
	facet->borderPlanes[facet->numBorders] = facet->surfacePlane;
	facet->borderNoAdjust[facet->numBorders] = 0;
	facet->borderInward[facet->numBorders] = qtrue;
	facet->numBorders++;
#endif //BSPC

}

typedef enum {
	EN_TOP,
	EN_RIGHT,
	EN_BOTTOM,
	EN_LEFT
} edgeName_t;

/*
==================
CM_PatchCollideFromGrid
==================
*/
static void CM_PatchCollideFromGrid( cGrid_t *grid, patchCollide_t *pf ) {
	int				i, j;
	bfixed			*p1, *p2, *p3;
	MAC_STATIC int				gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2];
	facet_t			*facet;
	int				borders[4];
	int				noAdjust[4];

	numPlanes = 0;
	numFacets = 0;

	// find the planes for each triangle of the grid
	for ( i = 0 ; i < grid->width - 1 ; i++ ) {
		for ( j = 0 ; j < grid->height - 1 ; j++ ) {
			p1 = grid->points[i][j];
			p2 = grid->points[i+1][j];
			p3 = grid->points[i+1][j+1];
			gridPlanes[i][j][0] = CM_FindPlane( p1, p2, p3 );

			p1 = grid->points[i+1][j+1];
			p2 = grid->points[i][j+1];
			p3 = grid->points[i][j];
			gridPlanes[i][j][1] = CM_FindPlane( p1, p2, p3 );
		}
	}

	// create the borders for each facet
	for ( i = 0 ; i < grid->width - 1 ; i++ ) {
		for ( j = 0 ; j < grid->height - 1 ; j++ ) {
			 
			borders[EN_TOP] = -1;
			if ( j > 0 ) {
				borders[EN_TOP] = gridPlanes[i][j-1][1];
			} else if ( grid->wrapHeight ) {
				borders[EN_TOP] = gridPlanes[i][grid->height-2][1];
			} 
			noAdjust[EN_TOP] = ( borders[EN_TOP] == gridPlanes[i][j][0] );
			if ( borders[EN_TOP] == -1 || noAdjust[EN_TOP] ) {
				borders[EN_TOP] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 0 );
			}

			borders[EN_BOTTOM] = -1;
			if ( j < grid->height - 2 ) {
				borders[EN_BOTTOM] = gridPlanes[i][j+1][0];
			} else if ( grid->wrapHeight ) {
				borders[EN_BOTTOM] = gridPlanes[i][0][0];
			}
			noAdjust[EN_BOTTOM] = ( borders[EN_BOTTOM] == gridPlanes[i][j][1] );
			if ( borders[EN_BOTTOM] == -1 || noAdjust[EN_BOTTOM] ) {
				borders[EN_BOTTOM] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 2 );
			}

			borders[EN_LEFT] = -1;
			if ( i > 0 ) {
				borders[EN_LEFT] = gridPlanes[i-1][j][0];
			} else if ( grid->wrapWidth ) {
				borders[EN_LEFT] = gridPlanes[grid->width-2][j][0];
			}
			noAdjust[EN_LEFT] = ( borders[EN_LEFT] == gridPlanes[i][j][1] );
			if ( borders[EN_LEFT] == -1 || noAdjust[EN_LEFT] ) {
				borders[EN_LEFT] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 3 );
			}

			borders[EN_RIGHT] = -1;
			if ( i < grid->width - 2 ) {
				borders[EN_RIGHT] = gridPlanes[i+1][j][1];
			} else if ( grid->wrapWidth ) {
				borders[EN_RIGHT] = gridPlanes[0][j][1];
			}
			noAdjust[EN_RIGHT] = ( borders[EN_RIGHT] == gridPlanes[i][j][0] );
			if ( borders[EN_RIGHT] == -1 || noAdjust[EN_RIGHT] ) {
				borders[EN_RIGHT] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 1 );
			}

			if ( numFacets == MAX_FACETS ) {
				Com_Error( ERR_DROP, "MAX_FACETS" );
			}
			facet = &facets[numFacets];
			Com_Memset( facet, 0, sizeof( *facet ) );

			if ( gridPlanes[i][j][0] == gridPlanes[i][j][1] ) {
				if ( gridPlanes[i][j][0] == -1 ) {
					continue;		// degenrate
				}
				facet->surfacePlane = gridPlanes[i][j][0];
				facet->numBorders = 4;
				facet->borderPlanes[0] = borders[EN_TOP];
				facet->borderNoAdjust[0] = noAdjust[EN_TOP];
				facet->borderPlanes[1] = borders[EN_RIGHT];
				facet->borderNoAdjust[1] = noAdjust[EN_RIGHT];
				facet->borderPlanes[2] = borders[EN_BOTTOM];
				facet->borderNoAdjust[2] = noAdjust[EN_BOTTOM];
				facet->borderPlanes[3] = borders[EN_LEFT];
				facet->borderNoAdjust[3] = noAdjust[EN_LEFT];
				CM_SetBorderInward( facet, grid, gridPlanes, i, j, -1 );
				if ( CM_ValidateFacet( facet ) ) {
					CM_AddFacetBevels( facet );
					numFacets++;
				}
			} else {
				// two seperate triangles
				facet->surfacePlane = gridPlanes[i][j][0];
				facet->numBorders = 3;
				facet->borderPlanes[0] = borders[EN_TOP];
				facet->borderNoAdjust[0] = noAdjust[EN_TOP];
				facet->borderPlanes[1] = borders[EN_RIGHT];
				facet->borderNoAdjust[1] = noAdjust[EN_RIGHT];
				facet->borderPlanes[2] = gridPlanes[i][j][1];
				if ( facet->borderPlanes[2] == -1 ) {
					facet->borderPlanes[2] = borders[EN_BOTTOM];
					if ( facet->borderPlanes[2] == -1 ) {
						facet->borderPlanes[2] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 4 );
					}
				}
 				CM_SetBorderInward( facet, grid, gridPlanes, i, j, 0 );
				if ( CM_ValidateFacet( facet ) ) {
					CM_AddFacetBevels( facet );
					numFacets++;
				}

				if ( numFacets == MAX_FACETS ) {
					Com_Error( ERR_DROP, "MAX_FACETS" );
				}
				facet = &facets[numFacets];
				Com_Memset( facet, 0, sizeof( *facet ) );

				facet->surfacePlane = gridPlanes[i][j][1];
				facet->numBorders = 3;
				facet->borderPlanes[0] = borders[EN_BOTTOM];
				facet->borderNoAdjust[0] = noAdjust[EN_BOTTOM];
				facet->borderPlanes[1] = borders[EN_LEFT];
				facet->borderNoAdjust[1] = noAdjust[EN_LEFT];
				facet->borderPlanes[2] = gridPlanes[i][j][0];
				if ( facet->borderPlanes[2] == -1 ) {
					facet->borderPlanes[2] = borders[EN_TOP];
					if ( facet->borderPlanes[2] == -1 ) {
						facet->borderPlanes[2] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 5 );
					}
				}
				CM_SetBorderInward( facet, grid, gridPlanes, i, j, 1 );
				if ( CM_ValidateFacet( facet ) ) {
					CM_AddFacetBevels( facet );
					numFacets++;
				}
			}
		}
	}

	// copy the results out
	pf->numPlanes = numPlanes;
	pf->numFacets = numFacets;
	pf->facets = (facet_t *)Hunk_Alloc( numFacets * sizeof( *pf->facets ), h_high );
	Com_Memcpy( pf->facets, facets, numFacets * sizeof( *pf->facets ) );
	pf->planes = (patchPlane_t *)Hunk_Alloc( numPlanes * sizeof( *pf->planes ), h_high );
	Com_Memcpy( pf->planes, planes, numPlanes * sizeof( *pf->planes ) );
}


/*
===================
CM_GeneratePatchCollide

Creates an internal structure that will be used to perform
collision detection with a patch mesh.

Points is packed as concatenated rows.
===================
*/
struct patchCollide_s	*CM_GeneratePatchCollide( int width, int height, bvec3_t *points ) {
	patchCollide_t	*pf;
	MAC_STATIC cGrid_t			grid;
	int				i, j;

	if ( width <= 2 || height <= 2 || !points ) {
		Com_Error( ERR_DROP, "CM_GeneratePatchFacets: bad parameters: (%i, %i, %p)",
			width, height, points );
	}

	if ( !(width & 1) || !(height & 1) ) {
		Com_Error( ERR_DROP, "CM_GeneratePatchFacets: even sizes are invalid for quadratic meshes" );
	}

	if ( width > MAX_GRID_SIZE || height > MAX_GRID_SIZE ) {
		Com_Error( ERR_DROP, "CM_GeneratePatchFacets: source is > MAX_GRID_SIZE" );
	}

	// build a grid
	grid.width = width;
	grid.height = height;
	grid.wrapWidth = qfalse;
	grid.wrapHeight = qfalse;
	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height ; j++ ) {
			VectorCopy( points[j*width + i], grid.points[i][j] );
		}
	}

	// subdivide the grid
	CM_SetGridWrapWidth( &grid );
	CM_SubdivideGridColumns( &grid );
	CM_RemoveDegenerateColumns( &grid );

	CM_TransposeGrid( &grid );

	CM_SetGridWrapWidth( &grid );
	CM_SubdivideGridColumns( &grid );
	CM_RemoveDegenerateColumns( &grid );

	// we now have a grid of points exactly on the curve
	// the aproximate surface defined by these points will be
	// collided against
	pf = (patchCollide_t *)Hunk_Alloc( sizeof( *pf ), h_high );
	ClearBounds( pf->bounds[0], pf->bounds[1] );
	for ( i = 0 ; i < grid.width ; i++ ) {
		for ( j = 0 ; j < grid.height ; j++ ) {
			AddPointToBounds( grid.points[i][j], pf->bounds[0], pf->bounds[1] );
		}
	}

	c_totalPatchBlocks += ( grid.width - 1 ) * ( grid.height - 1 );

	// generate a bsp tree for the surface
	CM_PatchCollideFromGrid( &grid, pf );

	// expand by one unit for epsilon purposes
	pf->bounds[0][0] -= BFIXED_1;
	pf->bounds[0][1] -= BFIXED_1;
	pf->bounds[0][2] -= BFIXED_1;

	pf->bounds[1][0] += BFIXED_1;
	pf->bounds[1][1] += BFIXED_1;
	pf->bounds[1][2] += BFIXED_1;

	return pf;
}

/*
================================================================================

TRACE TESTING

================================================================================
*/

/*
====================
CM_TracePointThroughPatchCollide

  special case for point traces because the patch collide "brushes" have no volume
====================
*/
void CM_TracePointThroughPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc ) {
	qboolean	frontFacing[MAX_PATCH_PLANES];
	bfixed		intersection[MAX_PATCH_PLANES];
	bfixed		intersect;
	const patchPlane_t	*planes;
	const facet_t	*facet;
	int			i, j, k;
	bfixed		offset;
	bfixed		d1, d2;
#ifndef BSPC
	static cvar_t *cv;
#endif //BSPC

#ifndef BSPC
	if ( !cm_playerCurveClip->integer || !tw->isPoint ) {
		return;
	}
#endif

	// determine the trace's relationship to all planes
	planes = pc->planes;
	for ( i = 0 ; i < pc->numPlanes ; i++, planes++ ) {
		offset = FIXED_VEC3DOT( tw->offsets[ planes->signbits ], planes->pd.normal );
		d1 = FIXED_VEC3DOT( tw->start, planes->pd.normal ) - planes->pd.dist + offset;
		d2 = FIXED_VEC3DOT( tw->end, planes->pd.normal ) - planes->pd.dist + offset;
		if ( d1 <= BFIXED_0 ) {
			frontFacing[i] = qfalse;
		} else {
			frontFacing[i] = qtrue;
		}
		if ( d1 == d2 ) {
			intersection[i] = BFIXED(99999,0);
		} else {
			intersection[i] = d1 / ( d1 - d2 );
			if ( intersection[i] <= BFIXED_0 ) {
				intersection[i] = BFIXED(99999,0);
			}
		}
	}


	// see if any of the surface planes are intersected
	facet = pc->facets;
	for ( i = 0 ; i < pc->numFacets ; i++, facet++ ) {
		if ( !frontFacing[facet->surfacePlane] ) {
			continue;
		}
		intersect = intersection[facet->surfacePlane];
		if ( intersect < BFIXED_0 ) {
			continue;		// surface is behind the starting point
		}
		if ( intersect > MAKE_BFIXED(tw->trace.fraction) ) {
			continue;		// already hit something closer
		}
		for ( j = 0 ; j < facet->numBorders ; j++ ) {
			k = facet->borderPlanes[j];
			if ( frontFacing[k] ^ facet->borderInward[j] ) {
				if ( intersection[k] > intersect ) {
					break;
				}
			} else {
				if ( intersection[k] < intersect ) {
					break;
				}
			}
		}
		if ( j == facet->numBorders ) {
			// we hit this facet
#ifndef BSPC
			if (!cv) {
				cv = Cvar_Get( "r_debugSurfaceUpdate", "1", 0 );
			}
			if (cv->integer) {
				debugPatchCollide = pc;
				debugFacet = facet;
			}
#endif //BSPC
			planes = &pc->planes[facet->surfacePlane];

			// calculate intersection with a slight pushoff
			offset = FIXED_VEC3DOT( tw->offsets[ planes->signbits ], planes->pd.normal );
			d1 = FIXED_VEC3DOT( tw->start, planes->pd.normal ) - planes->pd.dist + offset;
			d2 = FIXED_VEC3DOT( tw->end, planes->pd.normal ) - planes->pd.dist + offset;
			tw->trace.fraction = FIXED_RATIO_G( d1 - SURFACE_CLIP_EPSILON, d1 - d2 );

			if ( tw->trace.fraction < GFIXED_0 ) {
				tw->trace.fraction = GFIXED_0;
			}

			VectorCopy(planes->pd.normal, tw->trace.plane.normal);
			tw->trace.plane.dist = planes->pd.dist;
		}
	}
}

/*
====================
CM_CheckFacetPlane
====================
*/
int CM_CheckFacetPlane(planeDef_t &plane, bvec3_t start, bvec3_t end, gfixed *enterFrac, gfixed *leaveFrac, int *hit) {
	bfixed d1, d2;
	gfixed f;

	*hit = qfalse;

	d1 = FIXED_VEC3DOT( start, plane.normal ) - plane.dist;
	d2 = FIXED_VEC3DOT( end, plane.normal ) - plane.dist;

	// if completely in front of face, no intersection with the entire facet
	if (d1 > BFIXED_0 && ( d2 >= SURFACE_CLIP_EPSILON || d2 >= d1 )  ) {
		return qfalse;
	}

	// if it doesn't cross the plane, the plane isn't relevent
	if (d1 <= BFIXED_0 && d2 <= BFIXED_0 ) {
		return qtrue;
	}

	// crosses face
	if (d1 > d2) {	// enter
		f = FIXED_RATIO_G(d1-SURFACE_CLIP_EPSILON,d1-d2);
		if ( f < GFIXED_0 ) {
			f = GFIXED_0;
		}
		//always favor previous plane hits and thus also the surface plane hit
		if (f > *enterFrac) {
			*enterFrac = f;
			*hit = qtrue;
		}
	} else {	// leave
		f = FIXED_RATIO_G(d1+SURFACE_CLIP_EPSILON,d1-d2);
		if ( f > GFIXED_1 ) {
			f = GFIXED_1;
		}
		if (f < *leaveFrac) {
			*leaveFrac = f;
		}
	}
	return qtrue;
}

/*
====================
CM_TraceThroughPatchCollide
====================
*/
void CM_TraceThroughPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc ) {
	int i, j, hit, hitnum;
	bfixed offset, t;
	gfixed enterFrac, leaveFrac;
	patchPlane_t *planes;
	facet_t	*facet;
	planeDef_t plane, bestplane;
	bvec3_t startp, endp;
#ifndef BSPC
	static cvar_t *cv;
#endif //BSPC

	if (tw->isPoint) {
		CM_TracePointThroughPatchCollide( tw, pc );
		return;
	}

	facet = pc->facets;
	for ( i = 0 ; i < pc->numFacets ; i++, facet++ ) {
		enterFrac = -GFIXED_1;
		leaveFrac = GFIXED_1;
		hitnum = -1;
		//
		planes = &pc->planes[ facet->surfacePlane ];
		VectorCopy(planes->pd.normal, plane.normal);
		plane.dist = planes->pd.dist;
		if ( tw->sphere.use ) {
			// adjust the plane distance apropriately for radius
			plane.dist += tw->sphere.radius;

			// find the closest point on the capsule to the plane
			t = FIXED_VEC3DOT_R( plane.normal, tw->sphere.offset );
			if ( t > BFIXED_0 ) {
				VectorSubtract( tw->start, tw->sphere.offset, startp );
				VectorSubtract( tw->end, tw->sphere.offset, endp );
			}
			else {
				VectorAdd( tw->start, tw->sphere.offset, startp );
				VectorAdd( tw->end, tw->sphere.offset, endp );
			}
		}
		else {
			offset = FIXED_VEC3DOT( tw->offsets[ planes->signbits ], plane.normal);
			plane.dist -= offset;
			VectorCopy( tw->start, startp );
			VectorCopy( tw->end, endp );
		}

		if (!CM_CheckFacetPlane(plane, startp, endp, &enterFrac, &leaveFrac, &hit)) {
			continue;
		}
		if (hit) {
			VectorCopy(plane.normal, bestplane.normal);
			bestplane.dist=plane.dist;
		}

		for ( j = 0; j < facet->numBorders; j++ ) {
			planes = &pc->planes[ facet->borderPlanes[j] ];
			if (facet->borderInward[j]) {
				VectorNegate(planes->pd.normal, plane.normal);
				plane.dist = -planes->pd.dist;
			}
			else {
				VectorCopy(planes->pd.normal, plane.normal);
				plane.dist = planes->pd.dist;
			}
			if ( tw->sphere.use ) {
				// adjust the plane distance apropriately for radius
				plane.dist += tw->sphere.radius;

				// find the closest point on the capsule to the plane
				t = FIXED_VEC3DOT_R( plane.normal, tw->sphere.offset );
				if ( t > BFIXED_0 ) {
					VectorSubtract( tw->start, tw->sphere.offset, startp );
					VectorSubtract( tw->end, tw->sphere.offset, endp );
				}
				else {
					VectorAdd( tw->start, tw->sphere.offset, startp );
					VectorAdd( tw->end, tw->sphere.offset, endp );
				}
			}
			else {
				// NOTE: this works even though the plane might be flipped because the bbox is centered
				offset = FIXED_VEC3DOT( tw->offsets[ planes->signbits ], plane.normal);
				plane.dist += FIXED_ABS(offset);
				VectorCopy( tw->start, startp );
				VectorCopy( tw->end, endp );
			}

			if (!CM_CheckFacetPlane(plane, startp, endp, &enterFrac, &leaveFrac, &hit)) {
				break;
			}
			if (hit) {
				hitnum = j;
				VectorCopy(plane.normal, bestplane.normal);
				bestplane.dist=plane.dist;
			}
		}
		if (j < facet->numBorders) continue;
		//never clip against the back side
		if (hitnum == facet->numBorders - 1) continue;

		if (enterFrac < leaveFrac && enterFrac >= GFIXED_0) {
			if (enterFrac < tw->trace.fraction) {
				if (enterFrac < GFIXED_0) {
					enterFrac = GFIXED_0;
				}
#ifndef BSPC
				if (!cv) {
					cv = Cvar_Get( "r_debugSurfaceUpdate", "1", 0 );
				}
				if (cv && cv->integer) {
					debugPatchCollide = pc;
					debugFacet = facet;
				}
#endif //BSPC

				tw->trace.fraction = enterFrac;
				VectorCopy( bestplane.normal, tw->trace.plane.normal );
				tw->trace.plane.dist = bestplane.dist;
			}
		}
	}
}


/*
=======================================================================

POSITION TEST

=======================================================================
*/

/*
====================
CM_PositionTestInPatchCollide
====================
*/
qboolean CM_PositionTestInPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc ) {
	int i, j;
	bfixed offset, t;
	patchPlane_t *planes;
	facet_t	*facet;
	planeDef_t plane;
	bvec3_t startp;

	if (tw->isPoint) {
		return qfalse;
	}
	//
	facet = pc->facets;
	for ( i = 0 ; i < pc->numFacets ; i++, facet++ ) {
		planes = &pc->planes[ facet->surfacePlane ];
		VectorCopy(planes->pd.normal, plane.normal);
		plane.dist = planes->pd.dist;
		if ( tw->sphere.use ) {
			// adjust the plane distance apropriately for radius
			plane.dist += tw->sphere.radius;

			// find the closest point on the capsule to the plane
			t = FIXED_VEC3DOT_R( plane.normal, tw->sphere.offset );
			if ( t > BFIXED_0 ) {
				VectorSubtract( tw->start, tw->sphere.offset, startp );
			}
			else {
				VectorAdd( tw->start, tw->sphere.offset, startp );
			}
		}
		else {
			offset = FIXED_VEC3DOT( tw->offsets[ planes->signbits ], plane.normal);
			plane.dist -= offset;
			VectorCopy( tw->start, startp );
		}

		if ( FIXED_VEC3DOT_R( plane.normal, startp ) - plane.dist > BFIXED_0 ) {
			continue;
		}

		for ( j = 0; j < facet->numBorders; j++ ) {
			planes = &pc->planes[ facet->borderPlanes[j] ];
			if (facet->borderInward[j]) {
				VectorNegate(planes->pd.normal, plane.normal);
				plane.dist = -planes->pd.dist;
			}
			else {
				VectorCopy(planes->pd.normal, plane.normal);
				plane.dist = planes->pd.dist;
			}
			if ( tw->sphere.use ) {
				// adjust the plane distance apropriately for radius
				plane.dist += tw->sphere.radius;

				// find the closest point on the capsule to the plane
				t = FIXED_VEC3DOT_R( plane.normal, tw->sphere.offset );
				if ( t > BFIXED_0 ) {
					VectorSubtract( tw->start, tw->sphere.offset, startp );
				}
				else {
					VectorAdd( tw->start, tw->sphere.offset, startp );
				}
			}
			else {
				// NOTE: this works even though the plane might be flipped because the bbox is centered
				offset = FIXED_VEC3DOT( tw->offsets[ planes->signbits ], plane.normal);
				plane.dist += FIXED_ABS(offset);
				VectorCopy( tw->start, startp );
			}

			if ( FIXED_VEC3DOT_R( plane.normal, startp ) - plane.dist > BFIXED_0 ) {
				break;
			}
		}
		if (j < facet->numBorders) {
			continue;
		}
		// inside this patch facet
		return qtrue;
	}
	return qfalse;
}

/*
=======================================================================

DEBUGGING

=======================================================================
*/


/*
==================
CM_DrawDebugSurface

Called from the renderer
==================
*/
#ifndef BSPC
void BotDrawDebugPolygons(void (*drawPoly)(int color, int numPoints, bfixed *points), int value);
#endif

void CM_DrawDebugSurface( void (*drawPoly)(int color, int numPoints, bfixed *points) ) {
	static cvar_t	*cv;
#ifndef BSPC
	static cvar_t	*cv2;
#endif
	const patchCollide_t	*pc;
	facet_t			*facet;
	winding_t		*w;
	int				i, j, k, n;
	int				curplanenum, planenum, curinward, inward;
	planeDef_t		plane;
	bvec3_t mins = {-BFIXED(15,0), -BFIXED(15,0), -BFIXED(28,0)}, maxs = {BFIXED(15,0), BFIXED(15,0), BFIXED(28,0)};
	//bvec3_t mins = {BFIXED_0, BFIXED_0, BFIXED_0}, maxs = {BFIXED_0, BFIXED_0, BFIXED_0};
	bvec3_t v1;
	avec3_t v2;

#ifndef BSPC
	if ( !cv2 )
	{
		cv2 = Cvar_Get( "r_debugSurface", "0", 0 );
	}

	if (cv2->integer != 1)
	{
		BotDrawDebugPolygons(drawPoly, cv2->integer);
		return;
	}
#endif

	if ( !debugPatchCollide ) {
		return;
	}

#ifndef BSPC
	if ( !cv ) {
		cv = Cvar_Get( "cm_debugSize", "2", 0 );
	}
#endif
	pc = debugPatchCollide;

	for ( i = 0, facet = pc->facets ; i < pc->numFacets ; i++, facet++ ) {

		for ( k = 0 ; k < facet->numBorders + 1; k++ ) {
			//
			if (k < facet->numBorders) {
				planenum = facet->borderPlanes[k];
				inward = facet->borderInward[k];
			}
			else {
				planenum = facet->surfacePlane;
				inward = qfalse;
				//continue;
			}

			VectorCopy( pc->planes[ planenum ].pd.normal, plane.normal );
			plane.dist=pc->planes[ planenum ].pd.dist;

			//planenum = facet->surfacePlane;
			if ( inward ) {
				VectorSubtract( avec3_origin, plane.normal, plane.normal );
				plane.dist = -plane.dist;
			}

			plane.dist += MAKE_BFIXED(cv->value);
			//*
			for (n = 0; n < 3; n++)
			{
				if (plane.normal[n] > AFIXED_0) v1[n] = maxs[n];
				else v1[n] = mins[n];
			} //end for
			VectorNegate(plane.normal, v2);
			plane.dist += FIXED_ABS(FIXED_VEC3DOT(v1, v2));
			//*/

			w = BaseWindingForPlane( plane.normal,  plane.dist );
			for ( j = 0 ; j < facet->numBorders + 1 && w; j++ ) {
				//
				if (j < facet->numBorders) {
					curplanenum = facet->borderPlanes[j];
					curinward = facet->borderInward[j];
				}
				else {
					curplanenum = facet->surfacePlane;
					curinward = qfalse;
					//continue;
				}
				//
				if (curplanenum == planenum) continue;

				VectorCopy( pc->planes[ curplanenum ].pd.normal, plane.normal );
				plane.dist=pc->planes[ curplanenum ].pd.dist;
				if ( !curinward ) {
					VectorSubtract( avec3_origin, plane.normal, plane.normal );
					plane.dist = -plane.dist;
				}
		//			if ( !facet->borderNoAdjust[j] ) {
					plane.dist -= MAKE_BFIXED(cv->value);
		//			}
				for (n = 0; n < 3; n++)
				{
					if (plane.normal[n] > AFIXED_0) v1[n] = maxs[n];
					else v1[n] = mins[n];
				} //end for
				VectorNegate(plane.normal, v2);
				plane.dist -= FIXED_ABS(FIXED_VEC3DOT(v1, v2));

				ChopWindingInPlace( &w, plane.normal, plane.dist, BFIXED(0,1) );
			}
			if ( w ) {
					
				if ( facet == debugFacet ) {
					drawPoly( 4, w->numpoints, w->p[0] );
					//Com_Printf("blue facet has %d border planes\n", facet->numBorders);
				} else {
					
					drawPoly( 1, w->numpoints, w->p[0] );
				}
				FreeWinding( w );
			}
			else
				Com_Printf("winding chopped away by border planes\n");
		}
	}

	// draw the debug block
	{
		bvec3_t v[3];

		VectorCopy( debugBlockPoints[0], v[0] );
		VectorCopy( debugBlockPoints[1], v[1] );
		VectorCopy( debugBlockPoints[2], v[2] );
		drawPoly( 2, 3, v[0] );

		VectorCopy( debugBlockPoints[2], v[0] );
		VectorCopy( debugBlockPoints[3], v[1] );
		VectorCopy( debugBlockPoints[0], v[2] );
		drawPoly( 2, 3, v[0] );
	}

#if 0
	bvec3_t			v[4];

	v[0][0] = pc->bounds[1][0];
	v[0][1] = pc->bounds[1][1];
	v[0][2] = pc->bounds[1][2];

	v[1][0] = pc->bounds[1][0];
	v[1][1] = pc->bounds[0][1];
	v[1][2] = pc->bounds[1][2];

	v[2][0] = pc->bounds[0][0];
	v[2][1] = pc->bounds[0][1];
	v[2][2] = pc->bounds[1][2];

	v[3][0] = pc->bounds[0][0];
	v[3][1] = pc->bounds[1][1];
	v[3][2] = pc->bounds[1][2];

	drawPoly( 4, v[0] );
#endif
}
