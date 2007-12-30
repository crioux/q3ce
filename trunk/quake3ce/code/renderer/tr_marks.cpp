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
// tr_marks.c -- polygon projection on the world polygons

#include"renderer_pch.h"

#define MAX_VERTS_ON_POLY		64

#define MARKER_OFFSET			0	// 1

/*
=============
R_ChopPolyBehindPlane

Out must have space for two more vertexes than in
=============
*/
#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2
static void R_ChopPolyBehindPlane( int numInPoints, bvec3_t inPoints[MAX_VERTS_ON_POLY],
								   int *numOutPoints, bvec3_t outPoints[MAX_VERTS_ON_POLY], 
								   avec3_t normal, bvec_t dist, bvec_t epsilon) {
	bfixed		dists[MAX_VERTS_ON_POLY+4];
	int			sides[MAX_VERTS_ON_POLY+4];
	int			counts[3];
	bfixed		dot;
	int			i, j;
	bfixed		*p1, *p2, *clip;
	bfixed		d;

	// don't clip if it might overflow
	if ( numInPoints >= MAX_VERTS_ON_POLY - 2 ) {
		*numOutPoints = 0;
		return;
	}

	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	for ( i = 0 ; i < numInPoints ; i++ ) {
		dot = FIXED_VEC3DOT( inPoints[i], normal );
		dot -= dist;
		dists[i] = dot;
		if ( dot > epsilon ) {
			sides[i] = SIDE_FRONT;
		} else if ( dot < -epsilon ) {
			sides[i] = SIDE_BACK;
		} else {
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	*numOutPoints = 0;

	if ( !counts[0] ) {
		return;
	}
	if ( !counts[1] ) {
		*numOutPoints = numInPoints;
		Com_Memcpy( outPoints, inPoints, numInPoints * sizeof(bvec3_t) );
		return;
	}

	for ( i = 0 ; i < numInPoints ; i++ ) {
		p1 = inPoints[i];
		clip = outPoints[ *numOutPoints ];
		
		if ( sides[i] == SIDE_ON ) {
			VectorCopy( p1, clip );
			(*numOutPoints)++;
			continue;
		}
	
		if ( sides[i] == SIDE_FRONT ) {
			VectorCopy( p1, clip );
			(*numOutPoints)++;
			clip = outPoints[ *numOutPoints ];
		}

		if ( sides[i+1] == SIDE_ON || sides[i+1] == sides[i] ) {
			continue;
		}
			
		// generate a split point
		p2 = inPoints[ (i+1) % numInPoints ];

		d = dists[i] - dists[i+1];
		if ( d == BFIXED_0 ) {
			dot = BFIXED_0;
		} else {
			dot = dists[i] / d;
		}

		// clip xyz

		for (j=0 ; j<3 ; j++) {
			clip[j] = p1[j] + dot * ( p2[j] - p1[j] );
		}

		(*numOutPoints)++;
	}
}

/*
=================
R_BoxSurfaces_r

=================
*/
void R_BoxSurfaces_r(mnode_t *node, bvec3_t mins, bvec3_t maxs, surfaceType_t **list, int listsize, int *listlength, avec3_t dir) {

	int			s, c;
	msurface_t	*surf, **mark;

	// do the tail recursion in a loop
	while ( node->contents == -1 ) {
		s = BoxOnPlaneSide( mins, maxs, node->plane );
		if (s == 1) {
			node = node->children[0];
		} else if (s == 2) {
			node = node->children[1];
		} else {
			R_BoxSurfaces_r(node->children[0], mins, maxs, list, listsize, listlength, dir);
			node = node->children[1];
		}
	}

	// add the individual surfaces
	mark = node->firstmarksurface;
	c = node->nummarksurfaces;
	while (c--) {
		//
		if (*listlength >= listsize) break;
		//
		surf = *mark;
		// check if the surface has NOIMPACT or NOMARKS set
		if ( ( surf->shader->surfaceFlags & ( SURF_NOIMPACT | SURF_NOMARKS ) )
			|| ( surf->shader->contentFlags & CONTENTS_FOG ) ) {
			surf->viewCount = tr.viewCount;
		}
		// extra check for surfaces to avoid list overflows
		else if (*(surf->data) == SF_FACE) {
			// the face plane should go through the box
			s = BoxOnPlaneSide( mins, maxs, &(( srfSurfaceFace_t * ) surf->data)->plane );
			if (s == 1 || s == 2) {
				surf->viewCount = tr.viewCount;
			} else if (FIXED_VEC3DOT((( srfSurfaceFace_t * ) surf->data)->plane.normal, dir) > -AFIXED(0,5)) {
			// don't add faces that make sharp angles with the projection direction
				surf->viewCount = tr.viewCount;
			}
		}
		else if (*(surfaceType_t *) (surf->data) != SF_GRID) surf->viewCount = tr.viewCount;
		// check the viewCount because the surface may have
		// already been added if it spans multiple leafs
		if (surf->viewCount != tr.viewCount) {
			surf->viewCount = tr.viewCount;
			list[*listlength] = (surfaceType_t *) surf->data;
			(*listlength)++;
		}
		mark++;
	}
}

/*
=================
R_AddMarkFragments

=================
*/
void R_AddMarkFragments(int numClipPoints, bvec3_t clipPoints[2][MAX_VERTS_ON_POLY],
				   int numPlanes, avec3_t *normals, bfixed *dists,
				   int maxPoints, bvec3_t pointBuffer,
				   int maxFragments, markFragment_t *fragmentBuffer,
				   int *returnedPoints, int *returnedFragments,
				   bvec3_t mins, bvec3_t maxs) {
	int pingPong, i;
	markFragment_t	*mf;

	// chop the surface by all the bounding planes of the to be projected polygon
	pingPong = 0;

	for ( i = 0 ; i < numPlanes ; i++ ) {

		R_ChopPolyBehindPlane( numClipPoints, clipPoints[pingPong],
						   &numClipPoints, clipPoints[!pingPong],
							normals[i], dists[i], BFIXED(0,5) );
		pingPong ^= 1;
		if ( numClipPoints == 0 ) {
			break;
		}
	}
	// completely clipped away?
	if ( numClipPoints == 0 ) {
		return;
	}

	// add this fragment to the returned list
	if ( numClipPoints + (*returnedPoints) > maxPoints ) {
		return;	// not enough space for this polygon
	}
	/*
	// all the clip points should be within the bounding box
	for ( i = 0 ; i < numClipPoints ; i++ ) {
		int j;
		for ( j = 0 ; j < 3 ; j++ ) {
			if (clipPoints[pingPong][i][j] < mins[j] - GFIXED(0,5)) break;
			if (clipPoints[pingPong][i][j] > maxs[j] + GFIXED(0,5)) break;
		}
		if (j < 3) break;
	}
	if (i < numClipPoints) return;
	*/

	mf = fragmentBuffer + (*returnedFragments);
	mf->firstPoint = (*returnedPoints);
	mf->numPoints = numClipPoints;
	Com_Memcpy( pointBuffer + (*returnedPoints) * 3, clipPoints[pingPong], numClipPoints * sizeof(bvec3_t) );

	(*returnedPoints) += numClipPoints;
	(*returnedFragments)++;
}

/*
=================
R_MarkFragments

=================
*/
int R_MarkFragments( int numPoints, const bvec3_t *points, const bvec3_t projection,
				   int maxPoints, bvec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer ) {
	int				numsurfaces, numPlanes;
	int				i, j, k, m, n;
	surfaceType_t	*surfaces[64];
	bvec3_t			mins, maxs;
	int				returnedFragments;
	int				returnedPoints;
	avec3_t			normals[MAX_VERTS_ON_POLY+2];
	bfixed			dists[MAX_VERTS_ON_POLY+2];
	bvec3_t			clipPoints[2][MAX_VERTS_ON_POLY];
	int				numClipPoints;
	fixed32			*v;
	srfSurfaceFace_t *surf;
	srfGridMesh_t	*cv;
	drawVert_t		*dv;
	bvec3_t			normal;
	avec3_t			projectionDir;
	bvec3_t			v1, v2;
	int				*indexes;

	//increment view count for lfixed check prevention
	tr.viewCount++;

	//
	VectorNormalizeB2A( projection, projectionDir );
	// find all the brushes that are to be considered
	ClearBounds( mins, maxs );
	for ( i = 0 ; i < numPoints ; i++ ) {
		bvec3_t	temp;

		AddPointToBounds( points[i], mins, maxs );
		VectorAdd( points[i], projection, temp );
		AddPointToBounds( temp, mins, maxs );
		// make sure we get all the leafs (also the one(s) in front of the hit surface)
		FIXED_VEC3MA_R( points[i], -BFIXED(20,0), projectionDir, temp );
		AddPointToBounds( temp, mins, maxs );
	}

	if (numPoints > MAX_VERTS_ON_POLY) numPoints = MAX_VERTS_ON_POLY;
	// create the bounding planes for the to be projected polygon
	for ( i = 0 ; i < numPoints ; i++ ) {
		VectorSubtract(points[(i+1)%numPoints], points[i], v1);
		VectorAdd(points[i], projection, v2);
		VectorSubtract(points[i], v2, v2);
		bvec3_t tmp;
		CrossProduct(v1, v2, tmp);
		FIXED_FASTVEC3NORM(tmp);
		normals[i][0]=MAKE_AFIXED(tmp[0]);
		normals[i][1]=MAKE_AFIXED(tmp[1]);
		normals[i][2]=MAKE_AFIXED(tmp[2]);
		dists[i] = FIXED_VEC3DOT_R(normals[i], points[i]);
	}
	// add near and far clipping planes for projection
	VectorCopy(projectionDir, normals[numPoints]);
	dists[numPoints] = FIXED_VEC3DOT_R(normals[numPoints], points[0]) - BFIXED(32,0);
	VectorCopy(projectionDir, normals[numPoints+1]);
	VectorInverse(normals[numPoints+1]);
	dists[numPoints+1] = FIXED_VEC3DOT_R(normals[numPoints+1], points[0]) - BFIXED(20,0);
	numPlanes = numPoints + 2;

	numsurfaces = 0;
	R_BoxSurfaces_r(tr.world->nodes, mins, maxs, surfaces, 64, &numsurfaces, projectionDir);
	//assert(numsurfaces <= 64);
	//assert(numsurfaces != 64);

	returnedPoints = 0;
	returnedFragments = 0;

	for ( i = 0 ; i < numsurfaces ; i++ ) {

		if (*surfaces[i] == SF_GRID) {

			cv = (srfGridMesh_t *) surfaces[i];
			for ( m = 0 ; m < cv->height - 1 ; m++ ) {
				for ( n = 0 ; n < cv->width - 1 ; n++ ) {
					// We triangulate the grid and chop all triangles within
					// the bounding planes of the to be projected polygon.
					// LOD is not taken into account, not such a big deal though.
					//
					// It's probably much nicer to chop the grid itself and deal
					// with this grid as a normal SF_GRID surface so LOD will
					// be applied. However the LOD of that chopped grid must
					// be synced with the LOD of the original curve.
					// One way to do this; the chopped grid shares vertices with
					// the original curve. When LOD is applied to the original
					// curve the unused vertices are flagged. Now the chopped curve
					// should skip the flagged vertices. This still leaves the
					// problems with the vertices at the chopped grid edges.
					//
					// To avoid issues when LOD applied to "hollow curves" (like
					// the ones around many jump pads) we now just add a 2 unit
					// offset to the triangle vertices.
					// The offset is added in the vertex normal vector direction
					// so all triangles will still fit together.
					// The 2 unit offset should avoid pretty much all LOD problems.

					numClipPoints = 3;

					dv = cv->verts + m * cv->width + n;

					VectorCopy(dv[0].xyz, clipPoints[0][0]);
					FIXED_VEC3MA_R(clipPoints[0][0], BFIXED(MARKER_OFFSET,0), dv[0].normal, clipPoints[0][0]);
					VectorCopy(dv[cv->width].xyz, clipPoints[0][1]);
					FIXED_VEC3MA_R(clipPoints[0][1], BFIXED(MARKER_OFFSET,0), dv[cv->width].normal, clipPoints[0][1]);
					VectorCopy(dv[1].xyz, clipPoints[0][2]);
					FIXED_VEC3MA_R(clipPoints[0][2], BFIXED(MARKER_OFFSET,0), dv[1].normal, clipPoints[0][2]);
					// check the normal of this triangle
					VectorSubtract(clipPoints[0][0], clipPoints[0][1], v1);
					VectorSubtract(clipPoints[0][2], clipPoints[0][1], v2);
					CrossProduct(v1, v2, normal);
					FIXED_FASTVEC3NORM(normal);
					if (FIXED_VEC3DOT(normal, projectionDir) < -BFIXED(0,1)) {
						// add the fragments of this triangle
						R_AddMarkFragments(numClipPoints, clipPoints,
										   numPlanes, normals, dists,
										   maxPoints, pointBuffer,
										   maxFragments, fragmentBuffer,
										   &returnedPoints, &returnedFragments, mins, maxs);

						if ( returnedFragments == maxFragments ) {
							return returnedFragments;	// not enough space for more fragments
						}
					}

					VectorCopy(dv[1].xyz, clipPoints[0][0]);
					FIXED_VEC3MA_R(clipPoints[0][0], BFIXED(MARKER_OFFSET,0), dv[1].normal, clipPoints[0][0]);
					VectorCopy(dv[cv->width].xyz, clipPoints[0][1]);
					FIXED_VEC3MA_R(clipPoints[0][1], BFIXED(MARKER_OFFSET,0), dv[cv->width].normal, clipPoints[0][1]);
					VectorCopy(dv[cv->width+1].xyz, clipPoints[0][2]);
					FIXED_VEC3MA_R(clipPoints[0][2], BFIXED(MARKER_OFFSET,0), dv[cv->width+1].normal, clipPoints[0][2]);
					// check the normal of this triangle
					VectorSubtract(clipPoints[0][0], clipPoints[0][1], v1);
					VectorSubtract(clipPoints[0][2], clipPoints[0][1], v2);
					CrossProduct(v1, v2, normal);
					FIXED_FASTVEC3NORM(normal);
					if (FIXED_VEC3DOT(normal, projectionDir) < -BFIXED(0,05)) {
						// add the fragments of this triangle
						R_AddMarkFragments(numClipPoints, clipPoints,
										   numPlanes, normals, dists,
										   maxPoints, pointBuffer,
										   maxFragments, fragmentBuffer,
										   &returnedPoints, &returnedFragments, mins, maxs);

						if ( returnedFragments == maxFragments ) {
							return returnedFragments;	// not enough space for more fragments
						}
					}
				}
			}
		}
		else if (*surfaces[i] == SF_FACE) {

			surf = ( srfSurfaceFace_t * ) surfaces[i];
			// check the normal of this face
			if (FIXED_VEC3DOT(surf->plane.normal, projectionDir) > -AFIXED(0,5)) {
				continue;
			}

			/*
			VectorSubtract(clipPoints[0][0], clipPoints[0][1], v1);
			VectorSubtract(clipPoints[0][2], clipPoints[0][1], v2);
			CrossProduct(v1, v2, normal);
			VectorNormalize(normal);
			if (FIXED_VEC3DOT(normal, projectionDir) > -BFIXED(0,5)) continue;
			*/
			indexes = (int *)( (byte *)surf + surf->ofsIndices );
			for ( k = 0 ; k < surf->numIndices ; k += 3 ) {
				for ( j = 0 ; j < 3 ; j++ ) {
					v = surf->points[0] + VERTEXSIZE * indexes[k+j];;
					FIXED_VEC3MA_R( (const bfixed *)v, BFIXED(MARKER_OFFSET,0), surf->plane.normal, clipPoints[0][j] );
				}
				// add the fragments of this face
				R_AddMarkFragments( 3 , clipPoints,
								   numPlanes, normals, dists,
								   maxPoints, pointBuffer,
								   maxFragments, fragmentBuffer,
								   &returnedPoints, &returnedFragments, mins, maxs);
				if ( returnedFragments == maxFragments ) {
					return returnedFragments;	// not enough space for more fragments
				}
			}
			continue;
		}
		else {
			// ignore all other world surfaces
			// might be cool to also project polygons on a triangle soup
			// however this will probably create huge amounts of extra polys
			// even more than the projection onto curves
			continue;
		}
	}
	return returnedFragments;
}

