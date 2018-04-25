/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include  OS_HEADER(Engine/Memory, Mem_ps.h)

namespace usg
{

void mem::ps::InitialiseDefault(AllocatorData* pAllocators)
{

}

void mem::ps::Cleanup()
{

}

}

void operator delete(void* ptr) throw()
{
//	mem::Free(MEMTYPE_STANDARD, ptr);
}

/*
void* operator new[](size_t size)
{
//	ASSERT(false);	// Not allowing the global allocator
	return NULL;
}
*/


void operator delete[](void* ptr) throw()
{
//	usg::mem::Free(MEMTYPE_STANDARD, ptr);
}

