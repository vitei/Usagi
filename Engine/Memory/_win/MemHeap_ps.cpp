/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemHeap.h"
#include <malloc.h>

namespace usg{

// FIXME: Simply place holder that does nothing more than wrap around malloc, most sdks come with their own memory management code
// and in windows case it's just more efficient to use windows internal memory management
MemHeap_ps::MemHeap_ps()
{
	m_uSize = 0;
	m_uAllocated = 0;
}

MemHeap_ps::~MemHeap_ps()
{

}

void MemHeap_ps::Init(memsize uSize)
{
	m_uSize = uSize;
}

void MemHeap_ps::Init(void*, memsize uSize)
{
	m_uSize = uSize;
	DEBUG_PRINT("Memheap_ps::Init Windows does not support memheaps based on raw memory, wasting memory by attempting to pass one in");
}

void* MemHeap_ps::Alloc(memsize uSize, memsize uAlign, bool bGPUUse)
{
	void* pData = _aligned_malloc(uSize, uAlign);
	m_uAllocated += _aligned_msize(pData, uAlign, 0);
	return pData;
}

void  MemHeap_ps::Free(void* pFree, memsize uAlign, bool bGPUUse)
{
	ASSERT(pFree!=NULL);
	m_uAllocated -= _aligned_msize(pFree, uAlign, 0);
	_aligned_free(pFree);
}


}