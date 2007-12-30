#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <projects.h>
#include "xmalloc.h"


typedef BOOL  (__stdcall *GetSystemMemoryDivisionProc)(LPDWORD,LPDWORD,LPDWORD);
typedef DWORD (__stdcall *SetSystemMemoryDivisionProc)(DWORD);

bool HasMemoryDivision(void)
{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize=sizeof(osvi);
	GetVersionEx(&osvi);
	if(osvi.dwMajorVersion<5)
	{
		return true;
	}
	return false;
}

void ReadMemoryDivision(DWORD *pdwStorage, DWORD *pdwRam)
{
	HINSTANCE hCoreDll = LoadLibrary(_T("coredll.dll"));
	GetSystemMemoryDivisionProc procGet = (GetSystemMemoryDivisionProc)GetProcAddress(hCoreDll, _T("GetSystemMemoryDivision"));
	
	DWORD dwStoragePages;
	DWORD dwRamPages;
	DWORD dwPageSize;
	BOOL bResult = procGet(&dwStoragePages, &dwRamPages, &dwPageSize);
	if(!bResult)
	{
		::FreeLibrary(hCoreDll);
		return;
	}

	if(pdwStorage)
		*pdwStorage = dwStoragePages*dwPageSize;
	if(pdwRam)
		*pdwRam = dwRamPages*dwPageSize;
	
	::FreeLibrary(hCoreDll);
}

void WriteMemoryDivision(DWORD dwStorage)
{
	HINSTANCE hCoreDll = LoadLibrary(_T("coredll.dll"));
	SetSystemMemoryDivisionProc procSet = (SetSystemMemoryDivisionProc)GetProcAddress(hCoreDll, _T("SetSystemMemoryDivision"));
	GetSystemMemoryDivisionProc procGet = (GetSystemMemoryDivisionProc)GetProcAddress(hCoreDll, _T("GetSystemMemoryDivision"));
	
	DWORD dwStoragePages;
	DWORD dwRamPages;
	DWORD dwPageSize;
	BOOL bResult = procGet(&dwStoragePages, &dwRamPages, &dwPageSize);
	if(!bResult)
	{
		::FreeLibrary(hCoreDll);
		return;
	}

	procSet((dwStorage+(dwPageSize-1))/dwPageSize);
	
	::FreeLibrary(hCoreDll);
}

DWORD GetFreeProgramMemory(void)
{
	MEMORYSTATUS   memInfo;

    // Program memory.
    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatus(&memInfo);

	return memInfo.dwAvailPhys;
}

DWORD GetFreeStorageMemory(void)
{
	STORE_INFORMATION   si;

	// Storage memory.
    GetStoreInformation(&si);

	return si.dwFreeSize;
}

bool TryToGetProgramMemory(DWORD len)
{
	DWORD dwFreeProg=GetFreeProgramMemory();
	DWORD dwFreeStor=GetFreeStorageMemory();
	DWORD dwFreeTotal=dwFreeStor+dwFreeProg;

	if(!HasMemoryDivision())
	{
		return dwFreeProg>=(len+LOW_WATER_MARK);
	}

	if(dwFreeTotal<(len+LOW_WATER_MARK))
	{
		return false;
	}

	if(dwFreeProg>(len+(LOW_WATER_MARK/2)))
	{
		return true;
	}

	// Get memory division
	DWORD dwDivStorage,dwDivRam;
	ReadMemoryDivision(&dwDivStorage, &dwDivRam);

	// Figure out how much more memory we really need
	DWORD dwMoreNeeded=((len+(LOW_WATER_MARK/2)-dwFreeProg+65535)/65536)*65536;

	// See if we can get that from storage memory
	if(dwFreeStor<dwMoreNeeded)
	{
		return false;
	}

	// Shovel memory
	WriteMemoryDivision(dwDivStorage-dwMoreNeeded);

	return true;
}

void CenterMemoryDivision(void)
{
	if(!HasMemoryDivision())
	{
		return;
	}

	DWORD dwFreeProg=GetFreeProgramMemory();
	DWORD dwFreeStor=GetFreeStorageMemory();
	DWORD dwFreeTotal=dwFreeStor+dwFreeProg;

	DWORD dwHalfTotal=dwFreeTotal/2;


	// Get memory division
	DWORD dwDivStorage,dwDivRam;
	ReadMemoryDivision(&dwDivStorage, &dwDivRam);

	// Shovel memory
	if(dwFreeStor<dwHalfTotal)
	{
		WriteMemoryDivision(dwDivStorage+(dwHalfTotal-dwFreeStor));
	}
	else 
	{
		WriteMemoryDivision(dwDivStorage-(dwFreeStor-dwHalfTotal));
	}
}
  

bool CommitPages(void *mem, size_t len, int prot)
{
	char *cmem=(char *)mem;

	for(size_t i=0;i<len;i+=65536)
	{
		int slen=len-i;
		if(slen>65536)
		{
			slen=65536;
		}
		void *tmem=VirtualAlloc(cmem+i,slen,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
		if(tmem==NULL)
		{
			for(size_t j=0;j<i;j+=65536)
			{
				
			}
			return false;
		}
	}

	return true;
}




CXMalloc::CXMalloc(const TCHAR *svMapName)
{
	m_heap=NULL;
	m_heaplen=0;
	m_heapmap=NULL;
	m_heapmaplen=0;
	m_allocnum=0;
	m_memtype=NONE;

	_snwprintf(m_pMapName,64,L"%s",svMapName);
}

CXMalloc::~CXMalloc()
{
	Terminate();
}

bool CXMalloc::InitMalloc(size_t len)
{
	if(m_memtype!=NONE)
	{
		return false;
	}

	if(!TryToGetProgramMemory(len))
	{
		return false;
	}
	
	void *mem=malloc(len);
	if(mem==NULL)
	{
		return false;
	}
	if(!InitMemory(mem,len))
	{
		free(mem);
		return false;
	}
	m_memtype=MALLOC;
	return true;
}

bool CXMalloc::InitVirtual(size_t len)
{
	if(m_memtype!=NONE)
	{
		return false;
	}

	if(!TryToGetProgramMemory(len))
	{
		return false;
	}

	void *mem=VirtualAlloc(NULL,len,MEM_RESERVE,PAGE_EXECUTE_READWRITE);
	if(mem==NULL)
	{
		return false;
	}

	if(!CommitPages(mem,len,PAGE_EXECUTE_READWRITE))
	{
		VirtualFree(mem,0,MEM_RELEASE);
		return false;
	}

	if(!InitMemory(mem,len))
	{
		VirtualFree(mem,len,MEM_DECOMMIT);
		VirtualFree(mem,0,MEM_RELEASE);
		return false;
	}
	m_memtype=VIRTUAL;
	return true;
}

bool CXMalloc::InitHighVirtual(size_t len)
{
	if(m_memtype!=NONE)
	{
		return false;
	}

	if(!TryToGetProgramMemory(len))
	{
		return false;
	}
	
	void *mem=VirtualAlloc(NULL,len,MEM_RESERVE,PAGE_NOACCESS);
	if(mem==NULL)
	{
		return false;
	}

	if(!CommitPages(mem,len,PAGE_EXECUTE_READWRITE))
	{
		VirtualFree(mem,0,MEM_RELEASE);
		return false;
	}

	if(!InitMemory(mem,len))
	{
		VirtualFree(mem,len,MEM_DECOMMIT);
		VirtualFree(mem,0,MEM_RELEASE);
		return false;
	}
	m_memtype=HIGHVIRTUAL;
	return true;
}

bool CXMalloc::InitPFSwap(size_t len)
{
	if(m_memtype!=NONE)
	{
		return false;
	}

	if(!TryToGetProgramMemory(len))
	{
		return false;
	}

	HANDLE hfm=CreateFileMapping((INVALID_HANDLE_VALUE),NULL,PAGE_READWRITE|SEC_COMMIT,0,len,NULL);
	if(hfm==NULL)
	{
		return false;
	}

	LPVOID mem=MapViewOfFile(hfm,FILE_MAP_ALL_ACCESS,0,0,len);
	CloseHandle(hfm);
	if(mem==NULL)
	{
		return false;
	}
	
	if(!InitMemory(mem,len))
	{
		UnmapViewOfFile(mem);
		return false;
	}

	m_memtype=MAINSWAP;
	return true;
}


bool CXMalloc::InitMainSwap(size_t len)
{
	if(m_memtype!=NONE)
	{
		return false;
	}

	if(GetFreeStorageMemory()-len<LOW_WATER_MARK)
	{
		return false;
	}

	_snwprintf(m_pMapFileName,256,L"\\Temp\\%s.swp",m_pMapName);

	DWORD dwMode=FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_RANDOM_ACCESS;
	
	HANDLE hf=CreateFileForMapping(m_pMapFileName,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,dwMode,NULL);
	if(hf==NULL)
	{
		return false;
	}

	HANDLE hfm=CreateFileMapping(hf,NULL,PAGE_READWRITE|SEC_COMMIT,0,len,NULL);
	if(hfm==NULL)
	{
		CloseHandle(hf);
		DeleteFile(m_pMapFileName);
		return false;
	}

	LPVOID mem=MapViewOfFile(hfm,FILE_MAP_ALL_ACCESS,0,0,len);
	CloseHandle(hfm);
	CloseHandle(hf);
	if(mem==NULL)
	{
		DeleteFile(m_pMapFileName);
		return false;
	}
	
	if(!InitMemory(mem,len))
	{
		UnmapViewOfFile(mem);
		DeleteFile(m_pMapFileName);
		return false;
	}

	m_memtype=MAINSWAP;
	return true;
}

bool CXMalloc::InitFlashSwap(size_t len)
{
	if(m_memtype!=NONE)
	{
		return false;
	}
	
	LPVOID mem=NULL;

	WIN32_FIND_DATA wfd;
	HANDLE hfc=FindFirstFlashCard(&wfd);
	int c=0;
	if(hfc) 
	{
		do
		{
			_snwprintf(m_pMapFileName,256,L"\\%s\\%s.swp",wfd.cFileName,m_pMapName);
				
			DWORD dwMode=FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_RANDOM_ACCESS;
		
			HANDLE hf=CreateFileForMapping(m_pMapFileName,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,dwMode,NULL);
			if(hf==NULL)
			{
				continue;
			}

			HANDLE hfm=CreateFileMapping(hf,NULL,PAGE_READWRITE|SEC_COMMIT,0,len,NULL);
			if(hfm==NULL)
			{
				CloseHandle(hf);
				DeleteFile(m_pMapFileName);
				continue;
			}

			mem=MapViewOfFile(hfm,FILE_MAP_ALL_ACCESS,0,0,len);
			CloseHandle(hfm);
			CloseHandle(hf);
			if(mem==NULL)
			{
				DeleteFile(m_pMapFileName);
				continue;
			}
			
			break;
		}
		while(FindNextFlashCard(hfc,&wfd));
	}

	if(mem==NULL)
	{
		return false;
	}
	
	if(!InitMemory(mem,len))
	{
		UnmapViewOfFile(mem);
		return false;
	}	

	m_memtype=FLASHSWAP;
	return true;
}

bool CXMalloc::InitFileSwap(const TCHAR *fname, size_t len)
{
	if(m_memtype!=NONE)
	{
		return false;
	}
	
	_snwprintf(m_pMapFileName,256,L"%s",fname);
					
	DWORD dwMode=FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_RANDOM_ACCESS;


	HANDLE hf=CreateFileForMapping(m_pMapFileName,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,dwMode,NULL);
	if(hf==NULL)
	{
		return false;
	}

	HANDLE hfm=CreateFileMapping(hf,NULL,PAGE_READWRITE|SEC_COMMIT,0,len,NULL);
	if(hfm==NULL)
	{
		CloseHandle(hf);
		DeleteFile(m_pMapFileName);
		return false;
	}

	LPVOID mem=MapViewOfFile(hfm,FILE_MAP_ALL_ACCESS,0,0,len);
	CloseHandle(hfm);
	CloseHandle(hf);
	if(mem==NULL)
	{
		DeleteFile(m_pMapFileName);
		return false;
	}
	
	if(!InitMemory(mem,len))
	{
		UnmapViewOfFile(mem);
		DeleteFile(m_pMapFileName);
		return false;
	}	

	m_memtype=FILESWAP;
	return true;
}

bool CXMalloc::InitMemory(void *memory, size_t len)
{
	if(m_memtype!=NONE)
	{
		return false;
	}
	
	size_t realsize=len/HEAP_GRANULARITY * HEAP_GRANULARITY ;

	m_heap=(char *)memory;
	m_heaplen=realsize;
	
	m_heapmaplen=(realsize/HEAP_GRANULARITY );
	m_heapmap=(int *)malloc(m_heapmaplen*sizeof(int));
	if(m_heapmap==NULL)
	{
		return false;
	}
	memset(m_heapmap,0,m_heapmaplen*sizeof(int));
	memset(memory,0,len);

	m_memtype=MEMORY;

	return true;
}

bool CXMalloc::Terminate(void)
{
	switch(m_memtype)
	{
	case NONE:
		return false;
	case MEMORY:
		break;
	case MALLOC:
		free(m_heap);
		break;
	case VIRTUAL:
	case HIGHVIRTUAL:
		VirtualFree(m_heap,m_heaplen,MEM_DECOMMIT);
		VirtualFree(m_heap,0,MEM_RELEASE);
		break;
	case PFSWAP:
		UnmapViewOfFile(m_heap);
		break;		
	case MAINSWAP:
	case FLASHSWAP:
	case FILESWAP:		
		UnmapViewOfFile(m_heap);
		DeleteFile(m_pMapFileName);
		break;
	default:
		ASSERT(0);
		break;
	}

	m_memtype=NONE;
	return true;
}


void *CXMalloc::xmalloc(size_t size)
{
	size_t realsize=(size+(HEAP_GRANULARITY-1) )/HEAP_GRANULARITY *HEAP_GRANULARITY ;
	size_t blocks=realsize/HEAP_GRANULARITY;

	int i,j=-1;
	for(i=0;i<m_heapmaplen;i++)
	{
		if(m_heapmap[i]==0)
		{
			for(j=i+1;j<m_heapmaplen;j++)
			{
				if(m_heapmap[j]!=0)
				{
					break;
				}
				if((j-i)==blocks)
				{
					break;
				}
			}
			j--;
			if(((j-i)+1)==blocks)
			{
				break;
			}
		}
	}
	if(i==m_heapmaplen || j==-1)
	{
		ASSERT(0);
		return NULL;
	}
	
	m_allocnum++;

	int x;
	for(x=i;x<=j;x++)
	{
		m_heapmap[x]=m_allocnum;
	}

	return m_heap+(i*HEAP_GRANULARITY);
}

void CXMalloc::xfree(void *blk, size_t size)
{
	size_t realsize=(size+(HEAP_GRANULARITY-1) )/HEAP_GRANULARITY *HEAP_GRANULARITY ;
	size_t blocks=realsize/HEAP_GRANULARITY;

	int i=(((char *)blk)-m_heap)/HEAP_GRANULARITY;
	int j=i+blocks-1;
	
	int allocnum=m_heapmap[i];
	if(allocnum==0)
	{
		ASSERT(0);
		return;
	}

	int x;
	for(x=i;x<=j;x++)
	{
		if(m_heapmap[x]!=allocnum)
		{
			ASSERT(0);
			return;
		}
	}

	for(x=i;x<=j;x++)
	{
		m_heapmap[x]=0;
	}

	memset(blk,realsize,0);
}

size_t CXMalloc::GetTotalSize(void)
{
	return m_heaplen;
}

