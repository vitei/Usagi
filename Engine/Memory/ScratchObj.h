/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Self cleaning scoped memory handle
*****************************************************************************/
#ifndef _USG_MEMORY_SCRATCH_OBJ_H_
#define _USG_MEMORY_SCRATCH_OBJ_H_
#include "Engine/Common/Common.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Memory/ScratchRaw.h"

namespace usg {

template<class ObjType>
class ScratchObj : protected ScratchRaw
{
public:
	ScratchObj(ObjType*& objOut, uint32 uCount, uint32 uAlign = 4)
	{
		Init(sizeof(ObjType) * uCount, uAlign);
		objOut = (ObjType*)m_pRawData;
	}

	ScratchObj(uint32 uCount)
	{
		Init(sizeof(ObjType) * uCount, 4);
	}

	~ScratchObj()
	{

	}
   
	ObjType* GetData() { return (ObjType*)m_pRawData; }
};

}

#endif
