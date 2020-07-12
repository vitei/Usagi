/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A standard locked stack
//	We allow delete for the purposes of sanity checking only, freed blocks
//	can not be re-used. Alloc can not be called when frozen and Free can only
//	only be called when frozen
*****************************************************************************/
#ifndef _USG_MEMORY_DOUBLE_STACK_H_
#define _USG_MEMORY_DOUBLE_STACK_H_

#include "Engine/Memory/Mem.h"
#include OS_HEADER(Engine/Memory, MemBlock_ps.h)

namespace usg{

class DoubleStack
{
public:
	DoubleStack();
	~DoubleStack();

	void	Init(uint32 uDataSize);
	void*	AllocFront(MemAllocType eType, memsize uSize, memsize uAlign = 4, bool bNotifyGPU = false);
	void*	AllocBack(MemAllocType eType, memsize uSize, memsize uAlign = 4, bool bNotifyGPU = false);

	void	AddTag(bool bFront);
	void	Freeze(bool bFreeze);
	void	FreeToLastTag(bool bFront);

	void	Free(void* pData, bool bGPU = false);
	bool	OwnsData(void* pData) { return (((uint8*)pData) >= m_pRawBase && ((uint8*)pData) < m_pRawEnd ); }

	uint32	GetAllocated() const;
	uint32	GetSize() const;
#ifndef FINAL_BUILD
	memsize	GetAllocated(MemAllocType eType) const;
#endif

private:

#ifndef FINAL_BUILD
	struct AllocInfo
	{
		memsize uAllocSizes[ALLOC_TYPE_COUNT];
	};
#endif

	struct Tag
	{
		Tag*		pPrev;
#ifndef FINAL_BUILD
		AllocInfo	allocInfo;
#endif
	};

#ifdef DEBUG_MEMORY
	struct Header
	{
		Header*	pNextAlloc;
		Header*	pPrevAlloc;
		uint32	uSignature;
	};
#endif

#ifndef FINAL_BUILD
	AllocInfo	m_allocInfo[2];
#endif
	
	MemBlock_ps	m_memBlock;
	uint8*		m_pRawBase;
	uint8*		m_pRawEnd;
	Tag*		m_pLastTag[2];
	uint8*		m_pTop[2];
	bool		m_bFrozen;
};

}

#endif
