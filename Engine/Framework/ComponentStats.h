/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _COMPONENT_STATS_H
#define _COMPONENT_STATS_H



namespace usg{

class ComponentStats
{
public:
    
	static void Reset();
	static inline void ActivatedComponent(uint32 uTypeId);
	static bool GetFlagsDirty() { return mData.bDirty; }
	static void ClearFlagsDirty() { mData.bDirty = false;  }
	static uint32* GetComponentFlags() { return &mData.uComponentBitfield[0]; }
	static void DeactivatedComponent() { mData.uTotalActiveComponents--; }
    
	static void ActivatedEntity() { mData.uTotalActiveEntities++; }
	static void DeactivatedEntity() { mData.uTotalActiveEntities--; }
    
	
	static uint32 TotalActiveComponents() { return mData.uTotalActiveComponents; }
	static uint32 TotalActiveEntities() { return mData.uTotalActiveEntities; }
	
private:
	static const int MAX_BITFIELD_SIZE = 64;
	struct Data {
		Data()
		: uTotalBlocks(0)
		, uTotalActiveComponents(0)
		, uTotalActiveEntities(0)
		, bDirty(false)
		{
			for (uint32 i = 0; i < MAX_BITFIELD_SIZE; i++)
			{
				uComponentBitfield[i] = 0;
			}
		}

		uint32	uTotalBlocks;
		uint32	uTotalActiveComponents;
		uint32	uTotalActiveEntities;
		uint32  uComponentBitfield[MAX_BITFIELD_SIZE];
		bool	bDirty;
	};

	static Data         mData;
};

inline void ComponentStats::ActivatedComponent(uint32 uTypeId)
{
	uint32 uIndex = uTypeId / BITFIELD_LENGTH;
	uint32 uMask = (1 << (uTypeId % BITFIELD_LENGTH));
	mData.bDirty |= ((mData.uComponentBitfield[uIndex] & (uMask))==0);
	mData.uComponentBitfield[uIndex] |= uMask;
	mData.uTotalActiveComponents++;
}

}

#endif //_COMPONENT_STATS_H
