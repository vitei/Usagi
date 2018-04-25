/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemHeap.h"
#include "ScratchRaw.h"

namespace usg {

static MemHeap* g_pHeapScratch = nullptr;

ScratchRaw::ScratchRaw() : m_pRawData(nullptr)
{
	
}

void ScratchRaw::InitMemory(memsize uSize)
{
	static MemHeap s_heapScratch;
	g_pHeapScratch = &s_heapScratch;
	g_pHeapScratch->Initialize(uSize);
}

ScratchRaw::ScratchRaw(memsize uSize, memsize uAlign)
{
	m_pRawData = NULL;
	Init(uSize, uAlign);
}

ScratchRaw::~ScratchRaw()
{
	Free();
}

void ScratchRaw::Free()
{
	Free(&m_pRawData);
}

void ScratchRaw::Init(memsize uSize, memsize uAlign)
{
	Init(&m_pRawData, uSize, uAlign);
}

void ScratchRaw::Init(void** pRawData, memsize uSize, memsize uAlign)
{
	Free(pRawData);
	*pRawData = g_pHeapScratch->Allocate(uSize, uAlign, 0, ALLOC_LOADING);
}

void ScratchRaw::Free(void** pRawData)
{
	if(*pRawData)
	{
		g_pHeapScratch->Deallocate(*pRawData);
		*pRawData = NULL;
	}
}

}
   
