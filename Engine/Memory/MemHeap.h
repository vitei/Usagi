/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A non specific game heap
*****************************************************************************/
#pragma once

#ifndef USG_HEAP_H
#define USG_HEAP_H

#include "Engine/Memory/Mem.h"
#include "Engine/Core/Thread/CriticalSection.h"
#include OS_HEADER(Engine/Memory, MemHeap_ps.h)

namespace usg{

struct AllocHeader;

class MemHeap
{
public:
	MemHeap();
    void Initialize(memsize uSize);
	void Initialize(void* location, memsize uSize);
    
    void* Allocate(memsize bytes, memsize uAlign, uint8 uGroup, MemAllocType eType, bool bGpu = false);
	void* ReAlloc(void* pData, memsize uNewSize);
    static void Deallocate(void* pMem);
	static void* Reallocate(void* pMem, memsize uNewSize);
	static memsize GetAlignment(const void* const pMem);

	memsize AlignAddress(memsize uAddress, memsize uAlign);
	void CheckData();
	void PrintUsage();
	void PrintDynamicAllocTypes();
	void FreeGroup(uint32 uGroup);
	memsize GetSize() const { return m_platform.GetSize(); }
	memsize GetFreeSize() const { return m_platform.GetFreeSize(); }

	void SetStatic(bool bValue) { m_bStatic = bValue; }

#ifdef DEBUG_MEMORY
	memsize GetAllocated(MemAllocType eType) const { return m_uBytesAllocated[eType]; }
#endif

private:
	CriticalSection	m_criticalSection;

    void Deallocate(AllocHeader * pHeader);

	// TODO: If we keep using heaps we may want to remove the internal reliance on platform specific allocators
	// But as it's only for debug and scratch it seems excessive
	// This is even more important now that we are introducing the concept of mem groups in order to keep allocations
	// largely in line with NSubs... I worry about this decision
	MemHeap_ps		m_platform;

    uint32			m_uInstances;

	bool			m_bActive;
	bool			m_bStatic;

	AllocHeader*	m_pHeadAlloc;

#ifdef DEBUG_MEMORY
	memsize			m_uBytesAllocated[ALLOC_TYPE_COUNT];
    //uint32			m_uBytesPeak;
	// Static variables
    static int		s_nextAllocID;
#endif
};

} // namespace usagi


#endif // USG_HEAP_H
