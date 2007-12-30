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

#include"renderer_pch.h"

/*

This file does all of the processing necessary to turn a raw grid of points
read from the map file into a srfGridMesh_t ready for rendering.

The level of detail solution is direction independent, based only on subdivided
distance from the true curve.

Only a single entry point:

srfGridMesh_t *R_SubdividePatchToGrid( int width, int height,
								drawVert_t points[MAX_PATCH_SIZE*MAX_PATCH_SIZE] ) {

*/


/*
============
LerpDrawVert
============
*/
static void LerpDrawVert( drawVert_t *a, drawVert_t *b, drawVert_t *out ) {
	out->xyz[0] = GFIXED(0,5) * (a->xyz[0] + b->xyz[0]);
	out->xyz[1] = GFIXED(0,5) * (a->xyz[1] + b->xyz[1]);
	out->xyz[2] = GFIXED(0,5) * (a->xyz[2] + b->xyz[2]);

	out->st[0] = GFIXED(0,5) * (a->st[0] + b->st[0]);
	out->st[1] = GFIXED(0,5) * (a->st[1] + b->st[1]);

	out->lightmap[0] = GFIXED(0,5) * (a->lightmap[0] + b->lightmap[0]);
	out->lightmap[1] = GFIXED(0,5) * (a->lightmap[1] + b->lightmap[1]);

	out->color[0] = (a->color[0] + b->color[0]) >> 1;
	out->color[1] = (a->color[1] + b->color[1]) >> 1;
	out->color[2] = (a->color[2] + b->color[2]) >> 1;
	out->color[3] = (a->color[3] + b->color[3]) >> 1;
}

/*
============
Transpose
============
*/
static void Transpose( int width, int height, drawVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE] ) {
	int		i, j;
	drawVert_t	temp;

	if ( width > height ) {
		for ( i = 0 ; i < height ; i++ ) {
			for ( j = i + 1 ; j < width ; j++ ) {
				if ( j < height ) {
					// swap the value
					temp = ctrl[j][i];
					ctrl[j][i] = ctrl[i][j];
					ctrl[i][j] = temp;
				} else {
					// just copy
					ctrl[j][i] = ctrl[i][j];
				}
			}
		}
	} else {
		for ( i = 0 ; i < width ; i++ ) {
			for ( j = i + 1 ; j < height ; j++ ) {
				if ( j < width ) {
					// swap the value
					temp = ctrl[i][j];
					ctrl[i][j] = ctrl[j][i];
					ctrl[j][i] = temp;
				} else {
					// just copy
					ctrl[i][j] = ctrl[j][i];
				}
			}
		}
	}

}


/*
=================
MakeMeshNormals

Handles all the complicated wrapping and degenerate cases
=================
*/
static void MakeMeshNormals( int width, int height, drawVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE] ) {
	int		i, j, k, dist;
	bvec3_t	normal;
	bvec3_t	sum;
	int		count;
	bvec3_t	base;
	bvec3_t	delta;
	int		x, y;
	drawVert_t	*dv;
	bvec3_t		around[8], temp;
	qboolean	good[8];
	qboolean	wrapWidth, wrapHeight;
	lfixed len;
	lfixed lvec[3];

	static	int	neighbors[8][2] = {
	{0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}, {-1,1}
	};

	wrapWidth = qfalse;
	for ( i = 0 ; i < height ; i++ ) {
		VectorSubtract( ctrl[i][0].xyz, ctrl[i][width-1].xyz, delta );
		
		lvec[0]=MAKE_LFIXED(delta[0]); lvec[1]=MAKE_LFIXED(delta[1]); lvec[2]=MAKE_LFIXED(delta[2]);
		len = FIXED_VEC3LEN_SQ( lvec );
		if ( len > LFIXED_1 ) {
			break;
		}
	}
	if ( i == height ) {
		wrapWidth = qtrue;
	}

	wrapHeight = qfalse;
	for ( i = 0 ; i < width ; i++ ) {
		VectorSubtract( ctrl[0][i].xyz, ctrl[height-1][i].xyz, delta );

		lvec[0]=MAKE_LFIXED(delta[0]); lvec[1]=MAKE_LFIXED(delta[1]); lvec[2]=MAKE_LFIXED(delta[2]);
		len = FIXED_VEC3LEN_SQ( lvec );
		if ( len > LFIXED_1 ) {
			break;
		}
	}
	if ( i == width) {
		wrapHeight = qtrue;
	}


	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height ; j++ ) {
			count = 0;
			dv = &ctrl[j][i];
			VectorCopy( dv->xyz, base );
			for ( k = 0 ; k < 8 ; k++ ) {
				VectorClear( around[k] );
				good[k] = qfalse;

				for ( dist = 1 ; dist <= 3 ; dist++ ) {
					x = i + neighbors[k][0] * dist;
					y = j + neighbors[k][1] * dist;
					if ( wrapWidth ) {
						if ( x < 0 ) {
							x = width - 1 + x;
						} else if ( x >= width ) {
							x = 1 + x - width;
						}
					}
					if ( wrapHeight ) {
						if ( y < 0 ) {
							y = height - 1 + y;
						} else if ( y >= height ) {
							y = 1 + y - height;
						}
					}

					if ( x < 0 || x >= width || y < 0 || y >= height ) {
						break;					// edge of patch
					}
					VectorSubtract( ctrl[y][x].xyz, base, temp );
					if ( VectorNormalizeLong2( temp, temp ) == LFIXED_1 ) {
						continue;				// degenerate edge, get more dist
					} else {
						good[k] = qtrue;
						VectorCopy( temp, around[k] );
						break;					// good edge
					}
				}
			}

			VectorClear( sum );
			for ( k = 0 ; k < 8 ; k++ ) {
				if ( !good[k] || !good[(k+1)&7] ) {
					continue;	// didn't get two points
				}
				CrossProduct( around[(k+1)&7], around[k], normal );
				if ( VectorNormalizeLong2( normal, normal ) == LFIXED_1 ) {
					continue;
				}
				VectorAdd( normal, sum, sum );
				count++;
			}
			if ( count == 0 ) {
//printf("bad normal\n");
				count = 1;
			}
			VectorNormalizeB2A( sum, dv->normal );
		}
	}
}


/*
============
InvertCtrl
============
*/
static void InvertCtrl( int width, int height, drawVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE] ) {
	int		i, j;
	drawVert_t	temp;

	for ( i = 0 ; i < height ; i++ ) {
		for ( j = 0 ; j < width/2 ; j++ ) {
			temp = ctrl[i][j];
			ctrl[i][j] = ctrl[i][width-1-j];
			ctrl[i][width-1-j] = temp;
		}
	}
}


/*
=================
InvertErrorTable
=================
*/
static void InvertErrorTable( bfixed errorTable[2][MAX_GRID_SIZE], int width, int height ) {
	int		i;
	bfixed	copy[2][MAX_GRID_SIZE];

	Com_Memcpy( copy, errorTable, sizeof( copy ) );

	for ( i = 0 ; i < width ; i++ ) {
		errorTable[1][i] = copy[0][i];	//[width-1-i];
	}

	for ( i = 0 ; i < height ; i++ ) {
		errorTable[0][i] = copy[1][height-1-i];
	}

}

/*
==================
PutPointsOnCurve
==================
*/
static void PutPointsOnCurve( drawVert_t	ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE], 
							 int width, int height ) {
	int			i, j;
	drawVert_t	prev, next;

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 1 ; j < height ; j += 2 ) {
			LerpDrawVert( &ctrl[j][i], &ctrl[j+1][i], &prev );
			LerpDrawVert( &ctrl[j][i], &ctrl[j-1][i], &next );
			LerpDrawVert( &prev, &next, &ctrl[j][i] );
		}
	}


	for ( j = 0 ; j < height ; j++ ) {
		for ( i = 1 ; i < width ; i += 2 ) {
			LerpDrawVert( &ctrl[j][i], &ctrl[j][i+1], &prev );
			LerpDrawVert( &ctrl[j][i], &ctrl[j][i-1], &next );
			LerpDrawVert( &prev, &next, &ctrl[j][i] );
		}
	}
}

/*
=================
R_CreateSurfaceGridMesh
=================
*/
srfGridMesh_t *R_CreateSurfaceGridMesh(int width, int height,
								drawVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE], bfixed errorTable[2][MAX_GRID_SIZE] ) {
	int i, j, size;
	drawVert_t	*vert;
	bvec3_t		tmpVec;
	srfGridMesh_t *grid;

	// copy the results out to a grid
	size = (width * height - 1) * sizeof( drawVert_t ) + sizeof( *grid );

#ifdef PATCH_STITCHING
	grid = (srfGridMesh_t *)/*ri.Hunk_Alloc*/ ri.Malloc( size );
	Com_Memset(grid, 0, size);

	grid->widthLodError = (bfixed *)/*ri.Hunk_Alloc*/ ri.Malloc( width * 4 );
	Com_Memcpy( grid->widthLodError, errorTable[0], width * 4 );

	grid->heightLodError = (bfixed *)/*ri.Hunk_Alloc*/ ri.Malloc( height * 4 );
	Com_Memcpy( grid->heightLodError, errorTable[1], height * 4 );
#else
	grid = ri.Hunk_Alloc( size );
	Com_Memset(grid, 0, size);

	grid->widthLodError = ri.Hunk_Alloc( width * 4 );
	Com_Memcpy( grid->widthLodError, errorTable[0], width * 4 );

	grid->heightLodError = ri.Hunk_Alloc( height * 4 );
	Com_Memcpy( grid->heightLodError, errorTable[1], height * 4 );
#endif

	grid->width = width;
	grid->height = height;
	grid->surfaceType = SF_GRID;
	ClearBounds( grid->meshBounds[0], grid->meshBounds[1] );
	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height ; j++ ) {
			vert = &grid->verts[j*width+i];
			*vert = ctrl[j][i];
			AddPointToBounds( vert->xyz, grid->meshBounds[0], grid->meshBounds[1] );
		}
	}

	// compute local origin and bounds
	VectorAdd( grid->meshBounds[0], grid->meshBounds[1], grid->localOrigin );
	FIXED_VEC3SCALE( grid->localOrigin, BFIXED(0,5), grid->localOrigin );
	VectorSubtract( grid->meshBounds[0], grid->localOrigin, tmpVec );
	grid->meshRadius = FIXED_VEC3LEN( tmpVec );

	VectorCopy( grid->localOrigin, grid->lodOrigin );
	grid->lodRadius = grid->meshRadius;
	//
	return grid;
}

/*
=================
R_FreeSurfaceGridMesh
=================
*/
void R_FreeSurfaceGridMesh( srfGridMesh_t *grid ) {
	ri.Free(grid->widthLodError);
	ri.Free(grid->heightLodError);
	ri.Free(grid);
}

/*
=================
R_SubdividePatchToGrid
=================
*/
srfGridMesh_t *R_SubdividePatchToGrid( int width, int height,
								drawVert_t points[MAX_PATCH_SIZE*MAX_PATCH_SIZE] ) {
	int			i, j, k, l;
	drawVert_t	prev, next, mid;
	bfixed		len, maxLen;
	int			dir;
	int			t;
	MAC_STATIC drawVert_t	ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE];
	bfixed		errorTable[2][MAX_GRID_SIZE];

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height ; j++ ) {
			ctrl[j][i] = points[j*width+i];
		}
	}

	for ( dir = 0 ; dir < 2 ; dir++ ) {

		for ( j = 0 ; j < MAX_GRID_SIZE ; j++ ) {
			errorTable[dir][j] = BFIXED_0;
		}

		// horizontal subdivisions
		for ( j = 0 ; j + 2 < width ; j += 2 ) {
			// check subdivided midpoints against control points

			// FIXME: also check midpoints of adjacent patches against the control points
			// this would basically stitch all patches in the same LOD group together.

			maxLen = BFIXED_0;
			for ( i = 0 ; i < height ; i++ ) {
				bvec3_t		midxyz;
				bvec3_t		midxyz2;
				bvec3_t		dir;
				bvec3_t		projected;
				bfixed		d;

				// calculate the point on the curve
				for ( l = 0 ; l < 3 ; l++ ) {
					midxyz[l] = FIXED_DIVPOW2((ctrl[i][j].xyz[l] + FIXED_MULPOW2(ctrl[i][j+1].xyz[l],1) + ctrl[i][j+2].xyz[l] ),2);
				}

				// see how far off the line it is
				// using dist-from-line will not account for internal
				// texture warping, but it gives a lot less polygons than
				// dist-from-midpoint
				VectorSubtract( midxyz, ctrl[i][j].xyz, midxyz );
				VectorSubtract( ctrl[i][j+2].xyz, ctrl[i][j].xyz, dir );
				VectorNormalize( dir );

				d = FIXED_VEC3DOT( midxyz, dir );
				FIXED_VEC3SCALE( dir, d, projected );
				VectorSubtract( midxyz, projected, midxyz2);
				len = FIXED_VEC3LEN_SQ( midxyz2 );			// we will do the sqrt later
				if ( len > maxLen ) {
					maxLen = len;
				}
			}

			maxLen = FIXED_SQRT(maxLen);

			// if all the points are on the lines, remove the entire columns
			if ( maxLen < BFIXED(0,1) ) {
				errorTable[dir][j+1] = BFIXED(999,0);
				continue;
			}

			// see if we want to insert subdivided columns
			if ( width + 2 > MAX_GRID_SIZE ) {
				errorTable[dir][j+1] = BFIXED_1/maxLen;
				continue;	// can't subdivide any more
			}

			if ( maxLen <= MAKE_BFIXED(r_subdivisions->value) ) {
				errorTable[dir][j+1] = BFIXED_1/maxLen;
				continue;	// didn't need subdivision
			}

			errorTable[dir][j+2] = BFIXED_1/maxLen;

			// insert two columns and replace the peak
			width += 2;
			for ( i = 0 ; i < height ; i++ ) {
				LerpDrawVert( &ctrl[i][j], &ctrl[i][j+1], &prev );
				LerpDrawVert( &ctrl[i][j+1], &ctrl[i][j+2], &next );
				LerpDrawVert( &prev, &next, &mid );

				for ( k = width - 1 ; k > j + 3 ; k-- ) {
					ctrl[i][k] = ctrl[i][k-2];
				}
				ctrl[i][j + 1] = prev;
				ctrl[i][j + 2] = mid;
				ctrl[i][j + 3] = next;
			}

			// back up and recheck this set again, it may need more subdivision
			j -= 2;

		}

		Transpose( width, height, ctrl );
		t = width;
		width = height;
		height = t;
	}


	// put all the aproximating points on the curve
	PutPointsOnCurve( ctrl, width, height );

	// cull out any rows or columns that are colinear
	for ( i = 1 ; i < width-1 ; i++ ) {
		if ( errorTable[0][i] != BFIXED(999 ,0)) {
			continue;
		}
		for ( j = i+1 ; j < width ; j++ ) {
			for ( k = 0 ; k < height ; k++ ) {
				ctrl[k][j-1] = ctrl[k][j];
			}
			errorTable[0][j-1] = errorTable[0][j];
		}
		width--;
	}

	for ( i = 1 ; i < height-1 ; i++ ) {
		if ( errorTable[1][i] != BFIXED(999 ,0)) {
			continue;
		}
		for ( j = i+1 ; j < height ; j++ ) {
			for ( k = 0 ; k < width ; k++ ) {
				ctrl[j-1][k] = ctrl[j][k];
			}
			errorTable[1][j-1] = errorTable[1][j];
		}
		height--;
	}

#if 1
	// flip for longest tristrips as an optimization
	// the results should be visually identical with or
	// without this step
	if ( height > width ) {
		Transpose( width, height, ctrl );
		InvertErrorTable( errorTable, width, height );
		t = width;
		width = height;
		height = t;
		InvertCtrl( width, height, ctrl );
	}
#endif

	// calculate normals
	MakeMeshNormals( width, height, ctrl );

	return R_CreateSurfaceGridMesh( width, height, ctrl, errorTable );
}

/*
===============
R_GridInsertColumn
===============
*/
srfGridMesh_t *R_GridInsertColumn( srfGridMesh_t *grid, int column, int row, bvec3_t point, bfixed loderror ) {
	int i, j;
	int width, height, oldwidth;
	MAC_STATIC drawVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE];
	bfixed errorTable[2][MAX_GRID_SIZE];
	bfixed lodRadius;
	bvec3_t lodOrigin;

	oldwidth = 0;
	width = grid->width + 1;
	if (width > MAX_GRID_SIZE)
		return NULL;
	height = grid->height;
	for (i = 0; i < width; i++) {
		if (i == column) {
			//insert new column
			for (j = 0; j < grid->height; j++) {
				LerpDrawVert( &grid->verts[j * grid->width + i-1], &grid->verts[j * grid->width + i], &ctrl[j][i] );
				if (j == row)
					VectorCopy(point, ctrl[j][i].xyz);
			}
			errorTable[0][i] = loderror;
			continue;
		}
		errorTable[0][i] = grid->widthLodError[oldwidth];
		for (j = 0; j < grid->height; j++) {
			ctrl[j][i] = grid->verts[j * grid->width + oldwidth];
		}
		oldwidth++;
	}
	for (j = 0; j < grid->height; j++) {
		errorTable[1][j] = grid->heightLodError[j];
	}
	// put all the aproximating points on the curve
	//PutPointsOnCurve( ctrl, width, height );
	// calculate normals
	MakeMeshNormals( width, height, ctrl );

	VectorCopy(grid->lodOrigin, lodOrigin);
	lodRadius = grid->lodRadius;
	// free the old grid
	R_FreeSurfaceGridMesh(grid);
	// create a new grid
	grid = R_CreateSurfaceGridMesh( width, height, ctrl, errorTable );
	grid->lodRadius = lodRadius;
	VectorCopy(lodOrigin, grid->lodOrigin);
	return grid;
}

/*
===============
R_GridInsertRow
===============
*/
srfGridMesh_t *R_GridInsertRow( srfGridMesh_t *grid, int row, int column, bvec3_t point, bfixed loderror ) {
	int i, j;
	int width, height, oldheight;
	MAC_STATIC drawVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE];
	bfixed errorTable[2][MAX_GRID_SIZE];
	bfixed lodRadius;
	bvec3_t lodOrigin;

	oldheight = 0;
	width = grid->width;
	height = grid->height + 1;
	if (height > MAX_GRID_SIZE)
		return NULL;
	for (i = 0; i < height; i++) {
		if (i == row) {
			//insert new row
			for (j = 0; j < grid->width; j++) {
				LerpDrawVert( &grid->verts[(i-1) * grid->width + j], &grid->verts[i * grid->width + j], &ctrl[i][j] );
				if (j == column)
					VectorCopy(point, ctrl[i][j].xyz);
			}
			errorTable[1][i] = loderror;
			continue;
		}
		errorTable[1][i] = grid->heightLodError[oldheight];
		for (j = 0; j < grid->width; j++) {
			ctrl[i][j] = grid->verts[oldheight * grid->width + j];
		}
		oldheight++;
	}
	for (j = 0; j < grid->width; j++) {
		errorTable[0][j] = grid->widthLodError[j];
	}
	// put all the aproximating points on the curve
	//PutPointsOnCurve( ctrl, width, height );
	// calculate normals
	MakeMeshNormals( width, height, ctrl );

	VectorCopy(grid->lodOrigin, lodOrigin);
	lodRadius = grid->lodRadius;
	// free the old grid
	R_FreeSurfaceGridMesh(grid);
	// create a new grid
	grid = R_CreateSurfaceGridMesh( width, height, ctrl, errorTable );
	grid->lodRadius = lodRadius;
	VectorCopy(lodOrigin, grid->lodOrigin);
	return grid;
}
