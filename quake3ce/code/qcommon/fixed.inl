


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, class U, int P, class EXT,int S0, int S1>
fixed_base<T,U,P,EXT,S0,S1> fixed_base<T,U,P,EXT,S0,S1>::fastinvsqrt(void) const
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
T fixed_fastvec3invlen(T *v)
{
	//xxx
	return T();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<class T>
void fixed_fastvec3norm(T *v)
{
	typename T::exttype v0s=((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0);
	v0s*=v0s;
	typename T::exttype v1s=((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0);
	v1s*=v1s;
	typename T::exttype v2s=((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0);
	v2s*=v2s;

	T lensq;
	lensq.rep=(typename T::basetype)((v0s+v1s+v2s+T::round1)>>T::shift1);
	
	T ilen=lensq.fastinvsqrt();
	
	v[0]*=ilen;
	v[1]*=ilen;
	v[2]*=ilen;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<class B, class S>
B fixed_vec2dot(const B *x, const S *y)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename B::exttype dot=(( ((((typename B::exttype)x[0].rep)+B::round0)>>B::shift0)*((((typename S::exttype)y[0].rep)+S::round0)>>S::shift0)+
		              ((((typename B::exttype)x[1].rep)+B::round0)>>B::shift0)*((((typename S::exttype)y[1].rep)+S::round0)>>S::shift0) + round1bs ) >> shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(dot,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif
	return B::FromRep((typename B::basetype)dot);
}

template<class B, class S>
B fixed_vec3dot(const B *x, const S *y)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename B::exttype dot=(( ((((typename B::exttype)x[0].rep)+B::round0)>>B::shift0)*((((typename S::exttype)y[0].rep)+S::round0)>>S::shift0)+
		              ((((typename B::exttype)x[1].rep)+B::round0)>>B::shift0)*((((typename S::exttype)y[1].rep)+S::round0)>>S::shift0)+
					  ((((typename B::exttype)x[2].rep)+B::round0)>>B::shift0)*((((typename S::exttype)y[2].rep)+S::round0)>>S::shift0)+ round1bs ) >> shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(dot,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif
	return B::FromRep((typename B::basetype)dot);
}


template<class B, class S>
B fixed_vec4dot(const B *x, const S *y)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename B::exttype dot=(( ((((typename B::exttype)x[0].rep)+B::round0)>>B::shift0)*((((typename S::exttype)y[0].rep)+S::round0)>>S::shift0)+
		              ((((typename B::exttype)x[1].rep)+B::round0)>>B::shift0)*((((typename S::exttype)y[1].rep)+S::round0)>>S::shift0)+
					  ((((typename B::exttype)x[2].rep)+B::round0)>>B::shift0)*((((typename S::exttype)y[2].rep)+S::round0)>>S::shift0)+
					  ((((typename B::exttype)x[3].rep)+B::round0)>>B::shift0)*((((typename S::exttype)y[3].rep)+S::round0)>>S::shift0)+ round1bs ) >> shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(dot,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif
	return B::FromRep((typename B::basetype)dot);
}



template<class B, class S>
B fixed_vec2dot_r(const S *x, const B *y)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename B::exttype dot=(( ((((typename S::exttype)x[0].rep)+S::round0)>>S::shift0)*((((typename B::exttype)y[0].rep)+B::round0)>>B::shift0)+
		              ((((typename S::exttype)x[1].rep)+S::round0)>>S::shift0)*((((typename B::exttype)y[1].rep)+B::round0)>>B::shift0) + round1bs ) >> shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(dot,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif
	return B::FromRep((typename B::basetype)dot);
}

template<class B, class S>
B fixed_vec3dot_r(const S *x, const B *y)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename B::exttype dot=(( ((((typename S::exttype)x[0].rep)+S::round0)>>S::shift0)*((((typename B::exttype)y[0].rep)+B::round0)>>B::shift0)+
		              ((((typename S::exttype)x[1].rep)+S::round0)>>S::shift0)*((((typename B::exttype)y[1].rep)+B::round0)>>B::shift0)+
					  ((((typename S::exttype)x[2].rep)+S::round0)>>S::shift0)*((((typename B::exttype)y[2].rep)+B::round0)>>B::shift0)+ round1bs ) >> shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(dot,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif
	return B::FromRep((typename B::basetype)dot);
}


template<class B, class S>
B fixed_vec4dot_r(const S *x, const B *y)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename B::exttype dot=(( ((((typename S::exttype)x[0].rep)+S::round0)>>S::shift0)*((((typename B::exttype)y[0].rep)+B::round0)>>B::shift0)+
		              ((((typename S::exttype)x[1].rep)+S::round0)>>S::shift0)*((((typename B::exttype)y[1].rep)+B::round0)>>B::shift0)+
					  ((((typename S::exttype)x[2].rep)+S::round0)>>S::shift0)*((((typename B::exttype)y[2].rep)+B::round0)>>B::shift0)+
					  ((((typename S::exttype)x[3].rep)+S::round0)>>S::shift0)*((((typename B::exttype)y[3].rep)+B::round0)>>B::shift0)+ round1bs ) >> shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(dot,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif
	return B::FromRep((typename B::basetype)dot);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class B, class S>
void fixed_vec2scale(const B *v, const S scale, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename S::exttype s=(((typename S::exttype)scale.rep)+S::round0)>>S::shift0;
	typename B::exttype o0=(((((((typename B::exttype)v[0].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename B::exttype)v[1].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=B::FromRep((typename B::basetype)o0);
	o[1]=B::FromRep((typename B::basetype)o1);
}

template<class B, class S>
void fixed_vec3scale(const B *v, const S scale, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename S::exttype s=(((typename S::exttype)scale.rep)+S::round0)>>S::shift0;
	typename B::exttype o0=(((((((typename B::exttype)v[0].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename B::exttype)v[1].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o2=(((((((typename B::exttype)v[2].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=B::FromRep((typename B::basetype)o0);
	o[1]=B::FromRep((typename B::basetype)o1);
	o[2]=B::FromRep((typename B::basetype)o2);
}

template<class B, class S>
void fixed_vec4scale(const B *v, const S scale, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename S::exttype s=(((typename S::exttype)scale.rep)+S::round0)>>S::shift0;
	typename B::exttype o0=(((((((typename B::exttype)v[0].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename B::exttype)v[1].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o2=(((((((typename B::exttype)v[2].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o3=(((((((typename B::exttype)v[3].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o3,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=B::FromRep((typename B::basetype)o0);
	o[1]=B::FromRep((typename B::basetype)o1);
	o[2]=B::FromRep((typename B::basetype)o2);
	o[3]=B::FromRep((typename B::basetype)o3);
}

template<class B, class S>
void fixed_vec2scale_r(const S *v, const B scale, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename B::exttype s=(((typename B::exttype)scale.rep)+B::round0)>>B::shift0;
	typename B::exttype o0=(((((((typename S::exttype)v[0].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename S::exttype)v[1].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=B::FromRep((typename B::basetype)o0);
	o[1]=B::FromRep((typename B::basetype)o1);
}

template<class B, class S>
void fixed_vec3scale_r(const S *v, const B scale, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename B::exttype s=(((typename B::exttype)scale.rep)+B::round0)>>B::shift0;
	typename B::exttype o0=(((((((typename S::exttype)v[0].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename S::exttype)v[1].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o2=(((((((typename S::exttype)v[2].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=B::FromRep((typename B::basetype)o0);
	o[1]=B::FromRep((typename B::basetype)o1);
	o[2]=B::FromRep((typename B::basetype)o2);
}

template<class B, class S>
void fixed_vec4scale_r(const B *v, const S scale, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename B::exttype s=(((typename B::exttype)scale.rep)+B::round0)>>B::shift0;
	typename S::exttype o0=(((((((typename S::exttype)v[0].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename S::exttype o1=(((((((typename S::exttype)v[1].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename S::exttype o2=(((((((typename S::exttype)v[2].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename S::exttype o3=(((((((typename S::exttype)v[3].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o3,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=B::FromRep((typename B::basetype)o0);
	o[1]=B::FromRep((typename B::basetype)o1);
	o[2]=B::FromRep((typename B::basetype)o2);
	o[3]=B::FromRep((typename B::basetype)o3);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class B, class S>
void fixed_vec2ma(const B *v, const S scale, const B *b, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename S::exttype s=(((typename S::exttype)scale.rep)+S::round0)>>S::shift0;

	typename B::exttype o0=(((((((typename B::exttype)b[0].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename B::exttype)b[1].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=v[0]+B::FromRep((typename B::basetype)o0);
	o[1]=v[1]+B::FromRep((typename B::basetype)o1);
}

template<class B, class S>
void fixed_vec3ma(const B *v, const S scale, const B *b, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;
	
	typename S::exttype s=(((typename S::exttype)scale.rep)+S::round0)>>S::shift0;

	typename B::exttype o0=(((((((typename B::exttype)b[0].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename B::exttype)b[1].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o2=(((((((typename B::exttype)b[2].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=v[0]+B::FromRep((typename B::basetype)o0);
	o[1]=v[1]+B::FromRep((typename B::basetype)o1);
	o[2]=v[2]+B::FromRep((typename B::basetype)o2);
}

template<class B, class S>
void fixed_vec4ma(const B *v, const S scale, const B *b, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;
	
	typename S::exttype s=(((typename S::exttype)scale.rep)+S::round0)>>S::shift0;

	typename B::exttype o0=(((((((typename B::exttype)b[0].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename B::exttype)b[1].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o2=(((((((typename B::exttype)b[2].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o3=(((((((typename B::exttype)b[3].rep)+B::round0)>>B::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o3,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=v[0]+B::FromRep((typename B::basetype)o0);
	o[1]=v[1]+B::FromRep((typename B::basetype)o1);
	o[2]=v[2]+B::FromRep((typename B::basetype)o2);
	o[3]=v[3]+B::FromRep((typename B::basetype)o3);
}

template<class B, class S>
void fixed_vec2ma_r(const B *v, const B scale, const S *b, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;

	typename B::exttype s=(((typename B::exttype)scale.rep)+B::round0)>>B::shift0;

	typename B::exttype o0=(((((((typename S::exttype)b[0].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename S::exttype)b[1].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=v[0]+B::FromRep((typename B::basetype)o0);
	o[1]=v[1]+B::FromRep((typename B::basetype)o1);
}

template<class B, class S>
void fixed_vec3ma_r(const B *v, const B scale, const S *b, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;
	
	typename B::exttype s=(((typename B::exttype)scale.rep)+B::round0)>>B::shift0;

	typename B::exttype o0=(((((((typename S::exttype)b[0].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename S::exttype)b[1].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o2=(((((((typename S::exttype)b[2].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=v[0]+B::FromRep((typename B::basetype)o0);
	o[1]=v[1]+B::FromRep((typename B::basetype)o1);
	o[2]=v[2]+B::FromRep((typename B::basetype)o2);
}

template<class B, class S>
void fixed_vec4ma_r(const B *v, const B scale, const S *b, B *o)
{
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const typename B::exttype round1bs=(((typename B::exttype)1)<<shift1bs)>>1;
	
	typename B::exttype s=(((typename B::exttype)scale.rep)+B::round0)>>B::shift0;

	typename B::exttype o0=(((((((typename S::exttype)b[0].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o1=(((((((typename S::exttype)b[1].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o2=(((((((typename S::exttype)b[2].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);
	typename B::exttype o3=(((((((typename S::exttype)b[3].rep)+S::round0)>>S::shift0)*s)+round1bs)>>shift1bs);

#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(o0,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o1,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o2,typename B::exttype,typename B::basetype)) { ASSERT(0); }
	if(IS_BAD_CAST(o3,typename B::exttype,typename B::basetype)) { ASSERT(0); }
#endif

	o[0]=v[0]+B::FromRep((typename B::basetype)o0);
	o[1]=v[1]+B::FromRep((typename B::basetype)o1);
	o[2]=v[2]+B::FromRep((typename B::basetype)o2);
	o[3]=v[3]+B::FromRep((typename B::basetype)o3);
}



template<class T>
T fixed_vec2len(const T *v)
{
	typename T::exttype sumsq=(( ((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
	return T::Construct(::sqrt(((double)sumsq)/((double)T::one)));
}

template<class T>
T fixed_vec3len(const T *v)
{
	typename T::exttype sumsq=(( ((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
	return T::Construct(::sqrt(((double)sumsq)/((double)T::one)));
}

template<class T>
T fixed_vec4len(const T *v)
{
	typename T::exttype sumsq=(( ((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)+
		         ((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[2].rep)+T::round0)>>T::shift0)+
				 ((((typename T::exttype)v[3].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[3].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
	return T::Construct(::sqrt(((double)sumsq)/((double)T::one)));
}

template<class T>
T fixed_vec2len_sq(const T *v)
{
	typename T::exttype sumsq=(( ((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[0].rep)+T::round0)>>T::shift0)+
				 ((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0)*((((typename T::exttype)v[1].rep)+T::round0)>>T::shift0) + T::round1 )>>T::shift1);
#ifdef VERIFY_FIXEDREP 
	if(IS_BAD_CAST(sumsq,typename T::exttype,typename T::basetype)) { ASSERT(0); }
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
	if(IS_BAD_CAST(sumsq,typename T::exttype,typename T::basetype)) { ASSERT(0); }
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
	if(IS_BAD_CAST(sumsq,typename T::exttype,typename T::basetype)) { ASSERT(0); }
#endif
	return T::FromRep((typename T::basetype)sumsq);
}
