/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_ENGINE_MEMORY_MEMTYPE_WIN_PS_H_
#define _USG_ENGINE_MEMORY_MEMTYPE_WIN_PS_H_


// FIXME: Just a wrapper for new and delete at the moment

namespace usg{

class MemHeap_ps
{
public:
	MemHeap_ps();
	~MemHeap_ps();

	void Init(memsize uSize);
	void Init(void* location, memsize uSize);

	void*	Alloc(memsize uSize, memsize uAlign, bool bGPUUse);
	void	Free(void*, memsize uAlign, bool bGPUUse);

	uint32 GetSize() const { return (uint32)m_uSize; }
	uint32 GetFreeSize() const { return (uint32)m_uSize-(uint32)m_uAllocated; }

private:
	memsize m_uSize;
	memsize m_uAllocated;
};


}

#endif