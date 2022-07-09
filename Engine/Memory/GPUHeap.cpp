/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/GPUHeap.h"
#include "Engine/Graphics/Device/GFXDevice.h"

namespace usg {

GPUHeap::GPUHeap()
{
	//MemClear(m_memoryBlocks, sizeof(BlockInfo*)*MAX_GPU_ALLOCATIONS);
	m_memoryBlocks = NULL;
	m_iMergeFrames = 0;
}

GPUHeap::~GPUHeap()
{
	if (m_memoryBlocks)
	{
		mem::Free(m_memoryBlocks);
		m_memoryBlocks = nullptr;
	}
}


void GPUHeap::Init(void* pMem, memsize uSize, uint32 uMaxAllocs, bool bDelayFree)
{
	m_criticalSection.Initialize();

	m_pHeapMem = pMem;
	m_bDelayFree = bDelayFree;
	m_memoryBlocks = (BlockInfo*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_SYSTEM, sizeof(BlockInfo)*uMaxAllocs, 4, false);
	MemClear(m_memoryBlocks, sizeof(BlockInfo*)*uMaxAllocs);
	m_uMaxAllocs = uMaxAllocs;

	for(uint32 i=0; i<uMaxAllocs; i++)
	{
		BlockInfo* pInfo	= &m_memoryBlocks[i];
		pInfo->pAllocator	= NULL;
		pInfo->pLocation	= NULL;
		pInfo->uSize		= 0;
		pInfo->bValid		= false;

		pInfo->uFreeFrame = USG_INVALID_ID;
	}

	m_memoryBlocks[0].uSize		= uSize;
	m_memoryBlocks[0].pLocation	= m_pHeapMem;
	m_memoryBlocks[0].bValid		= true;

	m_freeList.push_back(&m_memoryBlocks[0]);
	for (uint32 i = 1; i < uMaxAllocs; i++)
	{
		m_unusedList.push_back(&m_memoryBlocks[i]);
	}
}

bool GPUHeap::CanAlloc(uint32 uCurrentFrame, uint32 uFreeFrame)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

	if (!m_bDelayFree)
	{
		return true;
	}

	if ( (uFreeFrame+ GFX_NUM_DYN_BUFF) < uCurrentFrame )
	{
		return true;
	}

	// Wrapped around
	if (uFreeFrame > uCurrentFrame)
	{
		return true;
	}

	return false;
}





void GPUHeap::AddAllocator(GFXDevice* pDevice, MemAllocator* pAllocator)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

	BlockInfo* pSmallest = NULL;
	uint32 uSpace = (uint32)AlignSizeUp(pAllocator->GetSize(), pAllocator->GetAlign());
	uint32 uCurrentFrame = pDevice->GetFrameCount();

	for( auto pInfo : m_freeList)
	{
		ASSERT(pInfo->bValid);

		// Find the smallest 
		if(CanAlloc(uCurrentFrame, pInfo->uFreeFrame) && pInfo->pAllocator == NULL && (pInfo->uSize >= uSpace) )
		{
			if(pSmallest == NULL || pInfo->uSize < pSmallest->uSize)
			{
				pSmallest = pInfo;
			}
		}
	}

	if(pSmallest)
	{
		pSmallest->pAllocator = pAllocator;
		SwitchList(pSmallest, m_freeList, m_allocList);
		AllocMemory(pSmallest);
		return;
	}

	ASSERT(false);
}

void GPUHeap::SwitchList(BlockInfo* pInfo, usg::list< BlockInfo* >& srcList, usg::list< BlockInfo* >& dstList)
{
	auto itr = eastl::find(srcList.begin(), srcList.end(), pInfo);
	ASSERT(itr != srcList.end());
	srcList.erase(itr);

	itr = eastl::find(srcList.begin(), srcList.end(), pInfo);
	ASSERT(itr == srcList.end());

	if (&dstList == &m_freeList)
	{
		dstList.insert(eastl::lower_bound(dstList.begin(), dstList.end(), pInfo), pInfo);
	}
	else
	{
		dstList.push_back(pInfo);
	}
}

void GPUHeap::RemoveAllocator(GFXDevice* pDevice, MemAllocator* pAllocator)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

	for(auto pInfo : m_allocList)
	{
		if( pInfo->pAllocator == pAllocator )
		{
			FreeMemory(pInfo);
			pInfo->pAllocator = NULL;
			// Even if blocks got merged we keep the free frame from the current block as it will be newer
			// This could only ever be a problem if we kept freeing neighboring allocations every frame
			pInfo->uFreeFrame = pDevice->GetFrameCount();
			return;
		}
	}

	ASSERT(false);
}


void GPUHeap::ReacquireAll()
{
	// Implement me
	ASSERT(false);
}

void GPUHeap::ReleaseAll()
{
	// Implement me
	ASSERT(false);
}

GPUHeap::BlockInfo* GPUHeap::FindUnusedBlock()
{
	BlockInfo* front = m_unusedList.front();
	m_unusedList.pop_front();
	return front;
}

void GPUHeap::AllocMemory(BlockInfo* pInfo)
{
	MemAllocator* pAllocator = pInfo->pAllocator;
	uint32 uSize = pAllocator->GetSize();
	uint32 uAlign = pAllocator->GetAlign();

	void* pData = NULL;

	pData = (void*)AlignAddress((memsize)pInfo->pLocation, uAlign);
	memsize blockSize = ((memsize)pData) + uSize;
	blockSize -= (memsize)pInfo->pLocation;
	pInfo->pAllocator = pAllocator;

	// We need a new block, quick add one
	if(blockSize < pInfo->uSize)
	{
		BlockInfo* pNext = FindUnusedBlock();
	
		m_freeList.push_back(pNext);

		pNext->bValid = true;
		pNext->uSize = pInfo->uSize - blockSize;
		pNext->pLocation = (void*)(((memsize)pInfo->pLocation)+blockSize);
		pNext->pAllocator = NULL;
		pInfo->uSize = blockSize;
	}

	pAllocator->Allocated(pData);

	m_iMergeFrames = GFX_NUM_DYN_BUFF;
}

void GPUHeap::FreeMemory(BlockInfo* pInfo)
{
	ASSERT(pInfo->pAllocator!=NULL);

	MemAllocator* pAllocator = pInfo->pAllocator;
	pInfo->pAllocator = NULL;

	SwitchList(pInfo, m_allocList, m_freeList);

	pAllocator->Released();

	m_iMergeFrames = GFX_NUM_DYN_BUFF;
}

void GPUHeap::MergeMemory(uint32 uCurrentFrame)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

	if (m_iMergeFrames <= 0)
		return;
	BlockInfo* pPrev = nullptr;

	for (auto itr : m_freeList)
	{
		if (pPrev && CanAlloc(uCurrentFrame, pPrev->uFreeFrame) && CanAlloc(uCurrentFrame, itr->uFreeFrame) )
		{
			uint8* pPrevEnd = ((uint8*)pPrev->pLocation) + pPrev->uSize;
			uint8* pCurr = (uint8*)itr->pLocation;

			if (pPrevEnd == pCurr)
			{
				pPrev->uSize += itr->uSize;
				itr->uSize = 0;
				itr->pLocation = nullptr;
				itr->uFreeFrame = USG_INVALID_ID;
				itr->bValid = false;
				SwitchList(itr, m_freeList, m_unusedList);
				// There will usually be space so don't go overboard trying to free
				return;
			}
		}

		pPrev = itr;
	}

	m_iMergeFrames--;
}


bool GPUHeap::CanAllocate(GFXDevice* pDevice, MemAllocator* pAllocator)
{
	memsize uSpace = AlignSizeUp(pAllocator->GetSize(), pAllocator->GetAlign());
	uint32 uCurrentFrame = pDevice->GetFrameCount();

	if (m_freeList.empty())
		return false;

	if(m_unusedList.empty())
		return false;

	for(auto pInfo : m_freeList)
	{
		ASSERT(pInfo->bValid);
		ASSERT(pInfo->pAllocator==NULL);
		// Find the smallest 
		if( pInfo->uSize >= uSpace && CanAlloc(uCurrentFrame, pInfo->uFreeFrame) )
		{
			return true;
		}
	}

	return false;
}


}
