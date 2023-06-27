/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/GPUHeap.h"
#include "Engine/Graphics/Device/GFXDevice.h"

#define MERGE_DELAY 3	// 3 frames

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

	m_uTotalSize = uSize;

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

	if ( (uFreeFrame+ MERGE_DELAY) < uCurrentFrame)
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
	memsize uSpace = AlignSizeUp(pAllocator->GetSize(), pAllocator->GetAlign());
	uint32 uCurrentFrame = pDevice->GetFrameCount();

	bool bHasNoFrontPadding = false;
	for( auto pInfo : m_freeList)
	{
		ASSERT(pInfo->bValid);

		// Find the smallest 
		memsize uFrontPadding = GetRequiredFrontPadding(pAllocator, pInfo);
		memsize uTotalSize = uSpace + uFrontPadding;
		if(CanAlloc(uCurrentFrame, pInfo->uFreeFrame) && pInfo->pAllocator == NULL && (pInfo->uSize >= uTotalSize) )
		{
			// If it's the first, or is the first without padding, or is the smallest without padding
			if( (pSmallest == nullptr)
				|| ((uFrontPadding == 0) && !bHasNoFrontPadding)
				|| ((uFrontPadding == 0) && (pInfo->uSize < pSmallest->uSize)) )
			{
				pSmallest = pInfo;
				bHasNoFrontPadding = (uFrontPadding == 0);
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
		dstList.insert(eastl::lower_bound(dstList.begin(), dstList.end(), pInfo, ComparePointers), pInfo);
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

	memsize uFrontPadding = GetRequiredFrontPadding(pInfo->pAllocator, pInfo);

	pData = (void*)AlignAddress((memsize)pInfo->pLocation, uAlign);
	ASSERT(memsize(pData)-(memsize)(pInfo->pLocation) == GetRequiredFrontPadding(pAllocator, pInfo));
	memsize blockSize = ((memsize)pData) + AlignSizeUp(uSize, uAlign);
	blockSize -= (memsize)pInfo->pLocation;
	ASSERT(blockSize <= pInfo->uSize);
	pInfo->pAllocator = pAllocator;

	// We need a new block, quick add one
	if(blockSize < pInfo->uSize)
	{
		BlockInfo* pNext = FindUnusedBlock();
	
		m_freeList.push_back(pNext);

		pNext->bValid = true;
		pNext->uSize = pInfo->uSize;
		pNext->pLocation = (void*)(((memsize)pInfo->pLocation)+blockSize);
		pNext->pAllocator = NULL;
		pInfo->uSize = (memsize)pNext->pLocation - (memsize)pInfo->pLocation;
		pNext->uSize -= pInfo->uSize;
	}

	pAllocator->Allocated(pData);

	m_iMergeFrames = MERGE_DELAY + 2;
}

void GPUHeap::Validate()
{
	memsize totalSize = 0;
	for (auto itr : m_freeList)
	{
		totalSize += itr->uSize;
	}

	for (auto itr : m_allocList)
	{
		totalSize += itr->uSize;
	}

	ASSERT(totalSize == m_uTotalSize);
}

void GPUHeap::FreeMemory(BlockInfo* pInfo)
{
	ASSERT(pInfo->pAllocator!=NULL);

	MemAllocator* pAllocator = pInfo->pAllocator;
	pInfo->pAllocator = NULL;

	SwitchList(pInfo, m_allocList, m_freeList);

	pAllocator->Released();

	m_iMergeFrames = MERGE_DELAY + 2;
}

bool GPUHeap::ComparePointers(const GPUHeap::BlockInfo* const& a, const GPUHeap::BlockInfo* const& b)
{
	return a->pLocation < b->pLocation ;
}

void GPUHeap::MergeMemory(uint32 uCurrentFrame, bool bFast)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

	if (m_iMergeFrames <= 0)
		return;
	BlockInfo* pPrev = nullptr;

	bool bFound = false;
	bool bPairFound = false;
	
	do 
	{
		bFound = false;
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
					itr->uFreeFrame = usg::Math::Max(itr->uFreeFrame, pPrev->uFreeFrame);
					itr->bValid = false;

					// There will usually be space so don't go overboard trying to free
					SwitchList(itr, m_freeList, m_unusedList);
					if(bFast)
					{
						bFound = true;
						break;
					}
					else
					{
						return;
					}
				}
			}

			pPrev = itr;
		}

	} while(bFound);
	
	m_iMergeFrames--;
}


memsize GPUHeap::GetSmallestBlock(GFXDevice* pDevice, MemAllocator* pAllocator)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

	memsize uSpace = AlignSizeUp(pAllocator->GetSize(), pAllocator->GetAlign());
	uint32 uCurrentFrame = pDevice->GetFrameCount();

	if (m_freeList.empty())
		return false;

	if (m_unusedList.empty())
		return false;

	BlockInfo* pSmallest = nullptr;
	for (auto pInfo : m_freeList)
	{
		memsize uFrontPadding = GetRequiredFrontPadding(pAllocator, pInfo);

		if (pInfo->uSize >= (uSpace + uFrontPadding) && CanAlloc(uCurrentFrame, pInfo->uFreeFrame))
		{
			if (!pSmallest || pInfo->uSize < pSmallest->uSize)
			{
				pSmallest = pInfo;
			}
		}
	}

	return pSmallest ? pSmallest->uSize : 0;
}

bool GPUHeap::CanAllocate(GFXDevice* pDevice, MemAllocator* pAllocator)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

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
		
		memsize uFrontPadding = GetRequiredFrontPadding(pAllocator, pInfo);

		if( pInfo->uSize >= (uSpace+ uFrontPadding) && CanAlloc(uCurrentFrame, pInfo->uFreeFrame) )
		{
			return true;
		}
	}

	return false;
}


}
