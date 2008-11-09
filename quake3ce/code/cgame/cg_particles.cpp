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
// Rafael particles
// cg_particles.c  

#include "cg_local.h"

#define BLOODRED	2
#define EMISIVEFADE	3
#define GREY75		4

typedef struct particle_s
{
	struct particle_s	*next;

	int		time;
	int		endtime;

	bvec3_t		org;
	bvec3_t		vel;
	bvec3_t		accel;
	int			color;
	gfixed		colorvel;
	gfixed		alpha;
	gfixed		alphavel;
	int			type;
	qhandle_t	pshader;
	
	gfixed		height;
	gfixed		width;
				
	gfixed		endheight;
	gfixed		endwidth;
	
	gfixed		start;
	gfixed		end;

	gfixed		startfade;
	qboolean	rotate;
	int			snum;
	
	qboolean	link;

	// Ridah
	int			shaderAnim;
	int			roll;

	int			accumroll;

} cparticle_t;

typedef enum
{
	P_NONE,
	P_WEATHER,
	P_FLAT,
	P_SMOKE,
	P_ROTATE,
	P_WEATHER_TURBULENT,
	P_ANIM,	// Ridah
	P_BAT,
	P_BLEED,
	P_FLAT_SCALEUP,
	P_FLAT_SCALEUP_FADE,
	P_WEATHER_FLURRY,
	P_SMOKE_IMPACT,
	P_BUBBLE,
	P_BUBBLE_TURBULENT,
	P_SPRITE
} particle_type_t;

#define	MAX_SHADER_ANIMS		32
#define	MAX_SHADER_ANIM_FRAMES	64

static const char *shaderAnimNames[MAX_SHADER_ANIMS] = {
	"explode1",
	"blacksmokeanim",
	"twiltb2",
	"expblue",
	"blacksmokeanimb",	// uses 'explode1' sequence
	"blood",
	NULL
};
static qhandle_t shaderAnims[MAX_SHADER_ANIMS][MAX_SHADER_ANIM_FRAMES];
static int	shaderAnimCounts[MAX_SHADER_ANIMS] = {
	23,
	25,
	45,
	25,
	23,
	5,
};
static gfixed	shaderAnimSTRatio[MAX_SHADER_ANIMS] = {
	GFIXED(1,405),
	GFIXED_1,
	GFIXED_1,
	GFIXED_1,
	GFIXED_1,
	GFIXED_1,
};
static int	numShaderAnims;
// done.

#define		PARTICLE_GRAVITY	40
#define		MAX_PARTICLES	1024 * 8

cparticle_t	*active_particles, *free_particles;
cparticle_t	particles[MAX_PARTICLES];
int		cl_numparticles = MAX_PARTICLES;

qboolean		initparticles = qfalse;
bvec3_t			vforward, vright, vup;
bvec3_t			rforward, rright, rup;

int			oldtime;

/*
===============
CL_ClearParticles
===============
*/
void CG_ClearParticles (void)
{
	int		i;

	memset( particles, 0, sizeof(particles) );

	free_particles = &particles[0];
	active_particles = NULL;

	for (i=0 ;i<cl_numparticles ; i++)
	{
		particles[i].next = &particles[i+1];
		particles[i].type = 0;
	}
	particles[cl_numparticles-1].next = NULL;

	oldtime = cg.time;

	// Ridah, init the shaderAnims
	for (i=0; shaderAnimNames[i]; i++) {
		int j;

		for (j=0; j<shaderAnimCounts[i]; j++) {
			shaderAnims[i][j] = _CG_trap_R_RegisterShader( va("%s%i", shaderAnimNames[i], j+1) );
		}
	}
	numShaderAnims = i;
	// done.

	initparticles = qtrue;
}


/*
=====================
CG_AddParticleToScene
=====================
*/
void CG_AddParticleToScene (cparticle_t *p, bvec3_t org, gfixed alpha)
{

	bvec3_t		point;
	polyVert_t	verts[4];
	gfixed		width;
	gfixed		height;
	int		time, time2;
	gfixed		ratio;
	gfixed		invratio;
	bvec3_t		color;
	polyVert_t	TRIverts[3];
	bvec3_t		rright2, rup2;

	if (p->type == P_WEATHER || p->type == P_WEATHER_TURBULENT || p->type == P_WEATHER_FLURRY
		|| p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT)
	{// create a front facing polygon
			
		if (p->type != P_WEATHER_FLURRY)
		{
			if (p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT)
			{
				if (org[2] > MAKE_BFIXED(p->end))			
				{	
					p->time = GFIXED(cg.time,0);
					VectorCopy (org, p->org); // Ridah, fixes rare snow flakes that flicker on the ground
									
					p->org[2] = MAKE_BFIXED(p->start + 
crandom () * GFIXED(4,0));
					
					if (p->type == P_BUBBLE_TURBULENT)
					{
						p->vel[0] = crandom_b() * BFIXED(4,0);
						p->vel[1] = crandom_b() * BFIXED(4,0);
					}
				
				}
			}
			else
			{
				if (org[2] < MAKE_BFIXED(p->end))			
				{	
					p->time = MAKE_GFIXED(cg.time);
					VectorCopy (org, p->org); // Ridah, fixes rare snow flakes that flicker on the ground
									
					while (p->org[2] < MAKE_BFIXED(p->end)) 
					{
						p->org[2] += MAKE_BFIXED(p->start - p->end); 
					}
					
					
					if (p->type == P_WEATHER_TURBULENT)
					{
						p->vel[0] = crandom_b() * BFIXED(16,0);
						p->vel[1] = crandom_b() * BFIXED(16,0);
					}
				
				}
			}
			

			// Rafael snow pvs check
			if (!p->link)
				return;

			p->alpha = GFIXED_1;
		}
		
		// Ridah, had to do this or MAX_POLYS is being exceeded in village1.bsp
		if (Distance( cg.snap->ps.origin, org ) > BFIXED(1024,0)) {
			return;
		}
		// done.
	
		if (p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT)
		{
			FIXED_VEC3MA (org, -p->height, vup, point);	
			FIXED_VEC3MA (point, -p->width, vright, point);	
			VectorCopy (point, verts[0].xyz);	
			verts[0].st[0] = GFIXED_0;	
			verts[0].st[1] = GFIXED_0;	
			verts[0].modulate[0] = 255;	
			verts[0].modulate[1] = 255;	
			verts[0].modulate[2] = 255;	
			verts[0].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * p->alpha);	

			FIXED_VEC3MA (org, -p->height, vup, point);	
			FIXED_VEC3MA (point, p->width, vright, point);	
			VectorCopy (point, verts[1].xyz);	
			verts[1].st[0] = GFIXED_0;	
			verts[1].st[1] = GFIXED_1;	
			verts[1].modulate[0] = 255;	
			verts[1].modulate[1] = 255;	
			verts[1].modulate[2] = 255;	
			verts[1].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * p->alpha);	

			FIXED_VEC3MA (org, p->height, vup, point);	
			FIXED_VEC3MA (point, p->width, vright, point);	
			VectorCopy (point, verts[2].xyz);	
			verts[2].st[0] = GFIXED_1;	
			verts[2].st[1] = GFIXED_1;	
			verts[2].modulate[0] = 255;	
			verts[2].modulate[1] = 255;	
			verts[2].modulate[2] = 255;	
			verts[2].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * p->alpha);	

			FIXED_VEC3MA (org, p->height, vup, point);	
			FIXED_VEC3MA (point, -p->width, vright, point);	
			VectorCopy (point, verts[3].xyz);	
			verts[3].st[0] = GFIXED_1;	
			verts[3].st[1] = GFIXED_0;	
			verts[3].modulate[0] = 255;	
			verts[3].modulate[1] = 255;	
			verts[3].modulate[2] = 255;	
			verts[3].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * p->alpha);	
		}
		else
		{
			FIXED_VEC3MA (org, -p->height, vup, point);	
			FIXED_VEC3MA (point, -p->width, vright, point);	
			VectorCopy( point, TRIverts[0].xyz );
			TRIverts[0].st[0] = GFIXED_1;
			TRIverts[0].st[1] = GFIXED_0;
			TRIverts[0].modulate[0] = 255;
			TRIverts[0].modulate[1] = 255;
			TRIverts[0].modulate[2] = 255;
			TRIverts[0].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * p->alpha);	

			FIXED_VEC3MA (org, p->height, vup, point);	
			FIXED_VEC3MA (point, -p->width, vright, point);	
			VectorCopy (point, TRIverts[1].xyz);	
			TRIverts[1].st[0] = GFIXED_0;
			TRIverts[1].st[1] = GFIXED_0;
			TRIverts[1].modulate[0] = 255;
			TRIverts[1].modulate[1] = 255;
			TRIverts[1].modulate[2] = 255;
			TRIverts[1].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * p->alpha);	

			FIXED_VEC3MA (org, p->height, vup, point);	
			FIXED_VEC3MA (point, p->width, vright, point);	
			VectorCopy (point, TRIverts[2].xyz);	
			TRIverts[2].st[0] = GFIXED_0;
			TRIverts[2].st[1] = GFIXED_1;
			TRIverts[2].modulate[0] = 255;
			TRIverts[2].modulate[1] = 255;
			TRIverts[2].modulate[2] = 255;
			TRIverts[2].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * p->alpha);	
		}
	
	}
	else if (p->type == P_SPRITE)
	{
		bvec3_t	rr, ru;
		avec3_t	rotate_ang;

		VectorSet (color, BFIXED_1, BFIXED_1, BFIXED_1);
		time = MAKE_GFIXED(cg.time) - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;

		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );

		if (p->roll) {
			vectoangles( cg.refdef.viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += MAKE_AFIXED(p->roll);
			avec3_t arr,aru;
			AngleVectors ( rotate_ang, NULL, arr, aru);
			VectorCopyA2B(arr,rr);
			VectorCopyA2B(aru,ru);
		}

		if (p->roll) {
			FIXED_VEC3MA (org, -height, ru, point);	
			FIXED_VEC3MA (point, -width, rr, point);	
		} else {
			FIXED_VEC3MA (org, -height, vup, point);	
			FIXED_VEC3MA (point, -width, vright, point);	
		}
		VectorCopy (point, verts[0].xyz);	
		verts[0].st[0] = GFIXED_0;	
		verts[0].st[1] = GFIXED_0;	
		verts[0].modulate[0] = 255;	
		verts[0].modulate[1] = 255;	
		verts[0].modulate[2] = 255;	
		verts[0].modulate[3] = 255;

		if (p->roll) {
			FIXED_VEC3MA (point, GFIXED(2,0)*height, ru, point);	
		} else {
			FIXED_VEC3MA (point, GFIXED(2,0)*height, vup, point);	
		}
		VectorCopy (point, verts[1].xyz);	
		verts[1].st[0] = GFIXED_0;	
		verts[1].st[1] = GFIXED_1;	
		verts[1].modulate[0] = 255;	
		verts[1].modulate[1] = 255;	
		verts[1].modulate[2] = 255;	
		verts[1].modulate[3] = 255;	

		if (p->roll) {
			FIXED_VEC3MA (point, GFIXED(2,0)*width, rr, point);	
		} else {
			FIXED_VEC3MA (point, GFIXED(2,0)*width, vright, point);	
		}
		VectorCopy (point, verts[2].xyz);	
		verts[2].st[0] = GFIXED_1;	
		verts[2].st[1] = GFIXED_1;	
		verts[2].modulate[0] = 255;	
		verts[2].modulate[1] = 255;	
		verts[2].modulate[2] = 255;	
		verts[2].modulate[3] = 255;	

		if (p->roll) {
			FIXED_VEC3MA (point, -GFIXED(2,0)*height, ru, point);	
		} else {
			FIXED_VEC3MA (point, -GFIXED(2,0)*height, vup, point);	
		}
		VectorCopy (point, verts[3].xyz);	
		verts[3].st[0] = GFIXED_1;	
		verts[3].st[1] = GFIXED_0;	
		verts[3].modulate[0] = 255;	
		verts[3].modulate[1] = 255;	
		verts[3].modulate[2] = 255;	
		verts[3].modulate[3] = 255;	
	}
	else if (p->type == P_SMOKE || p->type == P_SMOKE_IMPACT)
	{// create a front rotating facing polygon

		if ( p->type == P_SMOKE_IMPACT && Distance( cg.snap->ps.origin, org ) > BFIXED(1024,0)) {
			return;
		}

		if (p->color == BLOODRED)
			VectorSet (color, BFIXED(0,22), BFIXED_0, BFIXED_0);
		else if (p->color == GREY75)
		{
			bfixed	len;
			bfixed	greyit;
			bfixed	val;
			len = Distance (cg.snap->ps.origin, org);
			if (len==BFIXED_0)
				len = BFIXED_1;

			val = BFIXED(4096,0)/len;
			greyit = BFIXED(0,25) * val;
			if (greyit > BFIXED(0,5))
				greyit = BFIXED(0,5);

			VectorSet (color, greyit, greyit, greyit);
		}
		else
			VectorSet (color, BFIXED_1, BFIXED_1, BFIXED_1);

		time = MAKE_GFIXED(cg.time) - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;
		
		if (MAKE_GFIXED(cg.time) > p->startfade)
		{
			invratio = GFIXED_1 - ( (MAKE_GFIXED(cg.time) - p->startfade) / (p->endtime - p->startfade) );

			if (p->color == EMISIVEFADE)
			{
				gfixed fval;
				fval = (invratio * invratio);
				if (fval < GFIXED_0)
					fval = GFIXED_0;
				VectorSet (color, MAKE_BFIXED(fval) , MAKE_BFIXED(fval) , MAKE_BFIXED(fval) );
			}
			invratio *= p->alpha;
		}
		else 
			invratio = GFIXED_1 * p->alpha;

		if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO )
			invratio = GFIXED_1;

		if (invratio > GFIXED_1)
			invratio = GFIXED_1;
	
		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );

		if (p->type != P_SMOKE_IMPACT)
		{
			avec3_t temp;

			vectoangles (rforward, temp);
			p->accumroll += p->roll;
			temp[ROLL] += MAKE_AFIXED(p->accumroll) * AFIXED(0,1);
			AngleVectors ( temp, NULL, rright2, rup2);
		}
		else
		{
			VectorCopy (rright, rright2);
			VectorCopy (rup, rup2);
		}
		
		if (p->rotate)
		{
			FIXED_VEC3MA (org, -height, rup2, point);	
			FIXED_VEC3MA (point, -width, rright2, point);	
		}
		else
		{
			FIXED_VEC3MA (org, -p->height, vup, point);	
			FIXED_VEC3MA (point, -p->width, vright, point);	
		}
		VectorCopy (point, verts[0].xyz);	
		verts[0].st[0] = GFIXED_0;	
		verts[0].st[1] = GFIXED_0;	
		verts[0].modulate[0] = FIXED_TO_INT(BFIXED(255,0) * color[0]);	
		verts[0].modulate[1] = FIXED_TO_INT(BFIXED(255,0) * color[1]);	
		verts[0].modulate[2] = FIXED_TO_INT(BFIXED(255,0) * color[2]);	
		verts[0].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * invratio);	

		if (p->rotate)
		{
			FIXED_VEC3MA (org, -height, rup2, point);	
			FIXED_VEC3MA (point, width, rright2, point);	
		}
		else
		{
			FIXED_VEC3MA (org, -p->height, vup, point);	
			FIXED_VEC3MA (point, p->width, vright, point);	
		}
		VectorCopy (point, verts[1].xyz);	
		verts[1].st[0] = GFIXED_0;	
		verts[1].st[1] = GFIXED_1;	
		verts[1].modulate[0] = FIXED_TO_INT(BFIXED(255,0) * color[0]);	
		verts[1].modulate[1] = FIXED_TO_INT(BFIXED(255,0) * color[1]);	
		verts[1].modulate[2] = FIXED_TO_INT(BFIXED(255,0) * color[2]);	
		verts[1].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * invratio);	

		if (p->rotate)
		{
			FIXED_VEC3MA (org, height, rup2, point);	
			FIXED_VEC3MA (point, width, rright2, point);	
		}
		else
		{
			FIXED_VEC3MA (org, p->height, vup, point);	
			FIXED_VEC3MA (point, p->width, vright, point);	
		}
		VectorCopy (point, verts[2].xyz);	
		verts[2].st[0] = GFIXED_1;	
		verts[2].st[1] = GFIXED_1;	
		verts[2].modulate[0] = FIXED_TO_INT(BFIXED(255,0) * color[0]);	
		verts[2].modulate[1] = FIXED_TO_INT(BFIXED(255,0) * color[1]);	
		verts[2].modulate[2] = FIXED_TO_INT(BFIXED(255,0) * color[2]);	
		verts[2].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * invratio);	

		if (p->rotate)
		{
			FIXED_VEC3MA (org, height, rup2, point);	
			FIXED_VEC3MA (point, -width, rright2, point);	
		}
		else
		{
			FIXED_VEC3MA (org, p->height, vup, point);	
			FIXED_VEC3MA (point, -p->width, vright, point);	
		}
		VectorCopy (point, verts[3].xyz);	
		verts[3].st[0] = GFIXED_1;	
		verts[3].st[1] = GFIXED_0;	
		verts[3].modulate[0] = FIXED_TO_INT(BFIXED(255,0) * color[0]);	
		verts[3].modulate[1] = FIXED_TO_INT(BFIXED(255,0) * color[1]);	
		verts[3].modulate[2] = FIXED_TO_INT(BFIXED(255,0) * color[2]);	
		verts[3].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * invratio);	
		
	}
	else if (p->type == P_BLEED)
	{
		bvec3_t	rr, ru;
		avec3_t	rotate_ang;
		gfixed	alpha;

		alpha = p->alpha;
		
		if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO )
			alpha = GFIXED_1;

		if (p->roll) 
		{
			vectoangles( cg.refdef.viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += MAKE_AFIXED(p->roll);
			AngleVectors ( rotate_ang, NULL, rr, ru);
		}
		else
		{
			VectorCopy (vup, ru);
			VectorCopy (vright, rr);
		}

		FIXED_VEC3MA (org, -p->height, ru, point);	
		FIXED_VEC3MA (point, -p->width, rr, point);	
		VectorCopy (point, verts[0].xyz);	
		verts[0].st[0] = GFIXED_0;	
		verts[0].st[1] = GFIXED_0;	
		verts[0].modulate[0] = 111;	
		verts[0].modulate[1] = 19;	
		verts[0].modulate[2] = 9;	
		verts[0].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * alpha);	

		FIXED_VEC3MA (org, -p->height, ru, point);	
		FIXED_VEC3MA (point, p->width, rr, point);	
		VectorCopy (point, verts[1].xyz);	
		verts[1].st[0] = GFIXED_0;	
		verts[1].st[1] = GFIXED_1;	
		verts[1].modulate[0] = 111;	
		verts[1].modulate[1] = 19;	
		verts[1].modulate[2] = 9;	
		verts[1].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * alpha);	

		FIXED_VEC3MA (org, p->height, ru, point);	
		FIXED_VEC3MA (point, p->width, rr, point);	
		VectorCopy (point, verts[2].xyz);	
		verts[2].st[0] = GFIXED_1;	
		verts[2].st[1] = GFIXED_1;	
		verts[2].modulate[0] = 111;	
		verts[2].modulate[1] = 19;	
		verts[2].modulate[2] = 9;	
		verts[2].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * alpha);	

		FIXED_VEC3MA (org, p->height, ru, point);	
		FIXED_VEC3MA (point, -p->width, rr, point);	
		VectorCopy (point, verts[3].xyz);	
		verts[3].st[0] = GFIXED_1;	
		verts[3].st[1] = GFIXED_0;	
		verts[3].modulate[0] = 111;	
		verts[3].modulate[1] = 19;	
		verts[3].modulate[2] = 9;	
		verts[3].modulate[3] = FIXED_TO_INT(GFIXED(255,0) * alpha);

	}
	else if (p->type == P_FLAT_SCALEUP)
	{
		gfixed width, height;
		bfixed sinR, cosR;

		if (p->color == BLOODRED)
			VectorSet (color, BFIXED_1, BFIXED_1, BFIXED_1);
		else
			VectorSet (color, BFIXED(0,5), BFIXED(0,5), BFIXED(0,5));
		
		time = MAKE_GFIXED(cg.time) - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;

		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );

		if (width > p->endwidth)
			width = p->endwidth;

		if (height > p->endheight)
			height = p->endheight;

		sinR = MAKE_BFIXED(height * MAKE_GFIXED(FIXED_SIN(DEG2RAD_A(MAKE_AFIXED(p->roll))) * FIXED_SQRT(AFIXED(2,0))));
		cosR = MAKE_BFIXED(width * MAKE_GFIXED(FIXED_COS(DEG2RAD_A(MAKE_AFIXED(p->roll))) * FIXED_SQRT(AFIXED(2,0))));

		VectorCopy (org, verts[0].xyz);	
		verts[0].xyz[0] -= sinR;
		verts[0].xyz[1] -= cosR;
		verts[0].st[0] = GFIXED_0;	
		verts[0].st[1] = GFIXED_0;	
		verts[0].modulate[0] = FIXED_TO_INT(GFIXED(255,0) * color[0]);	
		verts[0].modulate[1] = FIXED_TO_INT(GFIXED(255,0) * color[1]);	
		verts[0].modulate[2] = FIXED_TO_INT(GFIXED(255,0) * color[2]);	
		verts[0].modulate[3] = 255;	

		VectorCopy (org, verts[1].xyz);	
		verts[1].xyz[0] -= cosR;	
		verts[1].xyz[1] += sinR;	
		verts[1].st[0] = GFIXED_0;	
		verts[1].st[1] = GFIXED_1;	
		verts[1].modulate[0] = FIXED_TO_INT(GFIXED(255,0) * color[0]);	
		verts[1].modulate[1] = FIXED_TO_INT(GFIXED(255,0) * color[1]);	
		verts[1].modulate[2] = FIXED_TO_INT(GFIXED(255,0) * color[2]);	
		verts[1].modulate[3] = 255;	

		VectorCopy (org, verts[2].xyz);	
		verts[2].xyz[0] += sinR;	
		verts[2].xyz[1] += cosR;	
		verts[2].st[0] = GFIXED_1;	
		verts[2].st[1] = GFIXED_1;	
		verts[2].modulate[0] = FIXED_TO_INT(GFIXED(255,0) * color[0]);	
		verts[2].modulate[1] = FIXED_TO_INT(GFIXED(255,0) * color[1]);	
		verts[2].modulate[2] = FIXED_TO_INT(GFIXED(255,0) * color[2]);	
		verts[2].modulate[3] = 255;	

		VectorCopy (org, verts[3].xyz);	
		verts[3].xyz[0] += cosR;	
		verts[3].xyz[1] -= sinR;	
		verts[3].st[0] = GFIXED_1;	
		verts[3].st[1] = GFIXED_0;	
		verts[3].modulate[0] = FIXED_TO_INT(GFIXED(255,0) * color[0]);	
		verts[3].modulate[1] = FIXED_TO_INT(GFIXED(255,0) * color[1]);	
		verts[3].modulate[2] = FIXED_TO_INT(GFIXED(255,0) * color[2]);	
		verts[3].modulate[3] = 255;		
	}
	else if (p->type == P_FLAT)
	{

		VectorCopy (org, verts[0].xyz);	
		verts[0].xyz[0] -= MAKE_BFIXED(p->height);	
		verts[0].xyz[1] -= MAKE_BFIXED(p->width);	
		verts[0].st[0] = GFIXED_0;	
		verts[0].st[1] = GFIXED_0;	
		verts[0].modulate[0] = 255;	
		verts[0].modulate[1] = 255;	
		verts[0].modulate[2] = 255;	
		verts[0].modulate[3] = 255;	

		VectorCopy (org, verts[1].xyz);	
		verts[1].xyz[0] -= MAKE_BFIXED(p->height);	
		verts[1].xyz[1] += MAKE_BFIXED(p->width);	
		verts[1].st[0] = GFIXED_0;	
		verts[1].st[1] = GFIXED_1;	
		verts[1].modulate[0] = 255;	
		verts[1].modulate[1] = 255;	
		verts[1].modulate[2] = 255;	
		verts[1].modulate[3] = 255;	

		VectorCopy (org, verts[2].xyz);	
		verts[2].xyz[0] += MAKE_BFIXED(p->height);	
		verts[2].xyz[1] += MAKE_BFIXED(p->width);	
		verts[2].st[0] = GFIXED_1;	
		verts[2].st[1] = GFIXED_1;	
		verts[2].modulate[0] = 255;	
		verts[2].modulate[1] = 255;	
		verts[2].modulate[2] = 255;	
		verts[2].modulate[3] = 255;	

		VectorCopy (org, verts[3].xyz);	
		verts[3].xyz[0] += MAKE_BFIXED(p->height);	
		verts[3].xyz[1] -= MAKE_BFIXED(p->width);	
		verts[3].st[0] = GFIXED_1;	
		verts[3].st[1] = GFIXED_0;	
		verts[3].modulate[0] = 255;	
		verts[3].modulate[1] = 255;	
		verts[3].modulate[2] = 255;	
		verts[3].modulate[3] = 255;	

	}
	// Ridah
	else if (p->type == P_ANIM) {
		bvec3_t	rr, ru;
		avec3_t	rotate_ang;
		int i, j;

		time = MAKE_GFIXED(cg.time) - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;
		if (ratio >= GFIXED_1) {
			ratio = GFIXED(0,9999);
		}

		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );

		// if we are "inside" this sprite, don't draw
		if (Distance( cg.snap->ps.origin, org ) < MAKE_BFIXED(width/GFIXED(1,5))) {
			return;
		}

		i = p->shaderAnim;
		j = FIXED_TO_INT(FIXED_FLOOR(ratio * MAKE_GFIXED(shaderAnimCounts[p->shaderAnim])));
		p->pshader = shaderAnims[i][j];

		if (p->roll) {
			vectoangles( cg.refdef.viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += MAKE_AFIXED(p->roll);
			AngleVectors ( rotate_ang, NULL, rr, ru);
		}

		if (p->roll) {
			FIXED_VEC3MA (org, -height, ru, point);	
			FIXED_VEC3MA (point, -width, rr, point);	
		} else {
			FIXED_VEC3MA (org, -height, vup, point);	
			FIXED_VEC3MA (point, -width, vright, point);	
		}
		VectorCopy (point, verts[0].xyz);	
		verts[0].st[0] = GFIXED_0;	
		verts[0].st[1] = GFIXED_0;	
		verts[0].modulate[0] = 255;	
		verts[0].modulate[1] = 255;	
		verts[0].modulate[2] = 255;	
		verts[0].modulate[3] = 255;

		if (p->roll) {
			FIXED_VEC3MA (point, GFIXED(2,0)*height, ru, point);	
		} else {
			FIXED_VEC3MA (point, GFIXED(2,0)*height, vup, point);	
		}
		VectorCopy (point, verts[1].xyz);	
		verts[1].st[0] = GFIXED_0;	
		verts[1].st[1] = GFIXED_1;	
		verts[1].modulate[0] = 255;	
		verts[1].modulate[1] = 255;	
		verts[1].modulate[2] = 255;	
		verts[1].modulate[3] = 255;	

		if (p->roll) {
			FIXED_VEC3MA (point, GFIXED(2,0)*width, rr, point);	
		} else {
			FIXED_VEC3MA (point, GFIXED(2,0)*width, vright, point);	
		}
		VectorCopy (point, verts[2].xyz);	
		verts[2].st[0] = GFIXED_1;	
		verts[2].st[1] = GFIXED_1;	
		verts[2].modulate[0] = 255;	
		verts[2].modulate[1] = 255;	
		verts[2].modulate[2] = 255;	
		verts[2].modulate[3] = 255;	

		if (p->roll) {
			FIXED_VEC3MA (point, -GFIXED(2,0)*height, ru, point);	
		} else {
			FIXED_VEC3MA (point, -GFIXED(2,0)*height, vup, point);	
		}
		VectorCopy (point, verts[3].xyz);	
		verts[3].st[0] = GFIXED_1;	
		verts[3].st[1] = GFIXED_0;	
		verts[3].modulate[0] = 255;	
		verts[3].modulate[1] = 255;	
		verts[3].modulate[2] = 255;	
		verts[3].modulate[3] = 255;	
	}
	// done.
	
	if (!p->pshader) {
// (SA) temp commented out for DM
//		CG_Printf ("CG_AddParticleToScene type %d p->pshader == ZERO\n", p->type);
		return;
	}

	if (p->type == P_WEATHER || p->type == P_WEATHER_TURBULENT || p->type == P_WEATHER_FLURRY)
		_CG_trap_R_AddPolyToScene( p->pshader, 3, TRIverts );
	else
		_CG_trap_R_AddPolyToScene( p->pshader, 4, verts );

}

// Ridah, made this static so it doesn't interfere with other files
static gfixed roll = GFIXED_0;

/*
===============
CG_AddParticles
===============
*/
void CG_AddParticles (void)
{
	cparticle_t		*p, *next;
	gfixed			alpha;
	gfixed			time, time2;
	bvec3_t			org;
	int				color;
	cparticle_t		*active, *tail;
	int				type;
	avec3_t			rotate_ang;

	if (!initparticles)
		CG_ClearParticles ();

	VectorCopyA2B( cg.refdef.viewaxis[0], vforward );
	VectorCopyA2B( cg.refdef.viewaxis[1], vright );
	VectorCopyA2B( cg.refdef.viewaxis[2], vup );

	vectoangles( cg.refdef.viewaxis[0], rotate_ang );
	roll += (MAKE_GFIXED(oldtime - oldtime) * GFIXED(0,1)) ;
	rotate_ang[ROLL] += MAKE_AFIXED(roll*GFIXED(0,9));
	AngleVectors ( rotate_ang, rforward, rright, rup);
	
	oldtime = cg.time;

	active = NULL;
	tail = NULL;

	for (p=active_particles ; p ; p=next)
	{

		next = p->next;

		time = (MAKE_GFIXED(cg.time ) - p->time) * GFIXED(0,001);

		alpha = p->alpha + time*p->alphavel;
		if (alpha <= GFIXED_0)
		{	// faded out
			p->next = free_particles;
			free_particles = p;
			p->type = 0;
			p->color = 0;
			p->alpha = GFIXED_0;
			continue;
		}

		if (p->type == P_SMOKE || p->type == P_ANIM || p->type == P_BLEED || p->type == P_SMOKE_IMPACT)
		{
			if (MAKE_GFIXED(cg.time) > p->endtime)
			{
				p->next = free_particles;
				free_particles = p;
				p->type = 0;
				p->color = 0;
				p->alpha = GFIXED_0;
			
				continue;
			}

		}

		if (p->type == P_WEATHER_FLURRY)
		{
			if (MAKE_GFIXED(cg.time) > p->endtime)
			{
				p->next = free_particles;
				free_particles = p;
				p->type = 0;
				p->color = 0;
				p->alpha = GFIXED_0;
			
				continue;
			}
		}


		if (p->type == P_FLAT_SCALEUP_FADE)
		{
			if (MAKE_GFIXED(cg.time) > p->endtime)
			{
				p->next = free_particles;
				free_particles = p;
				p->type = 0;
				p->color = 0;
				p->alpha = GFIXED_0;
				continue;
			}

		}

		if ((p->type == P_BAT || p->type == P_SPRITE) && p->endtime < GFIXED_0) {
			// temporary sprite
			CG_AddParticleToScene (p, p->org, alpha);
			p->next = free_particles;
			free_particles = p;
			p->type = 0;
			p->color = 0;
			p->alpha = GFIXED_0;
			continue;
		}

		p->next = NULL;
		if (!tail)
			active = tail = p;
		else
		{
			tail->next = p;
			tail = p;
		}

		if (alpha > GFIXED_1)
			alpha = GFIXED_1;

		color = p->color;

		time2 = time*time;

		org[0] = p->org[0] + p->vel[0]*time + p->accel[0]*time2;
		org[1] = p->org[1] + p->vel[1]*time + p->accel[1]*time2;
		org[2] = p->org[2] + p->vel[2]*time + p->accel[2]*time2;

		type = p->type;

		CG_AddParticleToScene (p, org, alpha);
	}

	active_particles = active;
}

/*
======================
CG_AddParticles
======================
*/
void CG_ParticleSnowFlurry (qhandle_t pshader, centity_t *cent)
{
	cparticle_t	*p;
	qboolean turb = qtrue;

	if (!pshader)
		CG_Printf ("CG_ParticleSnowFlurry pshader == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	p->color = 0;
	p->alpha = GFIXED(0,90);
	p->alphavel = GFIXED_0;

	p->start = MAKE_GFIXED(cent->currentState.origin2[0]);
	p->end = MAKE_GFIXED(cent->currentState.origin2[1]);
	
	p->endtime = MAKE_GFIXED(cg.time + cent->currentState.time);
	p->startfade = MAKE_GFIXED(cg.time + cent->currentState.time2);
	
	p->pshader = pshader;
	
	if (rand()%100 > 90)
	{
		p->height = GFIXED(32,0);
		p->width = GFIXED(32,0);
		p->alpha = GFIXED(0,10);
	}
	else
	{
		p->height = GFIXED_1;
		p->width = GFIXED_1;
	}

	p->vel[2] = -BFIXED(20,0);

	p->type = P_WEATHER_FLURRY;
	
	if (turb)
		p->vel[2] = -BFIXED(10,0);
	
	VectorCopy(cent->currentState.origin, p->org);

	p->org[0] = p->org[0];
	p->org[1] = p->org[1];
	p->org[2] = p->org[2];

	p->vel[0] = p->vel[1] = BFIXED_0;
	
	p->accel[0] = p->accel[1] = p->accel[2] = BFIXED_0;

	p->vel[0] += cent->currentState.angles[0] * BFIXED(32,0) + (crandom_b() * BFIXED(16,0));
	p->vel[1] += cent->currentState.angles[1] * BFIXED(32,0) + (crandom_b() * BFIXED(16,0));
	p->vel[2] += MAKE_BFIXED(cent->currentState.angles[2]);

	if (turb)
	{
		p->accel[0] = crandom_b () * BFIXED(16,0);
		p->accel[1] = crandom_b () * BFIXED(16,0);
	}

}

void CG_ParticleSnow (qhandle_t pshader, bvec3_t origin, bvec3_t origin2, int turb, gfixed range, int snum)
{
	cparticle_t	*p;

	if (!pshader)
		CG_Printf ("CG_ParticleSnow pshader == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	p->color = 0;
	p->alpha = GFIXED(0,40);
	p->alphavel = GFIXED_0;
	p->start = MAKE_GFIXED(origin[2]);
	p->end = MAKE_GFIXED(origin2[2]);
	p->pshader = pshader;
	p->height = GFIXED_1;
	p->width = GFIXED_1;
	
	p->vel[2] = -BFIXED(50,0);

	if (turb)
	{
		p->type = P_WEATHER_TURBULENT;
		p->vel[2] = -BFIXED(50,0) * GFIXED(1,3);
	}
	else
	{
		p->type = P_WEATHER;
	}
	
	VectorCopy(origin, p->org);

	p->org[0] = p->org[0] + ( crandom_b() * range);
	p->org[1] = p->org[1] + ( crandom_b() * range);
	p->org[2] = p->org[2] + ( crandom_b() * (p->start - p->end)); 

	p->vel[0] = p->vel[1] = BFIXED_0;
	
	p->accel[0] = p->accel[1] = p->accel[2] = BFIXED_0;

	if (turb)
	{
		p->vel[0] = crandom_b() * BFIXED(16,0);
		p->vel[1] = crandom_b() * BFIXED(16,0);
	}

	// Rafael snow pvs check
	p->snum = snum;
	p->link = qtrue;

}

void CG_ParticleBubble (qhandle_t pshader, bvec3_t origin, bvec3_t origin2, int turb, gfixed range, int snum)
{
	cparticle_t	*p;
	gfixed		randsize;

	if (!pshader)
		CG_Printf ("CG_ParticleSnow pshader == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	p->color = 0;
	p->alpha = GFIXED(0,40);
	p->alphavel = GFIXED_0;
	p->start = MAKE_GFIXED(origin[2]);
	p->end = MAKE_GFIXED(origin2[2]);
	p->pshader = pshader;
	
	randsize = GFIXED_1 + (crandom() * GFIXED(0,5));
	
	p->height = randsize;
	p->width = randsize;
	
	p->vel[2] = BFIXED(50,0) + ( crandom_b() * BFIXED(10,0) );

	if (turb)
	{
		p->type = P_BUBBLE_TURBULENT;
		p->vel[2] = BFIXED(50,0) * BFIXED(1,3);
	}
	else
	{
		p->type = P_BUBBLE;
	}
	
	VectorCopy(origin, p->org);

	p->org[0] = p->org[0] + ( crandom_b() * range);
	p->org[1] = p->org[1] + ( crandom_b() * range);
	p->org[2] = p->org[2] + ( crandom_b() * (p->start - p->end)); 

	p->vel[0] = p->vel[1] = BFIXED_0;
	
	p->accel[0] = p->accel[1] = p->accel[2] = BFIXED_0;

	if (turb)
	{
		p->vel[0] = crandom_b() * BFIXED(4,0);
		p->vel[1] = crandom_b() * BFIXED(4,0);
	}

	// Rafael snow pvs check
	p->snum = snum;
	p->link = qtrue;

}

void CG_ParticleSmoke (qhandle_t pshader, centity_t *cent)
{

	// using cent->density = enttime
	//		 cent->frame = startfade
	cparticle_t	*p;

	if (!pshader)
		CG_Printf ("CG_ParticleSmoke == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	
	p->endtime = MAKE_GFIXED(cg.time + cent->currentState.time);
	p->startfade = MAKE_GFIXED(cg.time + cent->currentState.time2);
	
	p->color = 0;
	p->alpha = GFIXED_1;
	p->alphavel = GFIXED_0;
	p->start = MAKE_GFIXED(cent->currentState.origin[2]);
	p->end = MAKE_GFIXED(cent->currentState.origin2[2]);
	p->pshader = pshader;
	p->rotate = qfalse;
	p->height = GFIXED(8,0);
	p->width = GFIXED(8,0);
	p->endheight = GFIXED(32,0);
	p->endwidth = GFIXED(32,0);
	p->type = P_SMOKE;
	
	VectorCopy(cent->currentState.origin, p->org);

	p->vel[0] = p->vel[1] = BFIXED_0;
	p->accel[0] = p->accel[1] = p->accel[2] = BFIXED_0;

	p->vel[2] = BFIXED(5,0);

	if (cent->currentState.frame == 1)// reverse gravity	
		p->vel[2] *= -BFIXED_1;

	p->roll = FIXED_TO_INT(BFIXED(8,0) + (crandom_b() * BFIXED(4,0)));
}


void CG_ParticleBulletDebris (bvec3_t org, bvec3_t vel, int duration)
{

	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	
	p->endtime = MAKE_GFIXED(cg.time + duration);
	p->startfade = MAKE_GFIXED(cg.time + duration/2);
	
	p->color = EMISIVEFADE;
	p->alpha = GFIXED_1;
	p->alphavel = GFIXED_0;

	p->height = GFIXED(0,5);
	p->width = GFIXED(0,5);
	p->endheight = GFIXED(0,5);
	p->endwidth = GFIXED(0,5);

	p->pshader = cgs.media.tracerShader;

	p->type = P_SMOKE;
	
	VectorCopy(org, p->org);

	p->vel[0] = vel[0];
	p->vel[1] = vel[1];
	p->vel[2] = vel[2];
	p->accel[0] = p->accel[1] = p->accel[2] = BFIXED_0;

	p->accel[2] = -BFIXED(60,0);
	p->vel[2] += -BFIXED(20,0);
	
}

/*
======================
CG_ParticleExplosion
======================
*/

void CG_ParticleExplosion (const char *animStr, bvec3_t origin, bvec3_t vel, int duration, int sizeStart, int sizeEnd)
{
	cparticle_t	*p;
	int anim;

	if (animStr < (char *)10)
		CG_Error( "CG_ParticleExplosion: animStr is probably an index rather than a string" );

	// find the animation string
	for (anim=0; shaderAnimNames[anim]; anim++) {
		if (!strcasecmp( animStr, shaderAnimNames[anim] ))
			break;
	}
	if (!shaderAnimNames[anim]) {
		CG_Error("CG_ParticleExplosion: unknown animation string: %s\n", animStr);
		return;
	}

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	p->alpha = GFIXED_1;
	p->alphavel = GFIXED_0;

	if (duration < 0) {
		duration *= -1;
		p->roll = 0;
	} else {
		p->roll = FIXED_TO_INT(crandom()*GFIXED(179,0));
	}

	p->shaderAnim = anim;

	p->width = MAKE_GFIXED(sizeStart);
	p->height = MAKE_GFIXED(sizeStart)*MAKE_GFIXED(shaderAnimSTRatio[anim]);	// for sprites that are stretch in either direction

	p->endheight = MAKE_GFIXED(sizeEnd);
	p->endwidth = MAKE_GFIXED(sizeEnd)*MAKE_GFIXED(shaderAnimSTRatio[anim]);

	p->endtime = MAKE_GFIXED(cg.time + duration);

	p->type = P_ANIM;

	VectorCopy( origin, p->org );
	VectorCopy( vel, p->vel );
	VectorClear( p->accel );

}

// Rafael Shrapnel
void CG_AddParticleShrapnel (localEntity_t *le)
{
	return;
}
// done.

int CG_NewParticleArea (int num)
{
	// const char *str;
	const char *str;
	const char *token;
	int type;
	bvec3_t origin, origin2;
	int		i;
	gfixed range = GFIXED_0;
	int turb;
	int	numparticles;
	int	snum;
	
	str = CG_ConfigString (num);
	if (!str[0])
		return (0);
	
	// returns type 128 64 or 32
	token = COM_Parse ((const char **)&str);
	type = atoi (token);
	
	if (type == 1)
		range = GFIXED(128,0);
	else if (type == 2)
		range = GFIXED(64,0);
	else if (type == 3)
		range = GFIXED(32,0);
	else if (type == 0)
		range = GFIXED(256,0);
	else if (type == 4)
		range = GFIXED(8,0);
	else if (type == 5)
		range = GFIXED(16,0);
	else if (type == 6)
		range = GFIXED(32,0);
	else if (type == 7)
		range = GFIXED(64,0);


	for (i=0; i<3; i++)
	{
		token = COM_Parse ((const char **)&str);
		origin[i] = MAKE_BFIXED(atof (token));
	}

	for (i=0; i<3; i++)
	{
		token = COM_Parse ((const char **)&str);
		origin2[i] = MAKE_BFIXED(atof (token));
	}
		
	token = COM_Parse ((const char **)&str);
	numparticles = atoi (token);
	
	token = COM_Parse ((const char **)&str);
	turb = atoi (token);

	token = COM_Parse ((const char **)&str);
	snum = atoi (token);
	
	for (i=0; i<numparticles; i++)
	{
		if (type >= 4)
			CG_ParticleBubble (cgs.media.waterBubbleShader, origin, origin2, turb, range, snum);
		else
			CG_ParticleSnow (cgs.media.waterBubbleShader, origin, origin2, turb, range, snum);
	}

	return (1);
}

void	CG_SnowLink (centity_t *cent, qboolean particleOn)
{
	cparticle_t		*p, *next;
	int id;

	id = cent->currentState.frame;

	for (p=active_particles ; p ; p=next)
	{
		next = p->next;
		
		if (p->type == P_WEATHER || p->type == P_WEATHER_TURBULENT)
		{
			if (p->snum == id)
			{
				if (particleOn)
					p->link = qtrue;
				else
					p->link = qfalse;
			}
		}

	}
}

void CG_ParticleImpactSmokePuff (qhandle_t pshader, bvec3_t origin)
{
	cparticle_t	*p;

	if (!pshader)
		CG_Printf ("CG_ParticleImpactSmokePuff pshader == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	p->alpha = GFIXED(0,25);
	p->alphavel = GFIXED_0;
	p->roll = FIXED_TO_INT(crandom()*GFIXED(179,0));

	p->pshader = pshader;

	p->endtime = MAKE_GFIXED(cg.time + 1000);
	p->startfade = MAKE_GFIXED(cg.time + 100);

	p->width = MAKE_GFIXED(rand()%4 + 8);
	p->height = MAKE_GFIXED(rand()%4 + 8);

	p->endheight = p->height * GFIXED(2,0);
	p->endwidth = p->width * GFIXED(2,0);

	p->endtime = MAKE_GFIXED(cg.time + 500);

	p->type = P_SMOKE_IMPACT;

	VectorCopy( origin, p->org );
	VectorSet(p->vel, BFIXED_0, BFIXED_0, BFIXED(20,0));
	VectorSet(p->accel, BFIXED_0, BFIXED_0, BFIXED(20,0));

	p->rotate = qtrue;
}

void CG_Particle_Bleed (qhandle_t pshader, bvec3_t start, avec3_t dir, int fleshEntityNum, int duration)
{
	cparticle_t	*p;

	if (!pshader)
		CG_Printf ("CG_Particle_Bleed pshader == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	p->alpha = GFIXED_1;
	p->alphavel = GFIXED_0;
	p->roll = 0;

	p->pshader = pshader;

	p->endtime = MAKE_GFIXED(cg.time + duration);
	
	if (fleshEntityNum)
		p->startfade = MAKE_GFIXED(cg.time);
	else
		p->startfade = MAKE_GFIXED(cg.time + 100);

	p->width = GFIXED(4,0);
	p->height = GFIXED(4,0);

	p->endheight = MAKE_GFIXED(4+rand()%3);
	p->endwidth = p->endheight;

	p->type = P_SMOKE;

	VectorCopy( start, p->org );
	p->vel[0] = BFIXED_0;
	p->vel[1] = BFIXED_0;
	p->vel[2] = -BFIXED(20,0);
	VectorClear( p->accel );

	p->rotate = qfalse;

	p->roll = rand()%179;
	
	p->color = BLOODRED;
	p->alpha = GFIXED(0,75);

}

void CG_Particle_OilParticle (qhandle_t pshader, centity_t *cent)
{
	cparticle_t	*p;

	int			time;
	int			time2;
	gfixed		ratio;

	gfixed	duration = GFIXED(1500,0);

	time = cg.time;
	time2 = cg.time + cent->currentState.time;

	ratio = GFIXED_1 - FIXED_INT32RATIO_G(time,time2);

	if (!pshader)
		CG_Printf ("CG_Particle_OilParticle == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	p->alpha = GFIXED_1;
	p->alphavel = GFIXED_0;
	p->roll = 0;

	p->pshader = pshader;

	p->endtime = MAKE_GFIXED(cg.time) + duration;
	
	p->startfade = p->endtime;

	p->width = GFIXED_1;
	p->height = GFIXED(3,0);

	p->endheight = GFIXED(3,0);
	p->endwidth = GFIXED_1;

	p->type = P_SMOKE;

	VectorCopy(cent->currentState.origin, p->org );	
	
	p->vel[0] = (cent->currentState.origin2[0] * (BFIXED(16,0) * ratio));
	p->vel[1] = (cent->currentState.origin2[1] * (BFIXED(16,0) * ratio));
	p->vel[2] = (cent->currentState.origin2[2]);

	p->snum = 1;

	VectorClear( p->accel );

	p->accel[2] = -BFIXED(20,0);

	p->rotate = qfalse;

	p->roll = rand()%179;
	
	p->alpha = GFIXED(0,75);

}


void CG_Particle_OilSlick (qhandle_t pshader, centity_t *cent)
{
	cparticle_t	*p;
	
  	if (!pshader)
		CG_Printf ("CG_Particle_OilSlick == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	
	if (cent->currentState.angles2[2]!=AFIXED_0)
		p->endtime = MAKE_GFIXED(cg.time) + MAKE_GFIXED(cent->currentState.angles2[2]);
	else
		p->endtime = MAKE_GFIXED(cg.time + 60000);

	p->startfade = p->endtime;

	p->alpha = GFIXED_1;
	p->alphavel = GFIXED_0;
	p->roll = 0;

	p->pshader = pshader;

	if (cent->currentState.angles2[0]!=AFIXED_0 || cent->currentState.angles2[1]!=AFIXED_0)
	{
		p->width = MAKE_GFIXED(cent->currentState.angles2[0]);
		p->height = MAKE_GFIXED(cent->currentState.angles2[0]);

		p->endheight = MAKE_GFIXED(cent->currentState.angles2[1]);
		p->endwidth = MAKE_GFIXED(cent->currentState.angles2[1]);
	}
	else
	{
		p->width = GFIXED(8,0);
		p->height = GFIXED(8,0);

		p->endheight = GFIXED(16,0);
		p->endwidth = GFIXED(16,0);
	}

	p->type = P_FLAT_SCALEUP;

	p->snum = 1;

	VectorCopy(cent->currentState.origin, p->org );
	
	p->org[2]+= BFIXED(0,55) + (crandom_b() * BFIXED(0,5));

	p->vel[0] = BFIXED_0;
	p->vel[1] = BFIXED_0;
	p->vel[2] = BFIXED_0;
	VectorClear( p->accel );

	p->rotate = qfalse;

	p->roll = rand()%179;
	
	p->alpha = GFIXED(0,75);

}

void CG_OilSlickRemove (centity_t *cent)
{
	cparticle_t		*p, *next;
	int				id;

	id = 1;

	if (!id)
		CG_Printf ("CG_OilSlickRevove NULL id\n");

	for (p=active_particles ; p ; p=next)
	{
		next = p->next;
		
		if (p->type == P_FLAT_SCALEUP)
		{
			if (p->snum == id)
			{
				p->endtime = MAKE_GFIXED(cg.time + 100);
				p->startfade = p->endtime;
				p->type = P_FLAT_SCALEUP_FADE;

			}
		}

	}
}

qboolean ValidBloodPool (bvec3_t start)
{
#define EXTRUDE_DIST	GFIXED(0,5)

	avec3_t	angles;
	bvec3_t	right, up;
	bvec3_t	this_pos, x_pos, center_pos, end_pos;
	gfixed	x, y;
	gfixed	fwidth, fheight;
	trace_t	trace;
	bvec3_t	normal;

	fwidth = GFIXED(16,0);
	fheight = GFIXED(16,0);

	VectorSet (normal, BFIXED_0, BFIXED_0, BFIXED_1);

	vectoangles (normal, angles);
	AngleVectors (angles, NULL, right, up);

	FIXED_VEC3MA (start, EXTRUDE_DIST, normal, center_pos);

	for (x= -fwidth/GFIXED(2,0); x<fwidth; x+= fwidth)
	{
		FIXED_VEC3MA (center_pos, x, right, x_pos);

		for (y= -fheight/GFIXED(2,0); y<fheight; y+= fheight)
		{
			FIXED_VEC3MA (x_pos, y, up, this_pos);
			FIXED_VEC3MA (this_pos, -EXTRUDE_DIST*GFIXED(2,0), normal, end_pos);
			
			CG_Trace (&trace, this_pos, NULL, NULL, end_pos, -1, CONTENTS_SOLID);

			
			if (trace.entityNum < (MAX_ENTITIES - 1)) // may only land on world
				return qfalse;

			if (!(!trace.startsolid && trace.fraction < GFIXED_1))
				return qfalse;
		
		}
	}

	return qtrue;
}

void CG_BloodPool (localEntity_t *le, qhandle_t pshader, trace_t *tr)
{	
	cparticle_t	*p;
	qboolean	legit;
	bvec3_t		start;
	gfixed		rndSize;
	
	if (!pshader)
		CG_Printf ("CG_BloodPool pshader == ZERO!\n");

	if (!free_particles)
		return;
	
	VectorCopy (tr->endpos, start);
	legit = ValidBloodPool (start);

	if (!legit) 
		return;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	
	p->endtime = MAKE_GFIXED(cg.time + 3000);
	p->startfade = p->endtime;

	p->alpha = GFIXED_1;
	p->alphavel = GFIXED_0;
	p->roll = 0;

	p->pshader = pshader;

	rndSize = GFIXED(0,4) + random()*GFIXED(0,6);

	p->width = GFIXED(8,0)*rndSize;
	p->height = GFIXED(8,0)*rndSize;

	p->endheight = GFIXED(16,0)*rndSize;
	p->endwidth = GFIXED(16,0)*rndSize;
	
	p->type = P_FLAT_SCALEUP;

	VectorCopy(start, p->org );
	
	p->vel[0] = BFIXED_0;
	p->vel[1] = BFIXED_0;
	p->vel[2] = BFIXED_0;
	VectorClear( p->accel );

	p->rotate = qfalse;

	p->roll = rand()%179;
	
	p->alpha = GFIXED(0,75);
	
	p->color = BLOODRED;
}

#define NORMALSIZE	16
#define LARGESIZE	32

void CG_ParticleBloodCloud (centity_t *cent, bvec3_t origin, avec3_t dir)
{
	gfixed	length;
	gfixed	dist;
	gfixed	crittersize;
	avec3_t	angles;
	bvec3_t forward;
	bvec3_t	point;
	cparticle_t	*p;
	int		i;
	
	dist = GFIXED_0;

	length = MAKE_GFIXED(FIXED_VEC3LEN (dir));
	vectoangles (dir, angles);
	AngleVectors (angles, forward, NULL, NULL);

	crittersize = MAKE_GFIXED(LARGESIZE);

	if (length!=GFIXED_0)
		dist = length / crittersize;

	if (dist < GFIXED_1)
		dist = GFIXED_1;

	VectorCopy (origin, point);

	for (i=0; MAKE_GFIXED(i)<dist; i++)
	{
		FIXED_VEC3MA (point, crittersize, forward, point);	
		
		if (!free_particles)
			return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = MAKE_GFIXED(cg.time);
		p->alpha = GFIXED_1;
		p->alphavel = GFIXED_0;
		p->roll = 0;

		p->pshader = cgs.media.smokePuffShader;

		p->endtime = MAKE_GFIXED(cg.time + 350) + (crandom() * GFIXED(100,0));
		
		p->startfade = MAKE_GFIXED(cg.time);
		
		p->width = MAKE_GFIXED(LARGESIZE);
		p->height = MAKE_GFIXED(LARGESIZE);
		p->endheight = MAKE_GFIXED(LARGESIZE);
		p->endwidth = MAKE_GFIXED(LARGESIZE);

		p->type = P_SMOKE;

		VectorCopy( origin, p->org );
		
		p->vel[0] = BFIXED_0;
		p->vel[1] = BFIXED_0;
		p->vel[2] = -BFIXED_1;
		
		VectorClear( p->accel );

		p->rotate = qfalse;

		p->roll = rand()%179;
		
		p->color = BLOODRED;
		
		p->alpha = GFIXED(0,75);
		
	}

	
}

void CG_ParticleSparks (bvec3_t org, bvec3_t vel, int duration, gfixed x, gfixed y, gfixed speed)
{
	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED( cg.time );
	
	p->endtime = MAKE_GFIXED(cg.time + duration);
	p->startfade = MAKE_GFIXED(cg.time + duration/2);
	
	p->color = EMISIVEFADE;
	p->alpha = GFIXED(0,4);
	p->alphavel = GFIXED_0;

	p->height = GFIXED(0,5);
	p->width = GFIXED(0,5);
	p->endheight = GFIXED(0,5);
	p->endwidth = GFIXED(0,5);

	p->pshader = cgs.media.tracerShader;

	p->type = P_SMOKE;
	
	VectorCopy(org, p->org);

	p->org[0] += (crandom_b() * x);
	p->org[1] += (crandom_b() * y);

	p->vel[0] = vel[0];
	p->vel[1] = vel[1];
	p->vel[2] = vel[2];

	p->accel[0] = p->accel[1] = p->accel[2] = BFIXED_0;

	p->vel[0] += (crandom_b() * GFIXED(4,0));
	p->vel[1] += (crandom_b() * GFIXED(4,0));
	p->vel[2] += (BFIXED(20,0) + (crandom_b() * GFIXED(10,0))) * speed;	

	p->accel[0] = crandom_b () * GFIXED(4,0);
	p->accel[1] = crandom_b () * GFIXED(4,0);
	
}

void CG_ParticleDust (centity_t *cent, bvec3_t origin, avec3_t dir)
{
	gfixed	length;
	gfixed	dist;
	gfixed	crittersize;
	avec3_t	angles;
	bvec3_t forward;
	bvec3_t	point;
	cparticle_t	*p;
	int		i;
	
	dist = GFIXED_0;

	VectorNegate (dir, dir);
	length = MAKE_GFIXED(FIXED_VEC3LEN (dir));
	vectoangles (dir, angles);
	AngleVectors (angles, forward, NULL, NULL);

	crittersize = MAKE_GFIXED(LARGESIZE);

	if (length!=GFIXED_0)
		dist = length / crittersize;

	if (dist < GFIXED_1)
		dist = GFIXED_1;

	VectorCopy (origin, point);

	for (i=0; MAKE_GFIXED(i)<dist; i++)
	{
		FIXED_VEC3MA (point, crittersize, forward, point);	
				
		if (!free_particles)
			return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = MAKE_GFIXED(cg.time);
		p->alpha = GFIXED(5,0);
		p->alphavel = GFIXED_0;
		p->roll = 0;

		p->pshader = cgs.media.smokePuffShader;

		// RF, stay around for long enough to expand and dissipate naturally
		if (length!=GFIXED_0)
			p->endtime = MAKE_GFIXED(cg.time + 4500) + (crandom() * GFIXED(3500,0));
		else
			p->endtime = MAKE_GFIXED(cg.time + 750) + (crandom() * GFIXED(500,0));
		
		p->startfade = MAKE_GFIXED(cg.time);
		
		p->width = MAKE_GFIXED(LARGESIZE);
		p->height = MAKE_GFIXED(LARGESIZE);

		// RF, expand while falling
		p->endheight = MAKE_GFIXED(LARGESIZE*3);
		p->endwidth = MAKE_GFIXED(LARGESIZE*3);

		if (length!=GFIXED_0)
		{
			p->width *= GFIXED(0,2);
			p->height *= GFIXED(0,2);

			p->endheight = MAKE_GFIXED(NORMALSIZE);
			p->endwidth = MAKE_GFIXED(NORMALSIZE);
		}

		p->type = P_SMOKE;

		VectorCopy( point, p->org );
		
		p->vel[0] = crandom_b()*GFIXED(6,0);
		p->vel[1] = crandom_b()*GFIXED(6,0);
		p->vel[2] = random_b()*GFIXED(20,0);

		// RF, add some gravity/randomness
		p->accel[0] = crandom_b()*GFIXED(3,0);
		p->accel[1] = crandom_b()*GFIXED(3,0);
		p->accel[2] = -BFIXED(PARTICLE_GRAVITY,0)*GFIXED(0,4);

		VectorClear( p->accel );

		p->rotate = qfalse;

		p->roll = rand()%179;
		
		p->alpha = GFIXED(0,75);
		
	}

	
}

void CG_ParticleMisc (qhandle_t pshader, bvec3_t origin, int size, int duration, gfixed alpha)
{
	cparticle_t	*p;

	if (!pshader)
		CG_Printf ("CG_ParticleImpactSmokePuff pshader == ZERO!\n");

	if (!free_particles)
		return;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = MAKE_GFIXED(cg.time);
	p->alpha = GFIXED_1;
	p->alphavel = GFIXED_0;
	p->roll = rand()%179;

	p->pshader = pshader;

	if (duration > 0)
		p->endtime = MAKE_GFIXED(cg.time + duration);
	else
		p->endtime = MAKE_GFIXED(duration);

	p->startfade = MAKE_GFIXED(cg.time);

	p->width = MAKE_GFIXED(size);
	p->height = MAKE_GFIXED(size);

	p->endheight = MAKE_GFIXED(size);
	p->endwidth = MAKE_GFIXED(size);

	p->type = P_SPRITE;

	VectorCopy( origin, p->org );

	p->rotate = qfalse;
}
