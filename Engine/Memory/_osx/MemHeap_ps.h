/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_ENGINE_MEMORY_MEMTYPE_WIN_PS_H_
#define _USG_ENGINE_MEMORY_MEMTYPE_WIN_PS_H_
#include "Engine/Common/Common.h"

// FIXME: Just a wrapper for new and delete at the moment

class MemHeap_ps
{
public:
	MemHeap_ps();
	~MemHeap_ps();

	void Init(uint32 uSize);
	void Init(void* location, uint32 uSize);

	void*	Alloc(uint32 uSize, uint32 uAlign, bool bGPUUse);
	void	Free(void*, bool bGPUUse);

	// FIXME: Implemenet a proper heap
	uint32 GetSize() const { return 1000000000; }
	uint32 GetFreeSize() const { return 1000000000; }

private:
	
};


#endif
