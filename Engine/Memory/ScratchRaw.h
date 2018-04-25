/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Self cleaning scoped memory handle
*****************************************************************************/
#pragma once
#ifndef USG_MEMORY_SCRATCH_RAW_H
#define USG_MEMORY_SCRATCH_RAW_H
#include "Engine/Common/Common.h"
#include "Engine/Memory/Mem.h"

namespace usg{

class ScratchRaw
{
public:
	ScratchRaw();
	ScratchRaw(memsize uSize, memsize uAlign);
	~ScratchRaw();

	void Init(memsize uSize, memsize uAlign);
	void Free();
	void* GetRawData() { return m_pRawData; }

	static void InitMemory(memsize uSize);

	// Use of the following two functions is strongly discouraged.  Where possible,
	// the concrete versions above should be used, as that will ensure the scratch
	// memory is cleaned up properly after use.  These functions are for the rare
	// case where you want to use some scratch memory but the pointer must last
	// beyond its original scope.  If you do use them, be careful to ensure you clean
	// up after yourself.
	static void Init(void** pRawData, memsize uSize, memsize uAlign);
	static void Free(void** pRawData);
protected:

	PRIVATIZE_COPY(ScratchRaw)

	void* 		m_pRawData;
};

}

#endif // USG_MEMORY_SCRATCH_RAW_H
