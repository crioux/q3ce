#ifndef __INC_XMALLOC_H
#define __INC_XMALLOC_H

#define HEAP_GRANULARITY (65536)
#define LOW_WATER_MARK (9*1024*1024)

class CXMalloc
{
private:

	char m_pMapName[64];
	char m_pMapFileName[512];

	char *m_heap;
	int m_heaplen;
	int *m_heapmap;
	int m_heapmaplen;
	int m_allocnum;
	
	enum MEMTYPE
	{
		NONE,
		MALLOC,
	//	VIRTUAL,
	//	HIGHVIRTUAL,
	//	PFSWAP,
	//	MAINSWAP,
	//	FLASHSWAP,
	//	FILESWAP,
		MEMORY
	};

	MEMTYPE m_memtype;

public:
	
	CXMalloc(const char *svMapName);
	~CXMalloc();
	
	bool InitMalloc(size_t len);
/*
	bool InitVirtual(size_t len);
	bool InitHighVirtual(size_t len);
	bool InitPFSwap(size_t len);
	bool InitMainSwap(size_t len);
	bool InitFlashSwap(size_t len);
	bool InitFileSwap(const TCHAR *fname, size_t len);
*/
	bool InitMemory(void *mem, size_t len);
	bool Terminate(void);

	size_t GetTotalSize(void);

public:
	void *xmalloc(size_t size);
	void xfree(void *blk, size_t len);
};


#endif

