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
 * $Id: streams.c 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "common.h"

static const datatable StreamParams[] = 
{
	{ STREAM_URL,			TYPE_STRING },
	{ STREAM_LENGTH,		TYPE_INT, DF_HIDDEN },
	{ STREAM_SILENT,		TYPE_BOOL, DF_HIDDEN },
	{ STREAM_CREATE,		TYPE_BOOL, DF_HIDDEN },
	{ STREAM_CONTENTTYPE,	TYPE_STRING, DF_HIDDEN|DF_RDONLY },
	{ STREAM_PRAGMA_SEND,	TYPE_STRING, DF_HIDDEN|DF_RDONLY },
	{ STREAM_PRAGMA_GET,	TYPE_STRING, DF_HIDDEN|DF_RDONLY },
	{ STREAM_COMMENT,		TYPE_COMMENT, DF_OUTPUT },
	{ STREAM_METAPROCESSOR,	TYPE_INT, DF_HIDDEN },

	DATATABLE_END(STREAM_CLASS)
};

int StreamEnum(void* p, int* No, datadef* Param)
{
	return NodeEnumTable(No,Param,StreamParams);
}

static int DummyRead(void* This,void* Data,int Size) { return -1; }
static int DummyReadBlock(void* This,block* Block,int Ofs,int Size) { return -1; }
static filepos_t DummySeek(void* This,filepos_t Pos,int SeekMode) { return -1; }
static int DummyWrite(void* This,const void* Data,int Size) { return -1; }
static int DummyEnumDir(void* This,const tchar_t* URL,const tchar_t* Exts,
						bool_t ExtFilter,streamdir* Item) { return ERR_FILE_NOT_FOUND; }

static int StreamCreate(stream* p)
{
	p->Enum = StreamEnum;
	p->Read = DummyRead;
	p->ReadBlock = DummyReadBlock;
	p->Write = DummyWrite;
	p->EnumDir = DummyEnumDir;
	p->Seek = DummySeek;
	return ERR_NONE;
}

static const nodedef Stream =
{
	0,
	STREAM_CLASS,
	NODE_CLASS,
	PRI_DEFAULT,
	(nodecreate)StreamCreate,
};

static const datatable StreamProcessParams[] = 
{
	{ STREAMPROCESS_INPUT,		TYPE_NODE, DF_HIDDEN, STREAM_CLASS },

	DATATABLE_END(STREAMPROCESS_CLASS)
};

static int StreamProcessEnum(void* p, int* No, datadef* Param)
{
	if (StreamEnum(p,No,Param)==ERR_NONE)
		return ERR_NONE;
	return NodeEnumTable(No,Param,StreamProcessParams);
}

static int StreamProcessCreate(stream* p)
{
	p->Enum = StreamProcessEnum;
	return ERR_NONE;
}

static const nodedef StreamProcess =
{
	0,
	STREAMPROCESS_CLASS,
	STREAM_CLASS,
	PRI_DEFAULT,
	(nodecreate)StreamProcessCreate,
};

void Stream_Init()
{
	NodeRegisterClass(&Stream);
	NodeRegisterClass(&StreamProcess);
}

void Stream_Done()
{
	NodeUnRegisterClass(STREAMPROCESS_CLASS);
	NodeUnRegisterClass(STREAM_CLASS);
}

const tchar_t* GetMime(const tchar_t* URL, tchar_t* Mime, bool_t* HasHost)
{
	const tchar_t* s = tcschr(URL,':');
	if (s && s[1] == '/' && s[2] == '/')
	{
		if (Mime)
			TcsNCpy(Mime,URL,s-URL+1);
		if (HasHost)
			*HasHost = TcsNICmp(URL,T("file"),4)!=0 &&
			           TcsNICmp(URL,T("mem"),3)!=0 &&
			           TcsNICmp(URL,T("pose"),4)!=0 &&
			           TcsNICmp(URL,T("vol"),3)!=0 &&
			           TcsNICmp(URL,T("slot"),4)!=0 &&
					   TcsNICmp(URL,T("simu"),4)!=0;
		s += 3;
	}
	else
	{
		if (HasHost)
			*HasHost = 0;
		if (Mime)
			tcscpy(Mime,T("file"));
		s = URL;
	}
	return s;
}

stream* GetStream(const tchar_t* URL, bool_t Silent)
{
	tchar_t Mime[MAXPATH];
	stream* Stream;

	GetMime(URL,Mime,NULL);
	Stream = (stream*)NodeCreate(NodeEnumClassEx(NULL,STREAM_CLASS,Mime,NULL,NULL,0));

	if (!Stream && !Silent)
	{
		TcsUpr(Mime);
		ShowError(0,ERR_ID,ERR_MIME_NOT_FOUND,Mime);
	}
	return Stream;
}

void StreamPrintfEx(stream* Stream, bool_t UTF8, const tchar_t* Msg,...)
{
	tchar_t* s0 = (tchar_t*)malloc(2048*sizeof(tchar_t));
	tchar_t* s1 = s0;
	tchar_t* s2 = s0+1024;

	va_list Args;
	va_start(Args,Msg);
	vstprintf(s1, Msg, Args);
	va_end(Args);

	while (*s1)
	{
		if (*s1 == 10)
			*(s2++) = 13;
		*(s2++) = *(s1++);
	}
	*s2=0;

	if (UTF8)
		TcsToUTF8((char*)s0,s0+1024,1024*sizeof(tchar_t));
	else
		TcsToStr((char*)s0,s0+1024,1024*sizeof(tchar_t));

	Stream->Write(Stream,s0,strlen((char*)s0)*sizeof(char));
	free(s0);
}

void StreamPrintf(stream* Stream, const tchar_t* Msg,...)
{
	tchar_t* s0 = (tchar_t*)malloc(2048*sizeof(tchar_t));
	tchar_t* s1 = s0;
	tchar_t* s2 = s0+1024;
	
	va_list Args;
	va_start(Args,Msg);
	vstprintf(s1, Msg, Args);
	va_end(Args);

	while (*s1)
	{
		if (*s1 == 10)
			*(s2++) = 13;
		*(s2++) = *(s1++);
	}
	*s2=0;

	TcsToStr((char*)s0,s0+1024,1024*sizeof(tchar_t));
	
	Stream->Write(Stream,s0,strlen((char*)s0)*sizeof(char));
	free(s0);
}

extern stream* FileCreate(const tchar_t*);
extern void FileRelease(stream*);

stream* StreamOpen(const tchar_t* Path, bool_t Write)
{
	stream* File = FileCreate(Path);
	if (File)
	{
		bool_t One = 1;
		File->Set(File,STREAM_SILENT,&One,sizeof(One));
		File->Set(File,STREAM_CREATE,&Write,sizeof(Write));

		if (File->Set(File,STREAM_URL,Path,(tcslen(Path)+1)*sizeof(tchar_t)) != ERR_NONE)
		{
			FileRelease(File);
			File = NULL;
		}
	}
	return File;
}

filepos_t StreamSeek(stream* File, filepos_t Ofs, int Mode)
{
	return File->Seek(File,Ofs,Mode);
}

int StreamRead(stream* File, void* p, int n)
{
	return File->Read(File,p,n);
}

int StreamWrite(stream* File, const void* p, int n)
{
	return File->Write(File,p,n);
}

int StreamClose(stream* File)
{
	File->Set(File,STREAM_URL,NULL,0);
	FileRelease(File);
	return 0;
}
