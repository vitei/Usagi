/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include <stdlib.h>



class MemBlock_ps
{
public:
	MemBlock_ps() { m_pData = NULL; }
	~MemBlock_ps() { Free(); }
	void* Init(uint32 uSize)
	{
		return malloc(uSize);
	}

	inline void Free()
	{
		if(m_pData)
		{
			free(m_pData);
			m_pData = NULL;
		}
	}

	void NotifyGPUAlloc(void* pData, uint32 uSize, uint32 uAlign) {}
	void NotifyGPUFree(void* pData) {}

private:
	void*	m_pData;

};
