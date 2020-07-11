/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemUtil.h"
#include "Engine/Memory/DoubleStack.h"

namespace usg{

DoubleStack::DoubleStack()
{
	m_pLastTag[0]	= NULL;
	m_pLastTag[1]	= NULL;

	m_pTop[0]		= NULL;
	m_pTop[1]		= NULL;

	m_bFrozen = false;

#ifndef FINAL_BUILD
	MemSet(&m_allocInfo[0], 0, sizeof(m_allocInfo[0]));
	MemSet(&m_allocInfo[1], 0, sizeof(m_allocInfo[1]));
#endif
}

void DoubleStack::Init(uint32 uDataSize)
{
	m_pRawBase = (uint8*)m_memBlock.Init(uDataSize);
	m_pRawEnd = m_pRawBase + uDataSize;

	m_pTop[0]	= m_pRawBase;
	m_pTop[1]	= m_pTop[0] + uDataSize;

	m_pLastTag[0]	= (Tag*)AllocFront(ALLOC_OBJECT, sizeof(Tag));
	m_pLastTag[1]	= (Tag*)AllocBack(ALLOC_OBJECT, sizeof(Tag));

	m_pLastTag[0]->pPrev = NULL;
	m_pLastTag[1]->pPrev = NULL;
}

DoubleStack::~DoubleStack()
{
}


void* DoubleStack::AllocFront(MemAllocType eType, memsize uSize, memsize uAlign, bool bGPU)
{
	if(m_bFrozen)
	{
		ASSERT(false);
		return NULL;
	}

	// TODO: Add header in debug mode
	memsize uAlignedSize = uSize + uAlign;
	memsize uAddress = (memsize)m_pTop[0];
	uint8* pEnd = m_pTop[0] + uAlignedSize;

	if(pEnd > m_pTop[1])
	{
		ASSERT(false);	// We've clashed with the other end of the stack
		return NULL;
	}

	memsize uAlignedAddress = AlignAddress(uAddress, uAlign);
	
	m_pTop[0] = pEnd;

	if(bGPU)
	{
		m_memBlock.NotifyGPUAlloc((void*)uAlignedAddress, uSize, uAlign);
	}

#ifndef FINAL_BUILD
	m_allocInfo[0].uAllocSizes[eType] += (uint32)uAlignedSize;
#endif

	return (void*)uAlignedAddress;
}

void* DoubleStack::AllocBack(MemAllocType eType, memsize uSize, memsize uAlign, bool bGPU)
{
	if(m_bFrozen)
	{
		ASSERT(false);
		return NULL;
	}

	// TODO: Add header in debug mode
	memsize uAlignedSize = uSize + uAlign;
	uint8* pEnd = m_pTop[1] - uAlignedSize;
	memsize uAddress = (memsize)pEnd;

	if(pEnd < m_pTop[0])
	{
		ASSERT(false);	// We've clashed with the other end of the stack
		return NULL;
	}

	memsize uAlignedAddress = AlignAddress(uAddress, uAlign);
	
	m_pTop[1] = pEnd;

#ifndef FINAL_BUILD
	m_allocInfo[1].uAllocSizes[eType] += uAlignedSize;
#endif

	return (void*)uAlignedAddress;
}

void DoubleStack::AddTag(bool bFront)
{
	uint32 uEndId = bFront ? 0 : 1;
	Tag*	pPrevTag = m_pLastTag[uEndId];
	
	if(bFront)
	{
		m_pLastTag[uEndId] = (Tag*)AllocFront(ALLOC_OBJECT, sizeof(Tag));
	}
	else
	{
		m_pLastTag[uEndId] = (Tag*)AllocBack(ALLOC_OBJECT, sizeof(Tag));
	}

	m_pLastTag[uEndId]->pPrev = pPrevTag;
#ifndef FINAL_BUILD
	m_pLastTag[uEndId]->allocInfo = m_allocInfo[uEndId];
#endif
}

void DoubleStack::Freeze(bool bFreeze)
{
	m_bFrozen = bFreeze;

/*	DEBUG_PRINT("Currently %f MB allocated\n", ((float)GetAllocated())/(1024.f*1024.f));

	for(int i=0; i<ALLOC_TYPE_COUNT; i++)
	{
		DEBUG_PRINT("Group %s has taken %f MB\n", mem::GetAllocString((MemAllocType)i),
		 ((float)(m_allocInfo[0].uAllocSizes[i]))/(1024.f*1024.f));
	}*/
}

void DoubleStack::FreeToLastTag(bool bFront)
{
	uint32 uEndId = bFront ? 0 : 1;
	Tag*	pTag = m_pLastTag[uEndId];

#ifndef FINAL_BUILD
	m_allocInfo[uEndId] =  pTag->allocInfo;
#endif
	m_pTop[uEndId] = (uint8*)pTag;
	if(pTag->pPrev)
	{
		m_pLastTag[uEndId] = pTag->pPrev;
	}
}

#ifdef DEBUG_MEMORY
void DoubleStack::Free(void* pData, bool bGPU)
{
#ifdef DEBUG_BUILD
	uint8* pU8Data = (uint8*)pData;
	// Not valid to clean up data until everything has been allocated
	//ASSERT(m_bFrozen);
	// Assert this memory is within the currently active tag zone
	ASSERT( ( ((pU8Data >= (uint8*)m_pLastTag[0]) && (pU8Data < m_pTop[0] ))
		|| ( (pU8Data < (uint8*)m_pLastTag[1]) && (pU8Data > m_pTop[1] )) )  );

#endif

	// TODO: Use the header to remove the item from the allocated list, and eventually assert they've all been cleaned up
	m_memBlock.NotifyGPUFree(pData);
}
#else
void DoubleStack::Free(void* pData, bool bGPU)
{
	// Just notify the GPU
	if(bGPU)
	{
		m_memBlock.NotifyGPUFree(pData);
	}
}
#endif


uint32	DoubleStack::GetAllocated() const
{
	memsize bottom = (memsize)(m_pTop[0]-m_pRawBase);
	memsize top = (memsize)(m_pRawEnd-m_pTop[1]);
	
	return (uint32)(bottom + top);
}


uint32 DoubleStack::GetSize() const
{
	return (uint32)(m_pRawEnd - m_pRawBase);
}

#ifndef FINAL_BUILD
memsize	DoubleStack::GetAllocated(MemAllocType eType) const
{
	return m_allocInfo[0].uAllocSizes[eType] + m_allocInfo[1].uAllocSizes[eType];
}
#endif

}

