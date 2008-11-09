#
# Q3PVR Makefile
# To use: 'make debug'or 'make release'
#

ARCH = x86
#ARCH := ARM
DEBUG_DEMO = BuildDebug_Demo_$(ARCH)
RELEASE_DEMO = BuildRelease_Demo_$(ARCH)
DEBUG_FULL = BuildDebug_Full_$(ARCH)
RELEASE_FULL = BuildRelease_Full_$(ARCH)
Q3PVRNAME = q3pvr_$(ARCH)
UINAME = ui$(ARCH).so
CGAMENAME = cgame$(ARCH).so
GAMENAME = qagame$(ARCH).so
VIDPLAYNAME = vidplay$(ARCH).a
LIBMADNAME = libmad$(ARCH).a
ROOT = $(shell pwd)
SDKPATH = $(ROOT)/../../SDKPackage
empty=

CC = gcc -c
CPP = g++ -c
LD = g++
LDSO = g++ -shared
AR = ar q
PIC = -fPIC -fvisibility=hidden

ifeq ($(MODE),$(DEBUG_DEMO))
MODEFLAGS = -gdwarf-2 -g3 -O0 -D_DEBUG=1
endif
ifeq ($(MODE),$(DEBUG_FULL))
MODEFLAGS = -gdwarf-2 -g3 -O0 -D_DEBUG=1
endif
ifeq ($(MODE),$(RELEASE_DEMO))
MODEFLAGS = -O2 -fomit-frame-pointer
endif
ifeq ($(MODE),$(RELEASE_FULL))
MODEFLAGS = -O2 -fomit-frame-pointer
endif

PLATFORM_DEFINES = -D_X86_=1 -DFPM_DEFAULT=1 -D_M_IX86=1 
#PLATFORM_DEFINES = -DFPM_DEFAULT=1 -DARM=1 -D_ARM=1 -DARMV4=1

SDK_INCLUDES = -I$(ROOT)/src/pvrsdk -I$(ROOT)/src/pvrsdk/API/KEGL -I$(ROOT)/src/pvrsdk/OS/LinuxX11 -I$(SDKPATH)/Builds/OGLES/Include -I$(SDKPATH)/Builds/OGLES/Include -I$(SDKPATH)/Builds/OGLES/LinuxPC/Include -I$(SDKPATH)/Tools -I$(SDKPATH)/Shell -I$(SDKPATH)/Shell/OS/LinuxX11 -I$(SDKPATH)/Shell/API/KEGL
SDK_LIBDIRS = -L$(SDKPATH)/Builds/OGLES/LinuxPC/Lib
SDK_LIBRARIES = -lGLES_CM
SDK_CPP_SOURCES = $(ROOT)/src/pvrsdk/PVRShell.cpp $(ROOT)/src/pvrsdk/API/KEGL/PVRShellAPI.cpp $(ROOT)/src/pvrsdk/OS/LinuxX11/PVRShellOS.cpp
#SDK_CPP_SOURCES = $(SDKPATH)/Shell/PVRShell.cpp $(SDKPATH)/Shell/API/KEGL/PVRShellAPI.cpp $(SDKPATH)/Shell/OS/LinuxX11/PVRShellOS.cpp

INCLUDES = $(SDK_INCLUDES) -I$(ROOT)/src/quake3ce/code -I$(ROOT)/src/quake3ce/code/cgame -I$(ROOT)/src/quake3ce/code/q3_ui -I$(ROOT)/src/quake3ce/code/game -I$(ROOT)/src/quake3ce/code/qcommon -I$(ROOT)/src/quake3ce/code/renderer -I$(ROOT)/src/quake3ce/code/client -I$(ROOT)/src/quake3ce/code/server -I$(ROOT)/src/quake3ce/code/pvr -I$(ROOT)/src/vidplay -I$(ROOT)/src/vidplay/common

LIBRARIES = $(SDK_LIBDIRS) -lc $(SDK_LIBRARIES)

DEFINES = -D_LINUX=1 -DUSE_MP3_SOUNDS=1 -DBOTLIB=1 -DNO_PLUGINS=1 $(PLATFORM_DEFINES)

CGAME_C_SOURCES = $(filter %.c, $(shell cat filelists/cgame_files)) 
CGAME_CPP_SOURCES = $(filter %.cpp, $(shell cat filelists/cgame_files))
CGAME_DEFINES = -DCGAME=1 -DDLL_ONLY=1 $(DEFINES)
CGAME_C_OBJECTS = $(patsubst %.c,$(MODE)/obj/cgame/%.o,$(CGAME_C_SOURCES)) 
CGAME_CPP_OBJECTS = $(patsubst %.cpp,$(MODE)/obj/cgame/%.o,$(CGAME_CPP_SOURCES)) 

GAME_C_SOURCES = $(filter %.c, $(shell cat filelists/game_files)) 
GAME_CPP_SOURCES = $(filter %.cpp, $(shell cat filelists/game_files))
GAME_DEFINES = -DGAME=1 -DDLL_ONLY=1 $(DEFINES)
GAME_C_OBJECTS = $(patsubst %.c,$(MODE)/obj/game/%.o,$(GAME_C_SOURCES)) 
GAME_CPP_OBJECTS = $(patsubst %.cpp,$(MODE)/obj/game/%.o,$(GAME_CPP_SOURCES)) 

UI_C_SOURCES = $(filter %.c, $(shell cat filelists/ui_files)) 
UI_CPP_SOURCES = $(filter %.cpp, $(shell cat filelists/ui_files))
UI_DEFINES = $(DEFINES) 
UI_C_OBJECTS = $(patsubst %.c,$(MODE)/obj/ui/%.o,$(UI_C_SOURCES)) 
UI_CPP_OBJECTS = $(patsubst %.cpp,$(MODE)/obj/ui/%.o,$(UI_CPP_SOURCES))
 
VIDPLAY_C_SOURCES = $(filter %.c, $(shell cat filelists/vidplay_files)) 
VIDPLAY_CPP_SOURCES = $(filter %.cpp, $(shell cat filelists/vidplay_files))
VIDPLAY_DEFINES = $(DEFINES) -DMPEG4=1 
VIDPLAY_C_OBJECTS = $(patsubst %.c,$(MODE)/obj/vidplay/%.o,$(VIDPLAY_C_SOURCES)) 
VIDPLAY_CPP_OBJECTS = $(patsubst %.cpp,$(MODE)/obj/vidplay/%.o,$(VIDPLAY_CPP_SOURCES)) 

LIBMAD_C_SOURCES = $(filter %.c, $(shell cat filelists/libmad_files)) 
LIBMAD_CPP_SOURCES = $(filter %.cpp, $(shell cat filelists/libmad_files))
LIBMAD_DEFINES = $(DEFINES) -DMPEG4=1
LIBMAD_C_OBJECTS = $(patsubst %.c,$(MODE)/obj/libmad/%.o,$(LIBMAD_C_SOURCES)) 
LIBMAD_CPP_OBJECTS = $(patsubst %.cpp,$(MODE)/obj/libmad/%.o,$(LIBMAD_CPP_SOURCES)) 

Q3PVR_C_SOURCES = $(filter %.c, $(shell cat filelists/q3pvr_files)) 
Q3PVR_CPP_SOURCES = $(filter %.cpp, $(shell cat filelists/q3pvr_files)) $(SDK_CPP_SOURCES)
Q3PVR_DEFINES = $(DEFINES)
Q3PVR_C_OBJECTS = $(patsubst %.c,$(MODE)/obj/q3pvr/%.o,$(Q3PVR_C_SOURCES)) 
Q3PVR_CPP_OBJECTS = $(patsubst %.cpp,$(MODE)/obj/q3pvr/%.o,$(Q3PVR_CPP_SOURCES)) 


.PHONY: debug-demo
debug-demo: 
	$(MAKE) $(DEBUG_DEMO)/$(Q3PVRNAME) MODE=$(DEBUG_DEMO) 
	$(MAKE) $(DEBUG_DEMO)/$(CGAMENAME) MODE=$(DEBUG_DEMO) 
	$(MAKE) $(DEBUG_DEMO)/$(GAMENAME) MODE=$(DEBUG_DEMO) 
	$(MAKE) $(DEBUG_DEMO)/$(UINAME) MODE=$(DEBUG_DEMO)

.PHONY: debug-full
debug-full: 
	$(MAKE) $(DEBUG_FULL)/$(Q3PVRNAME) MODE=$(DEBUG_FULL) 
	$(MAKE) $(DEBUG_FULL)/$(CGAMENAME) MODE=$(DEBUG_FULL) 
	$(MAKE) $(DEBUG_FULL)/$(GAMENAME) MODE=$(DEBUG_FULL) 
	$(MAKE) $(DEBUG_FULL)/$(UINAME) MODE=$(DEBUG_FULL)

debug-packdemo: debug-demo
	mkdir -p $(DEBUG_DEMO)/pack
	mkdir -p $(DEBUG_DEMO)/pack/baseq3
	cp ../demoq3/* $(DEBUG_DEMO)/pack/baseq3
	cp ../configs/linuxpvr$(ARCH).cfg $(DEBUG_DEMO)/pack/baseq3/default.cfg
	cp $(DEBUG_DEMO)/$(Q3PVRNAME) $(DEBUG_DEMO)/pack
	cp $(DEBUG_DEMO)/$(CGAMENAME) $(DEBUG_DEMO)/pack/baseq3
	cp $(DEBUG_DEMO)/$(GAMENAME) $(DEBUG_DEMO)/pack/baseq3
	cp $(DEBUG_DEMO)/$(UINAME) $(DEBUG_DEMO)/pack/baseq3

debug-packfull: debug-full
	mkdir -p $(DEBUG_FULL)/pack
	mkdir -p $(DEBUG_FULL)/pack/baseq3
	cp ../fullq3/* $(DEBUG_FULL)/pack/baseq3
	cp ../configs/linuxpvr$(ARCH).cfg $(DEBUG_FULL)/pack/baseq3/default.cfg
	cp $(DEBUG_FULL)/$(Q3PVRNAME) $(DEBUG_FULL)/pack
	cp $(DEBUG_FULL)/$(CGAMENAME) $(DEBUG_FULL)/pack/baseq3
	cp $(DEBUG_FULL)/$(GAMENAME) $(DEBUG_FULL)/pack/baseq3
	cp $(DEBUG_FULL)/$(UINAME) $(DEBUG_FULL)/pack/baseq3
	
	
.PHONY: release-demo
release-demo: 
	$(MAKE) $(RELEASE_DEMO)/$(Q3PVRNAME) MODE=$(RELEASE_DEMO) 
	$(MAKE) $(RELEASE_DEMO)/$(CGAMENAME) MODE=$(RELEASE_DEMO) 
	$(MAKE) $(RELEASE_DEMO)/$(GAMENAME) MODE=$(RELEASE_DEMO) 
	$(MAKE) $(RELEASE_DEMO)/$(UINAME) MODE=$(RELEASE_DEMO) 

.PHONY: release-full
release-full: 
	$(MAKE) $(RELEASE_FULL)/$(Q3PVRNAME) MODE=$(RELEASE_FULL) 
	$(MAKE) $(RELEASE_FULL)/$(CGAMENAME) MODE=$(RELEASE_FULL) 
	$(MAKE) $(RELEASE_FULL)/$(GAMENAME) MODE=$(RELEASE_FULL) 
	$(MAKE) $(RELEASE_FULL)/$(UINAME) MODE=$(RELEASE_FULL) 

release-packdemo: release-demo
	mkdir -p $(RELEASE_DEMO)/pack
	mkdir -p $(RELEASE_DEMO)/pack/baseq3
	cp ../demoq3/* $(RELEASE_DEMO)/pack/baseq3
	cp ../configs/linuxpvr$(ARCH).cfg $(RELEASE_DEMO)/pack/baseq3/default.cfg
	cp $(RELEASE_DEMO)/$(Q3PVRNAME) $(RELEASE_DEMO)/pack
	cp $(RELEASE_DEMO)/$(CGAMENAME) $(RELEASE_DEMO)/pack/baseq3
	cp $(RELEASE_DEMO)/$(GAMENAME) $(RELEASE_DEMO)/pack/baseq3
	cp $(RELEASE_DEMO)/$(UINAME) $(RELEASE_DEMO)/pack/baseq3

release-packfull: release-full
	mkdir -p $(RELEASE_FULL)/pack
	mkdir -p $(RELEASE_FULL)/pack/baseq3
	cp ../fullq3/* $(RELEASE_FULL)/pack/baseq3
	cp ../configs/linuxpvr$(ARCH).cfg $(RELEASE_FULL)/pack/baseq3/default.cfg
	cp $(RELEASE_FULL)/$(Q3PVRNAME) $(RELEASE_FULL)/pack
	cp $(RELEASE_FULL)/$(CGAMENAME) $(RELEASE_FULL)/pack/baseq3
	cp $(RELEASE_FULL)/$(GAMENAME) $(RELEASE_FULL)/pack/baseq3
	cp $(RELEASE_FULL)/$(UINAME) $(RELEASE_FULL)/pack/baseq3


clean:
	rm -rf $(DEBUG_DEMO)
	rm -rf $(RELEASE_DEMO)
	rm -rf $(DEBUG_FULL)
	rm -rf $(RELEASE_FULL)

$(UI_C_OBJECTS): $(MODE)/obj/ui/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(PIC) $(UI_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@
$(UI_CPP_OBJECTS): $(MODE)/obj/ui/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(PIC) $(UI_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@	

$(CGAME_C_OBJECTS): $(MODE)/obj/cgame/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(PIC) $(CGAME_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@
$(CGAME_CPP_OBJECTS): $(MODE)/obj/cgame/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(PIC) $(CGAME_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@

$(GAME_C_OBJECTS): $(MODE)/obj/game/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(PIC) $(GAME_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@
$(GAME_CPP_OBJECTS): $(MODE)/obj/game/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(PIC) $(GAME_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@

$(VIDPLAY_C_OBJECTS): $(MODE)/obj/vidplay/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(VIDPLAY_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@
$(VIDPLAY_CPP_OBJECTS): $(MODE)/obj/vidplay/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(VIDPLAY_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@

$(LIBMAD_C_OBJECTS): $(MODE)/obj/libmad/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LIBMAD_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@
$(LIBMAD_CPP_OBJECTS): $(MODE)/obj/libmad/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(LIBMAD_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@

$(Q3PVR_C_OBJECTS): $(MODE)/obj/q3pvr/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(Q3PVR_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@
$(Q3PVR_CPP_OBJECTS): $(MODE)/obj/q3pvr/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(Q3PVR_DEFINES) $(INCLUDES) $(MODEFLAGS) $< -o $@

$(MODE)/$(VIDPLAYNAME): $(VIDPLAY_C_OBJECTS) $(VIDPLAY_CPP_OBJECTS)
	mkdir -p $(MODE)
	rm -f $@
	$(AR) $@ $+

$(MODE)/$(LIBMADNAME): $(LIBMAD_C_OBJECTS) $(LIBMAD_CPP_OBJECTS)
	mkdir -p $(MODE)
	rm -f $@
	$(AR) $@ $+

$(MODE)/$(UINAME): $(UI_C_OBJECTS) $(UI_CPP_OBJECTS) $(MODE)/$(LIBMADNAME)
	mkdir -p $(MODE)
	$(LDSO) $(LDFLAGS) -o $@ $+ $(LIBRARIES)

$(MODE)/$(GAMENAME): $(GAME_C_OBJECTS) $(GAME_CPP_OBJECTS) $(MODE)/$(LIBMADNAME) 
	mkdir -p $(MODE)
	$(LDSO) $(LDFLAGS) -o $@ $+ $(LIBRARIES)

$(MODE)/$(CGAMENAME): $(CGAME_C_OBJECTS) $(CGAME_CPP_OBJECTS) $(MODE)/$(LIBMADNAME)
	mkdir -p $(MODE)
	$(LDSO) $(LDFLAGS) -o $@ $+ $(LIBRARIES)

$(MODE)/$(Q3PVRNAME): $(Q3PVR_C_OBJECTS) $(Q3PVR_CPP_OBJECTS) $(MODE)/$(LIBMADNAME) #$(MODE)/$(VIDPLAYNAME)
	mkdir -p $(MODE)
	$(LD) $(LDFLAGS) -o $@ $+ $(LIBRARIES)

	

