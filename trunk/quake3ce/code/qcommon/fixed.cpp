#ifndef FIXED_IS_FLOAT

#ifndef _WIN32
#define one 1
#define zero 0
#include<math.h>

#endif

#ifdef ARM

extern "C" void VEC3DOT32_11(INT32 *rep_a, INT32 *rep_b, INT32 * rep_out);

#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1>::TYPE fixed_base<T,U,P,EXT,S0,S1>::fast_invsqrt(void) const
{
	float number=this->ToFloat();
	
	long i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = number * 0.5f;
	y  = number;
	i  = * ( long * ) &y;						// evil floating point bit level hacking
	i  = 0x5f375a86 - ( i >> 1 );               // what the fuck v2.0?
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return Construct(y);  
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::sqrt(void) const
{
	return Construct(::sqrtf(ToFloat()));
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::sin(void) const
{
	return Construct(::sin(FIXED_TO_DOUBLE(*this)));
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::cos(void) const
{
	return Construct(::cos(FIXED_TO_DOUBLE(*this)));
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::tan(void) const
{
	return Construct(::tan(FIXED_TO_DOUBLE(*this)));
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::asin(void) const
{
	return Construct(::asin(FIXED_TO_DOUBLE(*this)));
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::acos(void) const
{
	return Construct(::acos(FIXED_TO_DOUBLE(*this)));
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::atan(void) const
{
	return Construct(::atan(FIXED_TO_DOUBLE(*this)));
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::atan2(const fixed_base y, const fixed_base x)
{
	return Construct(::atan2(FIXED_TO_DOUBLE(y),FIXED_TO_DOUBLE(x)));
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::mod(const fixed_base y) const
{
	return Construct(::fmod(FIXED_TO_DOUBLE(*this),FIXED_TO_DOUBLE(y)));
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::mod_clamp(const fixed_base y) const
{
	float a=FIXED_TO_FLOAT(*this);
	float b=FIXED_TO_FLOAT(y);
	return Construct((b/65536.0f) * ((float)(((int)(a*(65536.0f/b))) & 65535)));
}

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::pow(const fixed_base y) const
{
	return Construct(::pow(FIXED_TO_DOUBLE(*this),FIXED_TO_DOUBLE(y)));
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<class T>
void fixed_fast_vec3norm(T *v)
{
	typename T::exttype v0s=((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0);
	v0s*=v0s;
	typename T::exttype v1s=((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0);
	v1s*=v1s;
	typename T::exttype v2s=((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0);
	v2s*=v2s;

	T lensq;
	lensq.rep=((v0s+v1s+v2s+T::round1)>>T::shift1);
	
	T ilen=lensq.fast_invsqrt();
	
	v[0]*=ilen;
	v[1]*=ilen;
	v[2]*=ilen;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<class T>
T fixed_vec2dot(const T *x, const T *y)
{
	typename T::exttype dot=(( ((((typename T::exttype)x[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)y[0].rep)+T::round0)>>T::shift0)+
		       ((((typename T::exttype)x[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)y[1].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(dot,typename T::exttype,T)) { ASSERT(0); }
#endif
	return FromRep((typename T::basetype)dot);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
T fixed_vec3dot(const T *x, const T *y)
{
	typename T::exttype dot=(( ((((typename T::exttype)x[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)y[0].rep)+T::round0)>>T::shift0)+
		       ((((typename T::exttype)x[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)y[1].rep)+T::round0)>>T::shift0)+
			   ((((typename T::exttype)x[2].rep)+T::round0)>>T::shift0)*((((typename T::exttype)y[2].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(dot,typename T::exttype,T)) { ASSERT(0); }
#endif
	return FromRep((typename T::basetype)dot);
}

#ifdef ARM
template<>
bfixed fixed_vec3dot<bfixed>(const bfixed *x, const bfixed *y)
{
	bfixed out;
	VEC3DOT32_11((INT32 *)x,(INT32 *)y,&(out.rep));
	return out;
}
#endif


template<class T>
T fixed_vec4dot(const T *x, const T *y)
{
	typename T::exttype dot=(( ((((typename T::exttype)x[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)y[0].rep)+T::round0)>>T::shift0)+
		       ((((typename T::exttype)x[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)y[1].rep)+T::round0)>>T::shift0)+
		       ((((typename T::exttype)x[2].rep)+T::round0)>>T::shift0)*((((typename T::exttype)y[2].rep)+T::round0)>>T::shift0)+
			   ((((typename T::exttype)x[3].rep)+T::round0)>>T::shift0)*((((typename T::exttype)y[3].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(dot,typename T::exttype,T)) { ASSERT(0); }
#endif
	return FromRep((typename T::basetype)dot);
}


template<class T>
void fixed_vec2scale(const T *v, const T scale, T *o)
{
	typename T::exttype s=(((typename T::exttype)scale.rep)+T::round0)>>T::shift0;
	typename T::exttype o0=(((((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o1=(((((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename T::exttype,T)) { ASSERT(0); }
#endif

	o[0]=FromRep((typename T::basetype)o0);
	o[1]=FromRep((typename T::basetype)o1);
}

template<class T>
void fixed_vec3scale(const T *v, const T scale, T *o)
{
	typename T::exttype s=(((typename T::exttype)scale.rep)+T::round0)>>T::shift0;

	typename T::exttype o0=(((((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o1=(((((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o2=(((((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename T::exttype,T)) { ASSERT(0); }
#endif

	o[0]=FromRep((typename T::basetype)o0);
	o[1]=FromRep((typename T::basetype)o1);
	o[2]=FromRep((typename T::basetype)o2);
}

template<class T>
void fixed_vec4scale(const T *v, const T scale, T *o) 
{
	typename T::exttype s=(((typename T::exttype)scale.rep)+T::round0)>>T::shift0;

	typename T::exttype o0=(((((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o1=(((((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o2=(((((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o3=(((((((typename T::exttype)v[3].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o3,typename T::exttype,T)) { ASSERT(0); }
#endif

	o[0]=FromRep((typename T::basetype)o0);
	o[1]=FromRep((typename T::basetype)o1);
	o[2]=FromRep((typename T::basetype)o2);
	o[3]=FromRep((typename T::basetype)o3);
}


template<class T>
void fixed_vec2ma(const T *v, const T scale, const T *b, T *o)
{
	typename T::exttype s=(((typename T::exttype)scale.rep)+T::round0)>>T::shift0;

	typename T::exttype o0=(((((((typename T::exttype)b[0].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o1=(((((((typename T::exttype)b[1].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename T::exttype,T)) { ASSERT(0); }
#endif

	o[0]=v[0]+FromRep((typename T::basetype)o0);
	o[1]=v[1]+FromRep((typename T::basetype)o1);
}

template<class T>
void fixed_vec3ma(const T *v, const T scale, const T *b, T *o)
{
	typename T::exttype s=(((typename T::exttype)scale.rep)+T::round0)>>T::shift0;

	typename T::exttype o0=(((((((typename T::exttype)b[0].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o1=(((((((typename T::exttype)b[1].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o2=(((((((typename T::exttype)b[2].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename T::exttype,T)) { ASSERT(0); }
#endif

	o[0]=v[0]+FromRep((typename T::basetype)o0);
	o[1]=v[1]+FromRep((typename T::basetype)o1);
	o[2]=v[2]+FromRep((typename T::basetype)o2);
}

template<class T>
void fixed_vec4ma(const T *v, const T scale, const T *b, T *o)
{
	typename T::exttype s=(((typename T::exttype)scale.rep)+T::round0)>>T::shift0;

	typename T::exttype o0=(((((((typename T::exttype)b[0].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o1=(((((((typename T::exttype)b[1].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o2=(((((((typename T::exttype)b[2].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);
	typename T::exttype o3=(((((((typename T::exttype)b[3].rep)+T::round0)>>T::shift0)*s)+T::round1)>>T::shift1);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename T::exttype,T)) { ASSERT(0); }
	if(IS_BAD_CAST(o3,typename T::exttype,T)) { ASSERT(0); }
#endif

	o[0]=v[0]+FromRep((typename T::basetype)o0);
	o[1]=v[1]+FromRep((typename T::basetype)o1);
	o[2]=v[2]+FromRep((typename T::basetype)o2);
	o[3]=v[3]+FromRep((typename T::basetype)o3);
}

template<class T>
T fixed_vec2len(const T *v)
{
	typename T::exttype sumsq=(( ((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
	return T::Construct(sqrt(((double)sumsq)/((double)one)));
}

template<class T>
T fixed_vec3len(const T *v)
{
	typename T::exttype sumsq=(( ((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
	return T::Construct(sqrt(((double)sumsq)/((double)one)));
}

template<class T>
T fixed_vec4len(const T *v)
{
	typename T::exttype sumsq=(( ((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)+
				 ((((typename T::exttype)v[3].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[3].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
	return T::Construct(sqrt(((double)sumsq)/((double)one)));
}

template<class T>
T fixed_vec2len_sq(const T *v)
{
	typename T::exttype sumsq=(( ((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)+
				 ((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(sumsq,typename T::exttype,T)) { ASSERT(0); }
#endif
	return T::FromRep((typename T::basetype)sumsq);
}

template<class T>
T fixed_vec3len_sq(const T *v)
{
	typename T::exttype sumsq=(( ((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)+
				 ((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0) ));
	sumsq +=T::round1;
	
	sumsq = sumsq >> T::shift1;
#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(sumsq,typename T::exttype,T)) { ASSERT(0); }
#endif
	return T::FromRep((typename T::basetype)sumsq);
}

template<class T>
T fixed_vec4len_sq(const T *v)
{
	typename T::exttype sumsq=(( ((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)+
				 ((((typename T::exttype)v[3].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[3].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(sumsq,typename T::exttype,T)) { ASSERT(0); }
#endif
	return T::FromRep((typename T::basetype)sumsq);
}

////////////////////////////////////////////////////////////////

//template fixed_base<INT32,UINT32,20,INT64,0,20>;
//template fixed_base<INT32,UINT32,11,INT64,0,11>;
//template fixed_base<INT32,UINT32,24,INT64,0,24>;
//template fixed_base<INT32,UINT32,8,INT64,0,8>;
//template fixed_base<INT32,UINT32,16,INT64,0,16>;
//template fixed_base<INT64,UINT64,32,INT64,16,0>;

//template afixed;
//template bfixed;
//template cfixed;
//template dfixed;
//template gfixed;
//template lfixed;

////////////////////////////////////////////////////////////////

#endif

