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
	ResourcePointer(const ResourcePointer &rhs) { init(NULL); copy(rhs); }
	ResourcePointer& operator=(const ResourcePointer &rhs) { copy(rhs); return *this; }

	// Operator overloads
	PointerType* operator->() const { return m_pPointer; }
	PointerType& operator*() const { return *m_pPointer; }
	PointerType& operator[](memsize idx) { return m_pPointer[idx]; }
	operator bool() const { return m_pPointer != NULL; }

	// Mimicking C++ 11'd shr_ptr which only has tests against null_ptr
	bool operator==(const ResourcePointer &rhs) const { return rhs.m_pPointer == m_pPointer; }
	bool operator!=(const ResourcePointer &rhs) const { return rhs.m_pPointer != m_pPointer; }

	void reset(PointerType* pType = NULL);	// Call with NULL when the data has been manually deleted
	PointerType* get() const { return m_pPointer; }
	bool unique() const { return m_pNext == NULL && m_pPrev == NULL;  }


	uint32 use_count() const;

private:
	void removeRef();
	void destroy(PointerType* pType);
	void replacePointer(PointerType* pType, bool bPrev, bool bNext);


	void messageDataInvalidate();

	ResourcePointer*	m_pNext;
	ResourcePointer*	m_pPrev;

protected:
	void init(PointerType* pType);
	void copy(const ResourcePointer<PointerType>& copyData);

	PointerType*	m_pPointer;
};

template <class PointerType>
ResourcePointer<PointerType> make_shared(PointerType* pPointer)
{
	// Non optimal, copy constructor, but just need to match the c++ 11 
	ResourcePointer<PointerType> shared(pPointer);
	return shared;
}


template <class PointerType>
NO_INLINE_TEMPL ResourcePointer<PointerType>::ResourcePointer()
{
	init(NULL);
}

template <class PointerType>
NO_INLINE_TEMPL ResourcePointer<PointerType>::ResourcePointer(PointerType* pData)
{
	init(pData);
}

template <class PointerType>
NO_INLINE_TEMPL ResourcePointer<PointerType>::~ResourcePointer()
{
	removeRef();
}


template <class PointerType>
void ResourcePointer<PointerType>::removeRef()
{
	// Shared pointers clean themselves up if there are no references to them
	if (!m_pNext && !m_pPrev && m_pPointer)
	{
		// We don't clean up the data directly so that we don't have to expose the destructor
		// and have greater control (for example we could be referencing something from a fast pool
		ASSERT(false);
	}

	// Remove ourselves from the list
	if (m_pPrev)
	{
		m_pPrev->m_pNext = m_pNext;
	}
	if (m_pNext)
	{
		m_pNext->m_pPrev = m_pPrev;
	}
	m_pNext = NULL;
	m_pPrev = NULL;
	m_pPointer = NULL;
}

template <class PointerType>
NO_INLINE_TEMPL void ResourcePointer<PointerType>::messageDataInvalidate()
{
	ResourcePointer<PointerType>* pPtr = m_pNext;
	while (pPtr)
	{
		ResourcePointer<PointerType>* pNext = pPtr->m_pNext;
		pPtr->m_pPointer = nullptr;
		pPtr->m_pNext = nullptr;
		pPtr->m_pPrev = nullptr;
		pPtr = pNext;
	}

	pPtr = m_pPrev;
	while (pPtr)
	{
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
NO_INLINE_TEMPL void ResourcePointer<PointerType>::copy(const ResourcePointer<PointerType>& copyData)
{
	if(&copyData == this)
		return;

	if (copyData.m_pPointer == m_pPointer)
	{
		return;
	}

	removeRef();

	ResourcePointer<PointerType>& nonConstCopyData = *const_cast<ResourcePointer<PointerType>*>(&copyData);
	
	m_pNext = nonConstCopyData.m_pNext;
	m_pPrev = &nonConstCopyData;
	m_pPointer = nonConstCopyData.m_pPointer;

	if(m_pNext)
	{
		m_pNext->m_pPrev = this;
	}
	nonConstCopyData.m_pNext = this;

	ASSERT(m_pNext != this);
}

template <class PointerType>
NO_INLINE_TEMPL void ResourcePointer<PointerType>::reset(PointerType* pType)
{
	if (pType == NULL)
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
NO_INLINE_TEMPL void ResourcePointer<PointerType>::init(PointerType* pPointerData)
{
	m_pPointer = pPointerData;
	m_pNext = NULL;
	m_pPrev = NULL;
}

template <class PointerType>
NO_INLINE_TEMPL void ResourcePointer<PointerType>::destroy(PointerType* pPointerData)
{
	ASSERT(m_pPointer!=NULL);
	ASSERT(m_pPointer == pPointerData);
	messageDataInvalidate();
}


template <class PointerType>
NO_INLINE_TEMPL void ResourcePointer<PointerType>::replacePointer(PointerType* pType, bool bPrev, bool bNext)
{
	ResourcePointer<PointerType>* pPtr = m_pNext;
	while (pPtr)
	{
		ResourcePointer<PointerType>* pNext = pPtr->m_pNext;
		pPtr->m_pPointer = pType;
		pPtr = pNext;
	}

	pPtr = m_pPrev;
	while (pPtr)
	{
		ResourcePointer<PointerType>* pPrev = pPtr->m_pPrev;
		pPtr->m_pPointer = pType;
		pPtr = pPrev;
	}

	m_pPointer = pType;
}

#endif
