/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A memory pool which uses a little more memory but which
//	should avoid cache misses when traversing as much as possible by using
//	an interator which traverses through the objects in memory order
*****************************************************************************/
#ifndef _USG_MEMORY_FAST_POOL_NON_TYPE_SAFEH_
#define _USG_MEMORY_FAST_POOL_NON_TYPE_SAFEH_
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"

namespace usg {

	class UnTypesafeFastPool
	{
	protected:
		void(*m_pElementInitializer)(void*);
		void(*m_pElementDeinitializer)(void*);
	public:
		// Enable optimize for iteration where objects will rarely be added or removed, disable it when you want to
		// iterate through them in memory order
		UnTypesafeFastPool(size_t uElementSize, uint32 uGroupSize, bool bCanResize = true, bool bOptimizeForItr = true, decltype(m_pElementInitializer) pElementArrayAllocator = nullptr,
			decltype(m_pElementDeinitializer) pDeallocator = nullptr);
		~UnTypesafeFastPool();

		void*	Alloc();
		void	Free(void* p);

		uint32	Size() const { return m_uSize; }
		bool CanAlloc() { return (m_pFirstFree != NULL) || m_bCanResize; }

		// Use this wisely, no guarantee it's not been freed
		void* GetByIndex(uint32 uIndex);
		const void* GetByIndex(uint32 uIndex) const;
		uint32 GetIndex(void* pAlloc);
		void Clear();
	private:
		const size_t m_uElementSize;

		struct EntryHeader
		{
			EntryHeader* pNext;
			EntryHeader* pPrev;
			void* GetDataPointer()
			{
				return ((uint8*)this) + sizeof(EntryHeader);
			}
		};

		size_t GetEntrySize() const
		{
			return sizeof(EntryHeader) + m_uElementSize;
		}

		void	AddMemoryBlock();

		EntryHeader* GetHeaderFromData(void* pData)
		{
			uint8* pHeader = (uint8*)pData;
			pHeader -= (sizeof(EntryHeader*) * 2);
			return (EntryHeader*)pHeader;
		}

		struct MemBlock
		{
			void*		pEntries; // Pointer to an array where each element size is sizeof(EntryHeader) + m_uElementSize
			MemBlock*	pNext;
		};

		MemBlock*			m_pFirstMemBlock;

		EntryHeader*		m_pFirstAllocated;
		EntryHeader*		m_pFirstFree;

		uint32				m_uBlockCount;
		const uint32		m_uGroupSize;
		uint32				m_uSize;
		bool				m_bCanResize;
		bool				m_bOptimseForIttr;

		EntryHeader* Advance(EntryHeader* pEntry)
		{
			uint8* ptr = (uint8*)pEntry;
			ptr += GetEntrySize();
			return (EntryHeader*)ptr;
		}

		EntryHeader* Back(EntryHeader* pEntry)
		{
			uint8* ptr = (uint8*)pEntry;
			ptr -= GetEntrySize();
			return (EntryHeader*)ptr;
		}


	public:
		// Begin code relating to itteration of the pool elements
		template<typename AllocType>
		class Iterator
		{
		public:
			Iterator(EntryHeader* pEntry) { m_pCurrent = pEntry; }
			Iterator& operator++() { m_pCurrent = m_pCurrent->pNext; return *this; }
			Iterator& operator--() { m_pCurrent = m_pCurrent->pPrev; return *this; }

			const AllocType* operator*() const { return (const AllocType*)m_pCurrent->GetDataPointer(); }
			AllocType* operator*() { return (AllocType*)m_pCurrent->GetDataPointer(); }
			bool IsEnd() { return m_pCurrent == NULL; }

		protected:
			EntryHeader*		m_pCurrent;
		};

		class UnTypeSafeIterator
		{
		public:
			UnTypeSafeIterator(EntryHeader* pEntry) { m_pCurrent = pEntry; }
			UnTypeSafeIterator& operator++() { m_pCurrent = m_pCurrent->pNext; return *this; }
			UnTypeSafeIterator& operator--() { m_pCurrent = m_pCurrent->pPrev; return *this; }

			const void* operator*() const { return m_pCurrent->GetDataPointer(); }
			void* operator*() { return m_pCurrent->GetDataPointer(); }
			bool IsEnd() { return m_pCurrent == NULL; }

		protected:
			EntryHeader*		m_pCurrent;
		};

		// Allows deleting of unwanted elements
		template<typename AllocType>
		class DynamicIterator : public Iterator<AllocType>
		{
		public:
			DynamicIterator(EntryHeader* pEntry, UnTypesafeFastPool* pPool) : Iterator<AllocType>(pEntry) { m_pPool = pPool;  m_pPrev = NULL;  m_pNext = NULL; }
			DynamicIterator& operator++() { Iterator<AllocType>::m_pCurrent = Iterator<AllocType>::m_pCurrent ? Iterator<AllocType>::m_pCurrent->pNext : m_pNext; m_pNext = NULL; m_pPrev = NULL; return *this; }
			DynamicIterator& operator--() { Iterator<AllocType>::m_pCurrent = Iterator<AllocType>::m_pCurrent ? Iterator<AllocType>::m_pCurrent->pPrev : m_pPrev; m_pNext = NULL; m_pPrev = NULL; return *this; }

			const AllocType* operator*() const { return (const AllocType*)Iterator<AllocType>::m_pCurrent->GetDataPointer(); }
			AllocType* operator*()
			{
				return (AllocType*)Iterator<AllocType>::m_pCurrent->GetDataPointer();
			}
			bool		IsStart() { return (Iterator<AllocType>::m_pCurrent == NULL && m_pPrev == NULL); }
			bool		IsEnd() { return (Iterator<AllocType>::m_pCurrent == NULL && m_pNext == NULL); }

			void RemoveElement()
			{
				m_pNext = Iterator<AllocType>::m_pCurrent->pNext;
				m_pPrev = Iterator<AllocType>::m_pCurrent->pPrev;
				m_pPool->Free(Iterator<AllocType>::m_pCurrent->GetDataPointer());
				Iterator<AllocType>::m_pCurrent = NULL;
			}

		private:
			EntryHeader* 		m_pNext;
			EntryHeader* 		m_pPrev;
			UnTypesafeFastPool*	m_pPool;

		};

		template<typename AllocType>
		Iterator<AllocType>	Begin() const { return Iterator<AllocType>(m_pFirstAllocated); }

		UnTypeSafeIterator Begin() const { return UnTypeSafeIterator(m_pFirstAllocated); }

		template<typename AllocType>
		Iterator<AllocType>	EmptyBegin() const { return Iterator<AllocType>(m_pFirstFree); }

		template<typename AllocType>
		DynamicIterator<AllocType> _BeginDynamic() { return DynamicIterator<AllocType>(m_pFirstAllocated, this); }
	};

	template<typename AllocType>
	class FastPoolLight : public UnTypesafeFastPool
	{
	public:

		typedef typename UnTypesafeFastPool::DynamicIterator<AllocType> DynamicIterator;

		// Enable optimize for iteration where objects will rarely be added or removed, disable it when you want to
		// iterate through them in memory order
		FastPoolLight(uint32 uGroupSize, bool bCanResize = true, bool bOptimizeForItr = true) : UnTypesafeFastPool(sizeof(AllocType), uGroupSize, bCanResize, bOptimizeForItr)
		{
			m_pElementInitializer = [](void* ptr) {
				new(ptr)AllocType();
			};

			m_pElementDeinitializer = [](void* pEntryRaw) {
				AllocType* pEntry = (AllocType*)pEntryRaw;
				pEntry->~AllocType();
			};
		}

		DynamicIterator BeginDynamic()
		{
			return _BeginDynamic<AllocType>();
		}

	};
}

#endif
