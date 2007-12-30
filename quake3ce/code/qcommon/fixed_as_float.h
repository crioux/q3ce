#pragma once
#ifndef __INC_FIXED_AS_FLOAT_H
#define __INC_FIXED_AS_FLOAT_H


// DECLARATIONS /////////

typedef float afixed;
typedef float bfixed;
typedef float cfixed;
typedef float dfixed;
typedef float gfixed;
typedef double lfixed;

#define AFIXED(n,d) (n.d##f)
#define BFIXED(n,d) (n.d##f)
#define CFIXED(n,d) (n.d##f)
#define DFIXED(n,d) (n.d##f)
#define GFIXED(n,d) (n.d##f)
#define LFIXED(n,d) (n.d)


/// TESTS ////////////////
#define FIXED_NOT_ZERO(x) ((x)!=0)
#define FIXED_NOT_ONE(x) ((x)!=1)
#define FIXED_IS_ZERO(x) ((x)==0)
#define FIXED_IS_ONE(x) ((x)==1)


/// OPERATIONS ///////////////

inline float  AFIXED_RANDOM(void) { return ((float)(rand() & 65535))/65536.0f; }
inline float  BFIXED_RANDOM(void) { return ((float)(rand() & 65535))/65536.0f; }
inline float  CFIXED_RANDOM(void) { return ((float)(rand() & 65535))/65536.0f; }
inline float  DFIXED_RANDOM(void) { return ((float)(rand() & 65535))/65536.0f; }
inline float  GFIXED_RANDOM(void) { return ((float)(rand() & 65535))/65536.0f; }
inline double LFIXED_RANDOM(void) { return ((double)(rand() & 65535))/65536.0f; }


inline float  FIXED_ABS(float x) { return fabsf(x); }
inline double FIXED_ABS(double x) { return fabs(x); }
inline float  FIXED_SQRT(float x) { return sqrtf(x); }
inline double FIXED_SQRT(double x) { return sqrt(x); }
inline float  FIXED_MULPOW2(float x, int pow2) { return x*(float)(1<<pow2); }
inline double FIXED_MULPOW2(double x, int pow2) { return x/(double)(1<<pow2); }
inline float  FIXED_DIVPOW2(float x, int pow2) { return x/(float)(1<<pow2); }
inline double FIXED_DIVPOW2(double x, int pow2) { return x/(double)(1<<pow2); }
inline float  FIXED_FASTINVSQRT(float x) 
{
	long i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = x * 0.5f;
	y  = x;
	i  = * ( long * ) &y;						// evil floating point bit level hacking
	i  = 0x5f375a86 - ( i >> 1 );               // what the fuck v2.0?
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;    
}
inline double FIXED_FASTINVSQRT(double x) 
{
	INT64 i;
	double x2, y;
	const double threehalfs = 1.5;

	x2 = x * 0.5;
	y  = x;
	i  = * ( INT64  * ) &y;						// evil floating point bit level hacking
	i  = 0x5fe6ec85e7de30da - ( i >> 1 );       // what the fuck v2.1?
	y  = * ( double * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;        
}
inline float  FIXED_SNAP(float x) { return (float)(int)(x); }
inline double FIXED_SNAP(double x) { return (double)(int)(x); }
inline float  FIXED_VEC2DOT(const float *a, const float *b) { return a[0]*b[0]+a[1]*b[1]; }
inline double FIXED_VEC2DOT(const double *a, const double *b) { return a[0]*b[0]+a[1]*b[1]; }
inline float  FIXED_VEC3DOT(const float *a, const float *b) { return (a[0]*b[0])+(a[1]*b[1])+(a[2]*b[2]); }
inline double FIXED_VEC3DOT(const double *a, const double *b) { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }
inline float  FIXED_VEC4DOT(const float *a, const float *b) { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]; }
inline double FIXED_VEC4DOT(const double *a, const double *b) { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]; }
#define FIXED_VEC2DOT_R FIXED_VEC2DOT
#define FIXED_VEC3DOT_R FIXED_VEC3DOT
#define FIXED_VEC4DOT_R FIXED_VEC4DOT
inline void   FIXED_VEC2SCALE(const float *a, float s, float *o) { o[0]=a[0]*s; o[1]=a[1]*s; }
inline void   FIXED_VEC2SCALE(const double *a, double s, double *o) { o[0]=a[0]*s; o[1]=a[1]*s; }
inline void   FIXED_VEC3SCALE(const float *a, float s, float *o) { o[0]=a[0]*s; o[1]=a[1]*s; o[2]=a[2]*s; }
inline void   FIXED_VEC3SCALE(const double *a, double s, double *o) { o[0]=a[0]*s; o[1]=a[1]*s; o[2]=a[2]*s; }
inline void   FIXED_VEC4SCALE(const float *a, float s, float *o){  o[0]=a[0]*s; o[1]=a[1]*s; o[2]=a[2]*s; o[3]=a[3]*s; }
inline void   FIXED_VEC4SCALE(const double *a, double s, double *o) { o[0]=a[0]*s; o[1]=a[1]*s; o[2]=a[2]*s; o[3]=a[3]*s; }
#define FIXED_VEC2SCALE_R FIXED_VEC2SCALE
#define FIXED_VEC3SCALE_R FIXED_VEC3SCALE
#define FIXED_VEC4SCALE_R FIXED_VEC4SCALE
inline void   FIXED_VEC2MA(const float *v, float s, const float *b, float *o) { o[0]=v[0]+b[0]*s; o[1]=v[1]+b[1]*s; }
inline void   FIXED_VEC2MA(const double *v, double s, const double *b, double *o) { o[0]=v[0]+b[0]*s; o[1]=v[1]+b[1]*s; }
inline void   FIXED_VEC3MA(const float *v, float s, const float *b, float *o) { o[0]=v[0]+b[0]*s; o[1]=v[1]+b[1]*s; o[2]=v[2]+b[2]*s; }
inline void   FIXED_VEC3MA(const double *v, double s, const double *b, double *o) { o[0]=v[0]+b[0]*s; o[1]=v[1]+b[1]*s; o[2]=v[2]+b[2]*s; }
inline void   FIXED_VEC4MA(const float *v, float s, const float *b, float *o) { o[0]=v[0]+b[0]*s; o[1]=v[1]+b[1]*s; o[2]=v[2]+b[2]*s; o[3]=v[3]+b[3]*s; }
inline void   FIXED_VEC4MA(const double *v, double s, const double *b, double *o) { o[0]=v[0]+b[0]*s; o[1]=v[1]+b[1]*s; o[2]=v[2]+b[2]*s; o[3]=v[3]+b[3]*s; }
#define FIXED_VEC2MA_R FIXED_VEC2MA
#define FIXED_VEC3MA_R FIXED_VEC3MA
#define FIXED_VEC4MA_R FIXED_VEC4MA
inline float  FIXED_VEC2LEN(const float *v) { return (float)sqrt((((double)(v[0]))*((double)(v[0])))+(((double)(v[1]))*((double)(v[1])))); }
inline double FIXED_VEC2LEN(const double *v) { return sqrt((v[0]*v[0])+(v[1]*v[1])); }
inline float  FIXED_VEC3LEN(const float *v) { return (float)sqrt((((double)(v[0]))*((double)(v[0])))+(((double)(v[1]))*((double)(v[1])))+(((double)(v[2]))*((double)(v[2])))); }
inline double FIXED_VEC3LEN(const double *v) { return sqrt((v[0]*v[0])+(v[1]*v[1])+(((v[2]))*((v[2])))); }
inline float  FIXED_VEC4LEN(const float *v) { return (float)sqrt((((double)(v[0]))*((double)(v[0])))+(((double)(v[1]))*((double)(v[1])))+(((double)(v[2]))*((double)(v[2])))+(((double)(v[3]))*((double)(v[3])))); }
inline double FIXED_VEC4LEN(const double *v) { return sqrt((v[0]*v[0])+(v[1]*v[1])+(v[2]*v[2])+(v[3]*v[3])); }
inline float  FIXED_VEC2LEN_SQ(const float *v) { return (v[0]*v[0])+(v[1]*v[1]); }
inline double FIXED_VEC2LEN_SQ(const double *v) { return (v[0]*v[0])+(v[1]*v[1]); }
inline float  FIXED_VEC3LEN_SQ(const float *v) { return (v[0]*v[0])+(v[1]*v[1])+(v[2]*v[2]); }
inline double FIXED_VEC3LEN_SQ(const double *v) { return (v[0]*v[0])+(v[1]*v[1])+(v[2]*v[2]); }
inline float  FIXED_VEC4LEN_SQ(const float *v) { return (v[0]*v[0])+(v[1]*v[1])+(v[2]*v[2])+(v[3]*v[3]); }
inline double FIXED_VEC4LEN_SQ(const double *v) { return (v[0]*v[0])+(v[1]*v[1])+(v[2]*v[2])+(v[3]*v[3]); }
inline void   FIXED_FASTVEC3NORM(float *v) { float ilength=FIXED_FASTINVSQRT(FIXED_VEC3LEN_SQ(v)); v[0] *= ilength; v[1] *= ilength; v[2] *= ilength; }
inline void   FIXED_FASTVEC3NORM(double *v) { double ilength=FIXED_FASTINVSQRT(FIXED_VEC3LEN_SQ(v)); v[0] *= ilength; v[1] *= ilength; v[2] *= ilength; }
inline float  FIXED_FASTVEC3INVLEN(float *v) { return FIXED_FASTINVSQRT(FIXED_VEC3LEN_SQ(v)); }
inline double FIXED_FASTVEC3INVLEN(double *v) { return FIXED_FASTINVSQRT(FIXED_VEC3LEN_SQ(v)); }

/// MATH /////////////////

inline float  FIXED_SIN(float a) { return ((float)sin((double)a)); }
inline double FIXED_SIN(double a) { return sin(a); }
inline float  FIXED_COS(float a) { return ((float)cos((double)a)); }
inline double FIXED_COS(double a) { return cos(a); }
inline float  FIXED_TAN(float a) { return ((float)tan((float)a)); }
inline double FIXED_TAN(double a) { return tan(a); }
inline float  FIXED_ASIN(float a) { return ((float)asin((double)a)); }
inline double FIXED_ASIN(double a) { return asin(a); }
inline float  FIXED_ACOS(float a) { return ((float)asin((double)a)); }
inline double FIXED_ACOS(double a) { return asin(a); }
inline float  FIXED_ATAN(float a) { return ((float)atan((double)a)); }
inline double FIXED_ATAN(double a) { return atan(a); }

inline float FIXED_ATAN2(float y,float x) { return ((float)atan2((double)y,(double)x)); }
inline double FIXED_ATAN2(double y,double x) { return atan2(y,x); }
inline float FIXED_CEIL(float a) { return ((float)ceil((double)a)); }
inline double FIXED_CEIL(double a) { return ceil(a); }
inline float FIXED_FLOOR(float a) { return ((float)floor((double)a)); }
inline double FIXED_FLOOR(double a) { return floor(a); }
inline float FIXED_POW(float a, float b) { return ((float)pow((double)(a),(double)(b))); }
inline double FIXED_POW(double a, double b) { return pow(a,b); }
inline float FIXED_MOD(float a, float b) { return ((float)fmod((double)(a),(double)(b))); }
inline double FIXED_MOD(double a, double b) { return fmod(a,b); }
inline float FIXED_MOD_CLAMP(float a,float b) { return (b/65536.0f) * ((float)(((int)(a*(65536.0f/b))) & 65535)); }
inline double FIXED_MOD_CLAMP(double a,double b) { return (b/4294967296.0) * ((double)(((INT64)(a*(4294967296.0f/b))) & 4294967295)); }
inline float FIXED_MAX(float a,float b) { return (a < b) ? b : a ; }
inline double FIXED_MAX(double a,double b) { return (a < b) ? b : a ; }
inline float FIXED_MIN(float a,float b) { return (a < b) ? a : b ; }
inline double FIXED_MIN(double a,double b) { return (a < b) ? a : b ; }



inline int FIXED_AS_INT(float x) { return *(int *)(&(x)); }

inline float INT_AS_AFIXED(int x) { return *(float *)(&(x)); }
inline float INT_AS_BFIXED(int x) { return *(float *)(&(x)); }
inline float INT_AS_CFIXED(int x) { return *(float *)(&(x)); }
inline float INT_AS_DFIXED(int x) { return *(float *)(&(x)); }
inline float INT_AS_GFIXED(int x) { return *(float *)(&(x)); }

#define FIXED_TO_INT(x) ((int)(x))
#define FIXED_TO_FLOAT(x) ((float)(x))
#define FIXED_TO_DOUBLE(x) ((double)(x))

#define AFIXED_PI ((float)3.14159265358979323846)
#define BFIXED_PI ((float)3.14159265358979323846)
#define CFIXED_PI ((float)3.14159265358979323846)
#define DFIXED_PI ((float)3.14159265358979323846)
#define GFIXED_PI ((float)3.14159265358979323846)
#define LFIXED_PI (3.14159265358979323846)

#define AFIXED_1 (1.0f)
#define AFIXED_0 (0.0f)
#define BFIXED_1 (1.0f)
#define BFIXED_0 (0.0f)
#define CFIXED_1 (1.0f)
#define CFIXED_0 (0.0f)
#define DFIXED_1 (1.0f)
#define DFIXED_0 (0.0f)
#define GFIXED_1 (1.0f)
#define GFIXED_0 (0.0f)
#define LFIXED_1 (1.0)
#define LFIXED_0 (0.0)

#define MAKE_AFIXED(x) ((float)(x))
#define MAKE_BFIXED(x) ((float)(x))
#define MAKE_CFIXED(x) ((float)(x))
#define MAKE_DFIXED(x) ((float)(x))
#define MAKE_GFIXED(x) ((float)(x))
#define MAKE_LFIXED(x) ((double)(x))

inline INT32 FIXED_INT32SCALE(INT32 i, float s)
{
	return (INT32)(i*s);
}

inline float FIXED_INT32RATIO_G(INT32 n, INT32 d)
{
	return ((float)n)/((float)d);
}

inline float FIXED_INT32RATIO_A(INT32 n, INT32 d)
{
	return ((float)n)/((float)d);
}

inline float FIXED_INT32RATIO_B(INT32 n, INT32 d)
{
	return ((float)n)/((float)d);
}

inline float FIXED_RATIO_G(float n, float d)
{
	return n/d;
}

inline float FIXED_RATIO_B(float n, float d)
{
	return n/d;
}

inline float FIXED_RATIO_A(float n, float d)
{
	return n/d;
}




#define REINTERPRET_GFIXED(x) (x)
#define REINTERPRET_BFIXED(x) (x)

#endif