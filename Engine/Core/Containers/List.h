/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A doubly linked list of items, uses the array pool for allocation
*****************************************************************************/
#ifndef _USG_CORE_CONTAINERS_LIST_H_
#define	_USG_CORE_CONTAINERS_LIST_H_

#include "Engine/Core/Utility.h"
#include "Engine/Memory/MemUtil.h"

namespace usg
{

// Temporary until we have sorted out allocations
struct EntryAlloc;
EntryAlloc* ListAllocArray(uint32 uCount);
void ListFreeArray(EntryAlloc* pEntry, uint32 uCount);
void InitListMemory();

template <class ListType>
class List
{
private:

	struct Entry
	{
		Entry*		pNext;
		Entry*		pPrev;
		ListType*	pData;
#ifdef DEBUG_ARRAY_POOL
		uint32 uPad;
#endif
	};

public:
	List();
	List(uint32 uGroupSize);
	~List();

	// Only to be called before any allocations
	void SetGroupSize(uint32 uGroupSize);

	class Iterator
	{
	public:
		Iterator(Entry* pEntry) { m_pCurrent = pEntry; }
		Iterator& operator++() { m_pCurrent = m_pCurrent->pNext; return *this; }
		Iterator& operator--() { m_pCurrent = m_pCurrent->pPrev; return *this; }

		 const ListType* operator*() const { return m_pCurrent->pData; }
		 ListType* operator*() { return m_pCurrent->pData; }
		 bool		IsEnd() { return m_pCurrent == NULL; }

	private:
		Entry*	m_pCurrent;
		friend class List<ListType>;
	};

	void InsertBefore(Iterator &it, ListType* pData);
	//void InsertAfter(Iterator &it, ListType* pData);
	void AddToFront(ListType* pData);
	void AddToEnd(ListType* pData);
	ListType* PopFront();
	//void AddToFront(ListType* pData);
	bool Remove(ListType* pData);
	Iterator Erase(Iterator& it);
	bool Contains(ListType* pData);
	void Sort();

	template<class ComparisonObject>
	inline void Sort(const ComparisonObject& cmp);

	void Clear();
	uint32 GetSize() const { return m_uSize; }

	Iterator	Begin() const { return Iterator(m_pFirst); }
	Iterator	End() const { return Iterator(m_pLast); }

private:

	struct MemBlock
	{
		Entry*		pEntries;
		MemBlock*	pNext;
	};

	void EnsureSpace();
	

	MemBlock*	m_pFirstMemBlock;
	uint32		m_uGroupSize;

	Entry*		m_pFirstFree;
	
	Entry*		m_pFirst;
	Entry*		m_pLast;

	uint32		m_uSize;
};

template<class ListType>
List<ListType>::List()
{
	m_pFirstFree	= NULL;
	m_pLast			= NULL;
	m_pFirst		= NULL;
	m_pFirstMemBlock = NULL;
	m_uGroupSize	= 64;	// 20 pointer groups for now, this should probably be exposed in the constructor
	m_uSize			= 0;
}

template <class ListType>
List<ListType>::List(uint32 uGroupSize)
{
	m_pFirstFree	= NULL;
	m_pLast			= NULL;
	m_pFirst		= NULL;
	m_pFirstMemBlock = NULL;
	m_uGroupSize	= uGroupSize;
	m_uSize			= 0;
}

template <class ListType>
NO_INLINE_TEMPL List<ListType>::~List()
{
	MemBlock* pBlock = m_pFirstMemBlock;
	while (pBlock)
	{
		MemBlock* pNext = pBlock->pNext;
		// TODO: Replace with proper allocation
		if (pBlock->pEntries)
		{
			utl::SafeArrayDelete(&pBlock->pEntries);
		}
		utl::SafeDelete(&pBlock);

		pBlock = pNext;
	}
}

template <class ListType>
NO_INLINE_TEMPL void List<ListType>::SetGroupSize(uint32 uSize)
{
	m_uGroupSize = uSize;
}

template <class ListType>
NO_INLINE_TEMPL void List<ListType>::EnsureSpace()
{
	if (!m_pFirstFree)
	{
		// TODO: Give lists their own memheap
		MemType eHeapType = MEMTYPE_STANDARD;
		// TODO: Proper memory allocation
		MemBlock* pBlock = pBlock = vnewHeap(ALLOC_LIST, eHeapType) MemBlock;
		pBlock->pEntries = vnewHeap(ALLOC_LIST, eHeapType) Entry[m_uGroupSize];

		pBlock->pNext = m_pFirstMemBlock;
		m_pFirstMemBlock = pBlock;

		m_pFirstFree = &pBlock->pEntries[0];
		m_pFirstFree->pPrev = NULL;
		m_pFirstFree->pNext = &pBlock->pEntries[1];
		for (uint32 i = 1; i < m_uGroupSize - 1; i++)
		{
			pBlock->pEntries[i].pPrev = &pBlock->pEntries[i - 1];
			pBlock->pEntries[i].pNext = &pBlock->pEntries[i + 1];

		}
		pBlock->pEntries[m_uGroupSize - 1].pPrev = &pBlock->pEntries[m_uGroupSize - 2];
		pBlock->pEntries[m_uGroupSize - 1].pNext = NULL;
	}
}

template <class ListType>
NO_INLINE_TEMPL void List<ListType>::InsertBefore(typename List<ListType>::Iterator &it, ListType* pData)
{
	if(it.IsEnd())
	{
		AddToEnd(pData);
	}
	else if(it.m_pCurrent->pPrev == NULL)
	{
		AddToFront(pData);
	}
	else
	{
		EnsureSpace();

		Entry* pNewEntry = m_pFirstFree;
		m_pFirstFree = m_pFirstFree->pNext;

		ASSERT(it.m_pCurrent != NULL && it.m_pCurrent->pPrev != NULL);
		pNewEntry->pData  = pData;
		pNewEntry->pNext = it.m_pCurrent;
		pNewEntry->pPrev = it.m_pCurrent->pPrev;
		pNewEntry->pPrev->pNext = pNewEntry;
		pNewEntry->pNext->pPrev = pNewEntry;

		m_uSize++;
	}
}

template <class ListType>
NO_INLINE_TEMPL void List<ListType>::AddToFront(ListType* pData)
{
	EnsureSpace();

	Entry* pNewEntry	= m_pFirstFree;
	m_pFirstFree		= m_pFirstFree->pNext;

	pNewEntry->pPrev	= NULL;
	pNewEntry->pNext	= m_pFirst;
	pNewEntry->pData	= pData;
	if(m_pFirst && m_pLast)
	{
		m_pFirst->pPrev	= pNewEntry;
	}
	else
	{
		if(!m_pLast && m_pFirst)
		{
			m_pLast = m_pFirst;
			m_pLast->pPrev = pNewEntry;
			m_pLast->pNext = NULL;
			pNewEntry->pNext = m_pLast;
			pNewEntry->pPrev = NULL;
		}
		else
		{
			ASSERT(!m_pFirst);
			pNewEntry->pPrev	= NULL;
			pNewEntry->pNext	= NULL;
		}
	}
	m_pFirst = pNewEntry;
	m_uSize++;
}

template <class ListType>
NO_INLINE_TEMPL ListType* List<ListType>::PopFront()
{
	if (m_pFirst)
	{
		ListType* pType = m_pFirst->pData;
		Remove(pType);
		return pType;
	}
	return NULL;
}

template <class ListType>
NO_INLINE_TEMPL void List<ListType>::AddToEnd(ListType* pData)
{
	EnsureSpace();
	
	Entry* pNewEntry	= m_pFirstFree;
	m_pFirstFree		= m_pFirstFree->pNext;

	pNewEntry->pPrev	= m_pLast;
	pNewEntry->pNext	= NULL;
	pNewEntry->pData	= pData;
	if(m_pLast && m_pFirst)
	{
		m_pLast->pNext	= pNewEntry;
		m_pLast = pNewEntry;
	}
	else
	{
		if(!m_pFirst)
		{
			m_pFirst = pNewEntry;
		}
		else
		{
			ASSERT(!m_pLast);
			m_pLast = pNewEntry;
			pNewEntry->pPrev	= m_pFirst;
			m_pFirst->pNext		= m_pLast;
		}
	}
	
	m_uSize++;
}


template <class ListType>
NO_INLINE_TEMPL bool List<ListType>::Remove(ListType* pData)
{
	for(Iterator it = Begin(); !it.IsEnd(); ++it)
	{
		if(*it == pData)
		{
			Erase(it);
			return true;
		}
	}

	return false;
}

template <class ListType>
NO_INLINE_TEMPL typename List<ListType>::Iterator List<ListType>::Erase(typename List<ListType>::Iterator& it)
{
	Entry* pEntry = it.m_pCurrent;
	Entry* pNextEntry = pEntry->pNext;

	if(pEntry == m_pFirst)
	{
		m_pFirst = pEntry->pNext;
	}

	if(pEntry == m_pLast)
	{
		m_pLast = pEntry->pPrev;
	}

	if(pEntry->pNext)
	{
		pEntry->pNext->pPrev = pEntry->pPrev;
	}
	if(pEntry->pPrev)
	{
		pEntry->pPrev->pNext = pEntry->pNext;
	}

	pEntry->pNext = m_pFirstFree;
	pEntry->pPrev = NULL;
	m_pFirstFree = pEntry;

	m_uSize--;

	return Iterator(pNextEntry);
}

template <class ListType>
NO_INLINE_TEMPL bool List<ListType>::Contains(ListType* pData)
{
	Entry* pEntry = m_pFirst;
	while(pEntry)
	{
		if(pEntry->pData == pData)
		{
			return true;
		}
		pEntry = pEntry->pNext;
	}

	return false;
}

template <class ListType>
NO_INLINE_TEMPL void List<ListType>::Sort()
{
	ListType* pSwap;
	Entry* pEntry;
	Entry* pNext;

	bool bSwapped = false;
	do
	{
		bSwapped = false;
		pEntry = m_pFirst;
		while(pEntry && pEntry->pNext)
		{
			pNext = pEntry->pNext;
			// Don't actually re-order the pointers, it's quicker for most purposes to
			// switch the data they are pointing to instead, especially if the list was always appended to the end
			if( *pNext->pData < *pEntry->pData )
			{
				pSwap = pEntry->pData;
				pEntry->pData = pNext->pData;
				pNext->pData = pSwap;
				bSwapped = true;
			}
			pEntry = pNext;
		}

	}while(bSwapped);

}

template <class ListType>
template <class ComparisonObject>
NO_INLINE_TEMPL void List<ListType>::Sort(const ComparisonObject& cmp)
{
	ListType* pSwap;
	Entry* pEntry;
	Entry* pNext;

	bool bSwapped = false;
	do
	{
		bSwapped = false;
		pEntry = m_pFirst;
		while(pEntry && pEntry->pNext)
		{
			pNext = pEntry->pNext;
			// Don't actually re-order the pointers, it's quicker for most purposes to
			// switch the data they are pointing to instead, especially if the list was always appended to the end
			if( cmp(*pNext->pData, *pEntry->pData) )
			{
				pSwap = pEntry->pData;
				pEntry->pData = pNext->pData;
				pNext->pData = pSwap;
				bSwapped = true;
			}
			pEntry = pNext;
		}

	}while(bSwapped);

}

template <class ListType>
NO_INLINE_TEMPL void List<ListType>::Clear()
{
	m_pFirst = NULL;
	m_pLast = NULL;
	m_uSize = 0;

	if (m_pFirstMemBlock)
	{
		m_pFirstFree = &m_pFirstMemBlock->pEntries[0];
	}
	else
	{
		m_pFirstFree = NULL;
	}
	

//	Entry* pPrev = NULL;
	MemBlock* pBlock = m_pFirstMemBlock;
	MemBlock* pPrev = NULL;
	while(pBlock)
	{
		if(pPrev == NULL)
		{
			pBlock->pEntries[0].pPrev = NULL;
		}
		else
		{
			pBlock->pEntries[0].pPrev = &pPrev->pEntries[m_uGroupSize-1];
		}

		pBlock->pEntries[0].pNext = &pBlock->pEntries[1];

		for(uint32 i=1; i<m_uGroupSize-1; i++)
		{
			pBlock->pEntries[i].pPrev = &pBlock->pEntries[i-1];
			pBlock->pEntries[i].pNext = &pBlock->pEntries[i+1];
			pBlock->pEntries[i].pData = NULL;
		}
		if(pBlock->pNext == NULL)
		{
			pBlock->pEntries[m_uGroupSize-1].pNext = NULL;
		}
		else
		{
			pBlock->pEntries[m_uGroupSize-1].pNext = &pBlock->pNext->pEntries[0];
		}
//		pPrev = &m_pEntries[uList][m_uGroupSize-1];
		pPrev = pBlock;
		pBlock = pBlock->pNext;
	}
}

}


#endif

