/*************//***************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "Engine/Memory/ArrayPool.h"
#include "Engine/Core/Containers/List.h"


namespace usg {

// FIXME!!!
const int LIST_POOL_SIZE = 6000*10;

struct EntryAlloc
{
	void* a;
	void* b;
	void* c;
	#ifdef DEBUG_ARRAY_POOL
	uint32 uPad;
	#endif
};

//static ArrayPool<EntryAlloc>*	g_pool;

void InitListMemory()
{
//	g_pool = vnew(ALLOC_CONTAINER) ArrayPool<EntryAlloc>;
//g_pool->Init(LIST_POOL_SIZE, false);
}

EntryAlloc* ListAllocArray(uint32 uCount)
{
//	EntryAlloc* pEntryAlloc = NULL;
//	pEntryAlloc = g_pool->AllocArray( uCount );
//	return pEntryAlloc;
	return NULL;
}

void ListFreeArray(EntryAlloc* pEntries, uint32 uCount)
{
//	g_pool->FreeArray( pEntries, uCount );
}

}
