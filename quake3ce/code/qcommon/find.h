#ifndef __INC_FIND_H
#define __INC_FIND_H



typedef unsigned long _fsize_t; 

#define _A_NORMAL       0x00    /* Normal file - No read/write restrictions */
#define _A_RDONLY       0x01    /* Read only file */
#define _A_HIDDEN       0x02    /* Hidden file */
#define _A_SYSTEM       0x04    /* System file */
#define _A_SUBDIR       0x10    /* Subdirectory */
#define _A_ARCH         0x20    /* Archive file */


struct _finddata_t {
        unsigned    attrib;
        time_t      time_create;
        time_t      time_access;
        time_t      time_write;
        _fsize_t    size;
        char        name[260];
};



int __cdecl _findfirst(
         const char * szWild,
        struct _finddata_t * pfd
        );
int __cdecl _findclose(int hFile);

int __cdecl _findnext(int hFile, struct _finddata_t * pfd);

#endif