#ifndef __INC_DLL_LOAD_H
#define __INC_DLL_LOAD_H

#include<windows.h>

#define NTSIGNATURE(ptr) ((LPVOID)((BYTE *)(ptr) + ((PIMAGE_DOS_HEADER)(ptr))->e_lfanew))
#define SIZE_OF_NT_SIGNATURE (sizeof(DWORD))
#define PEFHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE))
#define OPTHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE+sizeof(IMAGE_FILE_HEADER)))
#define SECHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE+sizeof(IMAGE_FILE_HEADER)+sizeof(IMAGE_OPTIONAL_HEADER)))
#define RVATOVA(base,offset) ((LPVOID)((DWORD)(base)+(DWORD)(offset)))
#define VATORVA(base,offset) ((LPVOID)((DWORD)(offset)-(DWORD)(base)))

#define SIZE_OF_PARAMETER_BLOCK 4096
#define IMAGE_PARAMETER_MAGIC 0xCDC31337
#define MAX_DLL_PROCESSES 256
#define DLL_ATTACH 0
#define DLL_DETACH 1

// NEW FLAGS
#define REBIND_IMAGE_IMPORTS 0x00000100
#define RWX_PERMISSIONS 0x00000200
#define FORCE_LOAD_NEW_IMAGE 0x00000400

// Exported function types
typedef HMODULE TYPEOF_GetDLLHandle(const char *svName);
typedef DWORD TYPEOF_GetDLLFileName(HMODULE hModule, LPTSTR lpFileName, DWORD nSize);
typedef FARPROC TYPEOF_GetDLLProcAddress(HMODULE hModule, LPCSTR lpProcName);
typedef FARPROC TYPEOF_SetDLLProcAddress(HMODULE hModule, LPCSTR lpProcName, FARPROC fpAddr);
typedef BOOL TYPEOF_ResetDLLProcAddress(HMODULE hModule, LPCSTR lpProcName);
typedef HMODULE TYPEOF_LoadDLLFromImage(void *pDLLFileImage, const char *svMappingName, DWORD dwFlags);
typedef HMODULE TYPEOF_LoadDLLEx(LPCTSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef HMODULE TYPEOF_LoadDLL(LPCTSTR lpLibFileName);
typedef BOOL TYPEOF_FreeDLL(HMODULE hLibModule);

// Internal functions
BOOL MapDLLFromImage(void *pDLLFileImage, void *pMemoryImage);
BOOL PrepareDLLImage(void *pMemoryImage, DWORD dwImageSize, BOOL bResolve, BOOL bRebind);

// Primary functions
void InitializeDLLLoad(void);
void KillDLLLoad(void);

HMODULE GetDLLHandle(const char *svName);
DWORD GetDLLFileName(HMODULE hModule, LPTSTR lpFileName, DWORD nSize);
FARPROC GetDLLProcAddress(HMODULE hModule, LPCTSTR lpProcName);
FARPROC SetDLLProcAddress(HMODULE hModule, LPCTSTR lpProcName, FARPROC fpAddr);
BOOL ResetDLLProcAddress(HMODULE hModule, LPCTSTR lpProcName);
HMODULE LoadDLLFromImage(void *pDLLFileImage, const TCHAR *svMappingName, DWORD dwFlags);
HMODULE LoadDLLEx(LPCTSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE LoadDLL(LPCTSTR lpLibFileName);
BOOL FreeDLL(HMODULE hLibModule);

#endif