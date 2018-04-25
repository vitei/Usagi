/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine\Common\Common.h"
#include <malloc.h>

namespace usg{

class MemBlock_ps
{
public:
	MemBlock_ps() { m_pData = NULL; }
	~MemBlock_ps() { Free(); }
	void* Init(uint32 uSize)
	{
		return _aligned_malloc(uSize, 4);
	}

	inline void Free()
	{
		if(m_pData)
		{
			_aligned_free(m_pData);
			m_pData = NULL;
		}
	}

	void NotifyGPUAlloc(void* pData, memsize uSize, memsize uAlign) {}
	void NotifyGPUFree(void* pData) {}

private:
	void*	m_pData;

};

}
