#ifndef __INC_COMMON_PCH_H
#define __INC_COMMON_PCH_H

#include "../game/q_shared.h"
#include "qcommon.h"
#include "cm_local.h"
#include "cm_patch.h"
#include "unzip.h"

#include <time.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock.h>
#include"find.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include"fixed.h"

#ifdef _WIN32
#include<windows.h>
#endif

#include "GLES/gl.h"
#include "gleswrappers.h"



#endif

