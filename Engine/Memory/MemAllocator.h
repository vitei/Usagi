/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_ENGINE_MEMORY_MEM_ALLOCATOR_H_
#define _USG_ENGINE_MEMORY_MEM_ALLOCATOR_H_

#include "Engine/Memory/MemUtil.h"

// Heaps such as MEM1 and the foreground heap will be re-claimed by the 
// system when entering the home menu etc. This allocator provides the
// appropriate callbacks to release and re-allocate that memory

namespace usg {

class MemAllocator
{
public:
	MemAllocator();
	virtual ~MemAllocator();

	void Init(uint32 uSize, uint32 uAlign, bool bGPU);
	void Allocated(void* pData);
	void Released();

	virtual void AllocatedCallback(void* pMem) = 0;
	virtual void ReleasedCallback() = 0;

	void*	GetData() { return m_pData; }
	uint32	GetSize() { return m_uSize; }
	uint32	GetAlign() { return m_uAlign; }
	bool	IsGPUData() { return m_bGPUData; }


private:
	void*		m_pData;
	uint32		m_uSize;
	uint32		m_uAlign;
	bool		m_bGPUData;
};

}


#endif
