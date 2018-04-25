/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Self cleaning scoped memory handle
*****************************************************************************/
#ifndef _USG_MEMORY_SCRATCH_HANDLE_H_
#define _USG_MEMORY_SCRATCH_HANDLE_H_
#include "Engine/Common/Common.h"
#include "Engine/Memory/Mem.h"

namespace usg{

class ScratchRaw
{
public:
	ScratchRaw(uint32 uSize, uint32 uAlign);

	~ScratchRaw();
   
	void* GetRawData() { return m_pRawData; }

	static void InitMemory(uint32 uSize);
protected:
	void Init(uint32 uSize, uint32 uAlign);
	PRIVATIZE_COPY(ScratchHandle)

	void* 		m_pRawData;
};

}


#endif
