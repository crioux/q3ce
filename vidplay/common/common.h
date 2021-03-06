/*****************************************************************************
 *
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: common.h 202 2005-01-25 01:27:33Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

//#define NO_DYNCODE
//#define DUMPCODE
//#define SHOWDIFF

#if _MSC_VER > 1000
#pragma once
#pragma warning(push, 4)
#pragma warning(disable : 4100 4710 4514 4201 4714 4115 4206 4055 4214 4998)
#endif

#include "portab.h"

#define DLL 

#if defined(_WIN32) && (_WIN32_WCE < 400)
#ifndef NDEBUG
extern DLL void _Assert(const char* Exp, const char* File, int Line);
#define assert(x) if (!(x)) _Assert(#x,__FILE__,__LINE__);
#else
#define assert(x)
#endif
#else
#include <assert.h>
#endif

#include "err.h"
#include "mem.h"
#include "buffer.h"
#include "context.h"
#include "multithread.h"
#include "str.h"
#include "file.h"
#include "node.h"
#include "streams.h"
#include "audio.h"
#include "video.h"
#include "subtitle.h"
#include "blit.h"
#include "flow.h"
#include "codec.h"
#include "timer.h"
#include "platform.h"
#include "advanced.h"
#include "tools.h"
#include "format.h"
#include "format_base.h"
#include "bitstream.h"
#include "vlc.h"
#include "player.h"
#include "color.h"
#include "equalizer.h"
#include "playlist.h"
#include "nulloutput.h"
#include "probe.h"
#include "idct.h"
#include "overlay.h"
#include "rawaudio.h"
#include "waveout.h"
#include "association.h"
#include "color.h"
#include "equalizer.h"
#include "id3tag.h"

#ifdef __cplusplus
}
#endif

#undef DLL

#endif
