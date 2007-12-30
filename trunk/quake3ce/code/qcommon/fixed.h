#ifndef __INC_FIXED_H
#define __INC_FIXED_H


#include<windows.h>
#include<math.h>

//#define FIXED_IS_FLOAT 1


#ifdef FIXED_IS_FLOAT
#include"fixed_as_float.h"
#else
#include"fixed_as_fixed.h"
#endif


union fixed32
{
	afixed a;
	bfixed b;
	cfixed c;
	dfixed d;
	gfixed g;
	INT32 rep;
};

                                 
#endif