/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A memory pool for objects of the same size
//  (as memory must be contiguous it can not be used for similar sized objects
//	as MemPool can). Use cautiously as the lack of any headers means you
//	must pass in the same number to the call to free as allocate
//	The most expensive operation is freeing, however this should remain
//	relatively cheap assuming reasonably large sized contiguous blocks
*****************************************************************************/
#pragma once

#ifndef USG_ARRAY_POOL_H
#define USG_ARRAY_POOL_H
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "Engine/Memory/MemUtil.h"
#include "FastPool.h"


//#define DEBUG_ARRAY_POOL
#ifdef DEBUG_ARRAY_POOL
//#define ARRAY_POOL_DP	DEBUG_PRINT
#define ARRAY_POOL_DP(...)	NO_OPERATION
#else
#define ARRAY_POOL_DP(...)	NO_OPERATION
#endif

namespace usg{

template <class AllocType>
class ArrayPool
{
public:
	ArrayPool();
	ArrayPool(uint32 uGroupSize, bool bCanResize = true);
	~ArrayPool();

	bool		Init(uint32 uGroupSize, bool bCanResize = true);
	AllocType*	AllocArray(uint32 uCount);
	void		FreeArray(AllocType* p, uint32 uCount);

private:
	void		AddMemoryBlock();
	AllocType*	AllocateSlices(uint32 uBlock, uint32 uCount);
	void		RemoveFromBlock(uint32 uBlock, AllocType*, uint32 uCount);
	void		DebugFreeList();

	struct FreeSliceHeader
	{
		FreeSliceHeader*	pPrev;
		FreeSliceHeader*	pNext;
		uint32				uFreeCount;
		#ifdef DEBUG_ARRAY_POOL
		uint32				uSafety;
		#endif
	};

	enum
	{
		MAX_MEM_BLOCKS = 8,
		VALID_ID		= 0xB00B135,
		INVALID_ID		= 0xDEADF00D,
		ABSORBED_ID		= 0xD00FD00F
	};

	AllocType*			m_pMemBlocks[MAX_MEM_BLOCKS];
	FreeSliceHeader*	m_pFirstFree[MAX_MEM_BLOCKS];
	uint32				m_uBlockCount;
	uint32				m_uGroupSize;
	bool				m_bCanResize;

#ifdef DEBUG_ARRAY_POOL
	struct DebugData
	{
		AllocType*	pLoc;
		uint32		uCount;
	};

	FastPool<DebugData> 	m_allocations;
	uint32					m_uFreeCount;
#endif
};

template <class AllocType>
NO_INLINE_TEMPL ArrayPool<AllocType>::ArrayPool(uint32 uGroupSize, bool bCanResize)
:m_uBlockCount(0),
m_uGroupSize(uGroupSize)
#ifdef DEBUG_ARRAY_POOL
,m_allocations(500),
m_uFreeCount(0)
#endif
{
	ASSERT(sizeof(FreeSliceHeader)<=sizeof(AllocType));
	m_pFirstFree[0] = nullptr;
	MemSet(m_pMemBlocks, 0, sizeof(AllocType*)*MAX_MEM_BLOCKS);
	Init(uGroupSize, bCanResize);
}

template <class AllocType>
NO_INLINE_TEMPL ArrayPool<AllocType>::ArrayPool()
:m_uBlockCount(0),
m_uGroupSize(0),
m_bCanResize(false)
#ifdef DEBUG_ARRAY_POOL
,m_allocations(500),
m_uFreeCount(0)
#endif
{
	ASSERT(sizeof(FreeSliceHeader)<=sizeof(AllocType));
	m_pFirstFree[0] = nullptr;
	MemSet(m_pMemBlocks, 0, sizeof(AllocType*)*MAX_MEM_BLOCKS);
}

template <class AllocType>
NO_INLINE_TEMPL bool ArrayPool<AllocType>::Init(uint32 uGroupSize, bool bCanResize)
{
	m_bCanResize = bCanResize;
	m_uGroupSize = uGroupSize;
	AddMemoryBlock();
	return true;
}

template <class AllocType>
NO_INLINE_TEMPL ArrayPool<AllocType>::~ArrayPool()
{
	for(uint32 i=0; i<m_uBlockCount; i++)
	{
		// TODO: Replace with proper allocation
		if(m_pMemBlocks[i])
		{
			mem::Free(m_pMemBlocks[i]);
		}
	}
}

template <class AllocType>
NO_INLINE_TEMPL AllocType* ArrayPool<AllocType>::AllocateSlices(uint32 uBlock, uint32 uCount)
{
	ARRAY_POOL_DP("Allocate slices count %d\n", uCount);
	FreeSliceHeader* pFreeSlice = m_pFirstFree[uBlock];
	AllocType* pReturn = nullptr;
	while(pFreeSlice)
	{
		if(pFreeSlice->uFreeCount >= uCount)
		{
			if(pFreeSlice->uFreeCount > uCount)
			{
				FreeSliceHeader tmpHeader = *pFreeSlice;
#ifdef DEBUG_ARRAY_POOL
				ASSERT(pFreeSlice->uSafety == INVALID_ID);
				pFreeSlice->uSafety = VALID_ID;
#endif
				tmpHeader.uFreeCount -= uCount;
				FreeSliceHeader* pNextSlice = (FreeSliceHeader*)(((AllocType*)pFreeSlice) + uCount);
				if(tmpHeader.pPrev)
				{
					tmpHeader.pPrev->pNext = pNextSlice;
				}
				if(tmpHeader.pNext)
				{
					tmpHeader.pNext->pPrev = pNextSlice;
				}
				*pNextSlice = tmpHeader;
				if(m_pFirstFree[uBlock] == pFreeSlice)
				{
					m_pFirstFree[uBlock] = pNextSlice;
				}

				FreeSliceHeader* pCheck = m_pFirstFree[uBlock];
				while(pCheck && pCheck->pNext)
				{
					ASSERT(pCheck->pNext->pPrev!=nullptr);
					ASSERT(pCheck->pNext->pPrev == pCheck);
					pCheck = pCheck->pNext;
				}

				pReturn = (AllocType*)pFreeSlice;
				break;
			}
			else
			{
				// Exactly the right size so just slot us in
				if(m_pFirstFree[uBlock] == pFreeSlice)
					m_pFirstFree[uBlock] = pFreeSlice->pNext;

				if(pFreeSlice->pPrev)
				{
					pFreeSlice->pPrev->pNext = pFreeSlice->pNext;
				}
				if(pFreeSlice->pNext)
				{
					pFreeSlice->pNext->pPrev = pFreeSlice->pPrev;
				}

				FreeSliceHeader* pCheck = m_pFirstFree[uBlock];
				while(pCheck && pCheck->pNext)
				{
					ASSERT(pCheck->pNext->pPrev!=nullptr);
					ASSERT(pCheck->pNext->pPrev == pCheck);
					pCheck = pCheck->pNext;
				}

				pReturn = (AllocType*)pFreeSlice;
				break;
			}
		}
		pFreeSlice = pFreeSlice->pNext;
	}

	#ifdef DEBUG_ARRAY_POOL
	// Confirm we haven't already handed this one out
	for(typename FastPool<DebugData>::Iterator it = m_allocations.Begin(); !it.IsEnd(); ++it)
	{
		if((*it)->pLoc == pReturn)
		{
			ASSERT(false);
		}
	}

	if(pReturn)
	{
		DebugData* pData = m_allocations.Alloc();
		pData->uCount = uCount;
		pData->pLoc = pReturn;
		m_uFreeCount -= uCount;
	}
	#endif

	DebugFreeList();

	return pReturn;
}

template <class AllocType>
NO_INLINE_TEMPL AllocType* ArrayPool<AllocType>::AllocArray(uint32 uCount)
{
	if(uCount > m_uGroupSize)
	{
		ASSERT_MSG(false, "We can't allocate arrays of that size for this type of object");
		return nullptr;
	}

	AllocType* pAlloc = nullptr;
	for(uint32 uBlock=0; uBlock < m_uBlockCount; uBlock++)
	{
		pAlloc = AllocateSlices(uBlock, uCount);
		if(pAlloc)
		{
			return pAlloc;
		}
	}

	if(m_bCanResize)
	{
		// If we've got down here we don't have enough free blocks, create some more
		AddMemoryBlock();
		return AllocateSlices(m_uBlockCount - 1, uCount);
	}

	return nullptr;
}

template <class AllocType>
NO_INLINE_TEMPL void ArrayPool<AllocType>::RemoveFromBlock(uint32 uBlock, AllocType* pData, uint32 uCount)
{
	ARRAY_POOL_DP("Remove from block %d count %d\n", uBlock, uCount);
#ifdef DEBUG_ARRAY_POOL
	DebugData* pCmp = nullptr;
	for(typename FastPool<ArrayPool<AllocType>::DebugData>::Iterator it = m_allocations.Begin(); !it.IsEnd(); ++it)
	{
		if((*it)->pLoc == pData)
		{
			pCmp = *it;
			break;
		}
	}
	ASSERT(pCmp!=nullptr);
	ASSERT(pCmp->uCount == uCount);
	m_allocations.Free(pCmp);

	m_uFreeCount += uCount;
#endif

	FreeSliceHeader* pPrevHeader = m_pFirstFree[uBlock];
	FreeSliceHeader* pThisHeader = (FreeSliceHeader*)pData;
	if(!pPrevHeader || pPrevHeader > pThisHeader)
	{
		if(pPrevHeader)
			pPrevHeader->pPrev = pThisHeader;

		pThisHeader->pNext			= pPrevHeader;
		pThisHeader->pPrev			= nullptr;
		pThisHeader->uFreeCount		= uCount;
#ifdef DEBUG_ARRAY_POOL
		pThisHeader->uSafety		= INVALID_ID;
#endif
		m_pFirstFree[uBlock]		= pThisHeader;
		pPrevHeader = nullptr;	// Wasn't actually a previous header
	}
	else
	{
		while(pPrevHeader->pNext && pPrevHeader->pNext < pThisHeader)
		{
			pPrevHeader = pPrevHeader->pNext;
#ifdef DEBUG_ARRAY_POOL
			ASSERT(pPrevHeader->uSafety == INVALID_ID);
#endif
		}

		pThisHeader->uFreeCount = uCount;
#ifdef DEBUG_ARRAY_POOL
		pThisHeader->uSafety = INVALID_ID;
#endif
		pThisHeader->pNext = pPrevHeader->pNext;
		pThisHeader->pPrev = pPrevHeader;

		pPrevHeader->pNext = pThisHeader;

		if( pThisHeader->pNext )
		{
			pThisHeader->pNext->pPrev = pThisHeader;
		}
	}

	// Check if we can merge two adjacent blocks
	if(pThisHeader->pPrev )
	{
		memsize offset = ((memsize)pThisHeader-(memsize)pThisHeader->pPrev)/sizeof(AllocType);
		// There is nothing inbetween us and the next free block, merge us back into a single block
		if(offset==pThisHeader->pPrev->uFreeCount)
		{
			FreeSliceHeader* pTmp = pThisHeader->pPrev;
			
			pTmp->pNext = pThisHeader->pNext;
			if(pTmp->pNext)
			{
				pTmp->pNext->pPrev = pThisHeader->pPrev;
			}
			pTmp->uFreeCount += pThisHeader->uFreeCount;

#ifdef DEBUG_ARRAY_POOL
			pThisHeader->uSafety = ABSORBED_ID;
#endif
			// We're now gone
			pThisHeader = pTmp;

		}
	}

	// And now check in the other direction to see if we can remove the other adjacent block
	if(pThisHeader->pNext )
	{
		memsize offset = ((memsize)pThisHeader->pNext-(memsize)pThisHeader)/sizeof(AllocType);
		ASSERT(offset>=uCount);	
		// There is nothing inbetween us and the next free block, merge us back into a single block
		if(offset==uCount)
		{
			FreeSliceHeader* pTmp = pThisHeader->pNext;
#ifdef DEBUG_ARRAY_POOL
			pTmp->uSafety = ABSORBED_ID;
#endif
			pThisHeader->pNext = pTmp->pNext;
			if(pTmp->pNext)
			{
				pTmp->pNext->pPrev = pThisHeader;
			}
			pThisHeader->uFreeCount += pTmp->uFreeCount;
		}
	}
	ASSERT(m_pFirstFree[uBlock]!=nullptr);
	DebugFreeList();
}

template <class AllocType>
NO_INLINE_TEMPL void ArrayPool<AllocType>::DebugFreeList()
{
#ifdef DEBUG_ARRAY_POOL

	uint32 uBlockCheck = 0;
	uint32 uTotalSlices = 0;
	for(uint32 uBlock = 0; uBlock < m_uBlockCount; uBlock++)
	{
		FreeSliceHeader* pCheck = m_pFirstFree[uBlock];
		while(pCheck)
		{
			if(pCheck->pNext)
			{
				ASSERT(pCheck->pNext->pPrev!=nullptr);
				ASSERT(pCheck->pNext->pPrev == pCheck);
			}
			ARRAY_POOL_DP("Block %d Block count %d, Location %x, Safefty %x\n", uBlockCheck, pCheck->uFreeCount, pCheck, pCheck->uSafety);
			uBlockCheck++;
			uTotalSlices += pCheck->uFreeCount;
			pCheck = pCheck->pNext;
		}
	}
	ASSERT(m_uFreeCount == uTotalSlices);
	#endif
}

template <class AllocType>
NO_INLINE_TEMPL void ArrayPool<AllocType>::FreeArray(AllocType* p, uint32 uCount)
{
	for(uint32 i=0; i<m_uBlockCount; i++)
	{
		if(p >= m_pMemBlocks[i] && (p+uCount)<=(m_pMemBlocks[i]+m_uGroupSize))
		{
			RemoveFromBlock(i, p, uCount);
			return;
		}
	}
	ASSERT(false);
}

template <class AllocType>
NO_INLINE_TEMPL void ArrayPool<AllocType>::AddMemoryBlock()
{
	ARRAY_POOL_DP("AddMemoryBlock\n");
	if(m_uBlockCount >= MAX_MEM_BLOCKS)
	{
		ASSERT(false);
		return;
	}

	MemType eHeapType = MEMTYPE_STANDARD;
	m_pMemBlocks[m_uBlockCount] = (AllocType*)mem::Alloc(eHeapType, ALLOC_POOL, sizeof(AllocType)*m_uGroupSize, 128);

	FreeSliceHeader* pHeader = (FreeSliceHeader*)m_pMemBlocks[m_uBlockCount];


	m_pFirstFree[m_uBlockCount]	= pHeader;
	pHeader->pPrev				= nullptr;
	pHeader->pNext				= nullptr;
	pHeader->uFreeCount 		= m_uGroupSize;
#ifdef DEBUG_ARRAY_POOL
	pHeader->uSafety			= INVALID_ID;
#endif

	m_uBlockCount++;

#ifdef DEBUG_ARRAY_POOL
	m_uFreeCount += m_uGroupSize;
#endif
}

}

#endif // USG_ARRAY_POOL_H
