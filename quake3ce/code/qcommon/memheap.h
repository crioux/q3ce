#ifndef __INC_MEMHEAP_H
#define __INC_MEMHEAP_H

#include"unixdefs.h"

class CMemHeap
{

private:
	void *m_pBuffer;
	size_t m_nBufferLength; 
	void *m_pSpace;

public:
	CMemHeap();
	CMemHeap(void *mem, size_t len);
	~CMemHeap();
	bool Create(void *mem, size_t len);
	bool Destroy(void);

public:
	void *Alloc(size_t len, bool bFill=false, unsigned char fillvalue=0);
	void Free(void *mem);
	void *Realloc(void *mem, size_t newlen);
	size_t GetSize(void *mem);

public:
	size_t GetTotalSize(void);
	size_t GetUsedSize(void);
	size_t GetFreeSize(void);
	
	void Dump(void);
};

#endif

