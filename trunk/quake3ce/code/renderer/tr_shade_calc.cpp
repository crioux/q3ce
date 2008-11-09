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
// tr_shade_calc.c

#include"renderer_pch.h"


#define	WAVEVALUE( table, base, amplitude, phase, freq ) ((base) + table[ FIXED_INT32SCALE(FUNCTABLE_SIZE,(phase) + tess.shaderTime * (freq) ) & FUNCTABLE_MASK ] * (amplitude))


static afixed *TableForFunc( genFunc_t func ) 
{
	switch ( func )
	{
	case GF_SIN:
		return tr.sinTable;
	case GF_TRIANGLE:
		return tr.triangleTable;
	case GF_SQUARE:
		return tr.squareTable;
	case GF_SAWTOOTH:
		return tr.sawToothTable;
	case GF_INVERSE_SAWTOOTH:
		return tr.inverseSawToothTable;
	case GF_NONE:
	default:
		break;
	}

	ri.Error( ERR_DROP, "TableForFunc called with invalid function '%d' in shader '%s'\n", func, tess.shader->name );
	return NULL;
}

/*
** EvalWaveForm
**
** Evaluates a given waveForm_t, referencing backEnd.refdef.time directly
*/
static gfixed EvalWaveForm( const waveForm_t *wf ) 
{
	afixed	*table;

	table = TableForFunc( wf->func );

	return WAVEVALUE( table, wf->base, wf->amplitude, wf->phase, wf->frequency );
}

static gfixed EvalWaveFormClamped( const waveForm_t *wf )
{
	gfixed glow  = EvalWaveForm( wf );

	if ( glow < GFIXED_0 )
	{
		return GFIXED_0;
	}

	if ( glow > GFIXED_1 )
	{
		return GFIXED_1;
	}

	return glow;
}

/*
** RB_CalcStretchTexCoords
*/
void RB_CalcStretchTexCoords( const waveForm_t *wf, gfixed *st )
{
	gfixed p;
	texModInfo_t tmi;

	p = GFIXED_1 / EvalWaveForm( wf );

	tmi.matrix[0][0] = p;
	tmi.matrix[1][0] = GFIXED_0;
	tmi.translate[0] = GFIXED(0,5) - FIXED_DIVPOW2(p,1);

	tmi.matrix[0][1] = GFIXED_0;
	tmi.matrix[1][1] = p;
	tmi.translate[1] = GFIXED(0,5) - FIXED_DIVPOW2(p,1);

	RB_CalcTransformTexCoords( &tmi, st );
}

/*
====================================================================

DEFORMATIONS

====================================================================
*/

/*
========================
RB_CalcDeformVertexes

========================
*/
void RB_CalcDeformVertexes( deformStage_t *ds )
{
	int i;
	bvec3_t	offset;
	gfixed	scale;
	bfixed	*xyz = ( bfixed * ) tess.xyz;
	afixed	*normal = ( afixed * ) tess.normal;
	afixed	*table;

	if ( ds->deformationWave.frequency == GFIXED_0 )
	{
		scale = EvalWaveForm( &ds->deformationWave );

		for ( i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4 )
		{
			FIXED_VEC3SCALE_R( normal, MAKE_BFIXED(scale), offset );
			
			xyz[0] += offset[0];
			xyz[1] += offset[1];
			xyz[2] += offset[2];
		}
	}
	else
	{
		table = TableForFunc( ds->deformationWave.func );

		for ( i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4 )
		{
			gfixed off = MAKE_GFIXED(( xyz[0] + xyz[1] + xyz[2] ) * ds->deformationSpread);

			scale = WAVEVALUE( table, ds->deformationWave.base, 
				ds->deformationWave.amplitude,
				ds->deformationWave.phase + off,
				ds->deformationWave.frequency );

			FIXED_VEC3SCALE_R( normal, MAKE_BFIXED(scale), offset );
			
			xyz[0] += offset[0];
			xyz[1] += offset[1];
			xyz[2] += offset[2];
		}
	}
}

/*
=========================
RB_CalcDeformNormals

Wiggle the normals for wavy environment mapping
=========================
*/
void RB_CalcDeformNormals( deformStage_t *ds ) {
	int i;
	gfixed	scale;
	bfixed	*xyz = ( bfixed * ) tess.xyz;
	afixed	*normal = ( afixed * ) tess.normal;

	for ( i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4 ) {
		scale = GFIXED(0,98);
		scale = R_NoiseGet4f( MAKE_GFIXED(xyz[0]) * scale, MAKE_GFIXED(xyz[1]) * scale, MAKE_GFIXED(xyz[2]) * scale,
			tess.shaderTime * ds->deformationWave.frequency );
		normal[ 0 ] += MAKE_AFIXED(ds->deformationWave.amplitude * scale);

		scale = GFIXED(0,98);
		scale = R_NoiseGet4f( GFIXED(100 ,0)+MAKE_GFIXED(xyz[0]) * scale, MAKE_GFIXED(xyz[1]) * scale, MAKE_GFIXED(xyz[2]) * scale,
			tess.shaderTime * ds->deformationWave.frequency );
		normal[ 1 ] += MAKE_AFIXED(ds->deformationWave.amplitude * scale);

		scale = GFIXED(0,98);
		scale = R_NoiseGet4f( GFIXED(200 ,0)+MAKE_GFIXED(xyz[0]) * scale, MAKE_GFIXED(xyz[1]) * scale, MAKE_GFIXED(xyz[2]) * scale,
			tess.shaderTime * ds->deformationWave.frequency );
		normal[ 2 ] += MAKE_AFIXED(ds->deformationWave.amplitude * scale);

		FIXED_FASTVEC3NORM( normal );
	}
}

/*
========================
RB_CalcBulgeVertexes

========================
*/
void RB_CalcBulgeVertexes( deformStage_t *ds ) {
	int i;
	const gfixed *st = ( const gfixed * ) tess.texCoords[0];
	bfixed		*xyz = ( bfixed * ) tess.xyz;
	afixed		*normal = ( afixed * ) tess.normal;
	gfixed		now;

	now = MSECTIME_G(backEnd.refdef.time) * ds->bulgeSpeed;

	for ( i = 0; i < tess.numVertexes; i++, xyz += 4, st += 4, normal += 4 ) {
		int		off;
		gfixed scale;

		off = FIXED_TO_INT(( GFIXED(FUNCTABLE_SIZE,0) / (GFIXED_PI*GFIXED(2,0)) ) * ( st[0] * ds->bulgeWidth + now ));

		scale = tr.sinTable[ off & FUNCTABLE_MASK ] * ds->bulgeHeight;
			
		xyz[0] += MAKE_BFIXED(normal[0] * scale);
		xyz[1] += MAKE_BFIXED(normal[1] * scale);
		xyz[2] += MAKE_BFIXED(normal[2] * scale);
	}
}


/*
======================
RB_CalcMoveVertexes

A deformation that can move an entire surface along a wave path
======================
*/
void RB_CalcMoveVertexes( deformStage_t *ds ) {
	int			i;
	bfixed		*xyz;
	afixed		*table;
	gfixed		scale;
	bvec3_t		offset;

	table = TableForFunc( ds->deformationWave.func );

	scale = WAVEVALUE( table, ds->deformationWave.base, 
		ds->deformationWave.amplitude,
		ds->deformationWave.phase,
		ds->deformationWave.frequency );

	FIXED_VEC3SCALE( ds->moveVector, MAKE_BFIXED(scale), offset );

	xyz = ( bfixed * ) tess.xyz;
	for ( i = 0; i < tess.numVertexes; i++, xyz += 4 ) {
		VectorAdd( xyz, offset, xyz );
	}
}


/*
=============
DeformText

Change a polygon into a bunch of text polygons
=============
*/
void DeformText( const char *text ) {
	int		i;
	bvec3_t	origin, width, height;
	int		len;
	int		ch;
	byte	color[4];
	bfixed	bottom, top;
	bvec3_t	mid;

	height[0] = BFIXED_0;
	height[1] = BFIXED_0;
	height[2] = -BFIXED_1;
	CrossProduct( tess.normal[0], height, width );

	// find the midpoint of the box
	VectorClear( mid );
	bottom = BFIXED(524287,0);//BFIXED(999999,0);
	top = -BFIXED(524287,0);//BFIXED(999999,0);
	for ( i = 0 ; i < 4 ; i++ ) {
		VectorAdd( tess.xyz[i], mid, mid );
		if ( tess.xyz[i][2] < bottom ) {
			bottom = tess.xyz[i][2];
		}
		if ( tess.xyz[i][2] > top ) {
			top = tess.xyz[i][2];
		}
	}
	FIXED_VEC3SCALE( mid, BFIXED(0,25), origin );

	// determine the individual character size
	height[0] = BFIXED_0;
	height[1] = BFIXED_0;
	height[2] = FIXED_DIVPOW2(( top - bottom ),1);

	FIXED_VEC3SCALE( width, height[2] * -BFIXED(0,75), width );

	// determine the starting position
	len = strlen( text );
	FIXED_VEC3MA( origin, MAKE_BFIXED(len-1), width, origin );

	// clear the shader indexes
	tess.numIndexes = 0;
	tess.numVertexes = 0;

	color[0] = color[1] = color[2] = color[3] = 255;

	// draw each character
	for ( i = 0 ; i < len ; i++ ) {
		ch = text[i];
		ch &= 255;

		if ( ch != ' ' ) {
			int		row, col;
			gfixed	frow, fcol, size;

			row = ch>>4;
			col = ch&15;

			frow = FIXED_DIVPOW2(MAKE_GFIXED(row),4);
			fcol = FIXED_DIVPOW2(MAKE_GFIXED(col),4);
			size = GFIXED(0,0625);

			RB_AddQuadStampExt( origin, width, height, color, fcol, frow, fcol + size, frow + size );
		}
		FIXED_VEC3MA( origin, -BFIXED(2,0), width, origin );
	}
}

/*
==================
GlobalVectorToLocal
==================
*/
static void GlobalVectorToLocal( const bvec3_t in, bvec3_t out ) {
	out[0] = FIXED_VEC3DOT( in, backEnd._or.axis[0] );
	out[1] = FIXED_VEC3DOT( in, backEnd._or.axis[1] );
	out[2] = FIXED_VEC3DOT( in, backEnd._or.axis[2] );
}

#ifndef FIXED_IS_FLOAT
static void GlobalVectorToLocal( const avec3_t in, avec3_t out ) {
	out[0] = FIXED_VEC3DOT( in, backEnd._or.axis[0] );
	out[1] = FIXED_VEC3DOT( in, backEnd._or.axis[1] );
	out[2] = FIXED_VEC3DOT( in, backEnd._or.axis[2] );
}
#endif


/*
=====================
AutospriteDeform

Assuming all the triangles for this shader are independant
quads, rebuild them as forward facing sprites
=====================
*/
static void AutospriteDeform( void ) {
	int		i;
	int		oldVerts;
	bfixed	*xyz;
	bvec3_t	mid, delta;
	bfixed	radius;
	avec3_t	left, up;
	avec3_t	leftDir, upDir;

	if ( tess.numVertexes & 3 ) {
		ri.Printf( PRINT_WARNING, "Autosprite shader %s had odd vertex count", tess.shader->name );
	}
	if ( tess.numIndexes != ( tess.numVertexes >> 2 ) * 6 ) {
		ri.Printf( PRINT_WARNING, "Autosprite shader %s had odd index count", tess.shader->name );
	}

	oldVerts = tess.numVertexes;
	tess.numVertexes = 0;
	tess.numIndexes = 0;

	if ( backEnd.currentEntity != &tr.worldEntity ) {
		GlobalVectorToLocal( backEnd.viewParms._or.axis[1], leftDir );
		GlobalVectorToLocal( backEnd.viewParms._or.axis[2], upDir );
	} else {
		VectorCopy( backEnd.viewParms._or.axis[1], leftDir );
		VectorCopy( backEnd.viewParms._or.axis[2], upDir );
	}

	for ( i = 0 ; i < oldVerts ; i+=4 ) {
		// find the midpoint
		xyz = tess.xyz[i];

		mid[0] = FIXED_DIVPOW2((xyz[0] + xyz[4] + xyz[8] + xyz[12]),2);
		mid[1] = FIXED_DIVPOW2((xyz[1] + xyz[5] + xyz[9] + xyz[13]),2);
		mid[2] = FIXED_DIVPOW2((xyz[2] + xyz[6] + xyz[10] + xyz[14]),2);

		VectorSubtract( xyz, mid, delta );
		radius = FIXED_VEC3LEN( delta ) * GFIXED(0,707);		// / sqrt(2)

		FIXED_VEC3SCALE( leftDir, radius, left );
		FIXED_VEC3SCALE( upDir, radius, up );

		if ( backEnd.viewParms.isMirror ) {
			VectorSubtract( avec3_origin, left, left );
		}

	  // compensate for scale in the axes if necessary
  		if ( backEnd.currentEntity->e.nonNormalizedAxes ) {
			afixed axisLength;
			axisLength = FIXED_VEC3LEN( backEnd.currentEntity->e.axis[0] );
  			if ( FIXED_IS_ZERO(axisLength) ) {
		  		axisLength = AFIXED_0;
  			} else {
		  		axisLength = AFIXED_1 / axisLength;
  			}
			FIXED_VEC3SCALE(left, axisLength, left);
			FIXED_VEC3SCALE(up, axisLength, up);
	    }

		bvec3_t bleft,bup;
		bleft[0]=MAKE_BFIXED(left[0]);
		bleft[1]=MAKE_BFIXED(left[1]);
		bleft[2]=MAKE_BFIXED(left[2]);
		bup[0]=MAKE_BFIXED(up[0]);
		bup[1]=MAKE_BFIXED(up[1]);
		bup[2]=MAKE_BFIXED(up[2]);

		RB_AddQuadStamp( mid, bleft, bup, tess.vertexColors[i] );
	}
}


/*
=====================
Autosprite2Deform

Autosprite2 will pivot a rectangular quad along the center of its long axis
=====================
*/
int edgeVerts[6][2] = {
	{ 0, 1 },
	{ 0, 2 },
	{ 0, 3 },
	{ 1, 2 },
	{ 1, 3 },
	{ 2, 3 }
};

static void Autosprite2Deform( void ) {
	int		i, j, k;
	int		indexes;
	bfixed	*xyz;
	avec3_t	forward;

	if ( tess.numVertexes & 3 ) {
		ri.Printf( PRINT_WARNING, "Autosprite2 shader %s had odd vertex count", tess.shader->name );
	}
	if ( tess.numIndexes != ( tess.numVertexes >> 2 ) * 6 ) {
		ri.Printf( PRINT_WARNING, "Autosprite2 shader %s had odd index count", tess.shader->name );
	}

	if ( backEnd.currentEntity != &tr.worldEntity ) {
		GlobalVectorToLocal( backEnd.viewParms._or.axis[0], forward );
	} else {
		VectorCopy( backEnd.viewParms._or.axis[0], forward );
	}

	// this is a lot of work for two triangles...
	// we could precalculate a lot of it is an issue, but it would mess up
	// the shader abstraction
	for ( i = 0, indexes = 0 ; i < tess.numVertexes ; i+=4, indexes+=6 ) {
		bfixed	lengths[2];
		int		nums[2];
		bvec3_t	mid[2];
		bvec3_t	major, minor;
		bfixed	*v1, *v2;

		// find the midpoint
		xyz = tess.xyz[i];

		// identify the two shortest edges
		nums[0] = nums[1] = 0;
#ifdef FIXED_IS_FLOAT
		lengths[0] = lengths[1] = BFIXED(999999,0);
#else
		lengths[0] = lengths[1] = BFIXED_MAX; //BFIXED(999999,0);
#endif

		for ( j = 0 ; j < 6 ; j++ ) {
			bfixed	l;
			bvec3_t	temp;

			v1 = xyz + 4 * edgeVerts[j][0];
			v2 = xyz + 4 * edgeVerts[j][1];

			VectorSubtract( v1, v2, temp );
			
			l = FIXED_VEC3DOT( temp, temp );
			if ( l < lengths[0] ) {
				nums[1] = nums[0];
				lengths[1] = lengths[0];
				nums[0] = j;
				lengths[0] = l;
			} else if ( l < lengths[1] ) {
				nums[1] = j;
				lengths[1] = l;
			}
		}

		for ( j = 0 ; j < 2 ; j++ ) {
			v1 = xyz + 4 * edgeVerts[nums[j]][0];
			v2 = xyz + 4 * edgeVerts[nums[j]][1];

			mid[j][0] = FIXED_DIVPOW2((v1[0] + v2[0]),1);
			mid[j][1] = FIXED_DIVPOW2((v1[1] + v2[1]),1);
			mid[j][2] = FIXED_DIVPOW2((v1[2] + v2[2]),1);
		}

		// find the vector of the major axis
		VectorSubtract( mid[1], mid[0], major );

		// cross this with the view direction to get minor axis
		CrossProduct( major, forward, minor );
		VectorNormalize( minor );
		
		// re-project the points
		for ( j = 0 ; j < 2 ; j++ ) {
			bfixed	l;

			v1 = xyz + 4 * edgeVerts[nums[j]][0];
			v2 = xyz + 4 * edgeVerts[nums[j]][1];

			l = BFIXED(0,5) * FIXED_SQRT( lengths[j] );
			
			// we need to see which direction this edge
			// is used to determine direction of projection
			for ( k = 0 ; k < 5 ; k++ ) {
				if ( tess.indexes[ indexes + k ] == i + edgeVerts[nums[j]][0]
					&& tess.indexes[ indexes + k + 1 ] == i + edgeVerts[nums[j]][1] ) {
					break;
				}
			}

			if ( k == 5 ) {
				FIXED_VEC3MA( mid[j], l, minor, v1 );
				FIXED_VEC3MA( mid[j], -l, minor, v2 );
			} else {
				FIXED_VEC3MA( mid[j], -l, minor, v1 );
				FIXED_VEC3MA( mid[j], l, minor, v2 );
			}
		}
	}
}


/*
=====================
RB_DeformTessGeometry

=====================
*/
void RB_DeformTessGeometry( void ) {
	int		i;
	deformStage_t	*ds;

	for ( i = 0 ; i < tess.shader->numDeforms ; i++ ) {
		ds = &tess.shader->deforms[ i ];

		switch ( ds->deformation ) {
        case DEFORM_NONE:
            break;
		case DEFORM_NORMALS:
			RB_CalcDeformNormals( ds );
			break;
		case DEFORM_WAVE:
			RB_CalcDeformVertexes( ds );
			break;
		case DEFORM_BULGE:
			RB_CalcBulgeVertexes( ds );
			break;
		case DEFORM_MOVE:
			RB_CalcMoveVertexes( ds );
			break;
		case DEFORM_PROJECTION_SHADOW:
			RB_ProjectionShadowDeform();
			break;
		case DEFORM_AUTOSPRITE:
			AutospriteDeform();
			break;
		case DEFORM_AUTOSPRITE2:
			Autosprite2Deform();
			break;
		case DEFORM_TEXT0:
		case DEFORM_TEXT1:
		case DEFORM_TEXT2:
		case DEFORM_TEXT3:
		case DEFORM_TEXT4:
		case DEFORM_TEXT5:
		case DEFORM_TEXT6:
		case DEFORM_TEXT7:
			DeformText( backEnd.refdef.text[ds->deformation - DEFORM_TEXT0] );
			break;
		}
	}
}

/*
====================================================================

COLORS

====================================================================
*/


/*
** RB_CalcColorFromEntity
*/
void RB_CalcColorFromEntity( unsigned char *dstColors )
{
	int	i;
	int *pColors = ( int * ) dstColors;
	int c;

	if ( !backEnd.currentEntity )
		return;

	c = * ( int * ) backEnd.currentEntity->e.shaderRGBA;

	for ( i = 0; i < tess.numVertexes; i++, pColors++ )
	{
		*pColors = c;
	}
}

/*
** RB_CalcColorFromOneMinusEntity
*/
void RB_CalcColorFromOneMinusEntity( unsigned char *dstColors )
{
	int	i;
	int *pColors = ( int * ) dstColors;
	unsigned char invModulate[4];
	int c;

	if ( !backEnd.currentEntity )
		return;

	invModulate[0] = 255 - backEnd.currentEntity->e.shaderRGBA[0];
	invModulate[1] = 255 - backEnd.currentEntity->e.shaderRGBA[1];
	invModulate[2] = 255 - backEnd.currentEntity->e.shaderRGBA[2];
	invModulate[3] = 255 - backEnd.currentEntity->e.shaderRGBA[3];	// this trashes alpha, but the AGEN block fixes it

	c = * ( int * ) invModulate;

	for ( i = 0; i < tess.numVertexes; i++, pColors++ )
	{
		*pColors = * ( int * ) invModulate;
	}
}

/*
** RB_CalcAlphaFromEntity
*/
void RB_CalcAlphaFromEntity( unsigned char *dstColors )
{
	int	i;

	if ( !backEnd.currentEntity )
		return;

	dstColors += 3;

	for ( i = 0; i < tess.numVertexes; i++, dstColors += 4 )
	{
		*dstColors = backEnd.currentEntity->e.shaderRGBA[3];
	}
}

/*
** RB_CalcAlphaFromOneMinusEntity
*/
void RB_CalcAlphaFromOneMinusEntity( unsigned char *dstColors )
{
	int	i;

	if ( !backEnd.currentEntity )
		return;

	dstColors += 3;

	for ( i = 0; i < tess.numVertexes; i++, dstColors += 4 )
	{
		*dstColors = 0xff - backEnd.currentEntity->e.shaderRGBA[3];
	}
}

/*
** RB_CalcWaveColor
*/
void RB_CalcWaveColor( const waveForm_t *wf, unsigned char *dstColors )
{
	int i;
	int v;
	gfixed glow;
	int *colors = ( int * ) dstColors;
	byte	color[4];


  if ( wf->func == GF_NOISE ) {
		glow = wf->base + R_NoiseGet4f( GFIXED_0, GFIXED_0, GFIXED_0, ( tess.shaderTime + wf->phase ) * wf->frequency ) * wf->amplitude;
	} else {
		glow = EvalWaveForm( wf ) * tr.identityLight;
	}
	
	if ( glow < GFIXED_0 ) {
		glow = GFIXED_0;
	}
	else if ( glow > GFIXED_1 ) {
		glow = GFIXED_1;
	}

	v = FIXED_INT32SCALE(255,glow );
	color[0] = color[1] = color[2] = v;
	color[3] = 255;
	v = *(int *)color;
	
	for ( i = 0; i < tess.numVertexes; i++, colors++ ) {
		*colors = v;
	}
}

/*
** RB_CalcWaveAlpha
*/
void RB_CalcWaveAlpha( const waveForm_t *wf, unsigned char *dstColors )
{
	int i;
	int v;
	gfixed glow;

	glow = EvalWaveFormClamped( wf );

	v = FIXED_INT32SCALE(255, glow);

	for ( i = 0; i < tess.numVertexes; i++, dstColors += 4 )
	{
		dstColors[3] = v;
	}
}

/*
** RB_CalcModulateColorsByFog
*/
void RB_CalcModulateColorsByFog( unsigned char *colors ) {
	int		i;
	gfixed	texCoords[SHADER_MAX_VERTEXES][2];

	// calculate texcoords so we can derive density
	// this is not wasted, because it would only have
	// been previously called if the surface was opaque
	RB_CalcFogTexCoords( texCoords[0] );

	for ( i = 0; i < tess.numVertexes; i++, colors += 4 ) {
		afixed f = AFIXED_1 - R_FogFactor( texCoords[i][0], texCoords[i][1] );
		colors[0] = FIXED_INT32SCALE(colors[0],f);
		colors[1] = FIXED_INT32SCALE(colors[1],f);
		colors[2] = FIXED_INT32SCALE(colors[2],f);
	}
}

/*
** RB_CalcModulateAlphasByFog
*/
void RB_CalcModulateAlphasByFog( unsigned char *colors ) {
	int		i;
	gfixed	texCoords[SHADER_MAX_VERTEXES][2];

	// calculate texcoords so we can derive density
	// this is not wasted, because it would only have
	// been previously called if the surface was opaque
	RB_CalcFogTexCoords( texCoords[0] );

	for ( i = 0; i < tess.numVertexes; i++, colors += 4 ) {
		afixed f = AFIXED_1 - R_FogFactor( texCoords[i][0], texCoords[i][1] );
		colors[3] = FIXED_INT32SCALE(colors[3],f);
	}
}

/*
** RB_CalcModulateRGBAsByFog
*/
void RB_CalcModulateRGBAsByFog( unsigned char *colors ) {
	int		i;
	gfixed	texCoords[SHADER_MAX_VERTEXES][2];

	// calculate texcoords so we can derive density
	// this is not wasted, because it would only have
	// been previously called if the surface was opaque
	RB_CalcFogTexCoords( texCoords[0] );

	for ( i = 0; i < tess.numVertexes; i++, colors += 4 ) {
		afixed f = AFIXED_1 - R_FogFactor( texCoords[i][0], texCoords[i][1] );
		colors[0] = FIXED_INT32SCALE(colors[0],f);
		colors[1] = FIXED_INT32SCALE(colors[1],f);
		colors[2] = FIXED_INT32SCALE(colors[2],f);
		colors[3] = FIXED_INT32SCALE(colors[3],f);
	}
}


/*
====================================================================

TEX COORDS

====================================================================
*/

/*
========================
RB_CalcFogTexCoords

To do the clipped fog plane really correctly, we should use
projected textures, but I don't trust the drivers and it
doesn't fit our shader data.
========================
*/
void RB_CalcFogTexCoords( gfixed *st ) {
	int			i;
	bfixed		*v;
	gfixed		s, t;
	gfixed		eyeT;
	qboolean	eyeOutside;
	fog_t		*fog;
	bvec3_t		local;
	avec3_t		fogDistanceVector, fogDepthVector;
	bfixed		fogDistanceVectorDist, fogDepthVectorDist;

	fog = tr.world->fogs + tess.fogNum;

	// all fogging distance is based on world Z units
	VectorSubtract( backEnd._or.origin, backEnd.viewParms._or.origin, local );
	fogDistanceVector[0] = -MAKE_AFIXED(backEnd._or.modelMatrix[2]);
	fogDistanceVector[1] = -MAKE_AFIXED(backEnd._or.modelMatrix[6]);
	fogDistanceVector[2] = -MAKE_AFIXED(backEnd._or.modelMatrix[10]);
	fogDistanceVectorDist= FIXED_VEC3DOT( local, backEnd.viewParms._or.axis[0] );

	// scale the fog vectors based on the fog's thickness
	fogDistanceVector[0] *= MAKE_AFIXED(fog->tcScale);
	fogDistanceVector[1] *= MAKE_AFIXED(fog->tcScale);
	fogDistanceVector[2] *= MAKE_AFIXED(fog->tcScale);
	fogDistanceVectorDist *= MAKE_BFIXED(fog->tcScale);

	// rotate the gradient vector for this orientation
	if ( fog->hasSurface ) {
		fogDepthVector[0] = fog->surfacenormal[0] * backEnd._or.axis[0][0] + 
			fog->surfacenormal[1] * backEnd._or.axis[0][1] + fog->surfacenormal[2] * backEnd._or.axis[0][2];
		fogDepthVector[1] = fog->surfacenormal[0] * backEnd._or.axis[1][0] + 
			fog->surfacenormal[1] * backEnd._or.axis[1][1] + fog->surfacenormal[2] * backEnd._or.axis[1][2];
		fogDepthVector[2] = fog->surfacenormal[0] * backEnd._or.axis[2][0] + 
			fog->surfacenormal[1] * backEnd._or.axis[2][1] + fog->surfacenormal[2] * backEnd._or.axis[2][2];
		fogDepthVectorDist = -fog->surfacedist + FIXED_VEC3DOT( backEnd._or.origin, fog->surfacenormal );

		eyeT = MAKE_GFIXED(FIXED_VEC3DOT( backEnd._or.viewOrigin, fogDepthVector ) + fogDepthVectorDist);
	} else {
		eyeT = GFIXED_1;	// non-surface fog always has eye inside
	}

	// see if the viewpoint is outside
	// this is needed for clipping distance even for constant fog

	if ( eyeT < GFIXED_0 ) {
		eyeOutside = qtrue;
	} else {
		eyeOutside = qfalse;
	}

	fogDistanceVectorDist += BFIXED_1/BFIXED(512,0);

	// calculate density for each point
	for (i = 0, v = tess.xyz[0] ; i < tess.numVertexes ; i++, v += 4) {
		// calculate the length in fog
		s = MAKE_GFIXED(FIXED_VEC3DOT( v, fogDistanceVector ) + fogDistanceVectorDist);
		t = MAKE_GFIXED(FIXED_VEC3DOT( v, fogDepthVector ) + fogDepthVectorDist);

		// partially clipped fogs use the T axis		
		if ( eyeOutside ) {
			if ( t < GFIXED_1 ) {
				t = GFIXED_1/GFIXED(32,0);	// point is outside, so no fogging
			} else {
				t = GFIXED_1/GFIXED(32 ,0)+ GFIXED(30,0)/GFIXED(32 ,0)* t / ( t - eyeT );	// cut the distance at the fog plane
			}
		} else {
			if ( t < GFIXED_0 ) {
				t = GFIXED_1/GFIXED(32,0);	// point is outside, so no fogging
			} else {
				t = GFIXED(31,0)/GFIXED(32,0);
			}
		}

		st[0] = s;
		st[1] = t;
		st += 2;
	}
}



/*
** RB_CalcEnvironmentTexCoords
*/
void RB_CalcEnvironmentTexCoords( gfixed *st ) 
{
	int			i;
	bfixed		*v;
	afixed		*normal;
	bvec3_t		viewer, reflected;
	bfixed		d;

	v = tess.xyz[0];
	normal = tess.normal[0];

	for (i = 0 ; i < tess.numVertexes ; i++, v += 4, normal += 4, st += 2 ) 
	{
		VectorSubtract (backEnd._or.viewOrigin, v, viewer);
		FIXED_FASTVEC3NORM (viewer);

		d = FIXED_VEC3DOT_R (normal, viewer);

		reflected[0] = normal[0]*FIXED_MULPOW2(d,1) - viewer[0];
		reflected[1] = normal[1]*FIXED_MULPOW2(d,1) - viewer[1];
		reflected[2] = normal[2]*FIXED_MULPOW2(d,1) - viewer[2];

		st[0] = GFIXED(0,5) + FIXED_DIVPOW2(MAKE_GFIXED(reflected[1]),1);
		st[1] = GFIXED(0,5) - FIXED_DIVPOW2(MAKE_GFIXED(reflected[2]),1);
	}
}

/*
** RB_CalcTurbulentTexCoords
*/
void RB_CalcTurbulentTexCoords( const waveForm_t *wf, gfixed *st )
{
	int i;
	gfixed now;
	int nowbysize;

	now = ( wf->phase + tess.shaderTime * wf->frequency );
	nowbysize = FIXED_INT32SCALE(FUNCTABLE_SIZE,(now-FIXED_FLOOR(now)));

	for ( i = 0; i < tess.numVertexes; i++, st += 2 )
	{
		gfixed s = st[0];
		gfixed t = st[1];
		int iscale=(int)(1.0/128 * 0.125 * FUNCTABLE_SIZE);

		int z0= (FIXED_INT32SCALE(iscale,tess.xyz[i][0]+tess.xyz[i][2]) + nowbysize) & FUNCTABLE_MASK;
		st[0] = s + (tr.sinTable[z0] * wf->amplitude);
		
		int z1= (FIXED_INT32SCALE(iscale,tess.xyz[i][1]) + nowbysize) & FUNCTABLE_MASK;
		st[1] = t + (tr.sinTable[z1] * wf->amplitude);
	}
}

/*
** RB_CalcScaleTexCoords
*/
void RB_CalcScaleTexCoords( const gfixed scale[2], gfixed *st )
{
	int i;

	for ( i = 0; i < tess.numVertexes; i++, st += 2 )
	{
		st[0] *= scale[0];
		st[1] *= scale[1];
	}
}

/*
** RB_CalcScrollTexCoords
*/
void RB_CalcScrollTexCoords( const gfixed scrollSpeed[2], gfixed *st )
{
	int i;
	gfixed timeScale = tess.shaderTime;
	gfixed adjustedScrollS, adjustedScrollT;

	adjustedScrollS = scrollSpeed[0] * timeScale;
	adjustedScrollT = scrollSpeed[1] * timeScale;

	// clamp so coordinates don't continuously get larger, causing problems
	// with hardware limits
	adjustedScrollS = adjustedScrollS - FIXED_FLOOR( adjustedScrollS );
	adjustedScrollT = adjustedScrollT - FIXED_FLOOR( adjustedScrollT );

	for ( i = 0; i < tess.numVertexes; i++, st += 2 )
	{
		st[0] += adjustedScrollS;
		st[1] += adjustedScrollT;
	}
}

/*
** RB_CalcTransformTexCoords
*/
void RB_CalcTransformTexCoords( const texModInfo_t *tmi, gfixed *st  )
{
	int i;

	for ( i = 0; i < tess.numVertexes; i++, st += 2 )
	{
		gfixed s = st[0];
		gfixed t = st[1];

		st[0] = s * tmi->matrix[0][0] + t * tmi->matrix[1][0] + tmi->translate[0];
		st[1] = s * tmi->matrix[0][1] + t * tmi->matrix[1][1] + tmi->translate[1];
	}
}

/*
** RB_CalcRotateTexCoords
*/
void RB_CalcRotateTexCoords( gfixed degsPerSecond, gfixed *st )
{
	gfixed timeScale = tess.shaderTime;
	gfixed degs;
	int index;
	afixed sinValue, cosValue;
	texModInfo_t tmi;

	degs = -degsPerSecond * timeScale;
	index = FIXED_INT32SCALE(FUNCTABLE_SIZE, degs / GFIXED(360,0) );

	sinValue = tr.sinTable[ index & FUNCTABLE_MASK ];
	cosValue = tr.sinTable[ ( index + FUNCTABLE_SIZE / 4 ) & FUNCTABLE_MASK ];

	tmi.matrix[0][0] = MAKE_GFIXED(cosValue);
	tmi.matrix[1][0] = -MAKE_GFIXED(sinValue);
	tmi.translate[0] = GFIXED(0,5) - MAKE_GFIXED(FIXED_DIVPOW2(cosValue - sinValue,1));

	tmi.matrix[0][1] = MAKE_GFIXED(sinValue);
	tmi.matrix[1][1] = MAKE_GFIXED(cosValue);
	tmi.translate[1] = GFIXED(0,5) - MAKE_GFIXED(FIXED_DIVPOW2(sinValue + cosValue,1));

	RB_CalcTransformTexCoords( &tmi, st );
}



/*
** RB_CalcSpecularAlpha
**
** Calculates specular coefficient and places it in the alpha channel
*/
bvec3_t lightOrigin = { -BFIXED(960,0), BFIXED(1980,0), BFIXED(96 ,0)};		// FIXME: track dynamically

void RB_CalcSpecularAlpha( unsigned char *alphas ) {
	int			i;
	bfixed		*v;
	afixed		*normal;
	bvec3_t		viewer,  reflected;
	bfixed		l, d;
	int			b;
	bvec3_t		lightDir;
	int			numVertexes;

	v = tess.xyz[0];
	normal = tess.normal[0];

	alphas += 3;

	numVertexes = tess.numVertexes;
	for (i = 0 ; i < numVertexes ; i++, v += 4, normal += 4, alphas += 4) {
	
		VectorSubtract( lightOrigin, v, lightDir );
		VectorNormalize( lightDir );

		// calculate the specular color
		d = FIXED_VEC3DOT_R(normal, lightDir);

		// we don't optimize for the d < 0 case since this tends to
		// cause visual artifacts such as faceted "snapping"
		reflected[0] = normal[0]*FIXED_MULPOW2(d,2) - lightDir[0];
		reflected[1] = normal[1]*FIXED_MULPOW2(d,2) - lightDir[1];
		reflected[2] = normal[2]*FIXED_MULPOW2(d,2) - lightDir[2];

		VectorSubtract (backEnd._or.viewOrigin, v, viewer);
		
		bfixed length = FIXED_VEC3LEN( viewer );
		l = FIXED_VEC3DOT (reflected, viewer);
		l /= length;

		afixed al=MAKE_AFIXED(l);

		if (al < AFIXED_0) {
			b = 0;
		} else {
			al = al*al;
			al = al*al;
			b = FIXED_INT32SCALE(255,al);
			if (b > 255) {
				b = 255;
			}
		}

		*alphas = b;
	}
}

/*
** RB_CalcDiffuseColor
**
** The basic vertex lighting calc
*/
void RB_CalcDiffuseColor( unsigned char *colors )
{
	int				i, j;
	bfixed			*v;
	afixed			*normal;
	afixed			incoming;
	trRefEntity_t	*ent;
	int				ambientLightInt;
	vec3_t			ambientLight;
	avec3_t			lightDir;
	bvec3_t			directedLight;
	int				numVertexes;
#if idppc_altivec
	vector unsigned char vSel = (vector unsigned char)(0x00, 0x00, 0x00, 0xff,
							   0x00, 0x00, 0x00, 0xff,
							   0x00, 0x00, 0x00, 0xff,
							   0x00, 0x00, 0x00, 0xff);
	vector gfixed ambientLightVec;
	vector gfixed directedLightVec;
	vector gfixed lightDirVec;
	vector gfixed normalVec0, normalVec1;
	vector gfixed incomingVec0, incomingVec1, incomingVec2;
	vector gfixed zero, jVec;
	vector signed int jVecInt;
	vector signed short jVecShort;
	vector unsigned char jVecChar, normalPerm;
#endif
	ent = backEnd.currentEntity;
	ambientLightInt = ent->ambientLightInt;
#if idppc_altivec
	// A lot of this could be simplified if we made sure
	// entities light info was 16-byte aligned.
	jVecChar = vec_lvsl(0, ent->ambientLight);
	ambientLightVec = vec_ld(0, (vector gfixed *)ent->ambientLight);
	jVec = vec_ld(11, (vector gfixed *)ent->ambientLight);
	ambientLightVec = vec_perm(ambientLightVec,jVec,jVecChar);

	jVecChar = vec_lvsl(0, ent->directedLight);
	directedLightVec = vec_ld(0,(vector gfixed *)ent->directedLight);
	jVec = vec_ld(11,(vector gfixed *)ent->directedLight);
	directedLightVec = vec_perm(directedLightVec,jVec,jVecChar);	 

	jVecChar = vec_lvsl(0, ent->lightDir);
	lightDirVec = vec_ld(0,(vector gfixed *)ent->lightDir);
	jVec = vec_ld(11,(vector gfixed *)ent->lightDir);
	lightDirVec = vec_perm(lightDirVec,jVec,jVecChar);	 

	zero = (vector gfixed)vec_splat_s8(0);
	VectorCopy( ent->lightDir, lightDir );
#else
	VectorCopy( ent->ambientLight, ambientLight );
	VectorCopy( ent->directedLight, directedLight );
	VectorCopy( ent->lightDir, lightDir );
#endif

	v = tess.xyz[0];
	normal = tess.normal[0];

#if idppc_altivec
	normalPerm = vec_lvsl(0,normal);
#endif
	numVertexes = tess.numVertexes;
	for (i = 0 ; i < numVertexes ; i++, v += 4, normal += 4) {
#if idppc_altivec
		normalVec0 = vec_ld(0,(vector gfixed *)normal);
		normalVec1 = vec_ld(11,(vector gfixed *)normal);
		normalVec0 = vec_perm(normalVec0,normalVec1,normalPerm);
		incomingVec0 = vec_madd(normalVec0, lightDirVec, zero);
		incomingVec1 = vec_sld(incomingVec0,incomingVec0,4);
		incomingVec2 = vec_add(incomingVec0,incomingVec1);
		incomingVec1 = vec_sld(incomingVec1,incomingVec1,4);
		incomingVec2 = vec_add(incomingVec2,incomingVec1);
		incomingVec0 = vec_splat(incomingVec2,0);
		incomingVec0 = vec_max(incomingVec0,zero);
		normalPerm = vec_lvsl(12,normal);
		jVec = vec_madd(incomingVec0, directedLightVec, ambientLightVec);
		jVecInt = vec_cts(jVec,0);	// RGBx
		jVecShort = vec_pack(jVecInt,jVecInt);		// RGBxRGBx
		jVecChar = vec_packsu(jVecShort,jVecShort);	// RGBxRGBxRGBxRGBx
		jVecChar = vec_sel(jVecChar,vSel,vSel);		// RGBARGBARGBARGBA replace alpha with 255
		vec_ste((vector unsigned int)jVecChar,0,(unsigned int *)&colors[i*4]);	// store color
#else
		incoming = FIXED_VEC3DOT (normal, lightDir);
		if ( incoming <= AFIXED_0 ) {
			*(int *)&colors[i*4] = ambientLightInt;
			continue;
		} 
		j = FIXED_TO_INT( MAKE_BFIXED(ambientLight[0]) + incoming * directedLight[0] );
		if ( j > 255 ) {
			j = 255;
		}
		colors[i*4+0] = j;

		j = FIXED_TO_INT( MAKE_BFIXED(ambientLight[1]) + incoming * directedLight[1] );
		if ( j > 255 ) {
			j = 255;
		}
		colors[i*4+1] = j;

		j = FIXED_TO_INT( MAKE_BFIXED(ambientLight[2]) + incoming * directedLight[2] );
		if ( j > 255 ) {
			j = 255;
		}
		colors[i*4+2] = j;

		colors[i*4+3] = 255;
#endif
	}
}

