/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: Approaches the target until a certain minimum distance is reached.
 *****************************************************************************/

#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_APPROACH_TARGET_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_APPROACH_TARGET_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/FuzzyApproachTargetBehavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhApproachTarget : public bhFuzzyApproachTarget<ContextType>
{
public:
	void SetData(const usg::ai::ApproachTarget& data)
	{
		usg::ai::FuzzyApproachTarget fuzzyData;
		fuzzyData.fOffsetX = data.fOffsetX;
		fuzzyData.fOffsetY = data.fOffsetY;
		fuzzyData.fOffsetZ = data.fOffsetZ;
		fuzzyData.has_fOffsetX = data.has_fOffsetX;
		fuzzyData.has_fOffsetY = data.has_fOffsetY;
		fuzzyData.has_fOffsetZ = data.has_fOffsetZ;
		fuzzyData.has_usePathFinding = data.has_usePathFinding;
		fuzzyData.usePathFinding = data.usePathFinding;
		fuzzyData.maxMaxDistance = data.maxDistance;
		fuzzyData.minMaxDistance = data.maxDistance;
		fuzzyData.maxMinDistance = data.minDistance;
		fuzzyData.minMinDistance = data.minDistance;
		bhFuzzyApproachTarget<ContextType>::SetData(fuzzyData);
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_APPROACH_TARGET_BEHAVIOR__
