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
// tr_flares.c

#include"renderer_pch.h"

/*
=============================================================================

LIGHT FLARES

A light flare is an effect that takes place inside the eye when bright light
sources are visible.  The size of the flare reletive to the screen is nearly
constant, irrespective of distance, but the intensity should be proportional to the
projected area of the light source.

A surface that has been flagged as having a light flare will calculate the depth
buffer value that it's midpoint should have when the surface is added.

After all opaque surfaces have been rendered, the depth buffer is read back for
each flare in view.  If the point has not been obscured by a closer surface, the
flare should be drawn.

Surfaces that have a repeated texture should never be flagged as flaring, because
there will only be a single flare added at the midpoint of the polygon.

To prevent abrupt popping, the intensity of the flare is interpolated up and
down as it changes visibility.  This involves scene to scene state, unlike almost
all other aspects of the renderer, and is complicated by the fact that a single
frame may have multiple scenes.

RB_RenderFlares() will be called once per view (twice in a mirrored scene, potentially
up to five or more times in a frame with 3D status bar icons).

=============================================================================
*/


// flare states maintain visibility over multiple frames for fading
// layers: view, mirror, menu
typedef struct flare_s {
	struct		flare_s	*next;		// for active chain

	int			addedFrame;

	qboolean	inPortal;				// true if in a portal view of the scene
	int			frameSceneNum;
	void		*surface;
	int			fogNum;

	int			fadeTime;

	qboolean	visible;			// state of last test
	gfixed		drawIntensity;		// may be non 0 even if !visible due to fading

	int			windowX, windowY;
	bfixed		eyeZ;

	vec3_t		color;
} flare_t;

#define		MAX_FLARES		128

flare_t		r_flareStructs[MAX_FLARES];
flare_t		*r_activeFlares, *r_inactiveFlares;

/*
==================
R_ClearFlares
==================
*/
void R_ClearFlares( void ) {
	int		i;

	Com_Memset( r_flareStructs, 0, sizeof( r_flareStructs ) );
	r_activeFlares = NULL;
	r_inactiveFlares = NULL;

	for ( i = 0 ; i < MAX_FLARES ; i++ ) {
		r_flareStructs[i].next = r_inactiveFlares;
		r_inactiveFlares = &r_flareStructs[i];
	}
}


/*
==================
RB_AddFlare

This is called at surface tesselation time
==================
*/
void RB_AddFlare( void *surface, int fogNum, bvec3_t point, vec3_t color, avec3_t normal ) {
	int				i;
	flare_t			*f, *oldest;
	bvec3_t			local;
	bfixed			d;
	bvec4_t			eye, clip, normalized, window;

	backEnd.pc.c_flareAdds++;

	// if the point is off the screen, don't bother adding it
	// calculate screen coordinates and depth
	R_TransformModelToClip( point, backEnd._or.modelMatrix, 
		backEnd.viewParms.projectionMatrix, eye, clip );

	// check to see if the point is completely off screen
	for ( i = 0 ; i < 3 ; i++ ) {
		if ( clip[i] >= clip[3] || clip[i] <= -clip[3] ) {
			return;
		}
	}

	R_TransformClipToWindow( clip, &backEnd.viewParms, normalized, window );

	if ( window[0] < BFIXED_0 || window[0] >= MAKE_BFIXED(backEnd.viewParms.viewportWidth)
		|| window[1] < BFIXED_0 || window[1] >= MAKE_BFIXED(backEnd.viewParms.viewportHeight) ) {
		return;	// shouldn't happen, since we check the clip[] above, except for FP rounding
	}

	// see if a flare with a matching surface, scene, and view exists
	oldest = r_flareStructs;
	for ( f = r_activeFlares ; f ; f = f->next ) {
		if ( f->surface == surface && f->frameSceneNum == backEnd.viewParms.frameSceneNum
			&& f->inPortal == backEnd.viewParms.isPortal ) {
			break;
		}
	}

	// allocate a new one
	if (!f ) {
		if ( !r_inactiveFlares ) {
			// the list is completely full
			return;
		}
		f = r_inactiveFlares;
		r_inactiveFlares = r_inactiveFlares->next;
		f->next = r_activeFlares;
		r_activeFlares = f;

		f->surface = surface;
		f->frameSceneNum = backEnd.viewParms.frameSceneNum;
		f->inPortal = backEnd.viewParms.isPortal;
		f->addedFrame = -1;
	}

	if ( f->addedFrame != backEnd.viewParms.frameCount - 1 ) {
		f->visible = qfalse;
		f->fadeTime = backEnd.refdef.time - 2000;
	}

	f->addedFrame = backEnd.viewParms.frameCount;
	f->fogNum = fogNum;

	VectorCopy( color, f->color );

	// fade the intensity of the flare down as the
	// light surface turns away from the viewer
	if ( normal ) {
		VectorSubtract( backEnd.viewParms._or.origin, point, local );
		FIXED_FASTVEC3NORM( local );
		d = FIXED_VEC3DOT( local, normal );
		FIXED_VEC3SCALE( f->color, MAKE_GFIXED(d), f->color ); 
	}

	// save info needed to test
	f->windowX = FIXED_TO_INT(MAKE_BFIXED(backEnd.viewParms.viewportX) + window[0]);
	f->windowY = FIXED_TO_INT(MAKE_BFIXED(backEnd.viewParms.viewportY) + window[1]);

	f->eyeZ = eye[2];
}

/*
==================
RB_AddDlightFlares
==================
*/
void RB_AddDlightFlares( void ) {
	dlight_t		*l;
	int				i, j, k;
	fog_t			*fog;

	if ( !r_flares->integer ) {
		return;
	}

	l = backEnd.refdef.dlights;
	fog = tr.world->fogs;
	for (i=0 ; i<backEnd.refdef.num_dlights ; i++, l++) {

		// find which fog volume the light is in 
		for ( j = 1 ; j < tr.world->numfogs ; j++ ) {
			fog = &tr.world->fogs[j];
			for ( k = 0 ; k < 3 ; k++ ) {
				if ( l->origin[k] < fog->bounds[0][k] || l->origin[k] > fog->bounds[1][k] ) {
					break;
				}
			}
			if ( k == 3 ) {
				break;
			}
		}
		if ( j == tr.world->numfogs ) {
			j = 0;
		}

		RB_AddFlare( (void *)l, j, l->origin, l->color, NULL );
	}
}

/*
===============================================================================

FLARE BACK END

===============================================================================
*/

/*
==================
RB_TestFlare
==================
*/
void RB_TestFlare( flare_t *f ) {
	gfixed			depth;
	qboolean		visible;
	gfixed			fade;
	gfixed			screenZ;

	backEnd.pc.c_flareTests++;

	// doing a readpixels is as good as doing a glFinish(), so
	// don't bother with another sync
	glState.finishCalled = qfalse;

	// read back the z buffer contents
//	glReadPixels( f->windowX, f->windowY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth );

depth=GFIXED(0,5);

	screenZ = backEnd.viewParms.projectionMatrix[14] / 
		( ( GFIXED(2,0)*depth - GFIXED_1 ) * backEnd.viewParms.projectionMatrix[11] - backEnd.viewParms.projectionMatrix[10] );

	visible = ( MAKE_GFIXED(-f->eyeZ) - -screenZ ) < GFIXED(24,0);

	if ( visible ) {
		if ( !f->visible ) {
			f->visible = qtrue;
			f->fadeTime = backEnd.refdef.time - 1;
		}
		fade = MSECTIME_G( backEnd.refdef.time - f->fadeTime ) * MAKE_GFIXED(r_flareFade->value);
	} else {
		if ( f->visible ) {
			f->visible = qfalse;
			f->fadeTime = backEnd.refdef.time - 1;
		}
		fade = GFIXED_1 - ( MSECTIME_G( backEnd.refdef.time - f->fadeTime ) * MAKE_GFIXED(r_flareFade->value) );
	}

	if ( fade < GFIXED_0 ) {
		fade = GFIXED_0;
	}
	if ( fade > GFIXED_1 ) {
		fade = GFIXED_1;
	}

	f->drawIntensity = fade;
}


/*
==================
RB_RenderFlare
==================
*/
void RB_RenderFlare( flare_t *f ) {
	bfixed			size;
	vec3_t			color;
	int				iColor[3];

	backEnd.pc.c_flareRenders++;

	FIXED_VEC3SCALE( f->color, f->drawIntensity*tr.identityLight, color );
	iColor[0] = FIXED_TO_INT(color[0] * GFIXED(255,0));
	iColor[1] = FIXED_TO_INT(color[1] * GFIXED(255,0));
	iColor[2] = FIXED_TO_INT(color[2] * GFIXED(255,0));

	size = MAKE_BFIXED(backEnd.viewParms.viewportWidth) * ( MAKE_BFIXED(r_flareSize->value)/BFIXED(640,0) + BFIXED(8 ,0)/ -f->eyeZ );

	RB_BeginSurface( tr.flareShader, f->fogNum );

	// FIXME: use quadstamp?
	tess.xyz[tess.numVertexes][0] = MAKE_BFIXED(f->windowX) - size;
	tess.xyz[tess.numVertexes][1] = MAKE_BFIXED(f->windowY) - size;
	tess.texCoords[tess.numVertexes][0][0] = GFIXED_0;
	tess.texCoords[tess.numVertexes][0][1] = GFIXED_0;
	tess.vertexColors[tess.numVertexes][0] = iColor[0];
	tess.vertexColors[tess.numVertexes][1] = iColor[1];
	tess.vertexColors[tess.numVertexes][2] = iColor[2];
	tess.vertexColors[tess.numVertexes][3] = 255;
	tess.numVertexes++;

	tess.xyz[tess.numVertexes][0] = MAKE_BFIXED(f->windowX) - size;
	tess.xyz[tess.numVertexes][1] = MAKE_BFIXED(f->windowY) + size;
	tess.texCoords[tess.numVertexes][0][0] = GFIXED_0;
	tess.texCoords[tess.numVertexes][0][1] = GFIXED_1;
	tess.vertexColors[tess.numVertexes][0] = iColor[0];
	tess.vertexColors[tess.numVertexes][1] = iColor[1];
	tess.vertexColors[tess.numVertexes][2] = iColor[2];
	tess.vertexColors[tess.numVertexes][3] = 255;
	tess.numVertexes++;

	tess.xyz[tess.numVertexes][0] = MAKE_BFIXED(f->windowX) + size;
	tess.xyz[tess.numVertexes][1] = MAKE_BFIXED(f->windowY) + size;
	tess.texCoords[tess.numVertexes][0][0] = GFIXED_1;
	tess.texCoords[tess.numVertexes][0][1] = GFIXED_1;
	tess.vertexColors[tess.numVertexes][0] = iColor[0];
	tess.vertexColors[tess.numVertexes][1] = iColor[1];
	tess.vertexColors[tess.numVertexes][2] = iColor[2];
	tess.vertexColors[tess.numVertexes][3] = 255;
	tess.numVertexes++;

	tess.xyz[tess.numVertexes][0] = MAKE_BFIXED(f->windowX) + size;
	tess.xyz[tess.numVertexes][1] = MAKE_BFIXED(f->windowY) - size;
	tess.texCoords[tess.numVertexes][0][0] = GFIXED_1;
	tess.texCoords[tess.numVertexes][0][1] = GFIXED_0;
	tess.vertexColors[tess.numVertexes][0] = iColor[0];
	tess.vertexColors[tess.numVertexes][1] = iColor[1];
	tess.vertexColors[tess.numVertexes][2] = iColor[2];
	tess.vertexColors[tess.numVertexes][3] = 255;
	tess.numVertexes++;

	tess.indexes[tess.numIndexes++] = 0;
	tess.indexes[tess.numIndexes++] = 1;
	tess.indexes[tess.numIndexes++] = 2;
	tess.indexes[tess.numIndexes++] = 0;
	tess.indexes[tess.numIndexes++] = 2;
	tess.indexes[tess.numIndexes++] = 3;

	RB_EndSurface();
}

/*
==================
RB_RenderFlares

Because flares are simulating an occular effect, they should be drawn after
everything (all views) in the entire frame has been drawn.

Because of the way portals use the depth buffer to mark off areas, the
needed information would be lost after each view, so we are forced to draw
flares after each view.

The resulting artifact is that flares in mirrors or portals don't dim properly
when occluded by something in the main view, and portal flares that should
extend past the portal edge will be overwritten.
==================
*/
void RB_RenderFlares (void) {
	flare_t		*f;
	flare_t		**prev;
	qboolean	draw;

	if ( !r_flares->integer ) {
		return;
	}

//	RB_AddDlightFlares();

	// perform z buffer readback on each flare in this view
	draw = qfalse;
	prev = &r_activeFlares;
	while ( ( f = *prev ) != NULL ) {
		// throw out any flares that weren't added last frame
		if ( f->addedFrame < backEnd.viewParms.frameCount - 1 ) {
			*prev = f->next;
			f->next = r_inactiveFlares;
			r_inactiveFlares = f;
			continue;
		}

		// don't draw any here that aren't from this scene / portal
		f->drawIntensity = GFIXED_0;
		if ( f->frameSceneNum == backEnd.viewParms.frameSceneNum
			&& f->inPortal == backEnd.viewParms.isPortal ) {
			RB_TestFlare( f );
			if ( FIXED_NOT_ZERO(f->drawIntensity) ) {
				draw = qtrue;
			} else {
				// this flare has completely faded out, so remove it from the chain
				*prev = f->next;
				f->next = r_inactiveFlares;
				r_inactiveFlares = f;
				continue;
			}
		}

		prev = &f->next;
	}

	if ( !draw ) {
		return;		// none visible
	}

	if ( backEnd.viewParms.isPortal ) {
		glDisable (GL_CLIP_PLANE0);
	}

	glPushMatrix();
    glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
    glLoadIdentity();
	glOrthoX( REINTERPRET_GFIXED(MAKE_BFIXED(backEnd.viewParms.viewportX)),
			  REINTERPRET_GFIXED(MAKE_BFIXED(backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth)),
			  REINTERPRET_GFIXED(MAKE_BFIXED(backEnd.viewParms.viewportY)),
			  REINTERPRET_GFIXED(MAKE_BFIXED(backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight)),
			  REINTERPRET_GFIXED(MAKE_BFIXED(-99999)), 
			  REINTERPRET_GFIXED(MAKE_BFIXED(99999)) );

	for ( f = r_activeFlares ; f ; f = f->next ) {
		if ( f->frameSceneNum == backEnd.viewParms.frameSceneNum
			&& f->inPortal == backEnd.viewParms.isPortal
			&& FIXED_NOT_ZERO(f->drawIntensity) ) {
			RB_RenderFlare( f );
		}
	}

	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}

