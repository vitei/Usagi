/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An array which can be filled to any size, but is then "locked"
//	             to that size, after which elements cannot be added.
//	Warning: Until the array is locked it is using Scratch Memory, which means
//	         constructors/destructors aren't called.  If you call Clear() or
//	         delete the LockedArray before calling Lock(), the destructors of
//	         its objects will not be called!  If you Clear()/delete after locking,
//	         they will.
//	         Also note that any pointers/references you take to elements before
//	         locking the array will be invalidated after locking.
*****************************************************************************/

#ifndef _USG_CORE_CONTAINERS_LOCKEDARRAY_H_
#define _USG_CORE_CONTAINERS_LOCKEDARRAY_H_

#include "Engine/Memory/ScratchRaw.h"
#include "Engine/Memory/MemUtil.h"

namespace usg
{

template<typename T, size_t CHUNK_SIZE = 20>
class LockedArray
{
public:
	LockedArray();
	~LockedArray();

	size_t GetSize() const;

	const T& operator[](const size_t uIndex) const;
	T& operator[](const size_t uIndex);

	void Lock();
	void Clear();
private:
	PRIVATIZE_COPY(LockedArray)

	struct ScratchNode
	{
		T*           pData;
		size_t       uOffset;
		size_t       uSize;
		ScratchNode* pNext;
	};

	union
	{
		T*           m_pData;      // When locked
		ScratchNode* m_pFirstNode; // When not locked
	};

	ScratchNode* m_pLastNode;

	size_t m_uSize;
	size_t m_uCapacity;
	bool   m_bLocked;

	void ClearScratchList();
	T& GetIndexRef(const size_t uIndex) const;
	T& GetIndexRefFromScratch(const size_t uIndex) const;
	void GrowArrayToFitIndex(const size_t uIndex);
};

template<typename T, size_t CHUNK_SIZE>
LockedArray<T, CHUNK_SIZE>::LockedArray()
	: m_pData(NULL)
	, m_pLastNode(NULL)
	, m_uSize(0)
	, m_uCapacity(0)
	, m_bLocked(false)
{
}

template<typename T, size_t CHUNK_SIZE>
LockedArray<T, CHUNK_SIZE>::~LockedArray()
{
	Clear();
}

template<typename T, size_t CHUNK_SIZE>
size_t LockedArray<T, CHUNK_SIZE>::GetSize() const
{
	return m_uSize;
}

template<typename T, size_t CHUNK_SIZE>
inline const T& LockedArray<T, CHUNK_SIZE>::operator[](const size_t uIndex) const
{
	ASSERT(uIndex < m_uSize);
	return GetIndexRef(uIndex);
}

template<typename T, size_t CHUNK_SIZE>
inline T& LockedArray<T, CHUNK_SIZE>::operator[](const size_t uIndex)
{
	if(uIndex >= m_uSize)
	{
		if(uIndex >= m_uCapacity)
		{
			GrowArrayToFitIndex(uIndex);
		}

		m_uSize = uIndex + 1;
	}

	return GetIndexRef(uIndex);
}

template<typename T, size_t CHUNK_SIZE>
void LockedArray<T, CHUNK_SIZE>::Lock()
{
	ASSERT(!m_bLocked);

	if(m_uSize > 0)
	{
		T* pNewData = vnew(ALLOC_OBJECT) T[m_uSize];

		ScratchNode* pNode = m_pFirstNode;
		while(pNode != NULL)
		{
			const size_t uRemainder = m_uSize - pNode->uOffset;
			const size_t uNumToCopy = uRemainder < pNode->uSize ? uRemainder : pNode->uSize;
			MemCpy(&pNewData[pNode->uOffset], pNode->pData, uNumToCopy * sizeof(T));
			pNode = pNode->pNext;
		}

		ClearScratchList();

		m_pData = pNewData;
		m_uCapacity = m_uSize;
	}

	m_bLocked = true;
}

template<typename T, size_t CHUNK_SIZE>
void LockedArray<T, CHUNK_SIZE>::Clear()
{
	if(m_pData != NULL)
	{
		if(m_bLocked)
		{
			vdelete[] m_pData;
		}
		else
		{
			ClearScratchList();
		}
	}

	m_pData = NULL;
	m_uSize = 0;
	m_uCapacity = 0;
	m_bLocked = false;
	m_pLastNode = NULL;
}

template<typename T, size_t CHUNK_SIZE>
void LockedArray<T, CHUNK_SIZE>::ClearScratchList()
{
	ASSERT(!m_bLocked);
	ScratchNode* pNode = m_pFirstNode;

	while(pNode != NULL)
	{
		ScratchNode* pNext = pNode->pNext;

		ScratchRaw::Free((void**)&pNode->pData);
		ScratchRaw::Free((void**)&pNode);
		pNode = pNext;
	}

	m_pLastNode  = NULL;
	m_pFirstNode = NULL;
}

template<typename T, size_t CHUNK_SIZE>
inline T& LockedArray<T, CHUNK_SIZE>::GetIndexRef(const size_t uIndex) const
{
	return m_bLocked ? m_pData[uIndex] : GetIndexRefFromScratch(uIndex);
}

template<typename T, size_t CHUNK_SIZE>
T& LockedArray<T, CHUNK_SIZE>::GetIndexRefFromScratch(const size_t uIndex) const
{
	ScratchNode* pNode = m_pFirstNode;
	T* out = NULL;

	while(pNode != NULL)
	{
		ASSERT(uIndex >= pNode->uOffset);
		const size_t uOffsetIndex = uIndex - pNode->uOffset;
		if(uOffsetIndex < pNode->uSize)
		{
			out = &pNode->pData[uOffsetIndex];
			return *out;
		}

		pNode = pNode->pNext;
	}

	DEBUG_PRINT("Couldn't get node at index %d\n", uIndex);
	ASSERT(false);
	return *out; // NOTE should never get here; and if we do we're dereferencing NULL :-/
}

template<typename T, size_t CHUNK_SIZE>
void LockedArray<T, CHUNK_SIZE>::GrowArrayToFitIndex(const size_t uIndex)
{
	ASSERT(!m_bLocked);
	const size_t uIndexCapacityDifference = (uIndex + 1) - m_uCapacity;
	const size_t uNewChunkSize = uIndexCapacityDifference > CHUNK_SIZE ? uIndexCapacityDifference : CHUNK_SIZE;
	ScratchNode* pNewScratchNode = NULL;
	ScratchRaw::Init((void**)&pNewScratchNode, sizeof(ScratchNode), 4);

	pNewScratchNode->pData   = NULL;
	pNewScratchNode->uSize   = uNewChunkSize;
	pNewScratchNode->uOffset = m_pLastNode == NULL ? 0 : m_pLastNode->uOffset + m_pLastNode->uSize;
	pNewScratchNode->pNext   = NULL;

	ScratchRaw::Init((void**)&pNewScratchNode->pData, uNewChunkSize * sizeof(T), 4);
	MemClear(pNewScratchNode->pData, uNewChunkSize * sizeof(T));

	if(m_pFirstNode == NULL)
	{
		ASSERT(m_pLastNode == NULL);
		m_pFirstNode = m_pLastNode = pNewScratchNode;
	}
	else
	{
		ASSERT(m_pLastNode != NULL);
		m_pLastNode->pNext = pNewScratchNode;
		m_pLastNode = pNewScratchNode;
	}

	m_uCapacity = m_uCapacity + uNewChunkSize;
}

}

#endif //_USG_CORE_CONTAINERS_LOCKEDARRAY_H_
