/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A memory pool which uses a little more memory but which
//	should avoid cache misses when traversing as much as possible by using
//	an interator which traverses through the objects in memory order
*****************************************************************************/
#ifndef _USG_MEMORY_FAST_POOL_H_
#define _USG_MEMORY_FAST_POOL_H_
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"


namespace usg{

template <class AllocType>
class FastPool
{
public:
	// Enable optimize for iteration where objects will rarely be added or removed, disable it when you want to
	// iterate through them in memory order
	FastPool(uint32 uGroupSize, bool bCanResize = true, bool bOptimizeForItr = true);
	~FastPool();

	AllocType*	Alloc();
	void	Free(AllocType* p);
	uint32	Size() const { return m_uSize; }
	bool CanAlloc() { return (m_pFirstFree!=NULL) || m_bCanResize; }

	// Use this wisely, no guarantee it's not been freed
	AllocType* GetByIndex(uint32 uIndex);
	const AllocType* GetByIndex(uint32 uIndex) const;
	uint32 GetIndex(AllocType* pAlloc);
	void Clear();

private:
	PRIVATIZE_COPY(FastPool);


	struct Entry
	{
		Entry*		pNext;
		Entry*		pPrev;

		AllocType	data;
	};

	void	AddMemoryBlock();
	Entry*	GetHeaderFromData(AllocType* pData)
	{
		uint8* pHeader = (uint8*)pData;
		pHeader -= (sizeof(Entry*)*2);
		return (Entry*)pHeader;
	}

	struct MemBlock
	{
		Entry*		pEntries;
		MemBlock*	pNext;
	};

	MemBlock*			m_pFirstMemBlock;

	Entry*				m_pFirstAllocated;
	Entry*				m_pFirstFree;

	uint32				m_uBlockCount;
	uint32				m_uGroupSize;
	uint32				m_uSize;
	bool				m_bCanResize;
	bool				m_bOptimseForIttr;

public:
	// Begin code relating to itteration of the pool elements
	class Iterator
	{
	public:
		Iterator(Entry* pEntry) { m_pCurrent = pEntry; }
		Iterator& operator++() { m_pCurrent = m_pCurrent->pNext; return *this; }
		Iterator& operator--() { m_pCurrent = m_pCurrent->pPrev; return *this; }

		 const AllocType* operator*() const { return &m_pCurrent->data; }
		 AllocType* operator*() { return &m_pCurrent->data; }
		 bool		IsEnd() { return m_pCurrent == NULL; }

	protected:
		Entry*		m_pCurrent;
	};

	// Allows deleting of unwanted elements
	class DynamicIterator : public Iterator
	{
	public:
		DynamicIterator(Entry* pEntry, FastPool* pPool) : Iterator(pEntry) { m_pPool = pPool;  m_pPrev = NULL;  m_pNext = NULL; }
		DynamicIterator& operator++() { Iterator::m_pCurrent = Iterator::m_pCurrent ? Iterator::m_pCurrent->pNext : m_pNext; m_pNext = NULL; m_pPrev = NULL; return *this; }
		DynamicIterator& operator--() { Iterator::m_pCurrent = Iterator::m_pCurrent ? Iterator::m_pCurrent->pPrev : m_pPrev; m_pNext = NULL; m_pPrev = NULL; return *this; }

		 const AllocType* operator*() const { return &Iterator::m_pCurrent->data; }
		 AllocType* operator*() { return &Iterator::m_pCurrent->data; }
		 bool		IsStart() { return (Iterator::m_pCurrent == NULL && m_pPrev == NULL); }
		 bool		IsEnd() { return (Iterator::m_pCurrent == NULL && m_pNext == NULL); }

		 void RemoveElement()
		 { 
			m_pNext = Iterator::m_pCurrent->pNext;
			m_pPrev = Iterator::m_pCurrent->pPrev;
			m_pPool->Free(&Iterator::m_pCurrent->data);
			Iterator::m_pCurrent = NULL;
		}

	private:
		Entry* 		m_pNext;
		Entry* 		m_pPrev;
		FastPool*	m_pPool;

	};

	Iterator		Begin() const { return Iterator(m_pFirstAllocated); }
	// For calling memory initialization
	Iterator		EmptyBegin() const { return Iterator(m_pFirstFree); }
	DynamicIterator	BeginDynamic() { return DynamicIterator(m_pFirstAllocated, this); }
};


template <class AllocType>
FastPool<AllocType>::FastPool(uint32 uGroupSize, bool bCanResize, bool bOptimseForItr)
:m_uBlockCount(0),
m_uGroupSize(uGroupSize)
{
	m_uSize = 0;
	m_pFirstFree	= NULL;
	m_pFirstMemBlock = NULL;
	if(!bCanResize)
		AddMemoryBlock();
	m_pFirstAllocated	= NULL;
	m_bCanResize = bCanResize;
	m_bOptimseForIttr = bOptimseForItr;
}

template <class AllocType>
FastPool<AllocType>::~FastPool()
{
	MemBlock* pBlock = m_pFirstMemBlock;
	while(pBlock)
	{
		MemBlock* pNext = pBlock->pNext;
		// TODO: Replace with proper allocation
		if(pBlock->pEntries)
		{
			utl::SafeArrayDelete(&pBlock->pEntries);
		}
		pBlock = pNext;
	}

	pBlock = m_pFirstMemBlock;
	while (pBlock)
	{
		MemBlock* pNext = pBlock->pNext;
		utl::SafeDelete(&pBlock);
		pBlock = pNext;
	}
}

template <class AllocType>
void FastPool<AllocType>::AddMemoryBlock()
{
	MemType eHeapType = MEMTYPE_STANDARD;

	// Allocate a new block
	MemBlock* pBlock = vnewHeap(ALLOC_FASTPOOL, eHeapType) MemBlock;
	pBlock->pEntries = vnewHeap(ALLOC_FASTPOOL, eHeapType) Entry[m_uGroupSize];

	pBlock->pNext = NULL;

	// Find the insertion point
	if(!m_pFirstMemBlock)
	{
		m_pFirstMemBlock = pBlock;
	}
	else
	{
		MemBlock* pInsertionPoint = m_pFirstMemBlock;
		while(pInsertionPoint->pNext)
		{
			pInsertionPoint = pInsertionPoint->pNext;
		}
		pInsertionPoint->pNext = pBlock;
	}

	Entry* pHeader = (Entry*)pBlock->pEntries;

	if(!m_pFirstFree)
	{
		m_pFirstFree = pHeader;
		pHeader->pPrev = NULL;
	}
	else
	{
		Entry* pEntry = m_pFirstFree;
		while(pEntry->pNext)
		{
			pEntry = pEntry->pNext;
		}
		pEntry->pNext = pHeader;
		pHeader->pPrev = pEntry;
	}

	pHeader->pNext = &pBlock->pEntries[1];

	pHeader++;
	for(uint32 i=1; i<m_uGroupSize-1; i++)
	{
		pHeader->pNext = pHeader+1;
		pHeader->pPrev = pHeader-1;

		pHeader++;
	}

	pBlock->pEntries[m_uGroupSize-1].pPrev = &pBlock->pEntries[m_uGroupSize-2];
	pBlock->pEntries[m_uGroupSize-1].pNext = NULL;

	m_uBlockCount++;
}

template <class AllocType>
AllocType* FastPool<AllocType>::Alloc()
{
	if(!m_pFirstFree)
	{
		if(m_bCanResize || (m_pFirstMemBlock==NULL))
		{
			AddMemoryBlock();
		}
	}
	Entry* pAlloc = m_pFirstFree;
	ASSERT(pAlloc!=NULL);
	if(pAlloc)
	{	
		// First remove us from the free list
		m_pFirstFree = pAlloc->pNext;
		if(m_pFirstFree)
		{
			m_pFirstFree->pPrev = NULL;
		}

		if( !m_pFirstAllocated || (m_pFirstAllocated > pAlloc))
		{	
			// We're the first one
			pAlloc->pNext = m_pFirstAllocated;
			pAlloc->pPrev = NULL;
			m_pFirstAllocated = pAlloc;
		}
		else
		{
			Entry* cmp = m_pFirstAllocated;
			while( m_bOptimseForIttr && (cmp->pNext != NULL) && (cmp->pNext < pAlloc) )	
			{
				cmp = cmp->pNext;
			}

			// We want to be inserted after cmp to maintain a good cache consistency
			pAlloc->pNext =  cmp->pNext;
			pAlloc->pPrev = cmp;
		}
		if(pAlloc->pNext)
		{
			pAlloc->pNext->pPrev = pAlloc;
		}
		if(pAlloc->pPrev)
		{
			pAlloc->pPrev->pNext = pAlloc;
		}
	}

	if(pAlloc)
	{
		m_uSize++;
	}
	else
	{
		return NULL;
	}
	return &pAlloc->data;
}

template <class AllocType>
void FastPool<AllocType>::Free(AllocType* pData)
{
	Entry* pEntry = GetHeaderFromData(pData);
	Entry* pPrev = pEntry->pPrev;
	Entry* pNext = pEntry->pNext;

	m_uSize--;

	// Manually call the destructor
	//pData->~AllocType();
	// First remove us from the active list
	if(pPrev)
	{
		pPrev->pNext = pNext;
	}
	else
	{
		// This was the first item in the list, so update the first item to be our next pointer
		ASSERT(m_pFirstAllocated == pEntry);
		m_pFirstAllocated = pNext;
	}

	if(pNext)
	{
		pNext->pPrev = pPrev;
	}

	if(m_pFirstFree)
	{
		m_pFirstFree->pPrev = pEntry;
	}

	pEntry->pNext = m_pFirstFree;
	pEntry->pPrev = NULL;
	m_pFirstFree = pEntry;
}


template <class AllocType>
AllocType* FastPool<AllocType>::GetByIndex(uint32 uIndex)
{
	uint32 uBlock = uIndex/m_uGroupSize;
	ASSERT(uBlock < m_uBlockCount);
	uIndex -= (uBlock*m_uGroupSize);

	ASSERT(uIndex < m_uGroupSize);

	MemBlock* pBlock = m_pFirstMemBlock;
	for(uint32 i=0; i<uBlock; i++)
	{
		pBlock = pBlock->pNext;
		ASSERT(pBlock!=NULL);
	}

	return &pBlock->pEntries[uIndex].data;
}

template <class AllocType>
const AllocType* FastPool<AllocType>::GetByIndex(uint32 uIndex) const
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

	return &pBlock->pEntries[uIndex].data;
}

template <class AllocType>
uint32 FastPool<AllocType>::GetIndex(AllocType* pAlloc)
{
	uint32 uOffset = 0;
	uint32 uBlock = 0;
	MemBlock* pBlock = m_pFirstMemBlock;
	while(pBlock)
	{
		memsize blockSize = sizeof(Entry)*m_uGroupSize;

		if( (memsize)pAlloc > (memsize)pBlock->pEntries && (memsize)pAlloc < ((memsize)pBlock->pEntries + blockSize ) )
		{
			uOffset = uBlock * m_uGroupSize;
			break;
		}
		pBlock = pBlock->pNext;
		uBlock++;
	}
	ASSERT(pBlock!=NULL);

	memsize blockPos = (memsize(pBlock->pEntries));
	memsize allocPos = (memsize(pAlloc));
	memsize memOffset = allocPos-blockPos;
	memOffset /= sizeof(Entry);

	return (uOffset + (uint32)memOffset);
}

template <class AllocType>
void FastPool<AllocType>::Clear()
{
	FastPool<AllocType>::Iterator it = Begin();
	while(!it.IsEnd())
	{
		Free((*it));
		it = Begin();
	}
}

}

#endif
