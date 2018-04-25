/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Utility functions for memory
*****************************************************************************/
#ifndef _USG_MEM_UTIL_PS_H_
#define _USG_MEM_UTIL_PS_H_
#include "Engine/Common/Common.h"
#include <string.h>

namespace usg {

inline void MemSet(void* pPtr, uint8 uVal, uint32 uBytes)
{
	memset(pPtr, uVal, uBytes);
}

inline void MemClear(void* pPtr, uint32 uBytes)
{
	memset(pPtr, 0, uBytes);
}

inline void MemCpy(void* pDst, const void* pSrc, uint32 uSize)
{
	memcpy(pDst, pSrc, uSize);
}

}

#endif
