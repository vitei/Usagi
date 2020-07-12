/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Utility functions for memory
*****************************************************************************/
#ifndef _USG_MEM_UTIL_PS_H_
#define _USG_MEM_UTIL_PS_H_


namespace usg{

inline void MemSet(void* pPtr, uint8 uVal, memsize uBytes)
{
	memset(pPtr, uVal, uBytes);
}

inline void MemClear(void* pPtr, memsize uBytes)
{
	ZeroMemory(pPtr, uBytes);
}

inline void MemCpy(void* pDst, const void* pSrc, memsize uSize)
{
	memcpy(pDst, pSrc, uSize);
}

}

#endif
