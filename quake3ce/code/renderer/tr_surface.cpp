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
// tr_surf.c
#include"renderer_pch.h"

/*

  THIS ENTIRE FILE IS BACK END

backEnd.currentEntity will be valid.

Tess_Begin has already been called for the surface's shader.

The modelview matrix will be set.

It is safe to actually issue drawing commands here if you don't want to
use the shader system.
*/


//============================================================================


/*
==============
RB_CheckOverflow
==============
*/
void RB_CheckOverflow( int verts, int indexes ) {
	if (tess.numVertexes + verts < SHADER_MAX_VERTEXES
		&& tess.numIndexes + indexes < SHADER_MAX_INDEXES) {
		return;
	}

	RB_EndSurface();

	if ( verts >= SHADER_MAX_VERTEXES ) {
		ri.Error(ERR_DROP, "RB_CheckOverflow: verts > MAX (%d > %d)", verts, SHADER_MAX_VERTEXES );
	}
	if ( indexes >= SHADER_MAX_INDEXES ) {
		ri.Error(ERR_DROP, "RB_CheckOverflow: indices > MAX (%d > %d)", indexes, SHADER_MAX_INDEXES );
	}

	RB_BeginSurface(tess.shader, tess.fogNum );
}


/*
==============
RB_AddQuadStampExt
==============
*/
void RB_AddQuadStampExt( bvec3_t origin, bvec3_t left, bvec3_t up, byte *color, gfixed s1, gfixed t1, gfixed s2, gfixed t2 ) {
	avec3_t		normal;
	int			ndx;

	RB_CHECKOVERFLOW( 4, 6 );

	ndx = tess.numVertexes;

	// triangle indexes for a simple quad
	tess.indexes[ tess.numIndexes ] = ndx;
	tess.indexes[ tess.numIndexes + 1 ] = ndx + 1;
	tess.indexes[ tess.numIndexes + 2 ] = ndx + 3;

	tess.indexes[ tess.numIndexes + 3 ] = ndx + 3;
	tess.indexes[ tess.numIndexes + 4 ] = ndx + 1;
	tess.indexes[ tess.numIndexes + 5 ] = ndx + 2;

	tess.xyz[ndx][0] = origin[0] + left[0] + up[0];
	tess.xyz[ndx][1] = origin[1] + left[1] + up[1];
	tess.xyz[ndx][2] = origin[2] + left[2] + up[2];

	tess.xyz[ndx+1][0] = origin[0] - left[0] + up[0];
	tess.xyz[ndx+1][1] = origin[1] - left[1] + up[1];
	tess.xyz[ndx+1][2] = origin[2] - left[2] + up[2];

	tess.xyz[ndx+2][0] = origin[0] - left[0] - up[0];
	tess.xyz[ndx+2][1] = origin[1] - left[1] - up[1];
	tess.xyz[ndx+2][2] = origin[2] - left[2] - up[2];

	tess.xyz[ndx+3][0] = origin[0] + left[0] - up[0];
	tess.xyz[ndx+3][1] = origin[1] + left[1] - up[1];
	tess.xyz[ndx+3][2] = origin[2] + left[2] - up[2];


	// constant normal all the way around
	VectorSubtract(avec3_origin, backEnd.viewParms.or.axis[0], normal );

	tess.normal[ndx][0] = tess.normal[ndx+1][0] = tess.normal[ndx+2][0] = tess.normal[ndx+3][0] = normal[0];
	tess.normal[ndx][1] = tess.normal[ndx+1][1] = tess.normal[ndx+2][1] = tess.normal[ndx+3][1] = normal[1];
	tess.normal[ndx][2] = tess.normal[ndx+1][2] = tess.normal[ndx+2][2] = tess.normal[ndx+3][2] = normal[2];
	
	// standard square texture coordinates
	tess.texCoords[ndx][0][0] = tess.texCoords[ndx][1][0] = s1;
	tess.texCoords[ndx][0][1] = tess.texCoords[ndx][1][1] = t1;

	tess.texCoords[ndx+1][0][0] = tess.texCoords[ndx+1][1][0] = s2;
	tess.texCoords[ndx+1][0][1] = tess.texCoords[ndx+1][1][1] = t1;

	tess.texCoords[ndx+2][0][0] = tess.texCoords[ndx+2][1][0] = s2;
	tess.texCoords[ndx+2][0][1] = tess.texCoords[ndx+2][1][1] = t2;

	tess.texCoords[ndx+3][0][0] = tess.texCoords[ndx+3][1][0] = s1;
	tess.texCoords[ndx+3][0][1] = tess.texCoords[ndx+3][1][1] = t2;

	// constant color all the way around
	// should this be identity and let the shader specify from entity?
	* ( unsigned int * ) &tess.vertexColors[ndx] = 
	* ( unsigned int * ) &tess.vertexColors[ndx+1] = 
	* ( unsigned int * ) &tess.vertexColors[ndx+2] = 
	* ( unsigned int * ) &tess.vertexColors[ndx+3] = 
		* ( unsigned int * )color;


	tess.numVertexes += 4;
	tess.numIndexes += 6;
}

/*
==============
RB_AddQuadStamp
==============
*/
void RB_AddQuadStamp( bvec3_t origin, bvec3_t left, bvec3_t up, byte *color ) {
	RB_AddQuadStampExt( origin, left, up, color, GFIXED_0, GFIXED_0, GFIXED_1, GFIXED_1 );
}

/*
==============
RB_SurfaceSprite
==============
*/
static void RB_SurfaceSprite( void ) {
	bvec3_t		left, up;
	bfixed		radius;

	// calculate the xyz locations for the four corners
	radius = backEnd.currentEntity->e.radius;
	if ( backEnd.currentEntity->e.rotation == AFIXED_0 ) {
		FIXED_VEC3SCALE_R( backEnd.viewParms.or.axis[1], radius, left );
		FIXED_VEC3SCALE_R( backEnd.viewParms.or.axis[2], radius, up );
	} else {
		afixed	s, c;
		afixed	ang;
		
		ang = AFIXED_PI * backEnd.currentEntity->e.rotation / AFIXED(180,0);
		s = FIXED_SIN( ang );
		c = FIXED_COS( ang );

		FIXED_VEC3SCALE_R( backEnd.viewParms.or.axis[1], c * radius, left );
		FIXED_VEC3MA_R( left, -s * radius, backEnd.viewParms.or.axis[2], left );

		FIXED_VEC3SCALE_R( backEnd.viewParms.or.axis[2], c * radius, up );
		FIXED_VEC3MA_R( up, s * radius, backEnd.viewParms.or.axis[1], up );
	}
	if ( backEnd.viewParms.isMirror ) {
		VectorSubtract( bvec3_origin, left, left );
	}

	RB_AddQuadStamp( backEnd.currentEntity->e.origin, left, up, backEnd.currentEntity->e.shaderRGBA );
}


/*
=============
RB_SurfacePolychain
=============
*/
void RB_SurfacePolychain( srfPoly_t *p ) {
	int		i;
	int		numv;

	RB_CHECKOVERFLOW( p->numVerts, 3*(p->numVerts - 2) );

	// fan triangles into the tess array
	numv = tess.numVertexes;
	for ( i = 0; i < p->numVerts; i++ ) {
		VectorCopy( p->verts[i].xyz, tess.xyz[numv] );
		tess.texCoords[numv][0][0] = p->verts[i].st[0];
		tess.texCoords[numv][0][1] = p->verts[i].st[1];
		*(int *)&tess.vertexColors[numv] = *(int *)p->verts[ i ].modulate;

		numv++;
	}

	// generate fan indexes into the tess array
	for ( i = 0; i < p->numVerts-2; i++ ) {
		tess.indexes[tess.numIndexes + 0] = tess.numVertexes;
		tess.indexes[tess.numIndexes + 1] = tess.numVertexes + i + 1;
		tess.indexes[tess.numIndexes + 2] = tess.numVertexes + i + 2;
		tess.numIndexes += 3;
	}

	tess.numVertexes = numv;
}


/*
=============
RB_SurfaceTriangles
=============
*/
void RB_SurfaceTriangles( srfTriangles_t *srf ) {
	int			i;
	drawVert_t	*dv;
	bfixed		*xyz;
	afixed		*normal;
	gfixed		*texCoords;
	byte		*color;
	int			dlightBits;
	qboolean	needsNormal;

	dlightBits = srf->dlightBits[backEnd.smpFrame];
	tess.dlightBits |= dlightBits;

	RB_CHECKOVERFLOW( srf->numVerts, srf->numIndexes );

	for ( i = 0 ; i < srf->numIndexes ; i += 3 ) {
		tess.indexes[ tess.numIndexes + i + 0 ] = tess.numVertexes + srf->indexes[ i + 0 ];
		tess.indexes[ tess.numIndexes + i + 1 ] = tess.numVertexes + srf->indexes[ i + 1 ];
		tess.indexes[ tess.numIndexes + i + 2 ] = tess.numVertexes + srf->indexes[ i + 2 ];
	}
	tess.numIndexes += srf->numIndexes;

	dv = srf->verts;
	xyz = tess.xyz[ tess.numVertexes ];
	normal = tess.normal[ tess.numVertexes ];
	texCoords = tess.texCoords[ tess.numVertexes ][0];
	color = tess.vertexColors[ tess.numVertexes ];
	needsNormal = tess.shader->needsNormal;

	for ( i = 0 ; i < srf->numVerts ; i++, dv++, xyz += 4, normal += 4, texCoords += 4, color += 4 ) {
		xyz[0] = dv->xyz[0];
		xyz[1] = dv->xyz[1];
		xyz[2] = dv->xyz[2];

		if ( needsNormal ) {
			normal[0] = dv->normal[0];
			normal[1] = dv->normal[1];
			normal[2] = dv->normal[2];
		}

		texCoords[0] = dv->st[0];
		texCoords[1] = dv->st[1];

		texCoords[2] = dv->lightmap[0];
		texCoords[3] = dv->lightmap[1];

		*(int *)color = *(int *)dv->color;
	}

	for ( i = 0 ; i < srf->numVerts ; i++ ) {
		tess.vertexDlightBits[ tess.numVertexes + i] = dlightBits;
	}

	tess.numVertexes += srf->numVerts;
}



/*
==============
RB_SurfaceBeam
==============
*/
void RB_SurfaceBeam( void ) 
{
#define NUM_BEAM_SEGS 6
	refEntity_t *e;
	int	i;
	bvec3_t perpvec;
	avec3_t tmp;
	bvec3_t direction;
	avec3_t normalized_direction;
	bvec3_t	start_points[NUM_BEAM_SEGS], end_points[NUM_BEAM_SEGS];
	bvec3_t oldorigin, origin;

	e = &backEnd.currentEntity->e;

	oldorigin[0] = e->oldorigin[0];
	oldorigin[1] = e->oldorigin[1];
	oldorigin[2] = e->oldorigin[2];

	origin[0] = e->origin[0];
	origin[1] = e->origin[1];
	origin[2] = e->origin[2];

	direction[0] = oldorigin[0] - origin[0];
	direction[1] = oldorigin[1] - origin[1];
	direction[2] = oldorigin[2] - origin[2];

	if ( VectorNormalizeB2A( direction, normalized_direction ) == BFIXED_0 )
		return;

	PerpendicularVector( tmp, normalized_direction );

	FIXED_VEC3SCALE_R( tmp, BFIXED(4,0), perpvec );

	for ( i = 0; i < NUM_BEAM_SEGS ; i++ )
	{
		RotatePointAroundVector( start_points[i], normalized_direction, perpvec, FIXED_INT32RATIO_A(360*i,NUM_BEAM_SEGS));
//		VectorAdd( start_points[i], origin, start_points[i] );
		VectorAdd( start_points[i], direction, end_points[i] );
	}

	GL_Bind( tr.whiteImage );

	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );

	glColor3x( GLFIXED_1, GLFIXED_0, GLFIXED_0);

	glBegin( GL_TRIANGLE_STRIP );
	for ( i = 0; i <= NUM_BEAM_SEGS; i++ ) {
		bfixed *sp=start_points[i % NUM_BEAM_SEGS];
		bfixed *ep=end_points[i % NUM_BEAM_SEGS];
		glVertex3X( REINTERPRET_GFIXED(sp[0]),REINTERPRET_GFIXED(sp[1]),REINTERPRET_GFIXED(sp[2]) );
		glVertex3X( REINTERPRET_GFIXED(ep[0]),REINTERPRET_GFIXED(ep[1]),REINTERPRET_GFIXED(ep[2]) );
	}
	glEnd();
}

//================================================================================

static void DoRailCore( const bvec3_t start, const bvec3_t end, const avec3_t up, bfixed len, bfixed spanWidth )
{
	bfixed		spanWidth2;
	int			vbase;
	gfixed		t = FIXED_DIVPOW2(MAKE_GFIXED(len),8);

	vbase = tess.numVertexes;

	spanWidth2 = -spanWidth;

	// FIXME: use quad stamp?
	FIXED_VEC3MA_R( start, spanWidth, up, tess.xyz[tess.numVertexes] );
	tess.texCoords[tess.numVertexes][0][0] = GFIXED_0;
	tess.texCoords[tess.numVertexes][0][1] = GFIXED_0;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0]>>2;
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1]>>2;
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2]>>2;
	tess.numVertexes++;

	FIXED_VEC3MA_R( start, spanWidth2, up, tess.xyz[tess.numVertexes] );
	tess.texCoords[tess.numVertexes][0][0] = GFIXED_0;
	tess.texCoords[tess.numVertexes][0][1] = GFIXED_1;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
	tess.numVertexes++;

	FIXED_VEC3MA_R( end, spanWidth, up, tess.xyz[tess.numVertexes] );

	tess.texCoords[tess.numVertexes][0][0] = t;
	tess.texCoords[tess.numVertexes][0][1] = GFIXED_0;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
	tess.numVertexes++;

	FIXED_VEC3MA_R( end, spanWidth2, up, tess.xyz[tess.numVertexes] );
	tess.texCoords[tess.numVertexes][0][0] = t;
	tess.texCoords[tess.numVertexes][0][1] = GFIXED_1;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
	tess.numVertexes++;

	tess.indexes[tess.numIndexes++] = vbase;
	tess.indexes[tess.numIndexes++] = vbase + 1;
	tess.indexes[tess.numIndexes++] = vbase + 2;

	tess.indexes[tess.numIndexes++] = vbase + 2;
	tess.indexes[tess.numIndexes++] = vbase + 1;
	tess.indexes[tess.numIndexes++] = vbase + 3;
}

static void DoRailDiscs( int numSegs, const bvec3_t start, const avec3_t dir, const avec3_t right, const avec3_t up )
{
	int i;
	bvec3_t	pos[4];
	bvec3_t	v;
	int		spanWidth = r_railWidth->integer;
	afixed c, s;
	//gfixed		scale;

	if ( numSegs > 1 )
		numSegs--;
	if ( !numSegs )
		return;

	//scale = GFIXED(0,25);

	for ( i = 0; i < 4; i++ )
	{
		c = FIXED_COS( DEG2RAD_A( AFIXED(45 ,0)+ MAKE_AFIXED(i) * AFIXED(90 ,0)) );
		s = FIXED_SIN( DEG2RAD_A( AFIXED(45 ,0)+ MAKE_AFIXED(i) * AFIXED(90 ,0)) );
		v[0] = FIXED_DIVPOW2(( right[0] * c + up[0] * s ) * MAKE_BFIXED(spanWidth),2);
		v[1] = FIXED_DIVPOW2(( right[1] * c + up[1] * s ) * MAKE_BFIXED(spanWidth),2);
		v[2] = FIXED_DIVPOW2(( right[2] * c + up[2] * s ) * MAKE_BFIXED(spanWidth),2);
		VectorAdd( start, v, pos[i] );

		if ( numSegs > 1 )
		{
			// offset by 1 segment if we're doing a long distance shot
			//VectorAdd( pos[i], dir, pos[i] );
			pos[i][0]+=MAKE_BFIXED(dir[0]);
			pos[i][1]+=MAKE_BFIXED(dir[1]);
			pos[i][2]+=MAKE_BFIXED(dir[2]);
		}
	}

	for ( i = 0; i < numSegs; i++ )
	{
		int j;

		RB_CHECKOVERFLOW( 4, 6 );

		for ( j = 0; j < 4; j++ )
		{
			VectorCopy( pos[j], tess.xyz[tess.numVertexes] );
			tess.texCoords[tess.numVertexes][0][0] = MAKE_GFIXED( j < 2 );
			tess.texCoords[tess.numVertexes][0][1] = MAKE_GFIXED( j && j != 3 );
			tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
			tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
			tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
			tess.numVertexes++;

			pos[j][0]+=MAKE_BFIXED(dir[0]);
			pos[j][1]+=MAKE_BFIXED(dir[1]);
			pos[j][2]+=MAKE_BFIXED(dir[2]);
			//VectorAdd( pos[j], dir, pos[j] );
		}

		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 0;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 1;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 3;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 3;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 1;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 2;
	}
}

/*
** RB_SurfaceRailRinges
*/
void RB_SurfaceRailRings( void ) {
	refEntity_t *e;
	int			numSegs;
	int			len;
	avec3_t		vec;
	bvec3_t		tmp;
	avec3_t		right, up;
	bvec3_t		start, end;

	e = &backEnd.currentEntity->e;

	VectorCopy( e->oldorigin, start );
	VectorCopy( e->origin, end );

	// compute variables
	VectorSubtract( end, start, tmp );
	len = FIXED_TO_INT(VectorNormalizeB2A(tmp, vec ));
	MakeNormalVectors( vec, right, up );
	numSegs = FIXED_TO_INT(MAKE_LFIXED( len ) / r_railSegmentLength->value);
	if ( numSegs <= 0 ) {
		numSegs = 1;
	}

	FIXED_VEC3SCALE( vec, MAKE_BFIXED(r_railSegmentLength->value), vec );

	DoRailDiscs( numSegs, start, vec, right, up );
}

/*
** RB_SurfaceRailCore
*/
void RB_SurfaceRailCore( void ) {
	refEntity_t *e;
	int			len;
	avec3_t		right;
	bvec3_t		vec;
	bvec3_t		tmp;
	bvec3_t		start, end;
	bvec3_t		v1, v2;

	e = &backEnd.currentEntity->e;

	VectorCopy( e->oldorigin, start );
	VectorCopy( e->origin, end );

	VectorSubtract( end, start, vec );
	len = FIXED_TO_INT(VectorNormalize( vec ));

	// compute side vector
	VectorSubtract( start, backEnd.viewParms.or.origin, v1 );
	VectorNormalize( v1 );
	VectorSubtract( end, backEnd.viewParms.or.origin, v2 );
	VectorNormalize( v2 );
	CrossProduct( v1, v2, tmp );
	VectorNormalizeB2A( tmp, right );

	DoRailCore( start, end, right, MAKE_BFIXED(len), MAKE_BFIXED(r_railCoreWidth->integer) );
}

/*
** RB_SurfaceLightningBolt
*/
void RB_SurfaceLightningBolt( void ) {
	refEntity_t *e;
	int			len;
	avec3_t		vec, right;
	bvec3_t		tmp;
	bvec3_t		start, end;
	avec3_t		v1, v2;
	int			i;

	e = &backEnd.currentEntity->e;

	VectorCopy( e->oldorigin, end );
	VectorCopy( e->origin, start );

	// compute variables
	VectorSubtract( end, start, tmp );
	len = FIXED_TO_INT(VectorNormalizeB2A(tmp, vec ));

	// compute side vector
	VectorSubtract( start, backEnd.viewParms.or.origin, tmp);
	VectorNormalizeB2A(tmp, v1 );
	VectorSubtract( end, backEnd.viewParms.or.origin, tmp );
	VectorNormalizeB2A(tmp, v2 );
	CrossProduct( v1, v2, right );
	VectorNormalize( right );

	for ( i = 0 ; i < 4 ; i++ ) {
		avec3_t	temp;

		DoRailCore( start, end, right, MAKE_BFIXED(len), BFIXED(8 ,0));
		RotatePointAroundVector( temp, vec, right, AFIXED(45 ,0));
		VectorCopy( temp, right );
	}
}

/*
** VectorArrayNormalize
*
* The inputs to this routing seem to always be close to length = GFIXED_1 (about GFIXED(0,6) to GFIXED(2,0))
* This means that we don't have to worry about zero length or enormously long vectors.
*/
static void VectorArrayNormalize(vec4_t *normals, unsigned int count)
{
//    assert(count);
        
#if idppc
    {
        register gfixed half = GFIXED(0,5);
        register gfixed one  = GFIXED_1;
        gfixed *components = (gfixed *)normals;
        
        // Vanilla PPC code, but since PPC has a reciprocal square root estimate instruction,
        // runs *much* faster than calling sqrt().  We'll use a single Newton-Raphson
        // refinement step to get a little more precision.  This seems to yeild results
        // that are correct to 3 decimal places and usually correct to at least 4 (sometimes 5).
        // (That is, for the given input range of about GFIXED(0,6) to GFIXED(2,0)).
        do {
            gfixed x, y, z;
            gfixed B, y0, y1;
            
            x = components[0];
            y = components[1];
            z = components[2];
            components += 4;
            B = x*x + y*y + z*z;

#ifdef __GNUC__            
            asm("frsqrte %0,%1" : "=f" (y0) : "f" (B));
#else
			y0 = __frsqrte(B);
#endif
            y1 = y0 + half*y0*(one - B*y0*y0);

            x = x * y1;
            y = y * y1;
            components[-4] = x;
            z = z * y1;
            components[-3] = y;
            components[-2] = z;
        } while(count--);
    }
#else // No assembly version for this architecture, or C_ONLY defined
	// given the input, it's safe to call FIXED_FAST_VEC3NORM
    while (count--) {
        FIXED_FASTVEC3NORM(normals[0]);
        normals++;
    }
#endif

}



/*
** LerpMeshVertexes
*/
static void LerpMeshVertexes (md3Surface_t *surf, gfixed backlerp) 
{
	short	*oldXyz, *newXyz, *oldNormals, *newNormals;
	bfixed	*outXyz;
	afixed	*outNormal;
	gfixed	oldXyzScale, newXyzScale;
	gfixed	oldNormalScale, newNormalScale;
	int		vertNum;
	unsigned lat, lng;
	int		numVerts;

	outXyz = tess.xyz[tess.numVertexes];
	outNormal = tess.normal[tess.numVertexes];

	newXyz = (short *)((byte *)surf + surf->ofsXyzNormals)
		+ (backEnd.currentEntity->e.frame * surf->numVerts * 4);
	newNormals = newXyz + 3;

	newXyzScale = MD3_XYZ_SCALE * (GFIXED_1 - backlerp);
	newNormalScale = (GFIXED_1 - backlerp);

	numVerts = surf->numVerts;

	if ( backlerp == GFIXED_0 ) {
		//
		// just copy the vertexes
		//
		for (vertNum=0 ; vertNum < numVerts ; vertNum++,
			newXyz += 4, newNormals += 4,
			outXyz += 4, outNormal += 4) 
		{

			outXyz[0] = MAKE_BFIXED(newXyz[0]) * newXyzScale;
			outXyz[1] = MAKE_BFIXED(newXyz[1]) * newXyzScale;
			outXyz[2] = MAKE_BFIXED(newXyz[2]) * newXyzScale;

			lat = ( newNormals[0] >> 8 ) & 0xff;
			lng = ( newNormals[0] & 0xff );
			lat *= (FUNCTABLE_SIZE/256);
			lng *= (FUNCTABLE_SIZE/256);

			// decode X as cos( lat ) * sin( long )
			// decode Y as sin( lat ) * sin( long )
			// decode Z as cos( long )

			outNormal[0] = MAKE_AFIXED(tr.sinTable[(lat+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK] * tr.sinTable[lng]);
			outNormal[1] = MAKE_AFIXED(tr.sinTable[lat] * tr.sinTable[lng]);
			outNormal[2] = MAKE_AFIXED(tr.sinTable[(lng+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK]);
		}
	} else {
		//
		// interpolate and copy the vertex and normal
		//
		oldXyz = (short *)((byte *)surf + surf->ofsXyzNormals)
			+ (backEnd.currentEntity->e.oldframe * surf->numVerts * 4);
		oldNormals = oldXyz + 3;

		oldXyzScale = MD3_XYZ_SCALE * backlerp;
		oldNormalScale = backlerp;

		for (vertNum=0 ; vertNum < numVerts ; vertNum++,
			oldXyz += 4, newXyz += 4, oldNormals += 4, newNormals += 4,
			outXyz += 4, outNormal += 4) 
		{
			avec3_t uncompressedOldNormal, uncompressedNewNormal;

			// interpolate the xyz
			outXyz[0] = MAKE_BFIXED(oldXyz[0]) * oldXyzScale + MAKE_BFIXED(newXyz[0]) * newXyzScale;
			outXyz[1] = MAKE_BFIXED(oldXyz[1]) * oldXyzScale + MAKE_BFIXED(newXyz[1]) * newXyzScale;
			outXyz[2] = MAKE_BFIXED(oldXyz[2]) * oldXyzScale + MAKE_BFIXED(newXyz[2]) * newXyzScale;

			// FIXME: interpolate lat/long instead?
			lat = ( newNormals[0] >> 8 ) & 0xff;
			lng = ( newNormals[0] & 0xff );
			lat *= 4;
			lng *= 4;
			uncompressedNewNormal[0] = MAKE_AFIXED(tr.sinTable[(lat+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK] * tr.sinTable[lng]);
			uncompressedNewNormal[1] = MAKE_AFIXED(tr.sinTable[lat] * tr.sinTable[lng]);
			uncompressedNewNormal[2] = MAKE_AFIXED(tr.sinTable[(lng+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK]);

			lat = ( oldNormals[0] >> 8 ) & 0xff;
			lng = ( oldNormals[0] & 0xff );
			lat *= 4;
			lng *= 4;

			uncompressedOldNormal[0] = MAKE_AFIXED(tr.sinTable[(lat+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK] * tr.sinTable[lng]);
			uncompressedOldNormal[1] = MAKE_AFIXED(tr.sinTable[lat] * tr.sinTable[lng]);
			uncompressedOldNormal[2] = MAKE_AFIXED(tr.sinTable[(lng+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK]);

			outNormal[0] = MAKE_AFIXED(uncompressedOldNormal[0] * oldNormalScale) + MAKE_AFIXED(uncompressedNewNormal[0] * newNormalScale);
			outNormal[1] = MAKE_AFIXED(uncompressedOldNormal[1] * oldNormalScale) + MAKE_AFIXED(uncompressedNewNormal[1] * newNormalScale);
			outNormal[2] = MAKE_AFIXED(uncompressedOldNormal[2] * oldNormalScale) + MAKE_AFIXED(uncompressedNewNormal[2] * newNormalScale);

//			VectorNormalize (outNormal);
		}
    	VectorArrayNormalize((vec4_t *)tess.normal[tess.numVertexes], numVerts);
   	}
}

/*
=============
RB_SurfaceMesh
=============
*/
void RB_SurfaceMesh(md3Surface_t *surface) {
	int				j;
	gfixed			backlerp;
	int				*triangles;
	gfixed			*texCoords;
	int				indexes;
	int				Bob, Doug;
	int				numVerts;

	if (  backEnd.currentEntity->e.oldframe == backEnd.currentEntity->e.frame ) {
		backlerp = GFIXED_0;
	} else  {
		backlerp = backEnd.currentEntity->e.backlerp;
	}

	RB_CHECKOVERFLOW( surface->numVerts, surface->numTriangles*3 );

	LerpMeshVertexes (surface, backlerp);

	triangles = (int *) ((byte *)surface + surface->ofsTriangles);
	indexes = surface->numTriangles * 3;
	Bob = tess.numIndexes;
	Doug = tess.numVertexes;
	for (j = 0 ; j < indexes ; j++) {
		tess.indexes[Bob + j] = Doug + triangles[j];
	}
	tess.numIndexes += indexes;

	texCoords = (gfixed *) ((byte *)surface + surface->ofsSt);

	numVerts = surface->numVerts;
	for ( j = 0; j < numVerts; j++ ) {
		tess.texCoords[Doug + j][0][0] = texCoords[j*2+0];
		tess.texCoords[Doug + j][0][1] = texCoords[j*2+1];
		// FIXME: fill in lightmapST for completeness?
	}

	tess.numVertexes += surface->numVerts;

}


/*
==============
RB_SurfaceFace
==============
*/
void RB_SurfaceFace( srfSurfaceFace_t *surf ) {
	int			i;
	unsigned  *indices;
	glIndex_t *tessIndexes;
	fixed32		*v;
	afixed		*normal;
	int			ndx;
	int			Bob;
	int			numPoints;
	int			dlightBits;

	RB_CHECKOVERFLOW( surf->numPoints, surf->numIndices );

	dlightBits = surf->dlightBits[backEnd.smpFrame];
	tess.dlightBits |= dlightBits;

	indices = ( unsigned * ) ( ( ( char  * ) surf ) + surf->ofsIndices );

	Bob = tess.numVertexes;
	tessIndexes = tess.indexes + tess.numIndexes;
	for ( i = surf->numIndices-1 ; i >= 0  ; i-- ) {
		tessIndexes[i] = (unsigned)(unsigned short)(indices[i] + Bob);
	}

	tess.numIndexes += surf->numIndices;

	v = surf->points[0];

	ndx = tess.numVertexes;

	numPoints = surf->numPoints;

	if ( tess.shader->needsNormal ) {
		normal = surf->plane.normal;
		for ( i = 0, ndx = tess.numVertexes; i < numPoints; i++, ndx++ ) {
			VectorCopy( normal, tess.normal[ndx] );
		}
	}

	for ( i = 0, v = surf->points[0], ndx = tess.numVertexes; i < numPoints; i++, v += VERTEXSIZE, ndx++ ) {
		VectorCopy( (bfixed *)v, tess.xyz[ndx]);
		tess.texCoords[ndx][0][0] = v[3].g;
		tess.texCoords[ndx][0][1] = v[4].g;
		tess.texCoords[ndx][1][0] = v[5].g;
		tess.texCoords[ndx][1][1] = v[6].g;
		* ( unsigned int * ) &tess.vertexColors[ndx] = v[7].rep;
		tess.vertexDlightBits[ndx] = dlightBits;
	}


	tess.numVertexes += surf->numPoints;
}


static bfixed	LodErrorForVolume( bvec3_t local, bfixed radius ) {
	bvec3_t		world;
	bfixed		d;

	// never let it go negative
	if ( r_lodCurveError->value < LFIXED_0 ) {
		return BFIXED_0;
	}

	world[0] = local[0] * backEnd.or.axis[0][0] + local[1] * backEnd.or.axis[1][0] + 
		local[2] * backEnd.or.axis[2][0] + backEnd.or.origin[0];
	world[1] = local[0] * backEnd.or.axis[0][1] + local[1] * backEnd.or.axis[1][1] + 
		local[2] * backEnd.or.axis[2][1] + backEnd.or.origin[1];
	world[2] = local[0] * backEnd.or.axis[0][2] + local[1] * backEnd.or.axis[1][2] + 
		local[2] * backEnd.or.axis[2][2] + backEnd.or.origin[2];

	VectorSubtract( world, backEnd.viewParms.or.origin, world );
	d = FIXED_VEC3DOT( world, backEnd.viewParms.or.axis[0] );

	if ( d < BFIXED_0 ) {
		d = -d;
	}
	d -= radius;
	if ( d < BFIXED_1 ) {
		d = BFIXED_1;
	}

	return MAKE_BFIXED(r_lodCurveError->value) / d;
}

/*
=============
RB_SurfaceGrid

Just copy the grid of points and triangulate
=============
*/
void RB_SurfaceGrid( srfGridMesh_t *cv ) {
	int		i, j;
	bfixed	*xyz;
	gfixed	*texCoords;
	afixed	*normal;
	unsigned char *color;
	drawVert_t	*dv;
	int		rows, irows, vrows;
	int		used;
	int		widthTable[MAX_GRID_SIZE];
	int		heightTable[MAX_GRID_SIZE];
	bfixed	lodError;
	int		lodWidth, lodHeight;
	int		numVertexes;
	int		dlightBits;
	int		*vDlightBits;
	qboolean	needsNormal;

	dlightBits = cv->dlightBits[backEnd.smpFrame];
	tess.dlightBits |= dlightBits;

	// determine the allowable discrepance
	lodError = LodErrorForVolume( cv->lodOrigin, cv->lodRadius );

	// determine which rows and columns of the subdivision
	// we are actually going to use
	widthTable[0] = 0;
	lodWidth = 1;
	for ( i = 1 ; i < cv->width-1 ; i++ ) {
		if ( cv->widthLodError[i] <= lodError ) {
			widthTable[lodWidth] = i;
			lodWidth++;
		}
	}
	widthTable[lodWidth] = cv->width-1;
	lodWidth++;

	heightTable[0] = 0;
	lodHeight = 1;
	for ( i = 1 ; i < cv->height-1 ; i++ ) {
		if ( cv->heightLodError[i] <= lodError ) {
			heightTable[lodHeight] = i;
			lodHeight++;
		}
	}
	heightTable[lodHeight] = cv->height-1;
	lodHeight++;


	// very large grids may have more points or indexes than can be fit
	// in the tess structure, so we may have to issue it in multiple passes

	used = 0;
	rows = 0;
	while ( used < lodHeight - 1 ) {
		// see how many rows of both verts and indexes we can add without overflowing
		do {
			vrows = ( SHADER_MAX_VERTEXES - tess.numVertexes ) / lodWidth;
			irows = ( SHADER_MAX_INDEXES - tess.numIndexes ) / ( lodWidth * 6 );

			// if we don't have enough space for at least one strip, flush the buffer
			if ( vrows < 2 || irows < 1 ) {
				RB_EndSurface();
				RB_BeginSurface(tess.shader, tess.fogNum );
			} else {
				break;
			}
		} while ( 1 );
		
		rows = irows;
		if ( vrows < irows + 1 ) {
			rows = vrows - 1;
		}
		if ( used + rows > lodHeight ) {
			rows = lodHeight - used;
		}

		numVertexes = tess.numVertexes;

		xyz = tess.xyz[numVertexes];
		normal = tess.normal[numVertexes];
		texCoords = tess.texCoords[numVertexes][0];
		color = ( unsigned char * ) &tess.vertexColors[numVertexes];
		vDlightBits = &tess.vertexDlightBits[numVertexes];
		needsNormal = tess.shader->needsNormal;

		for ( i = 0 ; i < rows ; i++ ) {
			for ( j = 0 ; j < lodWidth ; j++ ) {
				dv = cv->verts + heightTable[ used + i ] * cv->width
					+ widthTable[ j ];

				xyz[0] = dv->xyz[0];
				xyz[1] = dv->xyz[1];
				xyz[2] = dv->xyz[2];
				texCoords[0] = dv->st[0];
				texCoords[1] = dv->st[1];
				texCoords[2] = dv->lightmap[0];
				texCoords[3] = dv->lightmap[1];
				if ( needsNormal ) {
					normal[0] = dv->normal[0];
					normal[1] = dv->normal[1];
					normal[2] = dv->normal[2];
				}
				* ( unsigned int * ) color = * ( unsigned int * ) dv->color;
				*vDlightBits++ = dlightBits;
				xyz += 4;
				normal += 4;
				texCoords += 4;
				color += 4;
			}
		}


		// add the indexes
		{
			int		numIndexes;
			int		w, h;

			h = rows - 1;
			w = lodWidth - 1;
			numIndexes = tess.numIndexes;
			for (i = 0 ; i < h ; i++) {
				for (j = 0 ; j < w ; j++) {
					int		v1, v2, v3, v4;
			
					// vertex order to be reckognized as tristrips
					v1 = numVertexes + i*lodWidth + j + 1;
					v2 = v1 - 1;
					v3 = v2 + lodWidth;
					v4 = v3 + 1;

					tess.indexes[numIndexes] = v2;
					tess.indexes[numIndexes+1] = v3;
					tess.indexes[numIndexes+2] = v1;
					
					tess.indexes[numIndexes+3] = v1;
					tess.indexes[numIndexes+4] = v3;
					tess.indexes[numIndexes+5] = v4;
					numIndexes += 6;
				}
			}

			tess.numIndexes = numIndexes;
		}

		tess.numVertexes += rows * lodWidth;

		used += rows - 1;
	}
}


/*
===========================================================================

NULL MODEL

===========================================================================
*/

/*
===================
RB_SurfaceAxis

Draws x/y/z lines from the origin for orientation debugging
===================
*/
void RB_SurfaceAxis( void ) {
	GL_Bind( tr.whiteImage );
	glLineWidthx( 3 );
	glBegin( GL_LINES );
	glColor3X( GFIXED_1, GFIXED_0, GFIXED_0 );
	glVertex3X( GFIXED_0, GFIXED_0, GFIXED_0 );
	glVertex3X( REINTERPRET_GFIXED(BFIXED(16,0)), GFIXED_0, GFIXED_0 );
	glColor3X( GFIXED_0,GFIXED_1,GFIXED_0 );
	glVertex3X( GFIXED_0,GFIXED_0,GFIXED_0 );
	glVertex3X( GFIXED_0,REINTERPRET_GFIXED(BFIXED(16,0)),GFIXED_0 );
	glColor3X( GFIXED_0,GFIXED_0,GFIXED_1 );
	glVertex3X( GFIXED_0,GFIXED_0,GFIXED_0 );
	glVertex3X( GFIXED_0,GFIXED_0,REINTERPRET_GFIXED(BFIXED(16,0)) );
	glEnd();
	glLineWidthx( 1 );
}

//===========================================================================

/*
====================
RB_SurfaceEntity

Entities that have a single procedurally generated surface
====================
*/
void RB_SurfaceEntity( surfaceType_t *surfType ) {
	switch( backEnd.currentEntity->e.reType ) {
	case RT_SPRITE:
		RB_SurfaceSprite();
		break;
	case RT_BEAM:
		RB_SurfaceBeam();
		break;
	case RT_RAIL_CORE:
		RB_SurfaceRailCore();
		break;
	case RT_RAIL_RINGS:
		RB_SurfaceRailRings();
		break;
	case RT_LIGHTNING:
		RB_SurfaceLightningBolt();
		break;
	default:
		RB_SurfaceAxis();
		break;
	}
	return;
}

void RB_SurfaceBad( surfaceType_t *surfType ) {
	ri.Printf( PRINT_ALL, "Bad surface tesselated.\n" );
}

#if 0

void RB_SurfaceFlare( srfFlare_t *surf ) {
	bvec3_t		left, up;
	gfixed		radius;
	byte		color[4];
	bvec3_t		dir;
	bvec3_t		origin;
	gfixed		d;

	// calculate the xyz locations for the four corners
	radius = 30;
	FIXED_VEC3SCALE( backEnd.viewParms.or.axis[1], radius, left );
	FIXED_VEC3SCALE( backEnd.viewParms.or.axis[2], radius, up );
	if ( backEnd.viewParms.isMirror ) {
		VectorSubtract( vec3_origin, left, left );
	}

	color[0] = color[1] = color[2] = color[3] = 255;

	FIXED_VEC3MA( surf->origin, GFIXED(3,0), surf->normal, origin );
	VectorSubtract( origin, backEnd.viewParms.or.origin, dir );
	VectorNormalize( dir );
	FIXED_VEC3MA( origin, r_ignore->value, dir, origin );

	d = -FIXED_VEC3DOT( dir, surf->normal );
	if ( d < 0 ) {
		return;
	}
#if 0
	color[0] *= d;
	color[1] *= d;
	color[2] *= d;
#endif

	RB_AddQuadStamp( origin, left, up, color );
}

#else

void RB_SurfaceFlare( srfFlare_t *surf ) {
#if 0
	bvec3_t		left, up;
	byte		color[4];

	color[0] = surf->color[0] * 255;
	color[1] = surf->color[1] * 255;
	color[2] = surf->color[2] * 255;
	color[3] = 255;

	VectorClear( left );
	VectorClear( up );

	left[0] = r_ignore->value;

	up[1] = r_ignore->value;
	
	RB_AddQuadStampExt( surf->origin, left, up, color, 0, 0, 1, 1 );
#endif
}

#endif



void RB_SurfaceDisplayList( srfDisplayList_t *surf ) {
	// all apropriate state must be set in RB_BeginSurface
	// this isn't implemented yet...
//	glCallList( surf->listNum );
}

void RB_SurfaceSkip( void *surf ) {
}


void (*rb_surfaceTable[SF_NUM_SURFACE_TYPES])( void *) = {
	(void(*)(void*))RB_SurfaceBad,			// SF_BAD, 
	(void(*)(void*))RB_SurfaceSkip,			// SF_SKIP, 
	(void(*)(void*))RB_SurfaceFace,			// SF_FACE,
	(void(*)(void*))RB_SurfaceGrid,			// SF_GRID,
	(void(*)(void*))RB_SurfaceTriangles,	// SF_TRIANGLES,
	(void(*)(void*))RB_SurfacePolychain,	// SF_POLY,
	(void(*)(void*))RB_SurfaceMesh,			// SF_MD3,
	(void(*)(void*))RB_SurfaceAnim,			// SF_MD4,
	(void(*)(void*))RB_SurfaceFlare,		// SF_FLARE,
	(void(*)(void*))RB_SurfaceEntity,		// SF_ENTITY
	(void(*)(void*))RB_SurfaceDisplayList	// SF_DISPLAY_LIST
};
