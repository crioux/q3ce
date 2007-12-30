#ifndef __INC_STDAFX_H
#define __INC_STDAFX_H


#define WIN32_LEAN_AND_MEAN
#ifndef STRICT
#define STRICT
#endif
#include <windows.h>

#include "common/common.h"
#include "ati3200/ati3200.h"
#include "zodiac/ati4200.h"
#include "intel2700g/intel2700g.h"
#include "mp3/mp3.h"
#include "splitter/avi.h"


#if defined(MPEG4_EXPORTS)
#define MPEG4
#if !defined(MIPS)
	#define MSMPEG4
#endif
#endif

#if defined(MSMPEG4_EXPORTS)
#if defined(MIPS)
	#define MSMPEG4
#endif
#endif

#include "mp4/mpeg4.h"

// maximun picture size in macroblocks (16x16)
// example MB_X2=6 -> 2^6=64 macroblocks -> 1024 pixels

#define MB_X2	6
#define MB_Y2	6
#define MB_X	(1<<MB_X2)
#define MB_Y	(1<<MB_Y2)

#define MAX_VLC 20000

#define DC_LUM_MASK	((MB_X*2)*4-1)
#define DC_CHR_MASK (MB_X*2-1)

// mpeg4 include files

#include "mp4/mp4_decode.h"
#include "mp4/mp4_header.h"
#include "mp4/mp4_mv.h"
#include "mp4/mp4_stream.h"
#include "mp4/mp4_mblock.h"
#include "mp4/mp4_vld.h"
#include "mp4/divx3_vlc.h"

#include "common/softidct/softidct.h"



#endif
