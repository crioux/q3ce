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
 * $Id: libmad.c 198 2005-01-14 16:33:12Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "../common/common.h"
#include "mp3.h"
#include "libmad/mad.h"
#include "libmad/frame.h"
#include "libmad/synth.h"

#define LIBMAD_EQ			0x110

typedef struct libmad
{
	codec Codec;
	equalizer Eq;

	buffer Buffer;
	struct mad_stream Stream;
	struct mad_frame Frame;
	struct mad_synth Synth;

	int BufferAlign;
	int AdjustBytes; // adjust (rewind) RefTime
	bool_t Skip; // skip next frame
	int AdjustTime;
	int FormatSet;

	mad_fixed_t Scale;
	mad_fixed_t EqTable[32];
	mad_fixed_t EqScale[32];

} libmad;

static const datatable Params[] = 
{
	{ LIBMAD_EQ,		TYPE_EQUALIZER, DF_HIDDEN },

	DATATABLE_END(LIBMAD_ID)
};

static int Enum(void* p, int* No, datadef* Param)
{
	if (CodecEnum(p,No,Param)==ERR_NONE)
		return ERR_NONE;
	return NodeEnumTable(No,Param,Params);
}

static void UpdateScale( libmad* p )
{
	int i;
	for (i=0;i<32;++i)
		p->EqScale[i] = mad_f_mul(p->EqTable[i],p->Scale);
}

static int UpdateEq( libmad* p )
{
	static const uint8_t Map[32] = 
	{
		0, 1, 2, 3, 4, 5, 6, 6, 
		7, 7, 7, 7, 8, 8, 8, 8,
		8, 8, 8, 8, 9, 9, 9, 9, 
		9, 9, 9, 9, 9, 9, 9, 9
	};
	static const mad_fixed_t Pow[37] = //mad_f_tofixed(pow(10,(n-18)/20.f))
	{
    /* -18 */ MAD_F(0x0203a7e5), /* 0.125892541 */
    /* -17 */ MAD_F(0x0242934b), /* 0.141253754 */
    /* -16 */ MAD_F(0x02892c18), /* 0.158489319 */
    /* -15 */ MAD_F(0x02d8621c), /* 0.177827941 */
    /* -14 */ MAD_F(0x0331426a), /* 0.199526231 */
    /* -13 */ MAD_F(0x0394faec), /* 0.223872114 */
    /* -12 */ MAD_F(0x0404de61), /* 0.251188643 */
    /* -11 */ MAD_F(0x048268de), /* 0.281838293 */
    /* -10 */ MAD_F(0x050f44d8), /* 0.316227766 */
    /*  -9 */ MAD_F(0x05ad50cd), /* 0.354813389 */
    /*  -8 */ MAD_F(0x065ea59f), /* 0.398107171 */
    /*  -7 */ MAD_F(0x07259db1), /* 0.446683592 */
    /*  -6 */ MAD_F(0x0804dce7), /* 0.501187234 */
    /*  -5 */ MAD_F(0x08ff599e), /* 0.562341325 */
    /*  -4 */ MAD_F(0x0a1866ba), /* 0.630957344 */
    /*  -3 */ MAD_F(0x0b53bef5), /* 0.707945784 */
    /*  -2 */ MAD_F(0x0cb59185), /* 0.794328235 */
    /*  -1 */ MAD_F(0x0e429057), /* 0.891250938 */
    /*   0 */ MAD_F(0x10000000), /* 1.000000000 */
    /*   1 */ MAD_F(0x11f3c99f), /* 1.122018454 */
    /*   2 */ MAD_F(0x14248ef8), /* 1.258925412 */
    /*   3 */ MAD_F(0x1699c0f7), /* 1.412537545 */
    /*   4 */ MAD_F(0x195bb8f6), /* 1.584893192 */
    /*   5 */ MAD_F(0x1c73d51c), /* 1.778279410 */
    /*   6 */ MAD_F(0x1fec982d), /* 1.995262315 */
    /*   7 */ MAD_F(0x23d1cd41), /* 2.238721139 */
    /*   8 */ MAD_F(0x2830afd3), /* 2.511886432 */
    /*   9 */ MAD_F(0x2d1818b3), /* 2.818382931 */
    /*  10 */ MAD_F(0x3298b075), /* 3.162277660 */
    /*  11 */ MAD_F(0x38c5280b), /* 3.548133892 */
    /*  12 */ MAD_F(0x3fb2783e), /* 3.981071706 */
    /*  13 */ MAD_F(0x477828f1), /* 4.466835922 */
    /*  14 */ MAD_F(0x5030a10c), /* 5.011872336 */
    /*  15 */ MAD_F(0x59f9802c), /* 5.623413252 */
    /*  16 */ MAD_F(0x64f40348), /* 6.309573445 */
    /*  17 */ MAD_F(0x71457596), /* 7.079457844 */
    /*  18 */ MAD_F(0x7f17af3b), /* 7.943282347 */
	};

	int i;
	for (i=0;i<32;++i)
	{
		int db = p->Eq.Amplify + p->Eq.Eq[Map[i]];
		if (db>18) db=18;
		if (db<-18)	db=-18;
		p->EqTable[i] = Pow[db+18];
	}

	if (!p->Eq.Enabled)
		p->Scale=MAD_F_ONE;

	UpdateScale(p);
	return ERR_NONE;
}

static INLINE void UpdateAdjustTime( libmad* p )
{
	if (p->Codec.In.Format.ByteRate)
		p->AdjustTime = (TICKSPERSEC * 4096) / p->Codec.In.Format.ByteRate;
	else
		p->AdjustTime = 0;
}

static int UpdateInput( libmad* p )
{
	BufferClear(&p->Buffer);
	mad_frame_finish(&p->Frame);
	mad_stream_finish(&p->Stream);
	mad_synth_finish(&p->Synth);

	if (p->Codec.In.Format.Type == PACKET_AUDIO)
	{
		PacketFormatPCM(&p->Codec.Out.Format,&p->Codec.In.Format,MAD_F_FRACBITS+1);
		p->Codec.Out.Format.Audio.Flags = PCM_PLANES;

		p->FormatSet = 0;
		p->AdjustBytes = 0;
		p->Scale = MAD_F_ONE;

		UpdateAdjustTime(p);
		UpdateEq(p);

		mad_stream_init(&p->Stream);
		mad_frame_init(&p->Frame);
		mad_synth_init(&p->Synth);
	}

	return ERR_NONE;
}

static void Rescale( libmad* p, mad_fixed_t v, int ramp )
{
	mad_fixed_t dv;
	int ch;

	if (p->Synth.pcm.length < 4) return;
	while ((1<<ramp) > p->Synth.pcm.length) --ramp;
	dv = (v-MAD_F_ONE) >> ramp;

	for (ch=0;ch<p->Synth.pcm.channels;++ch)
	{
		mad_fixed_t* i=p->Synth.pcm.samples[ch];
		mad_fixed_t* ie=i+(1<<ramp);

		v = MAD_F_ONE;
		for (;i!=ie;++i,v+=dv)
			i[0] = mad_f_mul(i[0],v);

		ie += p->Synth.pcm.length-(1<<ramp);
		for (;i!=ie;i+=4)
		{
			i[0] = mad_f_mul(i[0],v);
			i[1] = mad_f_mul(i[1],v);
			i[2] = mad_f_mul(i[2],v);
			i[3] = mad_f_mul(i[3],v);
		}
	}
}

static int Process( libmad* p, const packet* Packet, const flowstate* State )
{
	if (Packet)
	{
		// set new reftime
		if (Packet->RefTime >= 0)
		{
			p->Codec.Packet.RefTime = Packet->RefTime;
			p->AdjustBytes = p->Buffer.WritePos - p->Buffer.ReadPos;
		}

		// add new packet to buffer
		BufferWrite(&p->Buffer,Packet->Data[0],Packet->Length,p->BufferAlign);
		mad_stream_buffer(&p->Stream, p->Buffer.Data+p->Buffer.ReadPos, p->Buffer.WritePos-p->Buffer.ReadPos);
	}
	else
		p->Codec.Packet.RefTime = TIME_UNKNOWN;

	for (;;)
	{
		while (mad_frame_decode(&p->Frame, &p->Stream) == -1)
		{
			if (p->Stream.error == MAD_ERROR_BUFLEN || p->Stream.error == MAD_ERROR_BUFPTR)
			{
				BufferPack(&p->Buffer,p->Stream.next_frame - p->Stream.buffer);
				return ERR_NEED_MORE_DATA;
			}
			if (p->Stream.error == MAD_ERROR_NOMEM)
			{
				BufferDrop(&p->Buffer);
				return ERR_OUT_OF_MEMORY;
			}
		}

		if (p->Eq.Enabled)
		{
			int n = MAD_NCHANNELS(&p->Frame.header);
			int m = MAD_NSBSAMPLES(&p->Frame.header);
			int ch;

			for (ch=0;ch<n;++ch)
			{
				mad_fixed_t* i;
				mad_fixed_t* ie;
				int eq=0;

				ie=p->Frame.sbsample[ch][m];
				for (i=p->Frame.sbsample[ch][0];i!=ie;i+=4,eq+=4)
				{
					eq &= 31;
					i[0] = mad_f_mul(i[0],p->EqScale[eq+0]);
					i[1] = mad_f_mul(i[1],p->EqScale[eq+1]);
					i[2] = mad_f_mul(i[2],p->EqScale[eq+2]);
					i[3] = mad_f_mul(i[3],p->EqScale[eq+3]);
				}
			}
		}

		if (p->Skip)
		{
			// the first frame, it may be corrupt
			p->Skip--;
			continue;
		}

		mad_synth_frame(&p->Synth,&p->Frame);

		// handle stereo streams with random mono frames (is this really a good mp3?)
		if (p->Codec.Out.Format.Audio.Channels==2 && p->Synth.pcm.channels==1)
		{
			p->Synth.pcm.channels = 2;
			memcpy(p->Synth.pcm.samples[1],p->Synth.pcm.samples[0],
				p->Synth.pcm.length * sizeof(mad_fixed_t));
		}

		if (p->Codec.In.Format.ByteRate != (int)(p->Frame.header.bitrate >> 3))
		{
			p->Codec.In.Format.ByteRate = (p->Frame.header.bitrate >> 3);
			UpdateAdjustTime(p);
		}

		// adjust RefTime with AdjustBytes (now that we know the bitrate)
		if (p->Codec.Packet.RefTime >= 0)
			p->Codec.Packet.RefTime -= (p->AdjustBytes * p->AdjustTime) >> 12;

		// output format setup needed?
		if (p->Codec.Out.Format.Audio.SampleRate != (int)p->Synth.pcm.samplerate ||
			p->Codec.Out.Format.Audio.Channels != p->Synth.pcm.channels)
		{
			if (p->FormatSet)
			{
				p->FormatSet--;
				continue; // probably a bad frame, drop it
			}

			// set new output format
			p->Codec.In.Format.Audio.SampleRate = p->Codec.Out.Format.Audio.SampleRate = p->Synth.pcm.samplerate;
			p->Codec.In.Format.Audio.Channels = p->Codec.Out.Format.Audio.Channels = p->Synth.pcm.channels;
			ConnectionUpdate(&p->Codec.Node,CODEC_OUTPUT,p->Codec.Out.Pin.Node,p->Codec.Out.Pin.No);
		}
		p->FormatSet = 16; // reset countdown

		if (p->Eq.Enabled)
		{
			mad_fixed_t Max = 0;
			int ch;
			for (ch=0;ch<p->Synth.pcm.channels;++ch)
			{
				mad_fixed_t* i=p->Synth.pcm.samples[ch];
				mad_fixed_t* ie=i+p->Synth.pcm.length;
				for (;i!=ie;++i)
				{
					mad_fixed_t v = *i;
					if (v<0) v=-v;
					if (v>Max) Max=v;
				}
			}

			if (Max > MAD_F_ONE)
			{
				Max = (mad_fixed_t) (((int64_t)MAD_F_ONE * (int64_t)MAD_F_ONE *15/16) / Max);
				Rescale(p,Max,5);
				
				p->Scale = mad_f_mul(p->Scale,Max);
				UpdateScale(p);
			}
			else
			if (Max < ((MAD_F_ONE*6)/8) && p->Scale < (MAD_F_ONE - MAD_F_ONE/256))
			{
				Max = (MAD_F_ONE/31)*32;
				if (mad_f_mul(p->Scale,Max) > MAD_F_ONE)
					Max = (mad_fixed_t) (((int64_t)MAD_F_ONE * (int64_t)MAD_F_ONE) / p->Scale);
				Rescale(p,Max,10);
			
				p->Scale = mad_f_mul(p->Scale,Max);
				UpdateScale(p);
			}
		}
		break;
	}

	// build output packet
	p->Codec.Packet.Length = p->Synth.pcm.length * sizeof(mad_fixed_t);
	p->Codec.Packet.Data[0] = p->Synth.pcm.samples[0];
	p->Codec.Packet.Data[1] = p->Synth.pcm.samples[1];
	return ERR_NONE;
}

static int Set( libmad* p, int No, const void* Data, int Size )
{
	int Result = CodecSet(&p->Codec,No,Data,Size);
	switch (No)
	{
	case LIBMAD_EQ: SETVALUE(p->Eq,equalizer,UpdateEq(p)); break;
	}
	return Result;
}

static int Get( libmad* p, int No, void* Data, int Size )
{
	int Result = CodecGet(&p->Codec,No,Data,Size);
	switch (No)
	{
	case LIBMAD_EQ: GETVALUE(p->Eq,equalizer); break;
	}
	return Result;
}

static int Flush( libmad* p )
{
	BufferDrop(&p->Buffer);
	p->Skip = 1;

	mad_frame_finish(&p->Frame);
	mad_synth_finish(&p->Synth);
	mad_stream_finish(&p->Stream);

	mad_frame_init(&p->Frame);
	mad_synth_init(&p->Synth);
	mad_stream_init(&p->Stream);
	return ERR_NONE;
}

#ifdef BUILDFIXED

#include <math.h>

extern struct fixedfloat {
  unsigned long mantissa : 27;
  unsigned long exponent :  5;
} rq_table[8207];

#endif

static int Create( libmad* p )
{
	p->Codec.Node.Enum = (nodeenum)Enum;
	p->Codec.Node.Get = (nodeget)Get;
	p->Codec.Node.Set = (nodeset)Set;
	p->Codec.Process = (packetprocess)Process;
	p->Codec.UpdateInput = (nodefunc)UpdateInput;
	p->Codec.Flush = (nodefunc)Flush;
	p->BufferAlign = QueryPlatform(PLATFORM_LOWMEMORY)?4096:16384;
	return ERR_NONE;
}

#define XING_FRAMES     0x01
#define XING_BYTES		0x02
#define XING_TOC		0x04
#define XING_SCALE		0x08

typedef struct mp3
{
	rawaudio RawAudio;

	int Frames;
	int Bytes;

	bool_t TOC;
	uint8_t TOCTable[100];

	int VBRITableSize;
	int VBRIEntryFrames;
	int* VBRITable;

} mp3;

static int Read(format_reader* Reader,int n)
{
	int v=0;
	while (n--)
	{
		v = v << 8;
		v += Reader->Read8(Reader);
	}
	return v;
}

static int InitMP3(mp3* p)
{
	static const int RateTable[4] = { 44100, 48000, 32000, 99999 };
	format_reader* Reader;
	int i,SampleRate,Id,Mode,Layer,SamplePerFrame;
	int Result = RawAudioInit(&p->RawAudio);

	p->TOC = 0;
	p->Bytes = p->RawAudio.Format.FileSize - p->RawAudio.Head;
	p->Frames = 0;
	p->RawAudio.VBR = 0;

	Reader = p->RawAudio.Format.Reader;

	for (i=0;i<2048;++i)
		if (Reader->Read8(Reader) == 0xFF)
		{
			filepos_t Frame;

			i = Reader->Read8(Reader);
			if ((i & 0xE0) != 0xE0)
				continue;

			Id = (i >> 3) & 3;
			Layer = (i >> 1) & 3;
			SampleRate = RateTable[(Reader->Read8(Reader) >> 2) & 3];
			if (Id==2)
				SampleRate >>= 1; // MPEG2
			if (Id==0)
				SampleRate >>= 2; // MPEG2.5
			Mode = (Reader->Read8(Reader) >> 6) & 3;

			SamplePerFrame = (Layer == 3)?384:1152;

			Frame = Reader->FilePos;

			//Xing offset
			if (Id==3)
			{
				// MPEG1
				Reader->Skip(Reader,Mode==3 ? 17:32);
			}
			else
			{
				// MPEG2/2.5
				Reader->Skip(Reader,Mode==3 ? 9:17);
				if (Layer == 1) // layer-3
					SamplePerFrame = 576;
			}

			if (Reader->ReadLE32(Reader) == FOURCCLE('X','i','n','g'))
			{
				int Flags = Reader->ReadBE32(Reader);
				if (Flags & XING_FRAMES) 
					p->Frames = Reader->ReadBE32(Reader);

				if (Flags & XING_BYTES)
					p->Bytes = Reader->ReadBE32(Reader);

				if (Flags & XING_TOC)
				{
					p->TOC = 1;
					Reader->Read(Reader,p->TOCTable,100);
				}
			}
			else
			{
				Reader->Seek(Reader,Frame+32,SEEK_SET);

				if (Reader->ReadLE32(Reader) == FOURCCLE('V','B','R','I'))
				{
					int Scale,EntryBytes;

					Reader->Skip(Reader,2+2+2); //Version,Delay,Quality
					p->Bytes= Reader->ReadBE32(Reader);
					p->Frames = Reader->ReadBE32(Reader);

					p->VBRITableSize = Reader->ReadBE16(Reader)+1;
					Scale = Reader->ReadBE16(Reader);
					EntryBytes = Reader->ReadBE16(Reader);
					p->VBRIEntryFrames = Reader->ReadBE16(Reader);

					p->VBRITable = malloc(sizeof(int)*p->VBRITableSize);
					if (p->VBRITable)
						for (i=0;i<p->VBRITableSize;++i)
							p->VBRITable[i] = Read(Reader,EntryBytes)*Scale;
				}
			}

			if (p->Frames>0)
			{
				p->RawAudio.VBR = 1;
				p->RawAudio.Format.Duration = Scale(p->Frames,TICKSPERSEC*SamplePerFrame,SampleRate);

				if (p->Bytes>0)
					p->RawAudio.Format.Streams[0]->Format.ByteRate = Scale(p->Bytes,TICKSPERSEC,p->RawAudio.Format.Duration);
			}

			break;
		}

	Reader->Seek(Reader,p->RawAudio.Head,SEEK_SET);
	return Result;
}

static void DoneMP3(mp3* p)
{
	if (p->VBRITable)
	{
		free(p->VBRITable);
		p->VBRITable = NULL;
	}
}

static int SeekMP3(mp3* p, tick_t Time, filepos_t FilePos,bool_t PrevKey)
{
	if (FilePos < 0 && Time > 0 && p->RawAudio.Format.Duration > 0)
	{
		int i,a,b;

		if (Time > p->RawAudio.Format.Duration)
			Time = p->RawAudio.Format.Duration;

		if (p->VBRITable)
		{
			int i;
			tick_t Left;
			tick_t EntryTime = p->RawAudio.Format.Duration / p->VBRITableSize;

			FilePos = p->RawAudio.Head;

			Left = Time;
			for (i=0;Left>0 && i<p->VBRITableSize;++i)
			{
				FilePos += p->VBRITable[i];
				Left -= EntryTime;
			}

			if (i>0)
				FilePos += Scale(Left,p->VBRITable[i-1],EntryTime);
		}
		else
		if (p->TOC && p->Bytes>0)
		{
			i = Scale(Time,100,p->RawAudio.Format.Duration);

			if (i>99) i=99;
			a = p->TOCTable[i];
			if (i==99)
				b = 256;
			else
				b = p->TOCTable[i+1];

			a <<= 10;
			b <<= 10;
			a += Scale(Time - Scale(i,p->RawAudio.Format.Duration,100),b-a,p->RawAudio.Format.Duration);

			FilePos = p->RawAudio.Head + Scale(a,p->Bytes,256*1024);
		}
	}
	return RawAudioSeek(&p->RawAudio,Time,FilePos,PrevKey);
}

static int CreateMP3( mp3* p )
{
	p->RawAudio.Format.Init = (fmtfunc)InitMP3;
	p->RawAudio.Format.Done = (fmtvoid)DoneMP3;
	p->RawAudio.Format.Seek = (fmtseek)SeekMP3;
	return ERR_NONE;
}

static const nodedef MP3 = 
{
	sizeof(mp3),
	MP3_ID,
	RAWAUDIO_CLASS,
	PRI_DEFAULT-5,
	(nodecreate)CreateMP3,
};

static const nodedef LibMad = 
{
	sizeof(libmad),
	LIBMAD_ID,
	CODEC_CLASS,
	PRI_DEFAULT,
	(nodecreate)Create,
};

void LibMad_Init()
{
#ifdef BUILDFIXED
	int x;
	struct fixedfloat* p=rq_table;
	for (x=0;x<8207;++x,++p)
	{
		int exp;
		double v = frexp(pow(x,4./3.),&exp);
		if (exp) ++exp;
		p->exponent = (unsigned short)exp;
		p->mantissa = (int)(0x10000000 * (v*0.5));
	}
#endif

	NodeRegisterClass(&LibMad);
	NodeRegisterClass(&MP3);
}

void LibMad_Done()
{
	NodeUnRegisterClass(LIBMAD_ID);
	NodeUnRegisterClass(MP3_ID);
}
