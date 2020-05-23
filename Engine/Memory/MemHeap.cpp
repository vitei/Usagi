/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/String_Util.h"
#include "MemHeap.h"


#define MEMSYSTEM_SIGNATURE  0xF33DF33D
#define MEMSYSTEM_FREE_SIG   0xDEADC0DE

// Previously needed for other platforms
//#define GPU_NOTIFIES

namespace usg{

#ifdef DEBUG_MEMORY
int MemHeap::s_nextAllocID = 1;
#endif

struct AllocFooter
{
	// To check for overrun
	uint32		uSignature;
};


struct AllocHeader
{
#ifdef DEBUG_MEMORY
    uint32			uSignature;
    uint32			uAllocNum;
	memsize			uSize;
	MemAllocType	eType;
	bool			bStatic;
#endif
	AllocHeader*	pNext;
    AllocHeader*	pPrev;
	AllocFooter*	pFooter;
    MemHeap*		pHeap;
	memsize			uAlignOffset;
	uint16			uAlignment;
	uint8			uGroup;
#ifdef GPU_NOTIFIES
	uint8			uGPU;
#endif
};


MemHeap::MemHeap() :
	  m_uInstances(0)
	, m_bActive(false)
	, m_bStatic(false)
	, m_pHeadAlloc(nullptr)
{

}

void MemHeap::Initialize(memsize uSize)
{
	m_bActive        = false;
	m_uInstances	 = 0;
	m_bStatic 		= true;
#ifdef DEBUG_MEMORY
	for(uint32 i=0; i<ALLOC_TYPE_COUNT; i++)
		m_uBytesAllocated[i] = 0;
#endif
	m_pHeadAlloc     = nullptr;
	m_criticalSection.Initialize();
	m_platform.Init(uSize);
}


void MemHeap::Initialize(void* location, memsize uSize)
{

#ifdef DEBUG_MEMORY
	for(uint32 i=0; i<ALLOC_TYPE_COUNT; i++)
		m_uBytesAllocated[i] = 0;
#endif
	m_pHeadAlloc = nullptr;
	m_uInstances = 0;
	m_criticalSection.Initialize();
	m_platform.Init(location, uSize);
}


memsize MemHeap::AlignAddress(memsize uAddress, memsize uAlign)
{
	memsize uMask = uAlign-1;
	memsize uMisAlignment = (uAddress & uMask);
	memsize uAdjustment = uAlign - uMisAlignment;
	
	return uAddress + uAdjustment;
}


void* MemHeap::ReAlloc(void* pMem, memsize uNewSize)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

	AllocHeader* pHeader = (AllocHeader *)((char *)pMem - sizeof(AllocHeader));
	AllocHeader* pPrev = pHeader;

	void* pOrigAlloc = (void*)((memsize)pHeader - (memsize)pHeader->uAlignOffset);
	void* pNew = m_platform.ReAlloc((void*)pOrigAlloc, uNewSize);

	pHeader = (AllocHeader *)((char *)pMem - sizeof(AllocHeader));
	if (pHeader != pPrev)
	{
		if (pHeader->pPrev)
		{
			pHeader->pPrev->pNext = pHeader;
		}
		if (pHeader->pNext)
		{
			pHeader->pNext->pPrev = pHeader;
		}
		if (m_pHeadAlloc == pHeader)
		{
			m_pHeadAlloc = pHeader;
		}
	}
	return pNew;
}

void* MemHeap::Allocate(memsize uBytes, memsize uAlign, uint8 uGroup, MemAllocType eType, bool bGpu)
{
	CriticalSection::ScopedLock lock(m_criticalSection);
	if(uAlign < 4)
	{
		uAlign = 4;		// For the benefit of the AllocHeader
	}
    ASSERT((uAlign%4) == 0);
	memsize uRequestedBytes = uBytes + sizeof(AllocHeader) + uAlign;

#ifdef DEBUG_MEMORY
	uRequestedBytes = AlignAddress(uRequestedBytes, 4);
	uRequestedBytes += sizeof(AllocFooter);
#endif

	uint8* pMem = (uint8*)m_platform.Alloc(uRequestedBytes, 4, bGpu);
#ifndef FINAL_BUILD
	if (pMem == nullptr)
	{
		const memsize uSize = GetSize();
		const memsize uUsed = uSize - GetFreeSize();
		DEBUG_PRINT("Total heap size: %u Used: %u\n",uSize,uUsed);
		for (int i = 0; i < ALLOC_TYPE_COUNT; i++)
		{
			DEBUG_PRINT("%s : %u\n", mem::GetAllocString((MemAllocType)i),GetAllocated((MemAllocType)i));
		}
		ASSERT(false);
	}
#endif
	memsize dataLoc = AlignAddress((memsize)pMem+sizeof(AllocHeader), uAlign);
	// The header is just before the data
    AllocHeader * pHeader = (AllocHeader*) ( dataLoc - sizeof(AllocHeader) );
	
     pHeader->pPrev = nullptr;
#ifdef DEBUG_MEMORY
	pHeader->uSignature = MEMSYSTEM_SIGNATURE;
	pHeader->uSize = uBytes;
    pHeader->uAllocNum = s_nextAllocID++;
	pHeader->eType = eType;
	pHeader->bStatic = m_bStatic;
	pHeader->pFooter = (AllocFooter*)(pMem + uRequestedBytes - 4);
	AllocFooter* pFooter = pHeader->pFooter;
	pFooter->uSignature = MEMSYSTEM_SIGNATURE;
#endif
	pHeader->pNext = m_pHeadAlloc;
	pHeader->uGroup = uGroup;
	pHeader->pHeap = this;
	pHeader->uAlignment = static_cast<uint16>(uAlign);
#ifdef GPU_NOTIFIES
	pHeader->uGPU = bGpu ? 1 : 0;
#endif

    if (m_pHeadAlloc != nullptr)
	{
        m_pHeadAlloc->pPrev = pHeader;
	}
    m_pHeadAlloc = pHeader;

#ifdef DEBUG_MEMORY
    m_uBytesAllocated[eType] += uBytes;
    
#endif

    m_uInstances++;

	pHeader->uAlignOffset = ((memsize)pHeader - (memsize)pMem);

    return (void*)dataLoc; 
}


void MemHeap::Deallocate(void * pMem)
{
	if (pMem == nullptr)
	{
		return;
	}
    AllocHeader* pHeader = (AllocHeader *)((char *)pMem - sizeof(AllocHeader));
    pHeader->pHeap->Deallocate(pHeader);
}

void* MemHeap::Reallocate(void* pMem, memsize uNewSize)
{
	if (pMem == nullptr)
	{
		return nullptr;
	}
	AllocHeader* pHeader = (AllocHeader *)((char *)pMem - sizeof(AllocHeader));
	return pHeader->pHeap->ReAlloc(pHeader, uNewSize);
}

memsize MemHeap::GetAlignment(const void* const pMem)
{
	const AllocHeader* const pHeader = (AllocHeader *)((char *)pMem - sizeof(AllocHeader));
	return static_cast<memsize>(pHeader->uAlignment);
}

void MemHeap::Deallocate(AllocHeader * pHeader)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

#ifdef DEBUG_MEMORY
	ASSERT(pHeader->uSignature == MEMSYSTEM_SIGNATURE);
	pHeader->uSignature = MEMSYSTEM_FREE_SIG;
	AllocFooter* pFooter = pHeader->pFooter;
	ASSERT(pFooter->uSignature == MEMSYSTEM_SIGNATURE);
	pFooter->uSignature = MEMSYSTEM_FREE_SIG;
#endif

    if (pHeader->pPrev == nullptr)
    {
        ASSERT(pHeader == m_pHeadAlloc);
        m_pHeadAlloc = pHeader->pNext;
    }        
    else
	{
        pHeader->pPrev->pNext = pHeader->pNext;
	}

    if (pHeader->pNext != nullptr)
	{
        pHeader->pNext->pPrev = pHeader->pPrev;   
	}

#ifdef DEBUG_MEMORY
    m_uBytesAllocated[pHeader->eType] -= pHeader->uSize;
#endif

    m_uInstances--;
	void* pOrigAlloc = (void*)((memsize)pHeader - (memsize)pHeader->uAlignOffset);
	bool bGPU = false;
	uint32 uAlign = pHeader->uAlignment;
#ifdef GPU_NOTIFIES
	bGPU = pHeader->uGPU != 0;
#endif 
	m_platform.Free((void*)pOrigAlloc, uAlign, bGPU);
}

void MemHeap::FreeGroup(uint32 uGroup)
{
	// Iterate through the heap looking for items from this allocation group
	if(!m_pHeadAlloc)
		return; 

	AllocHeader* pHeader = m_pHeadAlloc;
	AllocHeader* pNext;
	while(pHeader)
	{
		pNext = pHeader->pNext;
		if(pHeader->uGroup == uGroup)
		{
			Deallocate(pHeader);
		}
		pHeader = pNext;
	}
	
}

void MemHeap::CheckData()
{
	if(!m_pHeadAlloc)
		return;
	
	AllocHeader*	pAlloc = m_pHeadAlloc;

	while(pAlloc)
	{
		ASSERT(pAlloc->uSignature == MEMSYSTEM_SIGNATURE);
		ASSERT(pAlloc->pFooter->uSignature == MEMSYSTEM_SIGNATURE);
		pAlloc = pAlloc->pNext;
	}
}

void MemHeap::PrintDynamicAllocTypes()
{
#ifdef DEBUG_MEMORY
	if(!m_pHeadAlloc)
		return;

	AllocHeader*	pAlloc = m_pHeadAlloc;

	while(pAlloc)
	{
		if(pAlloc->bStatic == false)
		{
			DEBUG_PRINT("Alloc Num %d of size %d and type %s\n", pAlloc->uAllocNum, pAlloc->uSize, mem::GetAllocString((MemAllocType)pAlloc->eType));
		}
		pAlloc = pAlloc->pNext;
	}

#endif	
}

void MemHeap::PrintUsage()
{
#ifdef DEBUG_MEMORY
	memsize uAllocSizes[ALLOC_TYPE_COUNT];
	for(uint32 i=0; i<ALLOC_TYPE_COUNT; i++)
	{
		uAllocSizes[i] = 0;
	}

	if(!m_pHeadAlloc)
		return;

	AllocHeader*	pAlloc = m_pHeadAlloc;

	while(pAlloc)
	{
		uAllocSizes[pAlloc->eType] += pAlloc->uSize;
		pAlloc = pAlloc->pNext;
	}

	for(uint32 i=0; i<ALLOC_TYPE_COUNT; i++)
	{
		DEBUG_PRINT("Using %d of %s\n", uAllocSizes[i], mem::GetAllocString((MemAllocType)i));
	}
#endif
}

}
