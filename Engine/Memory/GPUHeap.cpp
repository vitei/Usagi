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
}

GPUHeap::~GPUHeap()
{

}


void GPUHeap::Init(void* pMem, memsize uSize, uint32 uMaxAllocs, bool bDelayFree)
{
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

		pInfo->pListNext	= i < (uMaxAllocs-1) ? &m_memoryBlocks[i+1] : NULL;	
		pInfo->pListPrev	= i > 0 ? &m_memoryBlocks[i-1] : NULL;

		pInfo->pNext	= NULL;	
		pInfo->pPrev	= NULL;
		pInfo->uFreeFrame = USG_INVALID_ID;
	}

	m_memoryBlocks[0].uSize		= uSize;
	m_memoryBlocks[0].pLocation	= m_pHeapMem;
	m_memoryBlocks[0].bValid		= true;
	m_memoryBlocks[0].pListNext		= NULL;
	m_memoryBlocks[0].pListPrev		= NULL;

	m_pAllocList = NULL;
	m_pFreeList = &m_memoryBlocks[0];
	m_pUnusedList = &m_memoryBlocks[1];
}

bool GPUHeap::CanAlloc(uint32 uCurrentFrame, uint32 uFreeFrame)
{
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

void GPUHeap::SwitchList(BlockInfo* pInfo, BlockInfo** ppSrcList, BlockInfo** ppDstList)
{
	ASSERT(pInfo!=NULL);

	// Fist take us out of the list
	if(pInfo->pListNext)
	{
		pInfo->pListNext->pListPrev = pInfo->pListPrev;
	}
	if(pInfo->pListPrev)
	{
		pInfo->pListPrev->pListNext = pInfo->pListNext;
	}

	// Update the pointer if necessary
	if(*ppSrcList == pInfo)
	{
		*ppSrcList = pInfo->pListNext;
	}
	
	pInfo->pListPrev = NULL;
	pInfo->pListNext = *ppDstList;
	if(*ppDstList)
	{
		(*ppDstList)->pListPrev = pInfo;
	}
	*ppDstList = pInfo;
}

GPUHeap::BlockInfo* GPUHeap::PopList(BlockInfo** ppSrcList, BlockInfo** ppDstList)
{
	BlockInfo* pReturn = *ppSrcList;
	if(pReturn)
	{
		*ppSrcList = pReturn->pListNext;
		if(*ppSrcList)
		{
			(*ppSrcList)->pListPrev = NULL;
		}
		pReturn->pListPrev = NULL;
		pReturn->pListNext = *ppDstList;
		if(*ppDstList)
		{
			(*ppDstList)->pListPrev = pReturn;
		}
		*ppDstList = pReturn;
	}

	return pReturn;
}

void GPUHeap::AddAllocator(GFXDevice* pDevice, MemAllocator* pAllocator)
{
	BlockInfo* pSmallest = NULL;
	uint32 uSpace = (uint32)AlignSizeUp(pAllocator->GetSize(), pAllocator->GetAlign());
	uint32 uCurrentFrame = pDevice->GetFrameCount();

	BlockInfo* pInfo = m_pFreeList;

	while(pInfo)
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
		pInfo = pInfo->pListNext;
	}

	if(pSmallest)
	{
		pSmallest->pAllocator = pAllocator;
		SwitchList(pSmallest, &m_pFreeList, &m_pAllocList);
		AllocMemory(pSmallest);
		return;
	}

	ASSERT(false);
}

void GPUHeap::RemoveAllocator(GFXDevice* pDevice, MemAllocator* pAllocator)
{
	BlockInfo* pInfo = m_pAllocList;
	while(pInfo)
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
		pInfo = pInfo->pListNext;
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
	return PopList(&m_pUnusedList, &m_pFreeList);
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
	
		if(pInfo->pNext)
		{
			pInfo->pNext->pPrev = pNext;
		}
		pNext->bValid = true;
		pNext->uSize = pInfo->uSize - blockSize;
		pNext->pLocation = (void*)(((memsize)pInfo->pLocation)+blockSize);
		pNext->pPrev = pInfo;
		pNext->pNext = pInfo->pNext;
		pNext->pAllocator = NULL;

		pInfo->pNext = pNext;
		pInfo->uSize = blockSize;
	}

	pAllocator->Allocated(pData);
}

void GPUHeap::FreeMemory(BlockInfo* pInfo)
{
	ASSERT(pInfo->pAllocator!=NULL);

	MemAllocator* pAllocator = pInfo->pAllocator;
	pInfo->pAllocator = NULL;

	SwitchList(pInfo, &m_pAllocList, &m_pFreeList);
	// If the next block is empty then take over it's memory
	if(pInfo->pNext && pInfo->pNext->pAllocator == NULL)
	{
		pInfo->uSize += pInfo->pNext->uSize;
		pInfo->pNext->bValid = false;
		SwitchList(pInfo->pNext, &m_pFreeList, &m_pUnusedList);
		pInfo->pNext = pInfo->pNext->pNext;
		if(pInfo->pNext)
		{
			// Hook up the next pointer
			pInfo->pNext->pPrev = pInfo;
		}
	}

	// If the previous block is empty then have it take over our memory
	if(pInfo->pPrev && pInfo->pPrev->pAllocator == NULL)
	{
		BlockInfo* pPrev = pInfo->pPrev;
		pPrev->uSize += pInfo->uSize;
		pInfo->bValid = false;
		pPrev->pNext = pInfo->pNext;
		SwitchList(pInfo, &m_pFreeList, &m_pUnusedList);
		if(pPrev->pNext)
		{
			// Hook up the next pointer
			pPrev->pNext->pPrev = pPrev;
		}
	}

	pAllocator->Released();
}

bool GPUHeap::CanAllocate(GFXDevice* pDevice, MemAllocator* pAllocator)
{
	memsize uSpace = AlignSizeUp(pAllocator->GetSize(), pAllocator->GetAlign());
	uint32 uCurrentFrame = pDevice->GetFrameCount();

	BlockInfo* pInfo = m_pFreeList;

	if(!m_pUnusedList)
		return false;

	while(pInfo)
	{
		ASSERT(pInfo->bValid);
		ASSERT(pInfo->pAllocator==NULL);
		// Find the smallest 
		if( pInfo->uSize >= uSpace && CanAlloc(uCurrentFrame, pInfo->uFreeFrame) )
		{
			return true;
		}
		pInfo = pInfo->pListNext;
	}

	return false;
}


}
