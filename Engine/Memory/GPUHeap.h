/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A "heap" which manages the memory recquired for RenderTargets
//	etc. The allocations will be large and performed numerous times per frame
//	so we are creating a specialised system for this purpose
//	Totally not optimised, but only expecting a few very large blocks
*****************************************************************************/
#ifndef _USG_ENGINE_MEMORY_GPU_HEAP_H_
#define _USG_ENGINE_MEMORY_GPU_HEAP_H_

#include "Engine/Memory/MemUtil.h"
#include "Engine/Memory/MemAllocator.h"

namespace usg {

class GPUHeap
{
public:
	GPUHeap();
	~GPUHeap();

	void Init(void* pLoc, memsize uSize, uint32 uMaxAllocs);

	void ReacquireAll();
	void ReleaseAll();

	// Note that this allocator must remain valid throughout the lifetime of the allocation
	void AddAllocator(MemAllocator* pAllocator);
	void RemoveAllocator(MemAllocator* pAllocator);
	bool CanAllocate(MemAllocator* pAllocator);

private:
	
	struct BlockInfo
	{
		MemAllocator* 	pAllocator; // If NULL this is a free block
		void*			pLocation;
		memsize			uSize;

		BlockInfo*		pNext;
		BlockInfo*		pPrev;

		BlockInfo*		pListNext;
		BlockInfo*		pListPrev;

		bool			bValid;
	};

	void SwitchList(BlockInfo* pInfo, BlockInfo** ppSrcList, BlockInfo** ppDstList);
	BlockInfo* PopList(BlockInfo** ppSrcList, BlockInfo** ppDstList);

	void AllocMemory(BlockInfo* pInfo);
	void FreeMemory(BlockInfo* pInfo);
	memsize AlignAddress(memsize uAddress, memsize uAlign);
	BlockInfo* FindUnusedBlock();

	void*		m_pHeapMem;
	uint32		m_uMaxAllocs;
	BlockInfo*	m_memoryBlocks;

	BlockInfo*	m_pFreeList;
	BlockInfo*	m_pUnusedList;
	BlockInfo*	m_pAllocList;
};

}

#endif

