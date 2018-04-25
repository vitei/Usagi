/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/DoubleStack.h"
#include "Engine/Memory/MemHeap.h"
#include "Allocator.h"

namespace usg
{


HeapAllocator::HeapAllocator()
{
	m_pHeap = NULL;
	m_uGroup = 0;
}

HeapAllocator::~HeapAllocator()
{

}

void HeapAllocator::Init(MemHeap* pHeap, uint32 uGroup)
{
	ASSERT(!m_pHeap);
	m_pHeap = pHeap;
	m_uGroup = uGroup;
}

void HeapAllocator::SetGroup(uint32 uGroup)
{
	m_uGroup = uGroup;
}

void* HeapAllocator::Alloc(MemAllocType eType, memsize uSize, uint32 uAlign)
{
	return m_pHeap->Allocate(uSize, uAlign, m_uGroup, eType, false);
}

void HeapAllocator::Free(void* pMem)
{
	m_pHeap->Deallocate(pMem);
}


void HeapAllocator::FreeGroup()
{
	m_pHeap->FreeGroup(m_uGroup);
}

StackAllocator::StackAllocator()
{
	m_pStack = NULL;
	m_bFront = true;
}

StackAllocator::~StackAllocator()
{

}

void StackAllocator::Init(DoubleStack* pStack, bool bFront)
{
	m_pStack = pStack;
	m_bFront = bFront;
}

void* StackAllocator::Alloc(MemAllocType eType, memsize uSize, uint32 uAlign)
{
	if(m_bFront)
	{
		return m_pStack->AllocFront(eType, uSize, uAlign, false);
	}
	else
	{
		return m_pStack->AllocBack(eType, uSize, uAlign, false);
	}
}

void StackAllocator::Free(void* pMem)
{
	// DO NOTHING
	DEBUG_PRINT("Free called on a stack, that's not going to do much good\n");
}


}
