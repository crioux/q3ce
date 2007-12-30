/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*****************************************************************************
 * name:		snd_mem.c
 *
 * desc:		sound caching
 *
 * $Archive: /MissionPack/code/client/snd_mem.c $
 *
 *****************************************************************************/

#include"client_pch.h"
#include"xmalloc.h"

extern CXMalloc g_mem_snd;

#define USE_MP3_SOUNDS

#ifdef USE_MP3_SOUNDS
#define FPM_DEFAULT 1
#include"../vidplay/mp3/libmad/mad.h"
#endif




#define FCCBE(a,b,c,d) \
	(((unsigned char)a << 24) | ((unsigned char)b << 16) | \
	((unsigned char)c << 8) | ((unsigned char)d << 0))

#define FCCLE(a,b,c,d) \
	(((unsigned char)a << 0) | ((unsigned char)b << 8) | \
	((unsigned char)c << 16) | ((unsigned char)d << 24))

#ifdef BIG_ENDIAN
#define FCC(a,b,c,d) FCCBE(a,b,c,d)
#else
#define FCC(a,b,c,d) FCCLE(a,b,c,d)
#endif



/*
===============================================================================

memory management

===============================================================================
*/

static HANDLE s_buffer_maphandle=NULL;
static	sndBuffer	*buffer = NULL;
static	sndBuffer	*freelist = NULL;
static	int inUse = 0;
static	int totalInUse = 0;

short *sfxScratchBuffer = NULL;
sfx_t *sfxScratchPointer = NULL;
int	   sfxScratchIndex = 0;

void	SND_free(sndBuffer *v) {
	*(sndBuffer **)v = freelist;
	freelist = (sndBuffer*)v;
	inUse += sizeof(sndBuffer);
}

sndBuffer*	SND_malloc() {
	sndBuffer *v;
redo:
	if (freelist == NULL) {
		S_FreeOldestSound();
		goto redo;
	}

	inUse -= sizeof(sndBuffer);
	totalInUse += sizeof(sndBuffer);

	v = freelist;
	freelist = *(sndBuffer **)freelist;
	v->next = NULL;
	return v;
}

void SND_setup() {

	sndBuffer *p, *q;
	//cvar_t	*cv;
	int scs;
	int totalsndsize;

//	cv = Cvar_Get( "com_soundMegs", DEF_COMSOUNDMEGS, CVAR_LATCH | CVAR_ARCHIVE );

	//scs = (cv->integer*256);

//	buffer = malloc(scs*sizeof(sndBuffer) );
	
	totalsndsize=g_mem_snd.GetTotalSize();
	scs=totalsndsize/sizeof(sndBuffer);
	totalsndsize=scs*sizeof(sndBuffer);

	//s_buffer_maphandle=CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,totalsndsize,NULL);
	//buffer=(sndBuffer *)MapViewOfFile(s_buffer_maphandle,FILE_MAP_ALL_ACCESS,0,0,0);
	buffer=(sndBuffer *)g_mem_snd.xmalloc(totalsndsize);
	if ( !buffer ) {
		Com_Error( ERR_FATAL, "Sound buffer failed to allocate %i megs", totalsndsize / (1024*1024) );
	}
	memset(buffer,0,totalsndsize );

	// allocate the stack based hunk allocator
	sfxScratchBuffer = (short *)malloc(SND_CHUNK_SIZE * sizeof(short) * 4);	//Hunk_Alloc(SND_CHUNK_SIZE * sizeof(short) * 4);
	sfxScratchPointer = NULL;

	inUse = scs*sizeof(sndBuffer);
	p = buffer;;
	q = p + scs;
	while (--q > p)
		*(sndBuffer **)q = q-1;
	
	*(sndBuffer **)q = NULL;
	freelist = p + scs - 1;

	Com_Printf("Sound memory manager started\n");
}

/*
===============================================================================

WAV loading

===============================================================================
*/

static	byte	*data_p;
static	byte 	*iff_end;
static	byte 	*last_chunk;
static	byte 	*iff_data;
static	int 	iff_chunk_len;

static short GetLittleShort(void)
{
	short val = 0;
	val = *data_p;
	val = val + (*(data_p+1)<<8);
	data_p += 2;
	return val;
}

static int GetLittleLong(void)
{
	int val = 0;
	val = *data_p;
	val = val + (*(data_p+1)<<8);
	val = val + (*(data_p+2)<<16);
	val = val + (*(data_p+3)<<24);
	data_p += 4;
	return val;
}

static void FindNextChunk(char *name)
{
	while (1)
	{
		data_p=last_chunk;

		if (data_p >= iff_end)
		{	// didn't find the chunk
			data_p = NULL;
			return;
		}
		
		data_p += 4;
		iff_chunk_len = GetLittleLong();
		if (iff_chunk_len < 0)
		{
			data_p = NULL;
			return;
		}
		data_p -= 8;
		last_chunk = data_p + 8 + ( (iff_chunk_len + 1) & ~1 );
		if (!strncmp((char *)data_p, name, 4))
			return;
	}
}

static void FindChunk(char *name)
{
	last_chunk = iff_data;
	FindNextChunk (name);
}

/*
============
GetWavinfo
============
*/
static wavinfo_t GetWavinfo (char *name, byte *wav, int wavlength)
{
	wavinfo_t	info;

	Com_Memset (&info, 0, sizeof(info));

	if (!wav)
		return info;
		
	iff_data = wav;
	iff_end = wav + wavlength;

// find "RIFF" chunk
	FindChunk("RIFF");
	if (!(data_p && !strncmp((char *)data_p+8, "WAVE", 4)))
	{
		Com_Printf("Missing RIFF/WAVE chunks\n");
		return info;
	}

// get "fmt " chunk
	iff_data = data_p + 12;
// DumpChunks ();

	FindChunk("fmt ");
	if (!data_p)
	{
		Com_Printf("Missing fmt chunk\n");
		return info;
	}
	data_p += 8;
	info.format = GetLittleShort();
	info.channels = GetLittleShort();
	info.rate = GetLittleLong();
	data_p += 4+2;
	info.width = GetLittleShort() / 8;

	if (info.format != 1)
	{
		Com_Printf("Microsoft PCM format only\n");
		return info;
	}


// find data chunk
	FindChunk("data");
	if (!data_p)
	{
		Com_Printf("Missing data chunk\n");
		return info;
	}

	data_p += 4;
	info.samples = GetLittleLong () / info.width;
	info.dataofs = data_p - wav;

	return info;
}


/*
================
ResampleSfx

resample / decimate to the current source rate
================
*/
static void ResampleSfx( sfx_t *sfx, int inrate, int inwidth, byte *data, qboolean compressed ) {
	int		outcount;
	int		srcsample;
	lfixed	stepscale;
	int		i;
	int		sample, samplefrac, fracstep;
	int			part;
	sndBuffer	*chunk;
	
	stepscale = MAKE_LFIXED(inrate) / MAKE_LFIXED(dma.speed);	// this is usually GFIXED(0,5), 1, or 2

	outcount = FIXED_TO_INT(MAKE_LFIXED(sfx->soundLength) / stepscale);
	sfx->soundLength = outcount;

	samplefrac = 0;
	fracstep = FIXED_TO_INT(stepscale * LFIXED(256,0));
	chunk = sfx->soundData;

	for (i=0 ; i<outcount ; i++)
	{
		srcsample = samplefrac >> 8;
		samplefrac += fracstep;
		if( inwidth == 2 ) {
			sample = LittleShort ( ((short *)data)[srcsample] );
		} else {
			sample = (int)( (unsigned char)(data[srcsample]) - 128) << 8;
		}
		part  = (i&(SND_CHUNK_SIZE-1));
		if (part == 0) {
			sndBuffer	*newchunk;
			newchunk = SND_malloc();
			if (chunk == NULL) {
				sfx->soundData = newchunk;
			} else {
				chunk->next = newchunk;
			}
			chunk = newchunk;
		}

		chunk->sndChunk[part] = sample;
	}
}

/*
================
ResampleSfx

resample / decimate to the current source rate
================
*/
static int ResampleSfxRaw( short *sfx, int inrate, int inwidth, int samples, byte *data ) {
	int			outcount;
	int			srcsample;
	lfixed		stepscale;
	int			i;
	int			sample, samplefrac, fracstep;
	
	stepscale = MAKE_LFIXED(inrate) / MAKE_LFIXED(dma.speed);	// this is usually GFIXED(0,5), 1, or 2

	outcount = FIXED_TO_INT(MAKE_LFIXED(samples) / stepscale);

	samplefrac = 0;
	fracstep = FIXED_TO_INT(stepscale * LFIXED(256,0));

	for (i=0 ; i<outcount ; i++)
	{
		srcsample = samplefrac >> 8;
		samplefrac += fracstep;
		if( inwidth == 2 ) {
			sample = LittleShort ( ((short *)data)[srcsample] );
		} else {
			sample = (int)( (unsigned char)(data[srcsample]) - 128) << 8;
		}
		sfx[i] = sample;
	}
	return outcount;
}


//=============================================================================

#ifdef USE_MP3_SOUNDS

unsigned long psurand_gen(unsigned long state)
{
  return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}


short ScaleAudio( mad_fixed_t sample, mad_fixed_t *rndval_ptr )
{
	unsigned int scalebits;
	mad_fixed_t output, mask, rnd;

	// bias 
	output = sample + (1L << (MAD_F_FRACBITS + 1 - 16 - 1));	
	
  	scalebits = MAD_F_FRACBITS + 1 - 16;
  	mask = (1L << scalebits) - 1;

  	// dither 
	rnd = psurand_gen(*rndval_ptr);
  	output += (rnd & mask) - (*rndval_ptr & mask);
	*rndval_ptr = rnd;
	
  	// clip 
	if (output >= MAD_F_ONE)
    	output = MAD_F_ONE - 1;
	else if (output < -MAD_F_ONE)
    	output = -MAD_F_ONE;

  	// quantize 
  	output &= ~mask;

	// scale 
	return output >> scalebits;
}

#pragma pack(push,1)
typedef struct 
{
	DWORD dwRIFF;
	DWORD dwRiffLen;
	
	DWORD dwWAVE;
	
	DWORD dwFMT;
	DWORD dwFmtSize;
	WORD wCompression;
	WORD nChannels;
	DWORD dwSampleRate;
	DWORD dwAvgBytesPerSec;
	WORD wBlockAlign;
	WORD wSignificantBitsPerSample;
	
	DWORD dwDATA;
	DWORD dwLength;
} WAV;
#pragma pack(pop)


int ReadMP3ToWAV( const char *soundName, void **data )
{
	fileHandle_t	h;
	byte*			buf;
	int				len;
	
	len = FS_FOpenFileRead( soundName, &h, qfalse );
	
	if ( h == 0 ) {
		if ( data) {
			*data = NULL;
		}
		return -1;
	}
	
	if ( !data) {
		FS_FCloseFile( h);
		return -1;
	}

	mad_stream stream;
	mad_frame frame;
	mad_synth synth;
	mad_timer_t timer;
	
	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);
	mad_timer_reset(&timer);

#define INBUFFER_SIZE (65536)

	unsigned char in_buffer[INBUFFER_SIZE];
	int inbuf_cnt=0;
	int inlen=0;
	int outlen=0;
	int cur_in=0;
	int cur_out=0;
	
	inlen=FS_Read(in_buffer,INBUFFER_SIZE,h);
	inbuf_cnt=(inlen>INBUFFER_SIZE)?INBUFFER_SIZE:inlen;

	mad_stream_buffer( &stream, in_buffer, inbuf_cnt);
	mad_frame_decode(&frame, &stream);
	
	outlen = sizeof(WAV)+(int)(((float)inlen / (float)frame.header.bitrate * 8.0) * 2.0 * 22050.0);

	unsigned char *bufbase = (byte *)Hunk_AllocateTempMemory(outlen);
	*data = bufbase;
	
	buf=bufbase;
	WAV *hdr=(WAV *)buf;
	buf+=sizeof(WAV);

	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);

	mad_stream_init(&stream);
	mad_frame_init(&frame);
	
	mad_stream_buffer( &stream, in_buffer, inbuf_cnt);
	
	int rnum=0xCDC31337;

	cur_out=0;
	int nLast=0;
		
	while(!nLast)
	{
		while(mad_frame_decode(&frame, &stream)!=0) 
		{
		    if (stream.error==MAD_ERROR_BUFLEN)
			{
				// Slide stuff over in buffer
				int amtgood=stream.bufend - stream.next_frame;
				int amtdead=INBUFFER_SIZE-amtgood;
				memmove(in_buffer, stream.next_frame,amtgood);
				int readcnt=FS_Read(in_buffer+amtgood,amtdead,h);
				if(readcnt==0)
				{
					nLast=1;
					break;
				}
				inbuf_cnt=readcnt+amtgood;
				
				mad_stream_buffer( &stream, in_buffer, inbuf_cnt);
			}
			else if(stream.error==MAD_ERROR_NOMEM)
			{
				mad_synth_finish(&synth);
				mad_frame_finish(&frame);
				mad_stream_finish(&stream);

				FS_FCloseFile( h );
				
				Hunk_FreeTempMemory(bufbase);
				*data=NULL;
				return -1;
			}
		}
		if(nLast)
		{
			break;
		}

		mad_timer_add( &timer, frame.header.duration);

		mad_synth_frame( &synth, &frame );

		for(int i=0;i<synth.pcm.length;i++)
		{
			if((cur_out+1)>=outlen)
			{
				nLast=1;
				break;
			}

			short sample=ScaleAudio(synth.pcm.samples[0][i], &rnum);
			*(buf++)=((sample>>0)&0xff);
			*(buf++)=((sample>>8)&0xff);
			cur_out+=2;
		}	
	}

	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);

	FS_FCloseFile( h );


	hdr->dwRIFF=FCC('R','I','F','F');
	hdr->dwRiffLen=(cur_out+sizeof(WAV))-8;
	hdr->dwWAVE=FCC('W','A','V','E');
	
	hdr->dwFMT=FCC('f','m','t',' ');
	hdr->dwFmtSize=16;
	hdr->wCompression=1;
	hdr->nChannels=1;
	hdr->dwSampleRate=22050;
	hdr->wSignificantBitsPerSample=16;
	hdr->wBlockAlign=hdr->wSignificantBitsPerSample / 8 * hdr->nChannels;
	hdr->dwAvgBytesPerSec=((DWORD)hdr->wBlockAlign)*hdr->dwSampleRate;
	
	hdr->dwDATA=FCC('d','a','t','a');
	hdr->dwLength=cur_out;

	return cur_out+sizeof(WAV);
}

#endif

//=============================================================================

/*
==============
S_LoadSound

The filename may be different than sfx->name in the case
of a forced fallback of a player specific sound
==============
*/
qboolean S_LoadSound( sfx_t *sfx )
{
	byte	*data;
	short	*samples;
	wavinfo_t	info;
	int		size;

	// player specific sounds are never directly loaded
	if ( sfx->soundName[0] == '*') {
		return qfalse;
	}

	// load it in
#ifdef USE_MP3_SOUNDS
	char mp3name[256];
	strcpy(mp3name,sfx->soundName);
	int len=strlen(mp3name);
	mp3name[len-3]='m';
	mp3name[len-2]='p';
	mp3name[len-1]='3';
	size = ReadMP3ToWAV( mp3name, (void **)&data );
#else
	size = FS_ReadFile( sfx->soundName, (void **)&data );
#endif
	if ( !data ) {
		return qfalse;
	}

	info = GetWavinfo( sfx->soundName, data, size );
	if ( info.channels != 1 ) {
		Com_Printf ("%s is a stereo wav file\n", sfx->soundName);
		FS_FreeFile (data);
		return qfalse;
	}

	if ( info.width == 1 ) {
		Com_DPrintf(S_COLOR_YELLOW "WARNING: %s is a 8 bit wav file\n", sfx->soundName);
	}

	if ( info.rate != 22050 ) {
		Com_DPrintf(S_COLOR_YELLOW "WARNING: %s is not a 22kHz wav file\n", sfx->soundName);
	}


	sfx->lastTimeUsed = Com_Milliseconds()+1;

	// each of these compression schemes works just fine
	// but the 16bit quality is much nicer and with a local
	// install assured we can rely upon the sound memory
	// manager to do the right thing for us and page
	// sound in as needed

	if( sfx->soundCompressed == qtrue) {
		samples = (short *)Hunk_AllocateTempMemory(info.samples * sizeof(short) * 2);

		sfx->soundCompressionMethod = 1;
		sfx->soundData = NULL;
		sfx->soundLength = ResampleSfxRaw( samples, info.rate, info.width, info.samples, (data + info.dataofs) );
		S_AdpcmEncodeSound(sfx, samples);
	
		Hunk_FreeTempMemory(samples);

#if 0
	} else if (info.samples>(SND_CHUNK_SIZE*16) && info.width >1) {
		sfx->soundCompressionMethod = 3;
		sfx->soundData = NULL;
		sfx->soundLength = ResampleSfxRaw( samples, info.rate, info.width, info.samples, (data + info.dataofs) );
		encodeMuLaw( sfx, samples);
	} else if (info.samples>(SND_CHUNK_SIZE*6400) && info.width >1) {
		sfx->soundCompressionMethod = 2;
		sfx->soundData = NULL;
		sfx->soundLength = ResampleSfxRaw( samples, info.rate, info.width, info.samples, (data + info.dataofs) );
		encodeWavelet( sfx, samples);
#endif
	} else {
		sfx->soundCompressionMethod = 0;
		sfx->soundLength = info.samples;
		sfx->soundData = NULL;
		ResampleSfx( sfx, info.rate, info.width, data + info.dataofs, qfalse );
	}
	
#ifdef USE_MP3_SOUNDS
	Hunk_FreeTempMemory( data );
#else
	FS_FreeFile( data );
#endif


	return qtrue;
}

void S_DisplayFreeMemory() {
	Com_Printf("%d bytes free sound buffer memory, %d total used\n", inUse, totalInUse);
}
