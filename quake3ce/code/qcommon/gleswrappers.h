#pragma once

#include"../qcommon/fixed.h"
#include"GLES/egl.h"
#include"GLES/gl.h"

#define GLFLOAT_1 (1.0f)
#define GLFLOAT_0 (0.0f)
#define GLFIXED_1 (0x00010000)
#define GLFIXED_0 (0x00000000)

#ifdef FIXED_IS_FLOAT
#define GL_FIXED_OR_FLOAT GL_FLOAT
#else
#define GL_FIXED_OR_FLOAT GL_FIXED
#endif

union GLNUMBER
{
	GLfloat f;
	GLfixed x;
};

#define GW_BUFFERSIZE (16384)

//#define GL_POINTS                         0x0000
//#define GL_LINES                          0x0001
//#define GL_LINE_LOOP                      0x0002
//#define GL_LINE_STRIP                     0x0003
//#define GL_TRIANGLES                      0x0004
//#define GL_TRIANGLE_STRIP                 0x0005
//#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_QUAD_STRIP                     0x0008
#define GL_POLYGON                        0x0009

//////////////////////////////////////////////////////////////////////////////

extern GLfixed g_btfvals[256];

extern GLenum g_curmode;
extern int g_texfixedmode;
extern int g_vtxfixedmode;
extern GLNUMBER g_vtxbuf[GW_BUFFERSIZE];
extern GLNUMBER g_texbuf[GW_BUFFERSIZE];
//extern GLNUMBER g_texbuf2[GW_BUFFERSIZE];
extern unsigned short g_indices[GW_BUFFERSIZE];
extern int g_vtxsize;
extern int g_texsize;
extern int g_vtxlen;
extern int g_texlen;
extern int g_vtxmod3;
extern int g_texmod3;
extern int g_idxsize;

extern int g_glbegincount;

//////////////////////////////////////////////////////////////////////////////

void glBegin(GLenum mode);
void glEnd(void);

void glTexCoord2x (GLfixed s, GLfixed t);
void glTexCoord2f (GLfloat s, GLfloat t);
inline void glTexCoord2X (gfixed s, gfixed t) 
{
#ifdef FIXED_IS_FLOAT
	glTexCoord2f(s,t);
#else
	glTexCoord2x(s.rep,t.rep);
#endif
}

void glVertex2x (GLfixed x, GLfixed y);
void glVertex2f (GLfloat x, GLfloat y);
inline void glVertex2X(gfixed x, gfixed y) 
{
#ifdef FIXED_IS_FLOAT
	glVertex2f(x,y);
#else
	glVertex2x(x.rep,y.rep);
#endif
}


void glVertex3x (GLfixed x, GLfixed y, GLfixed z);
void glVertex3f (GLfloat x, GLfloat y, GLfloat z);
inline void glVertex3X(gfixed x, gfixed y, gfixed z) 
{
#ifdef FIXED_IS_FLOAT
	glVertex3f(x,y,z);
#else
	glVertex3x(x.rep,y.rep,z.rep);
#endif
}

#ifdef TARGET_AXIMX50V
typedef void (*TYPEOF_pClipPlanexIMG)( GLenum p, const GLfixed *eqn );
extern void (*pClipPlanexIMG)( GLenum p, const GLfixed *eqn );
#endif


//void glMultiTexCoord2x (GLfixed s, GLfixed t);
//void glMultiTexCoord2f (GLfloat s, GLfloat t);
//void glMultiTexCoord2X (gfixed s, gfixed t);

__inline void pushtexfixed(GLfixed x)
{
	ASSERT(g_texfixedmode==-1 || g_texfixedmode==1);
	g_texfixedmode=1;

	g_texbuf[g_texlen++].x=x;
	ASSERT(g_texlen<=GW_BUFFERSIZE);
}

__inline void pushvtxfixed(GLfixed x)
{
	ASSERT(g_vtxfixedmode==-1 || g_vtxfixedmode==1);
	g_vtxfixedmode=1;

	g_vtxbuf[g_vtxlen++].x=x;
	ASSERT(g_vtxlen<=GW_BUFFERSIZE);
}

__inline void pushtexfloat(GLfloat f)
{
	ASSERT(g_texfixedmode==-1 || g_texfixedmode==0);
	g_texfixedmode=0;

	g_texbuf[g_texlen++].f=f;
	ASSERT(g_texlen<=GW_BUFFERSIZE);
}

__inline void pushvtxfloat(GLfloat f)
{
	ASSERT(g_vtxfixedmode==-1 || g_vtxfixedmode==0);
	g_vtxfixedmode=0;

	g_vtxbuf[g_vtxlen++].f=f;
	ASSERT(g_vtxlen<=GW_BUFFERSIZE);
}


__inline void pushindex(unsigned short idx)
{
	g_indices[g_idxsize++]=idx;
	ASSERT(g_idxsize<=GW_BUFFERSIZE);
}


__inline void glTexCoord2xv (const GLfixed *v)
{
	glTexCoord2x(v[0],v[1]);
}
__inline void glTexCoord2fv (const GLfloat *v)
{
	glTexCoord2f(v[0],v[1]);
}
__inline void glTexCoord2Xv (const gfixed *v)
{
#ifdef FIXED_IS_FLOAT
	glTexCoord2f(v[0],v[1]);
#else
	glTexCoord2x(FIXED_GETREP(v[0]),FIXED_GETREP(v[1]));
#endif
	
}

__inline void glVertex3xv (const GLfixed *v)
{
	glVertex3x(v[0],v[1],v[2]);
}
__inline void glVertex3fv (const GLfloat *v)
{
	glVertex3f(v[0],v[1],v[2]);
}
__inline void glVertex3Xv (const gfixed *v)
{
#ifdef FIXED_IS_FLOAT
	glVertex3f(v[0],v[1],v[2]);
#else
	glVertex3x(FIXED_GETREP(v[0]),FIXED_GETREP(v[1]),FIXED_GETREP(v[2]));
#endif
}


__inline void glColor4ubv( const GLubyte *v)
{
	glColor4x(g_btfvals[(unsigned int)v[0]],g_btfvals[(unsigned int)v[1]],g_btfvals[(unsigned int)v[2]],g_btfvals[(unsigned int)v[3]]);
}

__inline void glColor3x(GLfixed r, GLfixed g, GLfixed b)
{
	glColor4x(r,g,b,GLFIXED_1);
}
__inline void glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
	glColor4f(r,g,b,GLFLOAT_1);
}
__inline void glColor3X(gfixed r, gfixed g, gfixed b)
{
#ifdef FIXED_IS_FLOAT
	glColor4f(r,g,b,GLFLOAT_1);
#else
	glColor4x(FIXED_GETREP(r),FIXED_GETREP(g),FIXED_GETREP(b),GLFIXED_1);
#endif
}

__inline void glArrayElement(GLint idx)
{
	pushindex(idx);
}


__inline void glLoadMatrixX(const gfixed *m)
{
#ifdef FIXED_IS_FLOAT
	glLoadMatrixf(m);
#else
	glLoadMatrixx((const GLfixed *)m);
#endif
}

__inline void glTranslateX(gfixed x, gfixed y, gfixed z)
{
#ifdef FIXED_IS_FLOAT
	glTranslatef(x,y,z);
#else
	glTranslatex(FIXED_GETREP(x),FIXED_GETREP(y),FIXED_GETREP(z));
#endif
}


__inline void glDepthRangeX( gfixed z0, gfixed z1)
{
#ifdef FIXED_IS_FLOAT
	glDepthRangef(z0,z1);
#else
	glDepthRangex(FIXED_GETREP(z0),FIXED_GETREP(z1));
#endif
}

__inline void glColor4X( gfixed r, gfixed g, gfixed b, gfixed a)
{
#ifdef FIXED_IS_FLOAT
	glColor4f(r,g,b,a);
#else
	glColor4x(FIXED_GETREP(r),FIXED_GETREP(g),FIXED_GETREP(b),FIXED_GETREP(a));
#endif
}


__inline void glPolygonOffsetX (gfixed factor, gfixed units)
{
#ifdef FIXED_IS_FLOAT
	glPolygonOffset(factor,units);
#else
	glPolygonOffsetx(FIXED_GETREP(factor),FIXED_GETREP(units));
#endif
}

__inline void glClearDepthX( gfixed d)
{
#ifdef FIXED_IS_FLOAT
	glClearDepthf(d);
#else
	glClearDepthx(FIXED_GETREP(d));
#endif
}

__inline void glAlphaFuncX( GLenum func, gfixed ref)
{
#ifdef FIXED_IS_FLOAT
	glAlphaFunc(func,ref);
#else
	glAlphaFuncx(func,FIXED_GETREP(ref));
#endif
}
			
__inline void glClearColorX( gfixed r, gfixed g, gfixed b, gfixed a )
{
#ifdef FIXED_IS_FLOAT
	glClearColor(r,g,b,a);
#else
	glClearColorx(FIXED_GETREP(r),FIXED_GETREP(g),FIXED_GETREP(b),FIXED_GETREP(a));
#endif
}
	
__inline void glClipPlaneX(GLenum plane, const gfixed *eq)
{
#ifdef FIXED_IS_FLOAT
	glClipPlanef(plane,eq);
#else
	#ifdef TARGET_AXIMX50V
/*
		if(pClipPlanexIMG)
		{
			(*pClipPlanexIMG)(plane,(const GLfixed *)eq);
		}
*/
	#else
		glClipPlanex(plane,(const GLfixed *)eq);
	#endif	
#endif
}
		

__inline void glOrthoX(gfixed left, gfixed right, gfixed bottom, gfixed top, gfixed near_val, gfixed far_val)
{
#ifdef FIXED_IS_FLOAT
	glOrthof(left,right,bottom,top,near_val,far_val);
#else
	glOrthox(FIXED_GETREP(left),FIXED_GETREP(right),FIXED_GETREP(bottom),FIXED_GETREP(top),FIXED_GETREP(near_val),FIXED_GETREP(far_val));
#endif	
}




