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
 * $Id: node.h 206 2005-03-19 15:03:03Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#ifndef __CONTEXT_H
#define __CONTEXT_H

#define CONTEXT_VERSION		0x210

typedef	int (*notifyfunc)(void* This,int Param,int Param2);

typedef struct notify
{
	notifyfunc Func;
	void* This;

} notify;

typedef struct context
{
	int Version;
	uint32_t ProgramId;
	const tchar_t* ProgramName;
	const tchar_t* ProgramVersion;
	const tchar_t* CmdLine;
	void* Wnd;
	void* NodeLock;
	array Node; 
	array NodeClass; // ordered by id
	array NodeClassPri; // ordered by priority|id
	array NodeModule;
	int LoadModuleNo;
	void* LoadModule;
	array StrTable[2];
	array StrBuffer;
	array StrModule;
	void* StrLock;
	int Lang;
	int CodePage;
	struct pcm_soft* PCM;
	struct blitpack* Blit;
	struct node* Platform;
	struct node* Advanced;
	struct node* Player;
	notify Error;
	int (*HwOrientation)(void*);
	void *HwOrientationContext;
	bool_t TryDynamic;
	int SettingsPage;
	int StartUpMemory;
	bool_t InHibernate;
	bool_t WaitDisable;
	int FtrId;

} context;

DLL bool_t Context_Init(const tchar_t* Name,const tchar_t* Version,int Id,const tchar_t* CmdLine);
DLL void Context_Done();
DLL void Context_Wnd(void*);
DLL context* Context();

#endif
