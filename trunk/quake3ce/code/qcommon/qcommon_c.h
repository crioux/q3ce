#pragma once 

typedef int qboolean;

#define qfalse (0)
#define qtrue (1)
typedef enum {
	FS_READ,
	FS_WRITE,
	FS_APPEND,
	FS_APPEND_SYNC
} fsMode_t;

typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

typedef int fileHandle_t;
extern int FS_FOpenFileRead( const char *filename, fileHandle_t *file, qboolean uniqueFILE );
#if defined(_DEBUG) && !defined(BSPC)
	#define ZONE_DEBUG
#endif
#ifdef ZONE_DEBUG
#define Z_TagMalloc(size, tag)			Z_TagMallocDebug(size, tag, #size, __FILE__, __LINE__)
#define Z_Malloc(size)					Z_MallocDebug(size, #size, __FILE__, __LINE__)
void *Z_TagMallocDebug( int size, int tag, char *label, char *file, int line );	// NOT 0 filled memory
void *Z_MallocDebug( int size, char *label, char *file, int line );			// returns 0 filled memory
#else
void *Z_TagMalloc( int size, int tag );	// NOT 0 filled memory
void *Z_Malloc( int size );			// returns 0 filled memory
#endif
void Z_Free( void *ptr );
void Z_FreeTags( int tag );
int Z_AvailableMemory( void );
void Z_LogHeap( void );

void Hunk_Clear( void );
void Hunk_ClearToMark( void );
void Hunk_SetMark( void );
qboolean Hunk_CheckMark( void );
void Hunk_ClearTempMemory( void );
void *Hunk_AllocateTempMemory( int size );
void Hunk_FreeTempMemory( void *buf );
int	Hunk_MemoryRemaining( void );
void Hunk_Log( void);

#define FS_GENERAL_REF	0x01
#define FS_UI_REF		0x02
#define FS_CGAME_REF	0x04
#define FS_QAGAME_REF	0x08
// number of id paks that will never be autodownloaded from baseq3
#define NUM_ID_PAKS		9

#define	MAX_FILE_HANDLES	64

#define BASEGAME "baseq3"

 qboolean FS_Initialized();

 void	FS_InitFilesystem (void);
 void	FS_Shutdown( qboolean closemfp );

 qboolean	FS_ConditionalRestart( int checksumFeed );
 void	FS_Restart( int checksumFeed );
// shutdown and restart the filesystem so changes to fs_gamedir can take effect

 char	**FS_ListFiles( const char *directory, const char *extension, int *numfiles );
// directory should not have either a leading or trailing /
// if extension is "/", only subdirectories will be returned
// the returned files will not include any directories or /

 void	FS_FreeFileList( char **list );

 qboolean FS_FileExists( const char *file );

 int		FS_LoadStack();

 int		FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );
 int		FS_GetModList(  char *listbuf, int bufsize );

 fileHandle_t	FS_FOpenFileWrite( const char *qpath );
// will properly create any needed paths and deal with seperater character issues

 int		FS_filelength( fileHandle_t f );
 fileHandle_t FS_SV_FOpenFileWrite( const char *filename );
 int		FS_SV_FOpenFileRead( const char *filename, fileHandle_t *fp );
 void	FS_SV_Rename( const char *from, const char *to );
 int		FS_FOpenFileRead( const char *qpath, fileHandle_t *file, qboolean uniqueFILE );
// if uniqueFILE is true, then a new FILE will be fopened even if the file
// is found in an already open pak file.  If uniqueFILE is false, you must call
// FS_FCloseFile instead of fclose, otherwise the pak FILE would be improperly closed
// It is generally safe to always set uniqueFILE to true, because the majority of
// file IO goes through FS_ReadFile, which Does The Right Thing already.

 int		FS_FileIsInPAK(const char *filename, int *pChecksum );
// returns 1 if a file is in the PAK file, otherwise -1

 int		FS_Write( const void *buffer, int len, fileHandle_t f );

 int		FS_Read2( void *buffer, int len, fileHandle_t f );
 int		FS_Read( void *buffer, int len, fileHandle_t f );
// properly handles partial reads and reads from other dlls

 void	FS_FCloseFile( fileHandle_t f );
// note: you can't just fclose from another DLL, due to MS libc issues

 int		FS_ReadFile( const char *qpath, void **buffer );
// returns the length of the file
// a null buffer will just return the file length without loading
// as a quick check for existance. -1 length == not present
// A 0 byte will always be appended at the end, so string ops are safe.
// the buffer should be considered read-only, because it may be cached
// for other uses.

 void	FS_ForceFlush( fileHandle_t f );
// forces flush on files we're writing to.

 void	FS_FreeFile( void *buffer );
// frees the memory returned by FS_ReadFile

 void	FS_WriteFile( const char *qpath, const void *buffer, int size );
// writes a complete file, creating any subdirectories needed

 int		FS_filelength( fileHandle_t f );
// doesn't work for files that are opened from a pack file

 int		FS_FTell( fileHandle_t f );
// where are we?

// get real file length. works for files opened from a pack file
 int		FS_FGetLength( fileHandle_t f );

 void	FS_Flush( fileHandle_t f );

 void 	 FS_Printf( fileHandle_t f, const char *fmt, ... );
// like fprintf

 int		FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
// opens a file for reading, writing, or appending depending on the value of mode

 int		FS_Seek( fileHandle_t f, long offset, int origin );
// seek on a file (doesn't work for zip files!!!!!!!!)

 qboolean FS_FilenameCompare( const char *s1, const char *s2 );

 const char *FS_GamePureChecksum( void );
// Returns the checksum of the pk3 from which the server loaded the qagame.qvm

 const char *FS_LoadedPakNames( void );
 const char *FS_LoadedPakChecksums( void );
 const char *FS_LoadedPakPureChecksums( void );
// Returns a space separated string containing the checksums of all loaded pk3 files.
// Servers with sv_pure set will get this string and pass it to clients.

 const char *FS_ReferencedPakNames( void );
 const char *FS_ReferencedPakChecksums( void );
 const char *FS_ReferencedPakPureChecksums( void );
// Returns a space separated string containing the checksums of all loaded 
// AND referenced pk3 files. Servers with sv_pure set will get this string 
// back from clients for pure validation 

 void FS_ClearPakReferences( int flags );
// clears referenced booleans on loaded pk3s

 void FS_PureServerSetReferencedPaks( const char *pakSums, const char *pakNames );
 void FS_PureServerSetLoadedPaks( const char *pakSums, const char *pakNames );
// If the string is empty, all data sources will be allowed.
// If not empty, only pk3 files that match one of the space
// separated checksums will be checked for files, with the
// sole exception of .cfg files.

 qboolean FS_idPak( char *pak, char *base );
 qboolean FS_ComparePaks( char *neededpaks, int len, qboolean dlstring );

 void FS_Rename( const char *from, const char *to );
