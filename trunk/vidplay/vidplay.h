#ifndef __INC_VIDPLAY_H
#define __INC_VIDPLAY_H

#ifdef __cplusplus
extern "C" 
{
#endif
	

int VidPlay_Init(void);
int VidPlay_Kill(void);

int VidPlay_Open(const char *svName, int x, int y, int w, int h, int loop);
int VidPlay_Play(int handle);
int VidPlay_Stop(int handle);
int VidPlay_Close(int handle);
int VidPlay_Process(int handle);
int VidPlay_SetExtents(int handle, int x, int y, int w, int h);
int VidPlay_SetLoop(int handle, int loop);


#ifdef __cplusplus
};
#endif

#endif

