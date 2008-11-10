#ifndef __INC_UNIXDEFS_H
#define __INC_UNIXDEFS_H

#include<unistd.h>
#include<stdint.h>
#include<sys/types.h>
//#include<X11/Xmd.h>

typedef uint64_t UINT64;
typedef int64_t INT64;
typedef uint32_t UINT32;
#ifndef XMD_H
typedef int32_t INT32;
#endif
typedef uint16_t UINT16;
#ifndef XMD_H
typedef int16_t INT16;
#endif
typedef uint32_t DWORD;
typedef uint16_t WORD;

typedef char TCHAR;

#define DebugBreak() assert(0)
#define ASSERT assert

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#define MAX_PATH 256


#endif

