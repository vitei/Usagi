/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemHeap.h"
//#include <malloc.h>

// FIXME: Simply place holder code to match the WiiU implementation for now,
// does nothing more than wrap around malloc
MemHeap_ps::MemHeap_ps()
{

}

MemHeap_ps::~MemHeap_ps()
{

}

void MemHeap_ps::Init(uint32 uSize)
{

}


void MemHeap_ps::Init(void*, uint32)
{

}

void* MemHeap_ps::Alloc(uint32 uSize, uint32 uAlign, bool bGPUUse)
{
	return malloc(uSize);
}

void  MemHeap_ps::Free(void* pFree, bool bGPUUse)
{
	ASSERT(pFree!=NULL);
	free(pFree);
}
