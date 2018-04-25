/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_ENGINE_MEMORY_PC_MEM_PS_H_
#define _USG_ENGINE_MEMORY_PC_MEM_PS_H_

namespace usg
{

const int MAIN_HEAP_SIZE	= 512*1024*1024;		// 512MB
const int NET_HEAP_SIZE	= 4*1024*1024;			// 4MB
const int SCRATCH_HEAP_SIZE = 20*1024*1024;			// 20MB for file loading etc
const int OVERFLOW_HEAP_SIZE = 64*1024*1024;		// 64MB for our safety overflow


enum MemType
{
	// Standard heaps
	MEMTYPE_STANDARD = 0,
	MEMTYPE_NETWORK,

#ifndef FINAL_BUILD
	MEMTYPE_OVERFLOW,
#endif
	
	MEMTYPE_COUNT
};

namespace mem
{
	struct AllocatorData;

	namespace ps
	{
		void InitialiseDefault(AllocatorData* pAllocators);
		void Cleanup();

		inline void* Alloc(MemType eType, MemAllocType eAlloc, uint32 uSize, uint32 uAlign) { return NULL; }
		inline void Free(void*) { }

		inline void AddMemoryTag() {}
		inline void FreeToLastTag() {}
	}
}

}

#endif
