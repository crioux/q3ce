

#include"fixed.h"
#ifndef _WIN32
#include"math.h"
#include"unixdefs.h"
#endif

#ifndef FIXED_IS_FLOAT

#ifndef FIXED_REFERENCE_IMPL

#ifdef ARM

extern "C" void VEC3DOT32_11(INT32 *vec_a, INT32 *vec_b, INT32 * rep_out);
extern "C" void VEC3DOT32_20(INT32 *vec_a, INT32 *vec_b, INT32 * rep_out);

template<> 
bfixed fixed_vec3dot<bfixed,bfixed>(const bfixed *x, const bfixed *y)
{
	bfixed out;
	VEC3DOT32_11((INT32 *)&(x[0].rep),(INT32 *)&(y[0].rep),&(out.rep));
	return out;
}

template<> 
bfixed fixed_vec3dot_r<bfixed,bfixed>(const bfixed *x, const bfixed *y)
{
	bfixed out;
	VEC3DOT32_11((INT32 *)&(x[0].rep),(INT32 *)&(y[0].rep),&(out.rep));
	return out;
}

template<> 
bfixed fixed_vec3dot<bfixed,afixed>(const bfixed *x, const afixed *y)
{
	bfixed out;
	VEC3DOT32_20((INT32 *)&(x[0].rep),(INT32 *)&(y[0].rep),&(out.rep));
	return out;
}

template<> 
bfixed fixed_vec3dot_r<bfixed,afixed>(const afixed *x, const bfixed *y)
{
	bfixed out;
	VEC3DOT32_20((INT32 *)&(y[0].rep),(INT32 *)&(x[0].rep),&(out.rep));
	return out;s

template<> 
afixed fixed_vec3dot<afixed,afixed>(const afixed *x, const afixed *y)
{
	afixed out;
	VEC3DOT32_20((INT32 *)&(x[0].rep),(INT32 *)&(y[0].rep),&(out.rep));
	return out;
}

template<> 
afixed fixed_vec3dot_r<afixed,afixed>(const afixed *x, const afixed *y)
{
	afixed out;
	VEC3DOT32_20((INT32 *)&(x[0].rep),(INT32 *)&(y[0].rep),&(out.rep));
	return out;
}


#endif

#endif

#endif

