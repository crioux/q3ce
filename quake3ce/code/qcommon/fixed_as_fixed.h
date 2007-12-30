#pragma once
#ifndef __INC_FIXED_AS_FIXED_H
#define __INC_FIXED_AS_FIXED_H


#ifdef _DEBUG
#define FORCEINLINE inline
#define VERIFY_FIXEDREP 1
//#define FIXED_REFERENCE_IMPL 1
#else
#define FORCEINLINE __forceinline
//#define FIXED_REFERENCE_IMPL 1
#endif

#pragma warning(push)
#pragma warning(disable:4293)

#define BREAK { __emit(0xE6000010); }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VERIFICATION /////////////

#define MAXFIXREPHIGH ((T)((((T)1)<<(((sizeof(T)*8)-P)-1))-1))
#define MINFIXREPHIGH ((-MAXFIXREPHIGH)-1)
#define MAXFIXREPLOW ((U)((((U)1)<<(P))-1))
#define MINFIXREPLOW ((U)0)

#ifdef VERIFY_FIXEDREP 
#define VERIFYFIXREPHIGH(n) { if(!((MINFIXREPHIGH<=((INT32)n)) && (((INT32)n)<=MAXFIXREPHIGH))) { wchar_t str[256]; _snwprintf(str,256,L"VERIFYFIXREPHIGH:%8.8X:%d with %d precision\n",n,n,P); OutputDebugString(str); BREAK; } }
#define VERIFYFIXREPLOW(d) { if(!((MINFIXREPLOW<=((UINT32)d)) && (((UINT32)d)<=MAXFIXREPLOW))) { wchar_t str[256]; _snwprintf(str,256,L"VERIFYFIXREPLOW:%8.8X:%d with %d precision\n",d,d,P); OutputDebugString(str); BREAK; } }
#define IS_BAD_CAST(X,FROM,TO)	\
	((((X)<((FROM)0)) && ((((X)&~((((FROM)1)<<(sizeof(TO)*8))-(FROM)1))!=~((((FROM)1)<<(sizeof(TO)*8))-(FROM)1)) || ((TO)(X))>=((TO)0))) ||	\
	(((X)>=((FROM)0)) && ((((X)&~((((FROM)1)<<(sizeof(TO)*8))-(FROM)1))!=((FROM)0)) || ((TO)(X))<((TO)0))))

template<class T> 
inline bool would_overflow (T a, T b)
{
	T c = a;
	if (a > 0 && b > 0)
	    // The sum of positive numbers can't be less than any of the numbers.
		return (c += b) < a;
	else if (a < 0 && b < 0)
	    // The sum of negative numbers can't be greater than any of the numbers.
		return (c += b) > a;
	else
	    // A negative and positive addition cannot overflow.
		return false;
}

//#define VERIFYFLTZERO(of,fr) { if(((of)==0.0f) != ((fr)==0)) { OutputDebugString(L"VERIFYFLTZERO: lost low-end precision\n"); BREAK; } }
//#define VERIFYDBLZERO(of,fr) { if(((of)==0.0) != ((fr)==0)) { OutputDebugString(L"VERIFYDBLZERO: lost low-end precision\n"); BREAK; } }
#define VERIFYFLTZERO(of,fr)
#define VERIFYDBLZERO(of,fr)

#else
#define VERIFYFIXREPHIGH(n) 
#define VERIFYFIXREPLOW(d) 
#define VERIFYFLTZERO(of,fr)
#define VERIFYDBLZERO(of,fr)
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DECLARATION /////////////

// T=base type
// U=unsigned base type
// EXT=extended precision type
// P=number of binary precision bits
// S0=number of extended pre-shift bits
// S1=number of extended post-shift bits
template<class T, class U, int P, class EXT,int S0, int S1>
class fixed_base
{
public:
	typedef T basetype;
	typedef U ubasetype;
	typedef EXT exttype;
	static const int prec=P;
	static const int shift0=S0;
	static const int shift1=S1;
	//static const EXT round0=((((EXT)1)<<shift0)>>1);
	//static const EXT round1=((((EXT)1)<<shift1)>>1);
	//static const EXT roundp=((((EXT)1)<<prec)>>1);
	static const EXT round0=0;
	static const EXT round1=0;
	static const EXT roundp=0;
	
	static const T lowmask=((((T)1)<<P)-1);
	static const T highmask=~((((T)1)<<P)-1);
	static const T one=(((T)1)<<P);
	static const T zero=((T)0);

public:
	T rep;

private:

	FORCEINLINE static  T MulRep(T a, T b) 
	{ 
		EXT out=(( ((((EXT)a)+round0)>>S0) * ((((EXT)b)+round0)>>S0) + round1 )>>S1); 
#ifdef VERIFY_FIXEDREP 
		if(IS_BAD_CAST(out,EXT,T))
		{
			wchar_t str[256];
			_snwprintf(str,256,L"MulRep32: a=%8.8X, b=%8.8X, out=%8.8X\n",a,b,(T)out);
			OutputDebugString(str);
			BREAK;
		}
#endif
		return (T)out;
	}

	FORCEINLINE static T fixed_base<T,U,P,EXT,S0,S1>::DivRep(T a, T b) 
	{
	#ifdef VERIFY_FIXEDREP 
		if(b==0 || ((EXT)b>>shift0)==0) { BREAK; }
	#endif
		EXT out=((((EXT)a)<<shift1)/(((EXT)b)>>shift0))<<shift0;
	#ifdef VERIFY_FIXEDREP 
		if(IS_BAD_CAST(out,EXT,T)) { BREAK; }
	#endif
		return (T)out;
	}


	
public:
	
	static FORCEINLINE fixed_base Construct(void) { fixed_base f; f.rep=(T)0; return f;}
	static FORCEINLINE fixed_base Construct(T n, U d) { 
		fixed_base f;
		VERIFYFIXREPHIGH(n);	
		VERIFYFIXREPLOW(d);	
		f.rep=((n<<P) | d); 
		return f;
	}
	static FORCEINLINE fixed_base Construct(const fixed_base &copy) { fixed_base f; f.rep=copy.rep; return f; }
	template<class X>
	static FORCEINLINE fixed_base Construct(X otherfixed) { 
		fixed_base f;
		const X::basetype round=(1<<(unsigned)(X::prec-prec))>>1;
		f.rep=(prec>X::prec) ?
			(((basetype)otherfixed.rep)<<(unsigned)(prec-X::prec)) :
			((basetype)((otherfixed.rep+round)>>(unsigned)(X::prec-prec))); 
		#ifdef VERIFY_FIXEDREP
		if(prec>X::prec)
		{
			basetype test=((basetype)otherfixed.rep);
			ubasetype bitmask=(~((~((ubasetype)0))>>(unsigned)(prec-X::prec)));
			{
				ASSERT((test & bitmask)==bitmask || (test & bitmask)==0);
				// Ensure sign didn't change
				if((test & bitmask)==bitmask)
				{
					ASSERT(f.rep<0);
				}
				else
				{
					ASSERT(f.rep>=0);
				}
			}
		}
		#endif
		return f;
	}
	static FORCEINLINE fixed_base Construct(bool otherint) { fixed_base f; int val=otherint?1:0; VERIFYFIXREPHIGH(val); f.rep=((T)val)<<P; return f; }
	static FORCEINLINE fixed_base Construct(char otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)otherint)<<P; return f; }
	static FORCEINLINE fixed_base Construct(signed char otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)otherint)<<P; return f; }
	static FORCEINLINE fixed_base Construct(short otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)otherint)<<P; return f;}
	static FORCEINLINE fixed_base Construct(int otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)otherint)<<P; return f;}
	static FORCEINLINE fixed_base Construct(INT64 otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)otherint)<<P; return f;}
	static FORCEINLINE fixed_base Construct(long otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)otherint)<<P; return f;}
	static FORCEINLINE fixed_base Construct(unsigned char otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)(U)otherint)<<P; return f;}
	static FORCEINLINE fixed_base Construct(unsigned short otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)(U)otherint)<<P; return f;}
	static FORCEINLINE fixed_base Construct(unsigned int otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)(U)otherint)<<P; return f;}
	static FORCEINLINE fixed_base Construct(UINT64 otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)(U)otherint)<<P; return f;}
	static FORCEINLINE fixed_base Construct(unsigned long otherint) { fixed_base f; VERIFYFIXREPHIGH(otherint); f.rep=((T)(U)otherint)<<P; return f;}
	static FORCEINLINE fixed_base Construct(float otherflt) { fixed_base f; VERIFYFIXREPHIGH((T)otherflt); f.rep=(T)(otherflt*(float)one); VERIFYFLTZERO(otherflt,f.rep); return f;}
	static FORCEINLINE fixed_base Construct(double otherdbl) { fixed_base f; VERIFYFIXREPHIGH((T)otherdbl); f.rep=(T)(otherdbl*(double)one); VERIFYDBLZERO(otherdbl,f.rep); return f; }

	static FORCEINLINE fixed_base FromRep(T newrep) { fixed_base f; f.rep=newrep; return f; }
	
	FORCEINLINE int ToInt() const { return (int)(rep>>P); }
	FORCEINLINE float ToFloat() const { return ((float)rep)/(float)one; }
	FORCEINLINE double ToDouble() const { return ((double)rep)/(double)one; }

	FORCEINLINE bool IsNotZero(void) const { return rep!=zero; }
	FORCEINLINE bool IsZero(void) const { return rep==zero; }
	FORCEINLINE bool IsNotOne(void) const { return rep!=one; }
	FORCEINLINE bool IsOne(void) const { return rep==one; }

	FORCEINLINE fixed_base operator-(void) const { return FromRep(-rep); }
	FORCEINLINE fixed_base operator+(void) const { return *this; }

	FORCEINLINE fixed_base operator+(const fixed_base other) const { 
#ifdef VERIFY_FIXEDREP
		ASSERT(!would_overflow(rep,other.rep));
#endif
		return FromRep(rep+other.rep); 
	}
	FORCEINLINE fixed_base operator-(const fixed_base other) const { 
#ifdef VERIFY_FIXEDREP
		ASSERT(!would_overflow(rep,-other.rep));
#endif
		return FromRep(rep-other.rep);
	}
	FORCEINLINE fixed_base operator*(const fixed_base other) const { return FromRep(MulRep(rep,other.rep)); }
	FORCEINLINE fixed_base operator/(const fixed_base other) const { return FromRep(DivRep(rep,other.rep)); }

	FORCEINLINE fixed_base& operator++() { 
#ifdef VERIFY_FIXEDREP
		ASSERT(!would_overflow<T>(rep,1));
#endif		
		rep++; 
		return *this; 
	}
	FORCEINLINE fixed_base operator++(int) { 
#ifdef VERIFY_FIXEDREP
		ASSERT(!would_overflow<T>(rep,1));
#endif			
		T temprep=rep; 
		++rep; 
		return FromRep(temprep); 
	}
	FORCEINLINE fixed_base& operator--() { 
#ifdef VERIFY_FIXEDREP
		ASSERT(!would_overflow<T>(rep,-1));
#endif		
		rep--; 
		return *this; 
	}
	FORCEINLINE fixed_base operator--(int) { 
#ifdef VERIFY_FIXEDREP
		ASSERT(!would_overflow<T>(rep,-1));
#endif		
		T temprep=rep; 
		--rep; 
		return FromRep(temprep); 
	}
	
	FORCEINLINE fixed_base & operator+=(const fixed_base other) { 
#ifdef VERIFY_FIXEDREP
		ASSERT(!would_overflow<T>(rep,other.rep));
#endif		
		rep+=other.rep; 
		return *this; 
	}
	FORCEINLINE fixed_base & operator-=(const fixed_base other) { 
#ifdef VERIFY_FIXEDREP
		ASSERT(!would_overflow<T>(rep,-other.rep));
#endif	
		rep-=other.rep; 
		return *this; 
	}
	FORCEINLINE fixed_base & operator*=(const fixed_base other) { rep=MulRep(rep,other.rep); return *this; }
	FORCEINLINE fixed_base & operator/=(const fixed_base other) { rep=DivRep(rep,other.rep); return *this; }

	FORCEINLINE bool operator!=(const fixed_base other) const { return rep!=other.rep;}
	FORCEINLINE bool operator==(const fixed_base other) const {return rep==other.rep;}
	FORCEINLINE bool operator<(const fixed_base other) const { return rep<other.rep;}
	FORCEINLINE bool operator<=(const fixed_base other) const {return rep<=other.rep;}
	FORCEINLINE bool operator>(const fixed_base other) const {return rep>other.rep;}
	FORCEINLINE bool operator>=(const fixed_base other) const {return rep>=other.rep;}

	fixed_base sqrt(void) const;
	
	FORCEINLINE fixed_base abs(void) const { return FromRep((rep ^ (rep >> (P-1))) + (rep < 0)); }
	FORCEINLINE fixed_base snap(void) const { return FromRep(rep & highmask); }
	
	fixed_base sin(void) const;
	fixed_base cos(void) const;
	fixed_base tan(void) const;
	fixed_base asin(void) const;
	fixed_base acos(void) const;
	fixed_base atan(void) const;
	static fixed_base atan2(fixed_base y, fixed_base x);
	FORCEINLINE fixed_base ceil(void) const { return FromRep((rep + lowmask)&highmask); }
	FORCEINLINE fixed_base floor(void) const { return FromRep(rep&highmask); }
	fixed_base mod(const fixed_base y) const;
	fixed_base mod_clamp(const fixed_base y) const;
	fixed_base pow(const fixed_base y) const;

	fixed_base fastinvsqrt(void) const;

	static FORCEINLINE fixed_base Random(void) { return FromRep(((T)rand()) & lowmask); }
	
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// IMPLEMENTATIONS /////////////

// AFIXED=12:20 (angles, rotations, trigonometry, fractions) 
typedef fixed_base<INT32,UINT32,20,INT64,0,20> afixed;
// BFIXED=21:11 (distances, positions, axes, world coordinates)
typedef fixed_base<INT32,UINT32,11,INT64,0,11> bfixed;
// CFIXED=8:24 (precise fractional quantities) 
typedef fixed_base<INT32,UINT32,24,INT64,0,24> cfixed;
// DFIXED=24:8 (time in milliseconds)
typedef fixed_base<INT32,UINT32,8,INT64,0,8> dfixed;
// GFIXED=16:16 (general purpose)
typedef fixed_base<INT32,UINT32,16,INT64,0,16> gfixed;
// LFIXED=32:32 (double-long general purpose)
typedef fixed_base<INT64,UINT64,32,INT64,16,0> lfixed;




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UTILITY TEMPLATES

template<class T> inline T fixed_fmax(const T a, const T b) { return T::FromRep(b.rep + ((a.rep - b.rep) & -(T::basetype)(a.rep > b.rep))); }
template<class T> inline T fixed_fmin(const T a, const T b) { return T::FromRep(b.rep + ((a.rep - b.rep) & -(T::basetype)(a.rep < b.rep))); }
template<class T> inline void fixed_fastvec3norm(T *v);
template<class T> inline T fixed_fastvec3invlen(T *v);
template<class B, class S> B fixed_vec2dot(const B *x, const S *y);
template<class B, class S> B fixed_vec3dot(const B *x, const S *y);
template<class B, class S> B fixed_vec4dot(const B *x, const S *y);
template<class B, class S> B fixed_vec2dot_r(const S *x, const B *y);
template<class B, class S> B fixed_vec3dot_r(const S *x, const B *y);
template<class B, class S> B fixed_vec4dot_r(const S *x, const B *y);
template<class B, class S> void fixed_vec2scale(const B *v, const S scale, B *o);
template<class B, class S> void fixed_vec3scale(const B *v, const S scale, B *o);
template<class B, class S> void fixed_vec4scale(const B *v, const S scale, B *o);
template<class B, class S> void fixed_vec2scale_r(const S *v, const B scale, B *o);
template<class B, class S> void fixed_vec3scale_r(const S *v, const B scale, B *o);
template<class B, class S> void fixed_vec4scale_r(const S *v, const B scale, B *o);
template<class B, class S> void fixed_vec2ma(const B *v, const S scale, const B *b, B *o);
template<class B, class S> void fixed_vec3ma(const B *v, const S scale, const B *b, B *o);
template<class B, class S> void fixed_vec4ma(const B *v, const S scale, const B *b, B *o);
template<class B, class S> void fixed_vec2ma_r(const B *v, const B scale, const S *b, B *o);
template<class B, class S> void fixed_vec3ma_r(const B *v, const B scale, const S *b, B *o);
template<class B, class S> void fixed_vec4ma_r(const B *v, const B scale, const S *b, B *o);
template<class T> T fixed_vec2len(const T *v);
template<class T> T fixed_vec3len(const T *v);
template<class T> T fixed_vec4len(const T *v);
template<class T> T fixed_vec2len_sq(const T *v);
template<class T> T fixed_vec3len_sq(const T *v);
template<class T> T fixed_vec4len_sq(const T *v);


// ARM Specializations
#ifndef FIXED_REFERENCE_IMPL

#ifdef ARM

template<> bfixed fixed_vec3dot<bfixed,bfixed>(const bfixed *x, const bfixed *y);
template<> bfixed fixed_vec3dot_r<bfixed,bfixed>(const bfixed *x, const bfixed *y);
template<> bfixed fixed_vec3dot<bfixed,afixed>(const bfixed *x, const afixed *y);
template<> bfixed fixed_vec3dot_r<bfixed,afixed>(const afixed *x, const bfixed *y);
template<> afixed fixed_vec3dot<afixed,afixed>(const afixed *x, const afixed *y);
template<> afixed fixed_vec3dot_r<afixed,afixed>(const afixed *x, const afixed *y);

#endif

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define AFIXED(n,d) (afixed::Construct((int)(n),(unsigned int)((0.d##f)*1048576.0f)))
#define BFIXED(n,d) (bfixed::Construct((int)(n),(unsigned int)((0.d##f)*2048.0f)))
#define CFIXED(n,d) (cfixed::Construct((int)(n),(unsigned int)((0.d##f)*16777216.0f)))
#define DFIXED(n,d) (dfixed::Construct((int)(n),(unsigned int)((0.d##f)*256.0f)))
#define GFIXED(n,d) (gfixed::Construct((int)(n),(unsigned int)((0.d##f)*65536.0f)))
#define LFIXED(n,d) (lfixed::Construct((int)(n),(unsigned int)((0.d##f)*4294967296.0f)))

#define MAKE_FIXED_RANDOM(q,t) inline q t##_RANDOM(void) { return q::Random(); }
#define MAKE_FIXED_ABS(q) inline q FIXED_ABS( const q &x ) { return x.abs(); }
#define MAKE_FIXED_SQRT(q) inline q FIXED_SQRT( const q &x ) { return x.sqrt(); }
#define MAKE_FIXED_NOT_ZERO(q) inline bool FIXED_NOT_ZERO(const q &x) { return x.IsNotZero(); }
#define MAKE_FIXED_NOT_ONE(q) inline bool FIXED_NOT_ONE(const q &x) { return x.IsNotOne(); }
#define MAKE_FIXED_IS_ZERO(q) inline bool FIXED_IS_ZERO(const q &x) { return x.IsZero(); }
#define MAKE_FIXED_IS_ONE(q) inline bool FIXED_IS_ONE(const q &x) { return x.IsOne(); }
#define MAKE_FIXED_SNAP(q) inline q FIXED_SNAP(const q &x) { return x.snap(); }
#define MAKE_FIXED_FASTINVSQRT(q) inline q FIXED_FASTINVSQRT(const q &x) { return x.fastinvsqrt(); }
#define MAKE_FIXED_FASTVEC3NORM(q) inline void FIXED_FASTVEC3NORM(q *v) { fixed_fastvec3norm(v); }
#define MAKE_FIXED_FASTVEC3INVLEN(q) inline q FIXED_FASTVEC3INVLEN(const q &x) { fixed_fastvec3invlen(v); }
#define MAKE_FIXED_MULPOW2(q) inline q FIXED_MULPOW2(const q &x, int pow2) { return q::FromRep(x.rep<<pow2); }
#define MAKE_FIXED_DIVPOW2(q) inline q FIXED_DIVPOW2(const q &x, int pow2) { return q::FromRep(x.rep>>pow2); }
#define MAKE_FIXED_SIN(q) inline q FIXED_SIN(const q &a) { return a.sin(); }
#define MAKE_FIXED_COS(q) inline q FIXED_COS(const q &a) { return a.cos(); }
#define MAKE_FIXED_TAN(q) inline q FIXED_TAN(const q &a) { return a.tan(); }
#define MAKE_FIXED_ASIN(q) inline q FIXED_ASIN(const q &a) { return a.asin(); }
#define MAKE_FIXED_ACOS(q) inline q FIXED_ACOS(const q &a) { return a.acos(); }
#define MAKE_FIXED_ATAN(q) inline q FIXED_ATAN(const q &a) { return a.atan(); }
#define MAKE_FIXED_ATAN2(q) inline q FIXED_ATAN2(const q &y, const q &x) { return q::atan2(y,x); }
#define MAKE_FIXED_FLOOR(q) inline q FIXED_FLOOR(const q &a) { return a.floor(); }
#define MAKE_FIXED_CEIL(q) inline q FIXED_CEIL(const q &a) { return a.ceil(); }
#define MAKE_FIXED_MOD(q) inline q FIXED_MOD(const q &a,const q &b) { return a.mod(b); }
#define MAKE_FIXED_MOD_CLAMP(q) inline q FIXED_MOD_CLAMP(const q &a, const q &b) { return a.mod_clamp(b); }
#define MAKE_FIXED_POW(q) inline q FIXED_POW(const q &a,const q &b) { return a.pow(b); }
#define MAKE_FIXED_MAX(q) inline q FIXED_MAX(const q &a,const q &b) { return fixed_fmax<q>(a,b); }
#define MAKE_FIXED_MIN(q) inline q FIXED_MIN(const q &a,const q &b) { return fixed_fmin<q>(a,b); }

#define MAKE_FIXED_EVERYTHING(q,t) \
	MAKE_FIXED_RANDOM(q,t)\
	MAKE_FIXED_ABS(q)\
	MAKE_FIXED_SQRT(q)\
	MAKE_FIXED_NOT_ZERO(q)\
	MAKE_FIXED_NOT_ONE(q)\
	MAKE_FIXED_IS_ZERO(q)\
	MAKE_FIXED_IS_ONE(q)\
	MAKE_FIXED_SNAP(q)\
	MAKE_FIXED_FASTINVSQRT(q)\
	MAKE_FIXED_FASTVEC3NORM(q)\
	MAKE_FIXED_MULPOW2(q)\
	MAKE_FIXED_DIVPOW2(q)\
	MAKE_FIXED_SIN(q)\
	MAKE_FIXED_COS(q)\
	MAKE_FIXED_TAN(q)\
	MAKE_FIXED_ASIN(q)\
	MAKE_FIXED_ACOS(q)\
	MAKE_FIXED_ATAN(q)\
	MAKE_FIXED_ATAN2(q)\
	MAKE_FIXED_FLOOR(q)\
	MAKE_FIXED_CEIL(q)\
	MAKE_FIXED_MOD(q)\
	MAKE_FIXED_MOD_CLAMP(q)\
	MAKE_FIXED_POW(q)\
	MAKE_FIXED_MAX(q)\
	MAKE_FIXED_MIN(q)

/////////////////
MAKE_FIXED_EVERYTHING(afixed,AFIXED)
MAKE_FIXED_EVERYTHING(bfixed,BFIXED)
MAKE_FIXED_EVERYTHING(cfixed,CFIXED)
MAKE_FIXED_EVERYTHING(dfixed,DFIXED)
MAKE_FIXED_EVERYTHING(gfixed,GFIXED)
MAKE_FIXED_EVERYTHING(lfixed,LFIXED)
/////////////////

#define FIXED_VEC2DOT(a,b) fixed_vec2dot((a),(b))
#define FIXED_VEC3DOT(a,b) fixed_vec3dot((a),(b))
#define FIXED_VEC4DOT(a,b) fixed_vec4dot((a),(b))
#define FIXED_VEC2DOT_R(a,b) fixed_vec2dot_r((a),(b))
#define FIXED_VEC3DOT_R(a,b) fixed_vec3dot_r((a),(b))
#define FIXED_VEC4DOT_R(a,b) fixed_vec4dot_r((a),(b))
#define FIXED_VEC2SCALE(a,s,o) fixed_vec2scale((a),(s),(o))
#define FIXED_VEC3SCALE(a,s,o) fixed_vec3scale((a),(s),(o))
#define FIXED_VEC4SCALE(a,s,o) fixed_vec4scale((a),(s),(o))
#define FIXED_VEC2SCALE_R(a,s,o) fixed_vec2scale_r((a),(s),(o))
#define FIXED_VEC3SCALE_R(a,s,o) fixed_vec3scale_r((a),(s),(o))
#define FIXED_VEC4SCALE_R(a,s,o) fixed_vec4scale_r((a),(s),(o))
#define FIXED_VEC2MA(v,s,b,o) fixed_vec2ma((v),(s),(b),(o))
#define FIXED_VEC3MA(v,s,b,o) fixed_vec3ma((v),(s),(b),(o))
#define FIXED_VEC4MA(v,s,b,o) fixed_vec4ma((v),(s),(b),(o))
#define FIXED_VEC2MA_R(v,s,b,o) fixed_vec2ma_r((v),(s),(b),(o))
#define FIXED_VEC3MA_R(v,s,b,o) fixed_vec3ma_r((v),(s),(b),(o))
#define FIXED_VEC4MA_R(v,s,b,o) fixed_vec4ma_r((v),(s),(b),(o))
#define FIXED_VEC2LEN(v) fixed_vec2len(v)
#define FIXED_VEC3LEN(v) fixed_vec3len(v)
#define FIXED_VEC4LEN(v) fixed_vec4len(v)
#define FIXED_VEC2LEN_SQ(v) fixed_vec2len_sq(v)
#define FIXED_VEC3LEN_SQ(v) fixed_vec3len_sq(v)
#define FIXED_VEC4LEN_SQ(v) fixed_vec4len_sq(v)

inline int FIXED_AS_INT(afixed x) { return *(int *)(&(x)); }
inline int FIXED_AS_INT(bfixed x) { return *(int *)(&(x)); }
inline int FIXED_AS_INT(cfixed x) { return *(int *)(&(x)); }
inline int FIXED_AS_INT(dfixed x) { return *(int *)(&(x)); }
inline int FIXED_AS_INT(gfixed x) { return *(int *)(&(x)); }

inline afixed INT_AS_AFIXED(int x) { return *(afixed *)(&(x)); }
inline bfixed INT_AS_BFIXED(int x) { return *(bfixed *)(&(x)); }
inline cfixed INT_AS_CFIXED(int x) { return *(cfixed *)(&(x)); }
inline dfixed INT_AS_DFIXED(int x) { return *(dfixed *)(&(x)); }
inline gfixed INT_AS_GFIXED(int x) { return *(gfixed *)(&(x)); }

#define FIXED_GETREP(x) ((x).rep)

#define AFIXED_PI AFIXED(3,1415927)
#define BFIXED_PI BFIXED(3,1415)
#define CFIXED_PI CFIXED(3,14159265)
#define DFIXED_PI DFIXED(3,142)
#define GFIXED_PI GFIXED(3,14159)
#define LFIXED_PI LFIXED(3,1415926535)

#define AFIXED_1 AFIXED(1,0)
#define AFIXED_0 AFIXED(0,0)
#define BFIXED_1 BFIXED(1,0)
#define BFIXED_0 BFIXED(0,0)
#define CFIXED_1 CFIXED(1,0)
#define CFIXED_0 CFIXED(0,0)
#define DFIXED_1 DFIXED(1,0)
#define DFIXED_0 DFIXED(0,0)
#define GFIXED_1 GFIXED(1,0)
#define GFIXED_0 GFIXED(0,0)
#define LFIXED_1 LFIXED(1,0)
#define LFIXED_0 LFIXED(0,0)
                                 
#define FIXED_TO_INT(x) ((x).ToInt())
#define FIXED_TO_FLOAT(x) ((x).ToFloat())
#define FIXED_TO_DOUBLE(x) ((x).ToDouble())


// Precision-sensitive operators
// D>B>G>A>C
template<class B, class S>
inline B SensitiveMultiply(B b,S s)
{	
	const int shift1bs=S::prec-B::shift0-S::shift0;
#ifdef VERIFY_FIXEDREP
	ASSERT(shift1bs>=0);
#endif
	const B::exttype round1bs=(((B::exttype)1)<<shift1bs)>>1;
	B::exttype out=( ((((((B::exttype)b.rep)+B::round0)>>B::shift0)*((((S::exttype)s.rep)+S::round0))>>S::shift0) + round1bs) >> shift1bs ); 
	return B::FromRep((B::basetype)out);
}

inline dfixed operator *(const dfixed &a, const bfixed &b) { return SensitiveMultiply(a,b); }
inline dfixed operator *(const bfixed &a, const dfixed &b) { return SensitiveMultiply(b,a); }
inline dfixed operator *(const dfixed &a, const gfixed &b) { return SensitiveMultiply(a,b); }
inline dfixed operator *(const gfixed &a, const dfixed &b) { return SensitiveMultiply(b,a); }
inline dfixed operator *(const dfixed &a, const afixed &b) { return SensitiveMultiply(a,b); }
inline dfixed operator *(const afixed &a, const dfixed &b) { return SensitiveMultiply(b,a); }
inline dfixed operator *(const dfixed &a, const cfixed &b) { return SensitiveMultiply(a,b); }
inline dfixed operator *(const cfixed &a, const dfixed &b) { return SensitiveMultiply(b,a); }

inline bfixed operator *(const bfixed &a, const gfixed &b) { return SensitiveMultiply(a,b); }
inline bfixed operator *(const gfixed &a, const bfixed &b) { return SensitiveMultiply(b,a); }
inline bfixed operator *(const bfixed &a, const afixed &b) { return SensitiveMultiply(a,b); }
inline bfixed operator *(const afixed &a, const bfixed &b) { return SensitiveMultiply(b,a); }
inline bfixed operator *(const bfixed &a, const cfixed &b) { return SensitiveMultiply(a,b); }
inline bfixed operator *(const cfixed &a, const bfixed &b) { return SensitiveMultiply(b,a); }

inline gfixed operator *(const gfixed &a, const afixed &b) { return SensitiveMultiply(a,b); }
inline gfixed operator *(const afixed &a, const gfixed &b) { return SensitiveMultiply(b,a); }
inline gfixed operator *(const gfixed &a, const cfixed &b) { return SensitiveMultiply(a,b); }
inline gfixed operator *(const cfixed &a, const gfixed &b) { return SensitiveMultiply(b,a); }

inline afixed operator *(const afixed &a, const cfixed &b) { return SensitiveMultiply(a,b); }
inline afixed operator *(const cfixed &a, const afixed &b) { return SensitiveMultiply(b,a); }


#define MAKE_AFIXED(x) (afixed::Construct((x)))
#define MAKE_BFIXED(x) (bfixed::Construct((x)))
#define MAKE_CFIXED(x) (cfixed::Construct((x)))
#define MAKE_DFIXED(x) (dfixed::Construct((x)))
#define MAKE_GFIXED(x) (gfixed::Construct((x)))
#define MAKE_LFIXED(x) (lfixed::Construct((x)))


inline INT32 FIXED_INT32SCALE(INT32 i, gfixed s)
{
	return (INT32)((((gfixed::exttype)i)*((gfixed::exttype)s.rep) + gfixed::roundp)>>gfixed::prec);
}

inline INT32 FIXED_INT32SCALE(INT32 i, afixed s)
{
	return (INT32)((((afixed::exttype)i)*((afixed::exttype)s.rep) + afixed::roundp)>>afixed::prec);
}

inline INT32 FIXED_INT32SCALE(INT32 i, bfixed s)
{
	return (INT32)((((bfixed::exttype)i)*((bfixed::exttype)s.rep) + bfixed::roundp)>>bfixed::prec);
}

inline gfixed FIXED_INT32RATIO_G(INT32 n, INT32 d)
{
	return gfixed::FromRep( (gfixed::basetype) ((((gfixed::exttype)n)<<gfixed::prec)/((gfixed::exttype)d)) );
}

inline afixed FIXED_INT32RATIO_A(INT32 n, INT32 d)
{
	return afixed::FromRep( (afixed::basetype) ((((afixed::exttype)n)<<afixed::prec)/((afixed::exttype)d)) );
}

inline bfixed FIXED_INT32RATIO_B(INT32 n, INT32 d)
{
	return bfixed::FromRep( (bfixed::basetype) ((((bfixed::exttype)n)<<bfixed::prec)/((bfixed::exttype)d)) );
}

template<class T>
inline gfixed FIXED_RATIO_G(T n, T d)
{
	return gfixed::FromRep( (gfixed::basetype) ((((gfixed::exttype)n.rep)<<gfixed::prec)/((gfixed::exttype)d.rep)) );
}

template<class T>
inline bfixed FIXED_RATIO_B(T n, T d)
{
	return bfixed::FromRep( (bfixed::basetype) ((((bfixed::exttype)n.rep)<<bfixed::prec)/((bfixed::exttype)d.rep)) );
}

template<class T>
inline afixed FIXED_RATIO_A(T n, T d)
{
	return afixed::FromRep( (afixed::basetype) ((((afixed::exttype)n.rep)<<afixed::prec)/((afixed::exttype)d.rep)) );
}


#define REINTERPRET_GFIXED(x) (*(gfixed *)(&(x)))
#define REINTERPRET_BFIXED(x) (*(bfixed *)(&(x)))

#define AFIXED_MAX (afixed::FromRep(INT_MAX));
#define AFIXED_MIN (afixed::FromRep(INT_MIN));
#define BFIXED_MAX (bfixed::FromRep(INT_MAX));
#define BFIXED_MIN (bfixed::FromRep(INT_MIN));
#define CFIXED_MAX (cfixed::FromRep(INT_MAX));
#define CFIXED_MIN (cfixed::FromRep(INT_MIN));
#define DFIXED_MAX (dfixed::FromRep(INT_MAX));
#define DFIXED_MIN (dfixed::FromRep(INT_MIN));
#define GFIXED_MAX (gfixed::FromRep(INT_MAX));
#define GFIXED_MIN (gfixed::FromRep(INT_MIN));


/////////////////////////////


#include"fixed.inl"

#pragma warning(pop)


#endif