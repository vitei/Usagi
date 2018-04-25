/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A pointer with inbuilt null checks, the creator of which can
//	NULL the contained values.
// 	For simplicity we are only allowing this value to be returned by reference
*****************************************************************************/
#ifndef _USG_CORE_CONTAINERS_SAFE_POINTER_H_
#define	_USG_CORE_CONTAINERS_SAFE_POINTER_H_
#include "Engine/Common/Common.h"
#include "Engine/Memory/ArrayPool.h"


// Need to specify this for any safe pointer overrides, see the effect system for an example
#define SAFEPOINTER_COPY(NameOfClass) 	NameOfClass(const NameOfClass &rhs) { Copy(rhs); } \
										NameOfClass& operator=(const NameOfClass &rhs) { Copy(rhs); return *this; }

template <class PointerType>
class SafePointer
{
public:
	SafePointer();
	~SafePointer();

	void Init(PointerType* pType);
	void Destroy(PointerType* pType);
	bool IsValid() const { return GetPointer() != NULL; }
	bool IsReferenced() const { return (m_pNext || m_pPrev); }
	void RemoveRef();

protected:
	PointerType* GetPointer() { return m_pPointer; }
	const PointerType* GetPointer() const { return m_pPointer; }
	void Copy(const SafePointer<PointerType>& copyData);

private:
	// We need to override the copy constructor that calls the copy command
	PRIVATIZE_COPY(SafePointer)

	void MessageDataInvalidate();

	SafePointer*	m_pNext;
	SafePointer*	m_pPrev;

	PointerType*	m_pPointer;
	bool			m_bDataOwner;
};


template <class PointerType>
NO_INLINE_TEMPL SafePointer<PointerType>::SafePointer()
{
	m_pPrev = NULL;
	m_pNext = NULL;
	m_pPointer = NULL;
	m_bDataOwner = false;
}

template <class PointerType>
NO_INLINE_TEMPL SafePointer<PointerType>::~SafePointer()
{
	RemoveRef();
}

template <class PointerType>
void SafePointer<PointerType>::RemoveRef()
{
	if (m_pPointer && m_bDataOwner)
	{
		MessageDataInvalidate();
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
NO_INLINE_TEMPL void SafePointer<PointerType>::MessageDataInvalidate()
{
	m_pPointer = NULL;
	m_bDataOwner = false;
	ASSERT(m_pNext != this);
	if(m_pNext)
	{
		m_pNext->m_pPrev = NULL;	// We will clear the previous ones ourselves
		m_pNext->MessageDataInvalidate();
		m_pNext = NULL;
	}
	if(m_pPrev)
	{
		m_pPrev->m_pNext = NULL;
		m_pPrev->MessageDataInvalidate();
		m_pPrev = NULL;
	}
	ASSERT(m_pNext != this);
}

template <class PointerType>
NO_INLINE_TEMPL void SafePointer<PointerType>::Copy(const SafePointer<PointerType>& copyData)
{
	if(&copyData == this)
		return;

	RemoveRef();

	SafePointer<PointerType>& nonConstCopyData = *const_cast<SafePointer<PointerType>*>(&copyData);
	
	m_pNext = nonConstCopyData.m_pNext;
	m_pPrev = &nonConstCopyData;
	m_pPointer = nonConstCopyData.m_pPointer;
	m_bDataOwner = false;

	if(m_pNext)
	{
		m_pNext->m_pPrev = this;
	}
	nonConstCopyData.m_pNext = this;

	ASSERT(m_pNext != this);
}

template <class PointerType>
NO_INLINE_TEMPL void SafePointer<PointerType>::Init(PointerType* pPointerData)
{
	ASSERT(m_pPointer == NULL);
	m_pPointer = pPointerData;
	m_pNext = NULL;
	m_pPrev = NULL;
	m_bDataOwner = true;
}

template <class PointerType>
NO_INLINE_TEMPL void SafePointer<PointerType>::Destroy(PointerType* pPointerData)
{
	ASSERT(m_pPointer!=NULL);
	ASSERT(m_pPointer == pPointerData);
	ASSERT(m_bDataOwner);
	MessageDataInvalidate();
}


#endif
