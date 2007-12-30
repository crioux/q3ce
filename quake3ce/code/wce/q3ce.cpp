// q3ce.cpp : Defines the entry point for the application.
//
#include<windows.h>
#include"gx.h"
#include"vidplay.h"
#include"dll_load.h"
#include"xmalloc.h"
#include"../wce/res/resource.h"

HINSTANCE g_hInstance;

bool g_use_dllload=false;

bool g_no_cinematics=false;


#ifdef _DEBUG
static DWORD s_DLL_SIZE=(5*1024*1024);
#else 
static DWORD s_DLL_SIZE=((int)(4.5*1024*1024));
#endif
static DWORD s_SND_SIZE=((int)(1.5*1024*1024));
static DWORD s_ZONE_SIZE=0;
static DWORD s_HUNK_SIZE=0;

static DWORD s_dwOldDivision=0;

static bool s_bVidPlay=false;

CXMalloc g_mem_dll(L"q3ce_dll");
CXMalloc g_mem_snd(L"q3ce_snd");
CXMalloc g_mem_zone(L"q3ce_zone");
CXMalloc g_mem_hunk(L"q3ce_hunk");

CXMalloc g_mem_all(L"q3ce_mem");

void PrepareGame(void);
void DoGameLoop(void);


BOOL CALLBACK DialogProc(
  HWND hwndDlg, 
  UINT uMsg, 
  WPARAM wParam, 
  LPARAM lParam
)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			*((volatile HWND *)lParam)=hwndDlg;
		}
		return TRUE;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					/// handle OK button click
					EndDialog(hwndDlg, IDOK);
				}
				return TRUE;

			case IDCANCEL:
				{
					/// handle Cancel button click
					EndDialog(hwndDlg, IDCANCEL);
				}
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}


DWORD MemDlgThread(LPVOID lpParameter)
{
	DialogBoxParam(g_hInstance,MAKEINTRESOURCE(ID_MEM_DLG),GetDesktopWindow(),DialogProc,(LPARAM)lpParameter);
	return 0;
}

DWORD MemDlg2Thread(LPVOID lpParameter)
{
	DialogBoxParam(g_hInstance,MAKEINTRESOURCE(ID_MEM_DLG2),GetDesktopWindow(),DialogProc,(LPARAM)lpParameter);
	return 0;
}

HWND PopMemDlg(void)
{
	volatile HWND foo=NULL;
	DWORD dwtid;
	HANDLE ht=CreateThread(NULL,0,MemDlgThread,(LPVOID)&foo,0,&dwtid);
	if(ht==NULL)
	{
		return NULL;
	}
	CloseHandle(ht);
	while(foo==NULL)
	{
		Sleep(0);
	}
	return foo;
}

HWND PopMemDlg2(void)
{
	volatile HWND foo=NULL;
	DWORD dwtid;
	HANDLE ht=CreateThread(NULL,0,MemDlg2Thread,(LPVOID)&foo,0,&dwtid);
	if(ht==NULL)
	{
		return NULL;
	}
	CloseHandle(ht);
	while(foo==NULL)
	{
		Sleep(0);
	}
	return foo;
}



bool Q3CE_InitMemory(void)
{
	// Figure out how much memory we can allocate to lg and hunk
	DWORD dwBaseReserve,dwAvailable;
	
	if(g_use_dllload)
	{
		dwBaseReserve=s_DLL_SIZE+s_SND_SIZE+65536;
	}
	else
	{
		dwBaseReserve=s_SND_SIZE+65536;
	}

	if(HasMemoryDivision())
	{	
		dwAvailable=(GetFreeProgramMemory()+GetFreeStorageMemory()-LOW_WATER_MARK-65536);	
	
		// Get old memory division
		ReadMemoryDivision(&s_dwOldDivision,NULL);
	}
	else
	{
		dwAvailable=(GetFreeProgramMemory()-LOW_WATER_MARK-65536);	
	}
	
	if(dwAvailable<(dwBaseReserve+(8*1024*1024)+(16*1024*1024)))
	{
		return false;
	}
	
	s_ZONE_SIZE=((int)((dwAvailable-dwBaseReserve)/3))/65536*65536;
	s_HUNK_SIZE=(dwAvailable-dwBaseReserve-s_ZONE_SIZE)/65536*65536;


	if(g_use_dllload && !g_mem_dll.InitHighVirtual(s_DLL_SIZE))
		return false;
	
	if(!g_mem_hunk.InitHighVirtual(s_HUNK_SIZE))
		return false;

	if(!g_mem_zone.InitHighVirtual(s_ZONE_SIZE))
		return false;
	
	if(!g_mem_snd.InitHighVirtual(s_SND_SIZE))
		return false;

	CenterMemoryDivision();

	return true;
}

bool Q3CE_TerminateMemory(void)
{
	HWND memdlg=PopMemDlg2();

	if(g_use_dllload)
	{

		g_mem_dll.Terminate();
	}

	g_mem_snd.Terminate();
	g_mem_zone.Terminate();
	g_mem_hunk.Terminate();

	// Restore old memory division
	if(s_dwOldDivision!=0)
	{
		WriteMemoryDivision(s_dwOldDivision);
	}

	EndDialog(memdlg,0);

	return true;
}


int Q3CE_OpenInput()
{
	return GXOpenInput();
}
	
int Q3CE_CloseInput()
{
	return GXCloseInput();
}

int Q3CE_OpenDisplay(HWND hWnd)
{
	return GXOpenDisplay(hWnd,GX_FULLSCREEN);
}
	
int Q3CE_CloseDisplay()
{
	return GXCloseDisplay();
}

void Q3CE_CloseAll()
{
	if(!g_no_cinematics)
	{
		if(s_bVidPlay)
		{
			VidPlay_Kill();
		}
	}

	GXCloseInput();
	GXCloseDisplay();
	Q3CE_TerminateMemory();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{

	g_hInstance=hInstance;

	HWND memdlg=PopMemDlg();
	
	// Preload DLLs to ensure all the code stays in memory
	HMODULE uilib=NULL,cglib=NULL,glib=NULL;

	if(!g_use_dllload)
	{
		TCHAR wcwd[256];
		TCHAR wdll[512];
		
		GetModuleFileName(NULL,wcwd,256);
		wchar_t *lastbs=wcsrchr(wcwd,L'\\');
		if(lastbs)
		{
			*lastbs='\0';
		}
		
		_snwprintf(wdll,512,L"%s\\baseq3\\uiarm.dll",wcwd);
		HMODULE uilib=LoadLibrary(wdll);
		_snwprintf(wdll,512,L"%s\\baseq3\\cgamearm.dll",wcwd);
		HMODULE cglib=LoadLibrary(wdll);
		_snwprintf(wdll,512,L"%s\\baseq3\\qagamearm.dll",wcwd);
		HMODULE glib=LoadLibrary(wdll);

		// If anything couldn't load using loadlibrary, switch to custom loader
		if(uilib==NULL || cglib==NULL || glib==NULL)
		{
			if(uilib)
			{
				FreeLibrary(uilib);
				uilib=NULL;
			}
			if(cglib)
			{
				FreeLibrary(cglib);
				cglib=NULL;
			}
			if(glib)
			{
				FreeLibrary(glib);
				glib=NULL;
			}
			g_use_dllload=true;
		}
	}

	if(!Q3CE_InitMemory())
	{
		EndDialog(memdlg,0);
		MessageBox(NULL,L"Insufficient memory to start Q3CE.\nTry freeing up more program or storage memory.\n",L"Allocation Failure",MB_OK|MB_ICONSTOP);
		return 0;
	}

	if(g_use_dllload)
	{
		InitializeDLLLoad();
	}

	if(!g_no_cinematics)
	{
		if(!VidPlay_Init())
		{
			s_bVidPlay=false;
		}
		else 
		{
			s_bVidPlay=true;
		}
	}

	EndDialog(memdlg,0);

#ifndef _DEBUG
	if(!g_use_dllload)
	{	
		// Free up library slots and proceed with normal execution
		if(uilib)
		{
			FreeLibrary(uilib);
			uilib=NULL;
		}
		if(cglib)
		{
			FreeLibrary(cglib);
			cglib=NULL;
		}
		if(glib)
		{
			FreeLibrary(glib);
			glib=NULL;
		}
	}
#endif

	PrepareGame();

	while(1)
	{
		DoGameLoop();	
	}

	return 0;
}

