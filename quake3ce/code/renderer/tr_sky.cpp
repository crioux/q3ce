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
// tr_sky.c
#include"renderer_pch.h"

#define SKY_SUBDIVISIONS		8
#define HALF_SKY_SUBDIVISIONS	4

static gfixed s_cloudTexCoords[6][SKY_SUBDIVISIONS+1][SKY_SUBDIVISIONS+1][2];
static gfixed s_cloudTexP[6][SKY_SUBDIVISIONS+1][SKY_SUBDIVISIONS+1];

/*
===================================================================================

POLYGON TO BOX SIDE PROJECTION

===================================================================================
*/

static bvec3_t sky_clip[6] = 
{
	{BFIXED_1,BFIXED_1,BFIXED_0},
	{BFIXED_1,-BFIXED_1,BFIXED_0},
	{BFIXED_0,-BFIXED_1,BFIXED_1},
	{BFIXED_0,BFIXED_1,BFIXED_1},
	{BFIXED_1,BFIXED_0,BFIXED_1},
	{-BFIXED_1,BFIXED_0,BFIXED_1} 
};

static gfixed	sky_mins[2][6], sky_maxs[2][6];
static gfixed	sky_min, sky_max;

/*
================
AddSkyPolygon
================
*/
static void AddSkyPolygon (int nump, bvec3_t vecs) 
{
	int		i,j;
	bvec3_t	v, av;
	gfixed	s, t;
	bfixed dv;
	int		axis;
	bfixed	*vp;
	// s = [0]/[2], t = [1]/[2]
	static int	vec_to_st[6][3] =
	{
		{-2,3,1},
		{2,3,-1},

		{1,3,2},
		{-1,3,-2},

		{-2,-1,3},
		{-2,1,-3}

	//	{-1,2,3},
	//	{1,2,-3}
	};

	// decide which face it maps to
	VectorCopy (bvec3_origin, v);
	for (i=0, vp=vecs ; i<nump ; i++, vp+=3)
	{
		VectorAdd (vp, v, v);
	}
	av[0] = FIXED_ABS(v[0]);
	av[1] = FIXED_ABS(v[1]);
	av[2] = FIXED_ABS(v[2]);
	if (av[0] > av[1] && av[0] > av[2])
	{
		if (v[0] < BFIXED_0)
			axis = 1;
		else
			axis = 0;
	}
	else if (av[1] > av[2] && av[1] > av[0])
	{
		if (v[1] < BFIXED_0)
			axis = 3;
		else
			axis = 2;
	}
	else
	{
		if (v[2] < BFIXED_0)
			axis = 5;
		else
			axis = 4;
	}

	// project new texture coords
	for (i=0 ; i<nump ; i++, vecs+=3)
	{
		j = vec_to_st[axis][2];
		if (j > 0)
			dv = vecs[j - 1];
		else
			dv = -vecs[-j - 1];
		if (dv < BFIXED(0,001))
			continue;	// don't divide by zero
		j = vec_to_st[axis][0];
		if (j < 0)
			s = MAKE_GFIXED(-vecs[-j -1] / dv);
		else
			s = MAKE_GFIXED(vecs[j-1] / dv);
		j = vec_to_st[axis][1];
		if (j < 0)
			t = MAKE_GFIXED(-vecs[-j -1] / dv);
		else
			t = MAKE_GFIXED(vecs[j-1] / dv);

		if (s < sky_mins[0][axis])
			sky_mins[0][axis] = s;
		if (t < sky_mins[1][axis])
			sky_mins[1][axis] = t;
		if (s > sky_maxs[0][axis])
			sky_maxs[0][axis] = s;
		if (t > sky_maxs[1][axis])
			sky_maxs[1][axis] = t;
	}
}

#define	ON_EPSILON		BFIXED(0,1)			// point on plane side epsilon
#define	MAX_CLIP_VERTS	64
/*
================
ClipSkyPolygon
================
*/
static void ClipSkyPolygon (int nump, bvec3_t vecs, int stage) 
{
	bfixed	*norm;
	bfixed	*v;
	qboolean	front, back;
	bfixed	d, e;
	bfixed	dists[MAX_CLIP_VERTS];
	int		sides[MAX_CLIP_VERTS];
	bvec3_t	newv[2][MAX_CLIP_VERTS];
	int		newc[2];
	int		i, j;

	if (nump > MAX_CLIP_VERTS-2)
		ri.Error (ERR_DROP, "ClipSkyPolygon: MAX_CLIP_VERTS");
	if (stage == 6)
	{	// fully clipped, so draw it
		AddSkyPolygon (nump, vecs);
		return;
	}

	front = back = qfalse;
	norm = sky_clip[stage];
	for (i=0, v = vecs ; i<nump ; i++, v+=3)
	{
		d = FIXED_VEC3DOT (v, norm);
		if (d > ON_EPSILON)
		{
			front = qtrue;
			sides[i] = SIDE_FRONT;
		}
		else if (d < -ON_EPSILON)
		{
			back = qtrue;
			sides[i] = SIDE_BACK;
		}
		else
			sides[i] = SIDE_ON;
		dists[i] = d;
	}

	if (!front || !back)
	{	// not clipped
		ClipSkyPolygon (nump, vecs, stage+1);
		return;
	}

	// clip it
	sides[i] = sides[0];
	dists[i] = dists[0];
	VectorCopy (vecs, (vecs+(i*3)) );
	newc[0] = newc[1] = 0;

	for (i=0, v = vecs ; i<nump ; i++, v+=3)
	{
		switch (sides[i])
		{
		case SIDE_FRONT:
			VectorCopy (v, newv[0][newc[0]]);
			newc[0]++;
			break;
		case SIDE_BACK:
			VectorCopy (v, newv[1][newc[1]]);
			newc[1]++;
			break;
		case SIDE_ON:
			VectorCopy (v, newv[0][newc[0]]);
			newc[0]++;
			VectorCopy (v, newv[1][newc[1]]);
			newc[1]++;
			break;
		}

		if (sides[i] == SIDE_ON || sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;

		d = dists[i] / (dists[i] - dists[i+1]);
		for (j=0 ; j<3 ; j++)
		{
			e = v[j] + d*(v[j+3] - v[j]);
			newv[0][newc[0]][j] = e;
			newv[1][newc[1]][j] = e;
		}
		newc[0]++;
		newc[1]++;
	}

	// continue
	ClipSkyPolygon (newc[0], newv[0][0], stage+1);
	ClipSkyPolygon (newc[1], newv[1][0], stage+1);
}

/*
==============
ClearSkyBox
==============
*/
static void ClearSkyBox (void) {
	int		i;

	for (i=0 ; i<6 ; i++) {
		sky_mins[0][i] = sky_mins[1][i] = GFIXED(9999,0);
		sky_maxs[0][i] = sky_maxs[1][i] = -GFIXED(9999,0);
	}
}

/*
================
RB_ClipSkyPolygons
================
*/
void RB_ClipSkyPolygons( shaderCommands_t *input )
{
	bvec3_t		p[5];	// need one extra point for clipping
	int			i, j;

	ClearSkyBox();

	for ( i = 0; i < input->numIndexes; i += 3 )
	{
		for (j = 0 ; j < 3 ; j++) 
		{
			VectorSubtract( input->xyz[input->indexes[i+j]],
							backEnd.viewParms.or.origin, 
							p[j] );
		}
		ClipSkyPolygon( 3, p[0], 0 );
	}
}

/*
===================================================================================

CLOUD VERTEX GENERATION

===================================================================================
*/

/*
** MakeSkyVec
**
** Parms: s, t range from -1 to 1
*/
static void MakeSkyVec( gfixed s, gfixed t, int axis, gfixed outSt[2], bvec3_t outXYZ )
{
	// 1 = s, 2 = t, 3 = 2048
	static int	st_to_vec[6][3] =
	{
		{3,-1,2},
		{-3,1,2},

		{1,3,2},
		{-1,-3,2},

		{-2,-1,3},		// 0 degrees yaw, look straight up
		{2,-1,-3}		// look straight down
	};

	bvec3_t		b;
	int			j, k;
	bfixed	boxSize;

	boxSize = backEnd.viewParms.zFar / BFIXED(1,75);		// div sqrt(3)
	b[0] = s*boxSize;
	b[1] = t*boxSize;
	b[2] = boxSize;

	for (j=0 ; j<3 ; j++)
	{
		k = st_to_vec[axis][j];
		if (k < 0)
		{
			outXYZ[j] = -b[-k - 1];
		}
		else
		{
			outXYZ[j] = b[k - 1];
		}
	}

	// avoid bilerp seam
	s = FIXED_DIVPOW2((s+GFIXED_1),1);
	t = FIXED_DIVPOW2((t+GFIXED_1),1);
	if (s < sky_min)
	{
		s = sky_min;
	}
	else if (s > sky_max)
	{
		s = sky_max;
	}

	if (t < sky_min)
	{
		t = sky_min;
	}
	else if (t > sky_max)
	{
		t = sky_max;
	}

	t = GFIXED_1 - t;


	if ( outSt )
	{
		outSt[0] = s;
		outSt[1] = t;
	}
}

static int	sky_texorder[6] = {0,2,1,3,4,5};
static bvec3_t	s_skyPoints[SKY_SUBDIVISIONS+1][SKY_SUBDIVISIONS+1];
static gfixed	s_skyTexCoords[SKY_SUBDIVISIONS+1][SKY_SUBDIVISIONS+1][2];

static void DrawSkySide( struct image_s *image, const int mins[2], const int maxs[2] )
{
	int s, t;

	GL_Bind( image );

	for ( t = mins[1]+HALF_SKY_SUBDIVISIONS; t < maxs[1]+HALF_SKY_SUBDIVISIONS; t++ )
	{
		glBegin( GL_TRIANGLE_STRIP );

		for ( s = mins[0]+HALF_SKY_SUBDIVISIONS; s <= maxs[0]+HALF_SKY_SUBDIVISIONS; s++ )
		{
			gfixed *pt;
			bfixed *pv;

			pt=s_skyTexCoords[t][s];
			glTexCoord2X( pt[0],pt[1] );
			pv=s_skyPoints[t][s];
			glVertex3X(REINTERPRET_GFIXED(pv[0]),
				       REINTERPRET_GFIXED(pv[1]),
					   REINTERPRET_GFIXED(pv[2]));

			pt=s_skyTexCoords[t+1][s];
			glTexCoord2X( pt[0],pt[1] );
			pv=s_skyPoints[t+1][s];
			glVertex3X(REINTERPRET_GFIXED(pv[0]),
				       REINTERPRET_GFIXED(pv[1]),
					   REINTERPRET_GFIXED(pv[2]));
		}

		glEnd();
	}
}

static void DrawSkyBox( shader_t *shader )
{
	int		i;

	sky_min = GFIXED_0;
	sky_max = GFIXED_1;

	Com_Memset( s_skyTexCoords, 0, sizeof( s_skyTexCoords ) );

	for (i=0 ; i<6 ; i++)
	{
		int sky_mins_subd[2], sky_maxs_subd[2];
		int s, t;

		sky_mins[0][i] = FIXED_FLOOR( sky_mins[0][i] * GFIXED(HALF_SKY_SUBDIVISIONS ,0)) / GFIXED(HALF_SKY_SUBDIVISIONS,0);
		sky_mins[1][i] = FIXED_FLOOR( sky_mins[1][i] * GFIXED(HALF_SKY_SUBDIVISIONS ,0)) / GFIXED(HALF_SKY_SUBDIVISIONS,0);
		sky_maxs[0][i] = FIXED_CEIL( sky_maxs[0][i] * GFIXED(HALF_SKY_SUBDIVISIONS ,0)) / GFIXED(HALF_SKY_SUBDIVISIONS,0);
		sky_maxs[1][i] = FIXED_CEIL( sky_maxs[1][i] * GFIXED(HALF_SKY_SUBDIVISIONS ,0)) / GFIXED(HALF_SKY_SUBDIVISIONS,0);

		if ( ( sky_mins[0][i] >= sky_maxs[0][i] ) ||
			 ( sky_mins[1][i] >= sky_maxs[1][i] ) )
		{
			continue;
		}

		sky_mins_subd[0] = FIXED_INT32SCALE(HALF_SKY_SUBDIVISIONS, sky_mins[0][i]);
		sky_mins_subd[1] = FIXED_INT32SCALE(HALF_SKY_SUBDIVISIONS, sky_mins[1][i]);
		sky_maxs_subd[0] = FIXED_INT32SCALE(HALF_SKY_SUBDIVISIONS, sky_maxs[0][i]);
		sky_maxs_subd[1] = FIXED_INT32SCALE(HALF_SKY_SUBDIVISIONS, sky_maxs[1][i]);

		if ( sky_mins_subd[0] < -HALF_SKY_SUBDIVISIONS ) 
			sky_mins_subd[0] = -HALF_SKY_SUBDIVISIONS;
		else if ( sky_mins_subd[0] > HALF_SKY_SUBDIVISIONS ) 
			sky_mins_subd[0] = HALF_SKY_SUBDIVISIONS;
		if ( sky_mins_subd[1] < -HALF_SKY_SUBDIVISIONS )
			sky_mins_subd[1] = -HALF_SKY_SUBDIVISIONS;
		else if ( sky_mins_subd[1] > HALF_SKY_SUBDIVISIONS ) 
			sky_mins_subd[1] = HALF_SKY_SUBDIVISIONS;

		if ( sky_maxs_subd[0] < -HALF_SKY_SUBDIVISIONS ) 
			sky_maxs_subd[0] = -HALF_SKY_SUBDIVISIONS;
		else if ( sky_maxs_subd[0] > HALF_SKY_SUBDIVISIONS ) 
			sky_maxs_subd[0] = HALF_SKY_SUBDIVISIONS;
		if ( sky_maxs_subd[1] < -HALF_SKY_SUBDIVISIONS ) 
			sky_maxs_subd[1] = -HALF_SKY_SUBDIVISIONS;
		else if ( sky_maxs_subd[1] > HALF_SKY_SUBDIVISIONS ) 
			sky_maxs_subd[1] = HALF_SKY_SUBDIVISIONS;

		//
		// iterate through the subdivisions
		//
		for ( t = sky_mins_subd[1]+HALF_SKY_SUBDIVISIONS; t <= sky_maxs_subd[1]+HALF_SKY_SUBDIVISIONS; t++ )
		{
			for ( s = sky_mins_subd[0]+HALF_SKY_SUBDIVISIONS; s <= sky_maxs_subd[0]+HALF_SKY_SUBDIVISIONS; s++ )
			{
				MakeSkyVec( FIXED_INT32RATIO_G( s - HALF_SKY_SUBDIVISIONS,HALF_SKY_SUBDIVISIONS), 
							FIXED_INT32RATIO_G( t - HALF_SKY_SUBDIVISIONS,HALF_SKY_SUBDIVISIONS), 
							i, 
							s_skyTexCoords[t][s], 
							s_skyPoints[t][s] );
			}
		}

		DrawSkySide( shader->sky.outerbox[sky_texorder[i]],
			         sky_mins_subd,
					 sky_maxs_subd );
	}

}

static void FillCloudySkySide( const int mins[2], const int maxs[2], qboolean addIndexes )
{
	int s, t;
	int vertexStart = tess.numVertexes;
	int tHeight, sWidth;

	tHeight = maxs[1] - mins[1] + 1;
	sWidth = maxs[0] - mins[0] + 1;

	for ( t = mins[1]+HALF_SKY_SUBDIVISIONS; t <= maxs[1]+HALF_SKY_SUBDIVISIONS; t++ )
	{
		for ( s = mins[0]+HALF_SKY_SUBDIVISIONS; s <= maxs[0]+HALF_SKY_SUBDIVISIONS; s++ )
		{
			VectorAdd( s_skyPoints[t][s], backEnd.viewParms.or.origin, tess.xyz[tess.numVertexes] );
			tess.texCoords[tess.numVertexes][0][0] = s_skyTexCoords[t][s][0];
			tess.texCoords[tess.numVertexes][0][1] = s_skyTexCoords[t][s][1];

			tess.numVertexes++;

			if ( tess.numVertexes >= SHADER_MAX_VERTEXES )
			{
				ri.Error( ERR_DROP, "SHADER_MAX_VERTEXES hit in FillCloudySkySide()\n" );
			}
		}
	}

	// only add indexes for one pass, otherwise it would draw multiple times for each pass
	if ( addIndexes ) {
		for ( t = 0; t < tHeight-1; t++ )
		{	
			for ( s = 0; s < sWidth-1; s++ )
			{
				tess.indexes[tess.numIndexes] = vertexStart + s + t * ( sWidth );
				tess.numIndexes++;
				tess.indexes[tess.numIndexes] = vertexStart + s + ( t + 1 ) * ( sWidth );
				tess.numIndexes++;
				tess.indexes[tess.numIndexes] = vertexStart + s + 1 + t * ( sWidth );
				tess.numIndexes++;

				tess.indexes[tess.numIndexes] = vertexStart + s + ( t + 1 ) * ( sWidth );
				tess.numIndexes++;
				tess.indexes[tess.numIndexes] = vertexStart + s + 1 + ( t + 1 ) * ( sWidth );
				tess.numIndexes++;
				tess.indexes[tess.numIndexes] = vertexStart + s + 1 + t * ( sWidth );
				tess.numIndexes++;
			}
		}
	}
}

static void FillCloudBox( const shader_t *shader, int stage )
{
	int i;

	for ( i =0; i < 6; i++ )
	{
		int sky_mins_subd[2], sky_maxs_subd[2];
		int s, t;
		gfixed MIN_T;

		if ( 1 ) // FIXME? shader->sky.fullClouds )
		{
			MIN_T = -GFIXED(HALF_SKY_SUBDIVISIONS,0);

			// still don't want to draw the bottom, even if fullClouds
			if ( i == 5 )
				continue;
		}
		else
		{
			switch( i )
			{
			case 0:
			case 1:
			case 2:
			case 3:
				MIN_T = -GFIXED_1;
				break;
			case 5:
				// don't draw clouds beneath you
				continue;
			case 4:		// top
			default:
				MIN_T = -GFIXED(HALF_SKY_SUBDIVISIONS,0);
				break;
			}
		}

		sky_mins[0][i] = MAKE_GFIXED(FIXED_FLOOR( MAKE_LFIXED(sky_mins[0][i]) * LFIXED(HALF_SKY_SUBDIVISIONS ,0)) / LFIXED(HALF_SKY_SUBDIVISIONS,0));
		sky_mins[1][i] = MAKE_GFIXED(FIXED_FLOOR( MAKE_LFIXED(sky_mins[1][i]) * LFIXED(HALF_SKY_SUBDIVISIONS ,0)) / LFIXED(HALF_SKY_SUBDIVISIONS,0));
		sky_maxs[0][i] = MAKE_GFIXED(FIXED_CEIL( MAKE_LFIXED(sky_maxs[0][i]) * LFIXED(HALF_SKY_SUBDIVISIONS ,0)) / LFIXED(HALF_SKY_SUBDIVISIONS,0));
		sky_maxs[1][i] = MAKE_GFIXED(FIXED_CEIL( MAKE_LFIXED(sky_maxs[1][i]) * LFIXED(HALF_SKY_SUBDIVISIONS ,0)) / LFIXED(HALF_SKY_SUBDIVISIONS,0));

		if ( ( sky_mins[0][i] >= sky_maxs[0][i] ) ||
			 ( sky_mins[1][i] >= sky_maxs[1][i] ) )
		{
			continue;
		}

		sky_mins_subd[0] = FIXED_INT32SCALE(HALF_SKY_SUBDIVISIONS, sky_mins[0][i]);
		sky_mins_subd[1] = FIXED_INT32SCALE(HALF_SKY_SUBDIVISIONS, sky_mins[1][i]);
		sky_maxs_subd[0] = FIXED_INT32SCALE(HALF_SKY_SUBDIVISIONS, sky_maxs[0][i]);
		sky_maxs_subd[1] = FIXED_INT32SCALE(HALF_SKY_SUBDIVISIONS, sky_maxs[1][i]);

		if ( sky_mins_subd[0] < -HALF_SKY_SUBDIVISIONS ) 
			sky_mins_subd[0] = -HALF_SKY_SUBDIVISIONS;
		else if ( sky_mins_subd[0] > HALF_SKY_SUBDIVISIONS ) 
			sky_mins_subd[0] = HALF_SKY_SUBDIVISIONS;
		if ( MAKE_GFIXED(sky_mins_subd[1]) < MIN_T )
			sky_mins_subd[1] = FIXED_TO_INT(MIN_T);
		else if ( sky_mins_subd[1] > HALF_SKY_SUBDIVISIONS ) 
			sky_mins_subd[1] = HALF_SKY_SUBDIVISIONS;

		if ( sky_maxs_subd[0] < -HALF_SKY_SUBDIVISIONS ) 
			sky_maxs_subd[0] = -HALF_SKY_SUBDIVISIONS;
		else if ( sky_maxs_subd[0] > HALF_SKY_SUBDIVISIONS ) 
			sky_maxs_subd[0] = HALF_SKY_SUBDIVISIONS;
		if ( MAKE_GFIXED(sky_maxs_subd[1]) < MIN_T )
			sky_maxs_subd[1] = FIXED_TO_INT(MIN_T);
		else if ( sky_maxs_subd[1] > HALF_SKY_SUBDIVISIONS ) 
			sky_maxs_subd[1] = HALF_SKY_SUBDIVISIONS;

		//
		// iterate through the subdivisions
		//
		for ( t = sky_mins_subd[1]+HALF_SKY_SUBDIVISIONS; t <= sky_maxs_subd[1]+HALF_SKY_SUBDIVISIONS; t++ )
		{
			for ( s = sky_mins_subd[0]+HALF_SKY_SUBDIVISIONS; s <= sky_maxs_subd[0]+HALF_SKY_SUBDIVISIONS; s++ )
			{
				MakeSkyVec( FIXED_INT32RATIO_G( s - HALF_SKY_SUBDIVISIONS, HALF_SKY_SUBDIVISIONS), 
							FIXED_INT32RATIO_G( t - HALF_SKY_SUBDIVISIONS, HALF_SKY_SUBDIVISIONS), 
							i, 
							NULL,
							s_skyPoints[t][s] );

				s_skyTexCoords[t][s][0] = s_cloudTexCoords[i][t][s][0];
				s_skyTexCoords[t][s][1] = s_cloudTexCoords[i][t][s][1];
			}
		}

		// only add indexes for first stage
		FillCloudySkySide( sky_mins_subd, sky_maxs_subd, ( stage == 0 ) );
	}
}

/*
** R_BuildCloudData
*/
void R_BuildCloudData( shaderCommands_t *input )
{
	int			i;
	shader_t	*shader;

	shader = input->shader;

	assert( shader->isSky );

	sky_min = GFIXED_1 / GFIXED(256,0);		// FIXME: not correct?
	sky_max = GFIXED(255,0) / GFIXED(256,0);

	// set up for drawing
	tess.numIndexes = 0;
	tess.numVertexes = 0;

	if ( FIXED_NOT_ZERO( input->shader->sky.cloudHeight ))
	{
		for ( i = 0; i < MAX_SHADER_STAGES; i++ )
		{
			if ( !tess.xstages[i] ) {
				break;
			}
			FillCloudBox( input->shader, i );
		}
	}
}

/*
** R_InitSkyTexCoords
** Called when a sky shader is parsed
*/
#define SQR( a ) ((a)*(a))
void R_InitSkyTexCoords( bfixed heightCloud )
{
	int i, s, t;
	bfixed radiusWorld = BFIXED(4096,0);
	double fradiusWorld = 4096.0;
	double fheightCloud = FIXED_TO_DOUBLE(heightCloud);
	float p;
	gfixed sRad, tRad;
	bvec3_t skyVec;
	bvec3_t v;
	float fskyvec[3];
				
	// init zfar so MakeSkyVec works even though
	// a world hasn't been bounded
	backEnd.viewParms.zFar = BFIXED(1024,0);

	for ( i = 0; i < 6; i++ )
	{
		for ( t = 0; t <= SKY_SUBDIVISIONS; t++ )
		{
			for ( s = 0; s <= SKY_SUBDIVISIONS; s++ )
			{
				// compute vector from view origin to sky side integral point
				MakeSkyVec( FIXED_INT32RATIO_G( s - HALF_SKY_SUBDIVISIONS, HALF_SKY_SUBDIVISIONS), 
							FIXED_INT32RATIO_G( t - HALF_SKY_SUBDIVISIONS, HALF_SKY_SUBDIVISIONS), 
							i, 
							NULL,
							skyVec );

				fskyvec[0]=FIXED_TO_FLOAT(skyVec[0]);
				fskyvec[1]=FIXED_TO_FLOAT(skyVec[1]);
				fskyvec[2]=FIXED_TO_FLOAT(skyVec[2]);

				// compute parametric value 'p' that intersects with cloud layer
				p = ( 1.0 / ( 2.0*
					( fskyvec[0]*fskyvec[0] + fskyvec[1]*fskyvec[1] + fskyvec[2]*fskyvec[2] ) 
					) ) *
					( -2.0* fskyvec[2] * fradiusWorld + 
					   2.0* sqrt( SQR( fskyvec[2] ) * SQR( fradiusWorld ) + 
					                             2.0* SQR( fskyvec[0] ) * fradiusWorld * fheightCloud +
								                 SQR( fskyvec[0] ) * SQR( fheightCloud ) + 
								                 2.0* SQR( fskyvec[1] ) * fradiusWorld * fheightCloud +
								                 SQR( fskyvec[1] ) * SQR( fheightCloud ) + 
								                 2.0* SQR( fskyvec[2] ) * fradiusWorld * fheightCloud +
								                 SQR( fskyvec[2] ) * SQR( fheightCloud ) ) );

				s_cloudTexP[i][t][s] = MAKE_GFIXED(p);

				// compute intersection point based on p
				FIXED_VEC3SCALE( skyVec, MAKE_BFIXED(p), v );
				v[2] += radiusWorld;

				// compute vector from world origin to intersection point 'v'
				VectorNormalize( v );

				sRad = MAKE_GFIXED(Q_acos( v[0] ));
				tRad = MAKE_GFIXED(Q_acos( v[1] ));

				s_cloudTexCoords[i][t][s][0] = sRad;
				s_cloudTexCoords[i][t][s][1] = tRad;
			}
		}
	}
}

//======================================================================================

/*
** RB_DrawSun
*/
void RB_DrawSun( void ) {
	bfixed		size;
	bfixed		dist;
	bvec3_t		origin;
	avec3_t		vec1, vec2;
	bvec3_t		bvec1, bvec2;
	bvec3_t		temp;

	if ( !backEnd.skyRenderedThisView ) {
		return;
	}
	if ( !r_drawSun->integer ) {
		return;
	}
	glLoadMatrixX( backEnd.viewParms.world.modelMatrix );
	glTranslateX ( REINTERPRET_GFIXED(backEnd.viewParms.or.origin[0]), 
				   REINTERPRET_GFIXED(backEnd.viewParms.or.origin[1]), 
				   REINTERPRET_GFIXED(backEnd.viewParms.or.origin[2]));

	dist = 	backEnd.viewParms.zFar / BFIXED(1,75);		// div sqrt(3)
	size = dist * BFIXED(0,4);

	FIXED_VEC3SCALE_R( tr.sunDirection, dist, origin );
	PerpendicularVector( vec1, tr.sunDirection );
	CrossProduct( tr.sunDirection, vec1, vec2 );

	FIXED_VEC3SCALE_R( vec1, size, bvec1 );
	FIXED_VEC3SCALE_R( vec2, size, bvec2 );

	// farthest depth range
	glDepthRangeX( GFIXED_1, GFIXED_1 );

	// FIXME: use quad stamp
	RB_BeginSurface( tr.sunShader, tess.fogNum );
		VectorCopy( origin, temp );
		VectorSubtract( temp, bvec1, temp );
		VectorSubtract( temp, bvec2, temp );
		VectorCopy( temp, tess.xyz[tess.numVertexes] );
		tess.texCoords[tess.numVertexes][0][0] = GFIXED_0;
		tess.texCoords[tess.numVertexes][0][1] = GFIXED_0;
		tess.vertexColors[tess.numVertexes][0] = 255;
		tess.vertexColors[tess.numVertexes][1] = 255;
		tess.vertexColors[tess.numVertexes][2] = 255;
		tess.numVertexes++;

		VectorCopy( origin, temp );
		VectorAdd( temp, bvec1, temp );
		VectorSubtract( temp, bvec2, temp );
		VectorCopy( temp, tess.xyz[tess.numVertexes] );
		tess.texCoords[tess.numVertexes][0][0] = GFIXED_0;
		tess.texCoords[tess.numVertexes][0][1] = GFIXED_1;
		tess.vertexColors[tess.numVertexes][0] = 255;
		tess.vertexColors[tess.numVertexes][1] = 255;
		tess.vertexColors[tess.numVertexes][2] = 255;
		tess.numVertexes++;

		VectorCopy( origin, temp );
		VectorAdd( temp, bvec1, temp );
		VectorAdd( temp, bvec2, temp );
		VectorCopy( temp, tess.xyz[tess.numVertexes] );
		tess.texCoords[tess.numVertexes][0][0] = GFIXED_1;
		tess.texCoords[tess.numVertexes][0][1] = GFIXED_1;
		tess.vertexColors[tess.numVertexes][0] = 255;
		tess.vertexColors[tess.numVertexes][1] = 255;
		tess.vertexColors[tess.numVertexes][2] = 255;
		tess.numVertexes++;

		VectorCopy( origin, temp );
		VectorSubtract( temp, bvec1, temp );
		VectorAdd( temp, bvec2, temp );
		VectorCopy( temp, tess.xyz[tess.numVertexes] );
		tess.texCoords[tess.numVertexes][0][0] = GFIXED_1;
		tess.texCoords[tess.numVertexes][0][1] = GFIXED_0;
		tess.vertexColors[tess.numVertexes][0] = 255;
		tess.vertexColors[tess.numVertexes][1] = 255;
		tess.vertexColors[tess.numVertexes][2] = 255;
		tess.numVertexes++;

		tess.indexes[tess.numIndexes++] = 0;
		tess.indexes[tess.numIndexes++] = 1;
		tess.indexes[tess.numIndexes++] = 2;
		tess.indexes[tess.numIndexes++] = 0;
		tess.indexes[tess.numIndexes++] = 2;
		tess.indexes[tess.numIndexes++] = 3;

	RB_EndSurface();

	// back to normal depth range
	glDepthRangeX( GFIXED_0, GFIXED_1);
}




/*
================
RB_StageIteratorSky

All of the visible sky triangles are in tess

Other things could be stuck in here, like birds in the sky, etc
================
*/
void RB_StageIteratorSky( void ) {
	if ( r_fastsky->integer ) {
		return;
	}

	// go through all the polygons and project them onto
	// the sky box to see which blocks on each side need
	// to be drawn
	RB_ClipSkyPolygons( &tess );

	// r_showsky will let all the sky blocks be drawn in
	// front of everything to allow developers to see how
	// much sky is getting sucked in
	if ( r_showsky->integer ) {
		glDepthRangeX( GFIXED_0, GFIXED_0 );
	} else {
		glDepthRangeX( GFIXED_1, GFIXED_1 );
	}

	// draw the outer skybox
	if ( tess.shader->sky.outerbox[0] && tess.shader->sky.outerbox[0] != tr.defaultImage ) {
		glColor4X( tr.identityLight, tr.identityLight, tr.identityLight,GFIXED_1 );
		
		glPushMatrix ();
		GL_State( 0 );
		glTranslateX (REINTERPRET_GFIXED(backEnd.viewParms.or.origin[0]), 
              		  REINTERPRET_GFIXED(backEnd.viewParms.or.origin[1]), 
			          REINTERPRET_GFIXED(backEnd.viewParms.or.origin[2]));

		DrawSkyBox( tess.shader );

		glPopMatrix();
	}

	// generate the vertexes for all the clouds, which will be drawn
	// by the generic shader routine
	R_BuildCloudData( &tess );

	RB_StageIteratorGeneric();

	// draw the inner skybox


	// back to normal depth range
	glDepthRangeX( GFIXED_0, GFIXED_1);

	// note that sky was drawn so we will draw a sun later
	backEnd.skyRenderedThisView = qtrue;
}

