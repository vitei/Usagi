/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Utility functions for memory
*****************************************************************************/
#ifndef _USG_MEM_UTIL_H_
#define _USG_MEM_UTIL_H_

#include OS_HEADER(Engine/Memory, MemUtil_ps.h)

namespace usg
{
	inline memsize AlignSizeUp(memsize size, memsize alignment)
	{
		memsize temp = (alignment - (size % alignment)) % alignment;

		return temp + size;
	}

	inline memsize AlignSizeDown(memsize size, memsize uAlign)
	{
		memsize uMask = uAlign - 1;
		memsize uMisAlignment = (size & uMask);

		return size - uMisAlignment;
	}

	inline memsize AlignAddress(memsize size, memsize alignment)
	{
		return AlignSizeUp(size, alignment);
	}

}


#endif
