/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/UnTypesafeFastPool.h"

namespace usg
{

	UnTypesafeFastPool::UnTypesafeFastPool(size_t uElementSize, uint32 uGroupSize, bool bCanResize, bool bOptimseForItr, decltype(m_pElementInitializer) pElementArrayAllocator,	decltype(m_pElementDeinitializer) pDeallocator): m_uElementSize(uElementSize), m_uBlockCount(0), m_uGroupSize(uGroupSize)
	{
		m_uSize = 0;
		m_pFirstFree = NULL;
		m_pFirstMemBlock = NULL;
		if (!bCanResize)
		{
			AddMemoryBlock();
		}
		m_pFirstAllocated = NULL;
		m_bCanResize = bCanResize;
		m_bOptimseForIttr = bOptimseForItr;

		m_pElementInitializer = pElementArrayAllocator;
		m_pElementDeinitializer = pDeallocator;
	}

	UnTypesafeFastPool::~UnTypesafeFastPool()
	{
		MemBlock* pBlock = m_pFirstMemBlock;
		while (pBlock)
		{
			MemBlock* pNext = pBlock->pNext;
			if (pBlock->pEntries)
			{
				if (m_pElementDeinitializer != nullptr)
				{
					uint8* pHeader = (uint8*)pBlock->pEntries;
					for (memsize i = 0; i < m_uGroupSize; i++)
					{
						EntryHeader* pEntry = (EntryHeader*)pHeader;
						m_pElementDeinitializer(pEntry->GetDataPointer());
						pHeader += GetEntrySize();
					}
				}
				uint8* pArray = (uint8*)pBlock->pEntries;
				vdelete[] pArray;
			}
			utl::SafeDelete(&pBlock);
			pBlock = pNext;
		}
	}

	void UnTypesafeFastPool::AddMemoryBlock()
	{
		MemType eHeapType = MEMTYPE_STANDARD;

		// Allocate a new block
		MemBlock* pBlock = vnewHeap(ALLOC_FASTPOOL, eHeapType) MemBlock;
		pBlock->pEntries = vnewHeap(ALLOC_FASTPOOL, eHeapType) uint8[m_uGroupSize*GetEntrySize()];

		// Run user-defiend constructor if necessary
		if (m_pElementInitializer != nullptr)
		{
			uint8* pHeader = (uint8*)pBlock->pEntries;
			for (memsize i = 0; i < m_uGroupSize; i++)
			{
				EntryHeader* pEntry = (EntryHeader*)pHeader;
				m_pElementInitializer(pEntry->GetDataPointer());
				pHeader += GetEntrySize();
			}
		}

		// DEBUG_PRINT("Allocating %d\n", m_uGroupSize*sizeof(Entry));
		
		pBlock->pNext = NULL;

		// Find the insertion point
		if (!m_pFirstMemBlock)
		{
			m_pFirstMemBlock = pBlock;
		}
		else
		{
			MemBlock* pInsertionPoint = m_pFirstMemBlock;

			if(m_bOptimseForIttr)
			{
				while (pInsertionPoint->pNext)
				{
					pInsertionPoint = pInsertionPoint->pNext;
				}
				pInsertionPoint->pNext = pBlock;
			}
			else
			{
				pBlock->pNext = m_pFirstMemBlock;
				m_pFirstMemBlock = pBlock;
			}
		}

		EntryHeader* pHeader = (EntryHeader*)pBlock->pEntries;

		if (!m_pFirstFree)
		{
			m_pFirstFree = pHeader;
			pHeader->pPrev = NULL;
		}
		else
		{
			EntryHeader* pEntry = m_pFirstFree;
			while (pEntry->pNext)
			{
				pEntry = pEntry->pNext;
			}
			pEntry->pNext = pHeader;
			pHeader->pPrev = pEntry;
		}

		pHeader->pNext = Advance(pHeader);

		pHeader = Advance(pHeader);
		for (uint32 i = 1; i < m_uGroupSize - 1; i++)
		{
			pHeader->pNext = Advance(pHeader);
			pHeader->pPrev = Back(pHeader);
			pHeader = Advance(pHeader);
		}

		EntryHeader* pLast = (EntryHeader*)(((uint8*)pBlock->pEntries) + (m_uGroupSize - 1)*GetEntrySize());

		pLast->pPrev = Back(pLast);
		pLast->pNext = NULL;

		m_uBlockCount++;
	}

	void* UnTypesafeFastPool::Alloc()
	{
		if (!m_pFirstFree)
		{
			if (m_bCanResize || (m_pFirstMemBlock == NULL))
			{
				AddMemoryBlock();
			}
		}
		EntryHeader* pAlloc = m_pFirstFree;
		ASSERT(pAlloc != NULL);
		if (pAlloc)
		{
			// First remove us from the free list
			m_pFirstFree = pAlloc->pNext;
			if (m_pFirstFree)
			{
				m_pFirstFree->pPrev = NULL;
			}

			if (!m_pFirstAllocated || (m_pFirstAllocated > pAlloc))
			{
				// We're the first one
				pAlloc->pNext = m_pFirstAllocated;
				pAlloc->pPrev = NULL;
				m_pFirstAllocated = pAlloc;
			}
			else
			{
				EntryHeader* cmp = m_pFirstAllocated;
				while (m_bOptimseForIttr && (cmp->pNext != NULL) && (cmp->pNext < pAlloc))
				{
					cmp = cmp->pNext;
				}

				// We want to be inserted after cmp to maintain a good cache consistency
				pAlloc->pNext = cmp->pNext;
				pAlloc->pPrev = cmp;
			}
			if (pAlloc->pNext)
			{
				pAlloc->pNext->pPrev = pAlloc;
			}
			if (pAlloc->pPrev)
			{
				pAlloc->pPrev->pNext = pAlloc;
			}
		}

		if (pAlloc)
		{
			m_uSize++;
		}
		else
		{
			return NULL;
		}
		return pAlloc->GetDataPointer();
	}

	void UnTypesafeFastPool::Free(void* pData)
	{
		int* intp = (int*)pData;

		EntryHeader* pEntry = GetHeaderFromData(pData);
		void* pDataFromHeader = pEntry->GetDataPointer();
		ASSERT(pDataFromHeader == pData);

		EntryHeader* pPrev = pEntry->pPrev;
		EntryHeader* pNext = pEntry->pNext;

		m_uSize--;

		// Manually call the destructor
		// First remove us from the active list
		if (pPrev)
		{
			pPrev->pNext = pNext;
		}
		else
		{
			// This was the first item in the list, so update the first item to be our next pointer
			ASSERT(m_pFirstAllocated == pEntry);
			m_pFirstAllocated = pNext;
		}

		if (pNext)
		{
			pNext->pPrev = pPrev;
		}

		if (m_pFirstFree)
		{
			m_pFirstFree->pPrev = pEntry;
		}

		pEntry->pNext = m_pFirstFree;
		pEntry->pPrev = NULL;
		m_pFirstFree = pEntry;
	}


	void* UnTypesafeFastPool::GetByIndex(uint32 uIndex)
	{
		uint32 uBlock = uIndex / m_uGroupSize;
		ASSERT(uBlock < m_uBlockCount);
		uIndex -= (uBlock*m_uGroupSize);

		ASSERT(uIndex < m_uGroupSize);

		MemBlock* pBlock = m_pFirstMemBlock;
		for (uint32 i = 0; i < uBlock; i++)
		{
			pBlock = pBlock->pNext;
			ASSERT(pBlock != NULL);
		}

		auto pEntry = (EntryHeader*)((uint8*)pBlock->pEntries + uIndex*GetEntrySize());
		return pEntry->GetDataPointer();
	}

	const void* UnTypesafeFastPool::GetByIndex(uint32 uIndex) const
	{
		uint32 uBlock = uIndex / m_uGroupSize;
		ASSERT(uBlock < m_uBlockCount);
		uIndex -= (uBlock*m_uGroupSize);

		ASSERT(uIndex < m_uGroupSize);

		MemBlock* pBlock = m_pFirstMemBlock;
		for (uint32 i = 0; i < uBlock; i++)
		{
			pBlock = pBlock->pNext;
			ASSERT(pBlock != NULL);
		}

		auto pEntry = (EntryHeader*)((uint8*)pBlock->pEntries + uIndex*GetEntrySize());
		return pEntry->GetDataPointer();
	}

	uint32 UnTypesafeFastPool::GetIndex(void* pAlloc)
	{
		uint32 uOffset = 0;
		uint32 uBlock = 0;
		MemBlock* pBlock = m_pFirstMemBlock;
		while (pBlock)
		{
			const memsize blockSize = GetEntrySize() * m_uGroupSize;

			if ((memsize)pAlloc > (memsize)pBlock->pEntries && (memsize)pAlloc < ((memsize)pBlock->pEntries + blockSize))
			{
				uOffset = uBlock * m_uGroupSize;
				break;
			}
			pBlock = pBlock->pNext;
			uBlock++;
		}
		ASSERT(pBlock != NULL);

		memsize blockPos = (memsize(pBlock->pEntries));
		memsize allocPos = (memsize(pAlloc));
		memsize memOffset = allocPos - blockPos;
		memOffset /= GetEntrySize();

		return (uOffset + (uint32)memOffset);
	}

	void UnTypesafeFastPool::Clear()
	{
		UnTypesafeFastPool::UnTypeSafeIterator it = Begin();
		while (!it.IsEnd())
		{
			Free((*it));
			it = Begin();
		}
	}

}