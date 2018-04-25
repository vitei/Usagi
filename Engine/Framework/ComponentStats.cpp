/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Framework/ComponentEntity.h"
#include "ComponentStats.h"

namespace usg {

ComponentStats::Data ComponentStats::mData;


void ComponentStats::Reset()
{
	mData.uTotalBlocks = 0;
	mData.uTotalActiveComponents = 0;
	mData.uTotalActiveEntities = 0;
	for (uint32 i = 0; i < MAX_BITFIELD_SIZE; i++)
	{
		mData.uComponentBitfield[i] = 0;
	}
	mData.bDirty = false;
}



}
