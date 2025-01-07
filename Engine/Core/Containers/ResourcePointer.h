/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Originally when we couldn't use C++ 11 this class started out
//  as a shared pointer but has evolved into a resource pointer which could be
//  invalidated even if there are dangling references.
*****************************************************************************/
#ifndef _USG_CORE_CONTAINERS_RESOURCE_POINTER_H_
#define	_USG_CORE_CONTAINERS_RESOURCE_POINTER_H_

#include "Engine/Memory/ArrayPool.h"

template <class PointerType>
class ResourcePointer
{
public:
	ResourcePointer();
	virtual ~ResourcePointer();

	ResourcePointer(PointerType* pData);

	// Copy constructors
	ResourcePointer(const ResourcePointer& rhs);
	ResourcePointer& operator=(const ResourcePointer& rhs);

	// Operator overloads
	PointerType* operator->() const;
	PointerType& operator*() const;
	operator bool() const;

	bool operator==(const ResourcePointer& rhs) const;
	bool operator!=(const ResourcePointer& rhs) const;

	void reset(PointerType* pType = nullptr);
	PointerType* get() const;
	bool unique() const;


private:
	void removeRef();
	void destroy(PointerType* pType);
	void replacePointer(PointerType* pType, bool bPrev, bool bNext);

	void messageDataInvalidate();

	ResourcePointer*	m_pNext;
	ResourcePointer*	m_pPrev;

	mutable std::mutex m_mutex;

	void init(PointerType* pType);
	void copy(const ResourcePointer<PointerType>& copyData);
	PointerType*	m_pPointer;
};

template <class PointerType>
ResourcePointer<PointerType>::ResourcePointer(const ResourcePointer<PointerType>& rhs)
{
	init(nullptr);
	copy(rhs);
}

template <class PointerType>
ResourcePointer<PointerType>& ResourcePointer<PointerType>::operator=(const ResourcePointer<PointerType>& rhs)
{
	if (this != &rhs)
	{
		copy(rhs);
	}
	return *this;
}

template <class PointerType>
ResourcePointer<PointerType>::ResourcePointer()
{
	init(nullptr);
}

template <class PointerType>
ResourcePointer<PointerType>::ResourcePointer(PointerType* pData)
{
	init(pData);
}

template <class PointerType>
ResourcePointer<PointerType>::~ResourcePointer()
{
	removeRef();
}

template <class PointerType>
void ResourcePointer<PointerType>::init(PointerType* pType)
{
	m_pPointer = pType;
	m_pNext = nullptr;
	m_pPrev = nullptr;
}

template <class PointerType>
void ResourcePointer<PointerType>::removeRef()
{
	// Shared pointers clean themselves up if there are no references to them
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_pNext && !m_pPrev && m_pPointer)
	{
		// We don't clean up the data directly so that we don't have to expose the destructor
		// and have greater control (for example we could be referencing something from a fast pool
		ASSERT(false);
	}

	// Remove ourselves from the list
	if (m_pPrev)
	{
		std::lock_guard<std::mutex> prevLock(m_pPrev->m_mutex);
		m_pPrev->m_pNext = m_pNext;
	}

	if (m_pNext)
	{
		std::lock_guard<std::mutex> nextLock(m_pNext->m_mutex);
		m_pNext->m_pPrev = m_pPrev;
	}

	m_pNext = nullptr;
	m_pPrev = nullptr;
	m_pPointer = nullptr;
}

template <class PointerType>
void ResourcePointer<PointerType>::messageDataInvalidate()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	ResourcePointer<PointerType>* pPtr = m_pNext;
	while (pPtr)
	{
		std::lock_guard<std::mutex> nextLock(pPtr->m_mutex);
		ResourcePointer<PointerType>* pNext = pPtr->m_pNext;
		pPtr->m_pPointer = nullptr;
		pPtr->m_pNext = nullptr;
		pPtr->m_pPrev = nullptr;
		pPtr = pNext;
	}

	pPtr = m_pPrev;
	while (pPtr)
	{
		std::lock_guard<std::mutex> prevLock(pPtr->m_mutex);
		ResourcePointer<PointerType>* pPrev = pPtr->m_pPrev;
		pPtr->m_pPointer = nullptr;
		pPtr->m_pNext = nullptr;
		pPtr->m_pPrev = nullptr;
		pPtr = pPrev;
	}

	m_pPrev = nullptr;
	m_pNext = nullptr;
	m_pPointer = nullptr;
}

template <class PointerType>
void ResourcePointer<PointerType>::copy(const ResourcePointer<PointerType>& copyData)
{
	if (&copyData == this)
	{
		return;
	}

	if (copyData.m_pPointer == m_pPointer)
	{
		return;
	}

	removeRef();

	ResourcePointer<PointerType>& nonConstCopyData = *const_cast<ResourcePointer<PointerType>*>(&copyData);
	{
		std::lock_guard<std::mutex> lock(nonConstCopyData.m_mutex);

		m_pNext = nonConstCopyData.m_pNext;
		m_pPrev = &nonConstCopyData;
		m_pPointer = nonConstCopyData.m_pPointer;

		if (m_pNext)
		{
			std::lock_guard<std::mutex> nextLock(m_pNext->m_mutex);
			m_pNext->m_pPrev = this;
		}
		nonConstCopyData.m_pNext = this;

		ASSERT(m_pNext != this);
	}
}

template <class PointerType>
void ResourcePointer<PointerType>::reset(PointerType* pType)
{
	if (pType == nullptr)
	{
		// The data is gone, clean up all the pointers
		destroy(m_pPointer);
	}
	else
	{
		// We have a new handle to point at, go fix up all our pointers
		replacePointer(pType, true, true);
	}
}

template <class PointerType>
void ResourcePointer<PointerType>::destroy(PointerType* pPointerData)
{
	ASSERT(m_pPointer != nullptr);
	ASSERT(m_pPointer == pPointerData);
	messageDataInvalidate();
}

template <class PointerType>
void ResourcePointer<PointerType>::replacePointer(PointerType* pType, bool bPrev, bool bNext)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	ResourcePointer<PointerType>* pPtr = m_pNext;
	while (pPtr)
	{
		{
			std::lock_guard<std::mutex> nextLock(pPtr->m_mutex);
			ResourcePointer<PointerType>* pNext = pPtr->m_pNext;
			pPtr->m_pPointer = pType;
			pPtr = pNext;
		}
	}

	pPtr = m_pPrev;
	while (pPtr)
	{
		{
			std::lock_guard<std::mutex> prevLock(pPtr->m_mutex);
			ResourcePointer<PointerType>* pPrev = pPtr->m_pPrev;
			pPtr->m_pPointer = pType;
			pPtr = pPrev;
		}
	}

	m_pPointer = pType;
}


// Operator overloads
template <class PointerType>
PointerType* ResourcePointer<PointerType>::operator->() const
{
	ASSERT(m_pPointer != nullptr);
	return m_pPointer;
}

template <class PointerType>
PointerType& ResourcePointer<PointerType>::operator*() const
{
	ASSERT(m_pPointer != nullptr);
	return *m_pPointer;
}

template <class PointerType>
ResourcePointer<PointerType>::operator bool() const
{
	return m_pPointer != nullptr;
}

template <class PointerType>
bool ResourcePointer<PointerType>::operator==(const ResourcePointer<PointerType>& rhs) const
{
	return m_pPointer == rhs.m_pPointer;
}

template <class PointerType>
bool ResourcePointer<PointerType>::operator!=(const ResourcePointer<PointerType>& rhs) const
{
	return !(*this == rhs);
}

// Get the raw pointer
template <class PointerType>
PointerType* ResourcePointer<PointerType>::get() const
{
	return m_pPointer;
}

// Check if this is the only reference to the resource
template <class PointerType>
bool ResourcePointer<PointerType>::unique() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_pNext == nullptr && m_pPrev == nullptr;
}

#endif
