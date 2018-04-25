/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef STRING_POINTER_HASH_H
#define STRING_POINTER_HASH_H

#include "Engine/Common/Common.h"
#include "Engine/Memory/MemHeap.h"
#include "Engine/Core/String/StringCRC.h"

#define HASH_AMORTIZED_INSERT_RATE (3)
#define HASH_MAX_LOAD (.7f)
#define HASH_MAX_LOAD_AMORTIZING (.9f)
#define HASH_GROWTH_RATE (1.62f) // Golden ratio works best

namespace usg
{

// Hash table that maps pointers to hashes

template <class T>
class StringPointerHash
{
	struct StringPointerHashNode
	{
		string_crc hash;
		T data;

		StringPointerHashNode* next;
	};

	StringPointerHashNode* Find(string_crc crc) const
	{
		if (Count() == 0)
			return 0;

		int idx = 0;
		StringPointerHashNode* node = nullptr;

		// If old table exists
		if (m_oldData != 0)
		{
			idx = crc.Get() % m_oldMax;
			node = &m_oldData[idx];
			while (node && node->hash.Get() != 0)
			{
				if (node->hash == crc)
					return node;
				node = node->next;
			}
		}

		// Check current table
		idx = crc.Get() % m_maxElements;
		node = &m_data[idx];
		while (node && node->hash.Get() != 0)
		{
			if (node->hash == crc)
				return node;
			node = node->next;
		}

		return 0;
	}

	void FreeOldTable()
	{
		if (m_oldData != nullptr)
		{
			for (int i = 0; i < m_oldMax; i++)
			{
				StringPointerHashNode* node = m_oldData[i].next;
				while (node)
				{
					StringPointerHashNode* del = node;
					node = node->next;

					m_allocator->Deallocate(del);
				}
			}
			m_allocator->Deallocate(m_oldData);
		}
		m_oldMax = 0;
		m_oldRemainingIndex = 0;
		m_oldData = 0;
	}

	void Allocate()
	{
		ASSERT(m_allocator != nullptr);

		const memsize allocSize = sizeof(StringPointerHashNode) * m_uHashStartingSize;

		// Allocate the table
		m_data = (StringPointerHashNode*)m_allocator->Allocate(allocSize, 4, 0, ALLOC_STRING_POINTER_HASH);

		MemSet(m_data, 0, allocSize);
		m_currentElements = 0;
		m_maxElements = m_uHashStartingSize;

		m_oldData = 0;
		m_oldMax = 0;
		m_oldActiveElements = 0;

		m_currentAmortizing = false;
	}

	void AllocateNewTable()
	{
		ASSERT(m_oldData == nullptr);

		// Save old data first
		m_oldData = m_data;
		m_oldMax = m_maxElements;
		m_oldActiveElements = m_currentElements;

		// Increase max elements
		m_maxElements = (sint32)(m_maxElements * HASH_GROWTH_RATE);

		const memsize allocSize = sizeof(StringPointerHashNode) * m_maxElements;
		
		// Allocate the new table
		m_data = (StringPointerHashNode*)m_allocator->Allocate(allocSize, 4, 0, ALLOC_STRING_POINTER_HASH);

		// memset it
		MemSet(m_data, 0, allocSize);

		// Save old data
		m_oldRemainingIndex = 0;
		m_currentElements = 0;
	}

	// Pull elements from the old table into the new table
	void Amortize()
	{
		if (m_oldData != 0 && m_currentAmortizing == false)
		{
			m_currentAmortizing = true;
			bool tableDone = false;
			// dpw 20160426 -- HASH_AMORTIZED_INSERT_RATE here causes a crash when not tuned
			//                 correctly.  For now I'm just ignoring it; if insertions become
			//                 a problem consider re-introducing it, but you will have to fix
			//                 the crash (to do with m_oldData not being cleared) if you do!
			for (int i = 0; !tableDone /*&& i < HASH_AMORTIZED_INSERT_RATE*/; i++)
			{
				StringPointerHashNode* node = &m_oldData[m_oldRemainingIndex];
				if (node->hash.Get() != 0 && m_oldActiveElements > 0)
				{
					// Add the old node value into the new table
					Insert(node->hash, node->data);

					if (node->next != 0)
					{
						StringPointerHashNode* next = node->next;
						node->hash = next->hash;
						node->data = next->data;
						node->next = next->next;

						// Free the node
						m_allocator->Deallocate(next);
					}
					else
					{
						m_oldData[m_oldRemainingIndex].hash.Clear();
						m_oldData[m_oldRemainingIndex].data = 0;
						m_oldRemainingIndex++;
					}
					m_oldActiveElements--;
				}
				else if (m_oldActiveElements > 0)
				{
					m_oldRemainingIndex++;
				}

				tableDone = (m_oldRemainingIndex >= m_oldMax || m_oldActiveElements <= 0);
			}
			m_currentAmortizing = false;
		}
		if (m_oldActiveElements == 0)
		{
			FreeOldTable();
		}
	}
public:
	StringPointerHash<T>(uint32 uHashStartingSize = 32)
		: m_uHashStartingSize(uHashStartingSize)
		, m_data(nullptr)
		, m_oldData(nullptr)
		, m_allocator(nullptr)
		, m_currentAmortizing(false)
		, m_oldRemainingIndex(0)
		, m_oldActiveElements(0)
		, m_oldMax(0)
		, m_currentElements(0)
		, m_maxElements(0)
	{
	}

	~StringPointerHash<T>()
	{
		Clear();
	}

	class Iterator
	{
	public:
		Iterator(StringPointerHash<T>* hash)
			: m_hash(hash), m_slotIdx(0), m_node(&hash->m_data[0])
		{
			if(m_node && m_node->data == nullptr) { ++(*this); }
		}

		Iterator& operator++()
		{
			m_node = m_node->next;
			while((m_node == nullptr || m_node->data == nullptr) && m_slotIdx < m_hash->m_maxElements)
			{
				if(m_node != nullptr)
				{
					m_node = m_node->next;
				}
				else
				{
					m_slotIdx++;

					if(m_slotIdx < m_hash->m_maxElements)
					{
						m_node = &m_hash->m_data[m_slotIdx];
					}
					else
					{
						m_node = nullptr;
					}
				}
			}

			return *this;
		}

		const T* operator*() const { return &m_node->data; }
		T* operator*() { return &m_node->data; }
		bool IsEnd() const { return m_node == nullptr; }
		string_crc GetKey() const {
			return m_node->hash.Get();
		}
	private:
		StringPointerHash<T>*  m_hash;
		sint32                 m_slotIdx;
		StringPointerHashNode* m_node;
	};
	friend class Iterator;

	Iterator Begin() { ASSERT(m_oldData == nullptr); return Iterator(this); }

	void SetAllocator(MemHeap* heap)
	{
		ASSERT(heap != 0);
		m_allocator = heap;
	}

	void Clear()
	{
		Shutdown();
	}
	bool IsEmpty() const
	{
		return (m_currentElements == 0);
	}
	int Count() const
	{
		return m_currentElements + m_oldActiveElements;
	}
	void Shutdown()
	{
		FreeOldTable();
		m_currentElements = 0;
		m_oldActiveElements = 0;

		if (m_data != nullptr)
		{
			for (int i = 0; i < m_maxElements; i++)
			{
				StringPointerHashNode* node = m_data[i].next;
				while (node)
				{
					StringPointerHashNode* del = node;
					node = node->next;

					// Free the node
					m_allocator->Deallocate(del);
				}
			}

			// Free the table
			m_allocator->Deallocate(m_data);
		}
		m_data = nullptr;
		m_maxElements = 0;
	}
	bool Insert(char* key, T data)
	{
		return Insert(string_crc(key), data);
	}
	bool Insert(string_crc key, T data)
	{
		// Can't insert elements with no key
		if (key.Get() == 0)
			return false;

		// Can't insert elements with no data
		if (data == 0)
			return false;

		// If it's already added we have a problem
		if (Exists(key) && !m_currentAmortizing)
			return false;

		// First insertion? create the table
		if (m_data == nullptr)
		{
			Allocate();
		}

		// Acquire a slot
		sint32 slotIdx = key.Get() % m_maxElements;
		StringPointerHashNode* node = &m_data[slotIdx];

		if (node->hash.Get() != 0)
		{
			while (node->next != nullptr)
			{
				node = node->next;
			}

			// Allocate the node
			StringPointerHashNode* next = (StringPointerHashNode*)m_allocator->Allocate(sizeof(StringPointerHashNode), 4, 0, ALLOC_STRING_POINTER_HASH);
			node->next = next;

			// advance the pointer
			node = node->next;
			node->next = nullptr;
		}

		node->hash = key;
		node->data = data;

		m_currentElements++;

		// Allocate new table if we are over the max load
		// If we are currently amortizing, give a little extra space as buffer
		const float maxLoad = m_currentAmortizing ? HASH_MAX_LOAD_AMORTIZING : HASH_MAX_LOAD;
		const float currentLoad = (float)m_currentElements / (float)m_maxElements;
		if (currentLoad >= maxLoad)
			AllocateNewTable();
		
		// Amortize
		Amortize();

		return true;
	}

	bool Exists(const char* key) const
	{
		return Exists(string_crc(key));
	}
	bool Exists(string_crc key) const
	{
		return Find(key) != nullptr;
	}
	T Delete(const char* key)
	{
		return Delete(string_crc(key));
	}
	T Delete(string_crc key)
	{
		int idx = 0;
		StringPointerHashNode* node = nullptr;

		// If old table exists
		if (m_oldData != nullptr)
		{
			idx = key.Get() % m_oldMax;
			node = &m_oldData[idx];
			
			if (node && node->hash == key)
			{
				m_oldActiveElements--;
				T data = node->data;

				if (node->next)
				{
					StringPointerHashNode* del = node->next;
					node->next = del->next;
					node->hash = del->hash;
					node->data = del->data;

					// Delete this pointer
					m_allocator->Deallocate(del);
				}
				else {
					node->hash.Clear();
					node->data = nullptr;
				}

				return data;
			}
			while (node && node->next != nullptr)
			{
				if (node->next->hash == key)
				{
					T data = node->next->data;

					StringPointerHashNode* del = node->next;
					node->next = del->next;

					// Delete the pointer
					m_allocator->Deallocate(del);
					m_oldActiveElements--;
					return data;
				}
				node = node->next;
			}
		}

		// Check current table
		idx = key.Get() % m_maxElements;
		node = &m_data[idx];

		if (node && node->hash == key)
		{
			m_currentElements--;
			T data = node->data;

			if (node->next)
			{

				StringPointerHashNode* del = node->next;
				node->next = del->next;
				node->hash = del->hash;
				node->data = del->data;

				// Delete the pointer
				m_allocator->Deallocate(del);
			}
			else {
				node->hash.Clear();
				node->data = nullptr;
			}

			return data;
		}
		while (node && node->next != nullptr)
		{
			if (node->next->hash == key)
			{
				T data = node->next->data;

				StringPointerHashNode* del = node->next;
				node->next = del->next;

				// Delete the pointer
				m_allocator->Deallocate(del);
				m_currentElements--;
				return data;
			}
			node = node->next;
		}
		return nullptr;
	}

	T Get(const char* key) const
	{
		return Get(string_crc(key));
	}
	T Get(string_crc key) const
	{
		StringPointerHashNode* node = Find(key);
		if (node == nullptr)
		{
			return nullptr;
		}

		return node->data;
	}

	T* GetPtr(string_crc key) const
	{
		StringPointerHashNode* node = Find(key);
		if (node == 0)
			return nullptr;
		return &node->data;
	}

	void Set(string_crc crc, T data)
	{
		StringPointerHashNode* node = Find(crc);
		if (node != nullptr)
		{
			node->data = data;
			return;
		}
	}

private:
	const uint32 m_uHashStartingSize;
	StringPointerHashNode* m_data;
	StringPointerHashNode* m_oldData;

	MemHeap* m_allocator;

	bool m_currentAmortizing;
	sint32 m_oldRemainingIndex;
	sint32 m_oldActiveElements;
	sint32 m_oldMax;
	sint32 m_currentElements;
	sint32 m_maxElements;
};

} // namespace usg

#endif // STRING_POINTER_HASH_H
