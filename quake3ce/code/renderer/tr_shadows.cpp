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

  for a projection shadow:

  point[x] += light vector * ( z - shadow plane )
  point[y] +=
  point[z] = shadow plane

  1 0 light[x] / light[z]

*/

typedef struct {
	int		i2;
	int		facing;
} edgeDef_t;

#define	MAX_EDGE_DEFS	32

static	edgeDef_t	edgeDefs[SHADER_MAX_VERTEXES][MAX_EDGE_DEFS];
static	int			numEdgeDefs[SHADER_MAX_VERTEXES];
static	int			facing[SHADER_MAX_INDEXES/3];

void R_AddEdgeDef( int i1, int i2, int facing ) {
	int		c;

	c = numEdgeDefs[ i1 ];
	if ( c == MAX_EDGE_DEFS ) {
		return;		// overflow
	}
	edgeDefs[ i1 ][ c ].i2 = i2;
	edgeDefs[ i1 ][ c ].facing = facing;

	numEdgeDefs[ i1 ]++;
}

void R_RenderShadowEdges( void ) {
	int		i;

#if 0
	int		numTris;

	// dumb way -- render every triangle's edges
	numTris = tess.numIndexes / 3;

	for ( i = 0 ; i < numTris ; i++ ) {
		int		i1, i2, i3;

		if ( !facing[i] ) {
			continue;
		}

		i1 = tess.indexes[ i*3 + 0 ];
		i2 = tess.indexes[ i*3 + 1 ];
		i3 = tess.indexes[ i*3 + 2 ];

		glBegin( GL_TRIANGLE_STRIP );
		glVertex3fv( tess.xyz[ i1 ] );
		glVertex3fv( tess.xyz[ i1 + tess.numVertexes ] );
		glVertex3fv( tess.xyz[ i2 ] );
		glVertex3fv( tess.xyz[ i2 + tess.numVertexes ] );
		glVertex3fv( tess.xyz[ i3 ] );
		glVertex3fv( tess.xyz[ i3 + tess.numVertexes ] );
		glVertex3fv( tess.xyz[ i1 ] );
		glVertex3fv( tess.xyz[ i1 + tess.numVertexes ] );
		glEnd();
	}
#else
	int		c, c2;
	int		j, k;
	int		i2;
	int		c_edges, c_rejected;
	int		hit[2];

	// an edge is NOT a silhouette edge if its face doesn't face the light,
	// or if it has a reverse paired edge that also faces the light.
	// A well behaved polyhedron would have exactly two faces for each edge,
	// but lots of models have dangling edges or overfanned edges
	c_edges = 0;
	c_rejected = 0;

	for ( i = 0 ; i < tess.numVertexes ; i++ ) {
		c = numEdgeDefs[ i ];
		for ( j = 0 ; j < c ; j++ ) {
			if ( !edgeDefs[ i ][ j ].facing ) {
				continue;
			}

			hit[0] = 0;
			hit[1] = 0;

			i2 = edgeDefs[ i ][ j ].i2;
			c2 = numEdgeDefs[ i2 ];
			for ( k = 0 ; k < c2 ; k++ ) {
				if ( edgeDefs[ i2 ][ k ].i2 == i ) {
					hit[ edgeDefs[ i2 ][ k ].facing ]++;
				}
			}

			// if it doesn't share the edge with another front facing
			// triangle, it is a sil edge
			if ( hit[ 1 ] == 0 ) {
				glBegin( GL_TRIANGLE_STRIP );
				bfixed *p;
				p=tess.xyz[ i ];
				glVertex3X( REINTERPRET_GFIXED(p[0]),
					        REINTERPRET_GFIXED(p[1]),
					        REINTERPRET_GFIXED(p[2]) );
				p=tess.xyz[ i + tess.numVertexes  ];
				glVertex3X( REINTERPRET_GFIXED(p[0]),
					        REINTERPRET_GFIXED(p[1]),
							REINTERPRET_GFIXED(p[2]) );
				p=tess.xyz[ i2 ];
				glVertex3X( REINTERPRET_GFIXED(p[0]),
					        REINTERPRET_GFIXED(p[1]),
							REINTERPRET_GFIXED(p[2]) );
				p=tess.xyz[ i2 + tess.numVertexes ];
				glVertex3X( REINTERPRET_GFIXED(p[0]),
					        REINTERPRET_GFIXED(p[1]),
							REINTERPRET_GFIXED(p[2]) );
				glEnd();
				c_edges++;
			} else {
				c_rejected++;
			}
		}
	}
#endif
}

/*
=================
RB_ShadowTessEnd

triangleFromEdge[ v1 ][ v2 ]


  set triangle from edge( v1, v2, tri )
  if ( facing[ triangleFromEdge[ v1 ][ v2 ] ] && !facing[ triangleFromEdge[ v2 ][ v1 ] ) {
  }
=================
*/
void RB_ShadowTessEnd( void ) {
	int		i;
	int		numTris;
	avec3_t	lightDir;

	// we can only do this if we have enough space in the vertex buffers
	if ( tess.numVertexes >= SHADER_MAX_VERTEXES / 2 ) {
		return;
	}

	if ( glConfig.stencilBits < 4 ) {
		return;
	}

	VectorCopy( backEnd.currentEntity->lightDir, lightDir );

	// project vertexes away from light direction
	for ( i = 0 ; i < tess.numVertexes ; i++ ) {
		FIXED_VEC3MA_R( tess.xyz[i], -BFIXED(512,0), lightDir, tess.xyz[i+tess.numVertexes] );
	}

	// decide which triangles face the light
	Com_Memset( numEdgeDefs, 0, 4 * tess.numVertexes );

	numTris = tess.numIndexes / 3;
	for ( i = 0 ; i < numTris ; i++ ) {
		int		i1, i2, i3;
		bvec3_t	d1, d2, normal;
		bfixed	*v1, *v2, *v3;
		bfixed	d;

		i1 = tess.indexes[ i*3 + 0 ];
		i2 = tess.indexes[ i*3 + 1 ];
		i3 = tess.indexes[ i*3 + 2 ];

		v1 = tess.xyz[ i1 ];
		v2 = tess.xyz[ i2 ];
		v3 = tess.xyz[ i3 ];

		VectorSubtract( v2, v1, d1 );
		VectorSubtract( v3, v1, d2 );
		CrossProduct( d1, d2, normal );

		d = FIXED_VEC3DOT( normal, lightDir );
		if ( d > BFIXED_0 ) {
			facing[ i ] = 1;
		} else {
			facing[ i ] = 0;
		}

		// create the edges
		R_AddEdgeDef( i1, i2, facing[ i ] );
		R_AddEdgeDef( i2, i3, facing[ i ] );
		R_AddEdgeDef( i3, i1, facing[ i ] );
	}

	// draw the silhouette edges

	GL_Bind( tr.whiteImage );
	glEnable( GL_CULL_FACE );
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );
	glColor3X( GFIXED(0,2), GFIXED(0,2), GFIXED(0,2) );

	// don't write to the color buffer
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

	glEnable( GL_STENCIL_TEST );
	glStencilFunc( GL_ALWAYS, 1, 255 );

	// mirrors have the culling order reversed
	if ( backEnd.viewParms.isMirror ) {
		glCullFace( GL_FRONT );
		glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );

		R_RenderShadowEdges();

		glCullFace( GL_BACK );
		glStencilOp( GL_KEEP, GL_KEEP, GL_DECR );

		R_RenderShadowEdges();
	} else {
		glCullFace( GL_BACK );
		glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );

		R_RenderShadowEdges();

		glCullFace( GL_FRONT );
		glStencilOp( GL_KEEP, GL_KEEP, GL_DECR );

		R_RenderShadowEdges();
	}


	// reenable writing to the color buffer
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
}


/*
=================
RB_ShadowFinish

Darken everything that is is a shadow volume.
We have to delay this until everything has been shadowed,
because otherwise shadows from different body parts would
overlap and lfixed darken.
=================
*/
void RB_ShadowFinish( void ) {
	if ( r_shadows->integer != 2 ) {
		return;
	}
	if ( glConfig.stencilBits < 4 ) {
		return;
	}
	glEnable( GL_STENCIL_TEST );
	glStencilFunc( GL_NOTEQUAL, 0, 255 );

	glDisable (GL_CLIP_PLANE0);
	glDisable (GL_CULL_FACE);

	GL_Bind( tr.whiteImage );

    glLoadIdentity ();

	glColor3X( GFIXED(0,6), GFIXED(0,6), GFIXED(0,6));
	GL_State( GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO );

//	glColor3X( GFIXED_1, GFIXED_0, GFIXED_0 );
//	GL_State( GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );

	glBegin( GL_QUADS );
	glVertex3X( GFIXED(-100,0), GFIXED(100,0), GFIXED(-10,0) );
	glVertex3X( GFIXED(100,0), GFIXED(100,0), GFIXED(-10,0) );
	glVertex3X( GFIXED(100,0), GFIXED(-100,0), GFIXED(-10,0) );
	glVertex3X( GFIXED(-100,0), GFIXED(-100,0), GFIXED(-10,0) );
	glEnd ();

	glColor4X(GFIXED_1,GFIXED_1,GFIXED_1,GFIXED_1);
	glDisable( GL_STENCIL_TEST );
}


/*
=================
RB_ProjectionShadowDeform

=================
*/
void RB_ProjectionShadowDeform( void ) {
	bfixed	*xyz;
	int		i;
	bfixed	h;
	avec3_t	ground;
	avec3_t	light;
	bfixed	groundDist;
	afixed	d;
	avec3_t	lightDir;

	xyz = ( bfixed * ) tess.xyz;

	ground[0] = backEnd._or.axis[0][2];
	ground[1] = backEnd._or.axis[1][2];
	ground[2] = backEnd._or.axis[2][2];

	groundDist = backEnd._or.origin[2] - backEnd.currentEntity->e.shadowPlane;

	VectorCopy( backEnd.currentEntity->lightDir, lightDir );
	d = FIXED_VEC3DOT( lightDir, ground );
	// don't let the shadows get too long or go negative
	if ( d < AFIXED(0,5) ) {
		FIXED_VEC3MA( lightDir, (AFIXED(0,5) - d), ground, lightDir );
		d = FIXED_VEC3DOT( lightDir, ground );
	}
	d = AFIXED_1 / d;

	light[0] = lightDir[0] * d;
	light[1] = lightDir[1] * d;
	light[2] = lightDir[2] * d;

	for ( i = 0; i < tess.numVertexes; i++, xyz += 4 ) {
		h = FIXED_VEC3DOT( xyz, ground ) + groundDist;

		xyz[0] -= light[0] * h;
		xyz[1] -= light[1] * h;
		xyz[2] -= light[2] * h;
	}
}
