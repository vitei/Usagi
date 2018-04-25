/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Utility function and constant file
*****************************************************************************/

#ifndef _USG_UTILITY
#define	_USG_UTILITY
#include "Engine/Common/Common.h"
#include <string.h>

namespace utl
{
	template<class ObjectType>
	void SafeDelete( ObjectType** deletionObject )
	{
		if( deletionObject!= NULL )
		{
			vdelete *deletionObject;
			*deletionObject = NULL;
		}
	}

	template<class ObjectType>
	inline void SafeArrayDelete( ObjectType** deletionArray )
	{
		if( *deletionArray != NULL )
		{
			vdelete[] *deletionArray;
			*deletionArray = NULL;
		}
	}

	template<class ObjectType>
	void SafeRelease( ObjectType** deletionObj )
	{
		if( *deletionObj )
		{
			(*deletionObj)->Release();
			*deletionObj = NULL;
		}
	}

	template<class ObjectType>
	void Swap( ObjectType& ObjA, ObjectType& ObjB )
	{
		ObjectType temp = ObjA;
		ObjA = ObjB;
		ObjB = temp;
	}

	template<class ObjectType>
	void MoveElements(ObjectType* pBuffer, uint32 uDstIndex, uint32 uSrcIndex, uint32 uCount)
	{
		if((uDstIndex != uSrcIndex) && uCount > 0)
		{
			memmove(&(pBuffer[uDstIndex]), &(pBuffer[uSrcIndex]), sizeof(ObjectType) * uCount);
		}
	}

	template<class ObjectType>
	void RandomShuffle(ObjectType* pArray, const sint32 iSize)
	{
		if(iSize < 2)
		{
			return;	//	no need to shuffle if the array only has one value
		}

		for(sint32 i = iSize - 1; i >= 0; --i)
		{
			Swap(pArray[i], pArray[rand() % (i + 1)]);
		}
	}

	uint32 CRC32(const char* szString, uint32 acc = 0);
	uint32 CRC32(const void* pData, uint32 uSize, uint32 acc = 0);
}

#endif
