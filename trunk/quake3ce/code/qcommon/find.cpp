#include"common_pch.h"


int __cdecl _findfirst(
         const char * szWild,
        struct _finddata_t * pfd
        )

{
    WIN32_FIND_DATA wfd;
    HANDLE          hFile;
    DWORD           err;
	TCHAR buf[2048];

	_snwprintf(buf,2048,L"%S",szWild);

    if ((hFile = FindFirstFile(buf, &wfd)) == INVALID_HANDLE_VALUE) {
        err = GetLastError();
        switch (err) {
            case ERROR_NO_MORE_FILES:
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                break;

            case ERROR_NOT_ENOUGH_MEMORY:
                break;

            default:
                break;
        }
        return (-1);
    }

    pfd->attrib       = (wfd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
                      ? 0 : wfd.dwFileAttributes;
    pfd->time_create  = 0;
    pfd->time_access  = 0;
    pfd->time_write   = 0;

    pfd->size         = wfd.nFileSizeLow;

    sprintf(pfd->name, "%S", wfd.cFileName);

    return ((int)hFile);
}


int __cdecl _findnext(int hFile, struct _finddata_t * pfd)
{
    WIN32_FIND_DATA wfd;
    DWORD           err;

    if (!FindNextFile((HANDLE)hFile, &wfd)) {
        err = GetLastError();
        switch (err) {
            case ERROR_NO_MORE_FILES:
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                break;

            case ERROR_NOT_ENOUGH_MEMORY:
                break;

            default:
                break;
        }
        return (-1);
    }

    pfd->attrib       = (wfd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
                      ? 0 : wfd.dwFileAttributes;
    pfd->time_create  = 0;
    pfd->time_access  = 0;
    pfd->time_write   = 0;

    pfd->size         = wfd.nFileSizeLow;

    sprintf(pfd->name,"%S", wfd.cFileName);

    return (0);
}


int __cdecl _findclose(int hFile)
{
    if (!FindClose((HANDLE)hFile)) {
        return (-1);
    }
    return (0);
}

