/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Moves the agent along a path until the destination is reached.
//	This Behavior requires the use of pathfinding since it is moving along a path.
//	That means that the usage of NavigationGrid is needed to create a path.
*****************************************************************************/

#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_MOVE_TOWARDS_POSITION_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_MOVE_TOWARDS_POSITION_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
#include "Engine/AI/NavigationWrapper.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class bhMoveTowardsPosition : public IBehavior<ContextType>
{
public:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		NavigationWrapper& navigation = ctx.Navigation();

		if(!navigation.HasDestination())
		{
			return BH_FAILURE;
		}

		if (navigation.ApproachPosition(navigation.GetDestination(),8.0f))
		{
			return BH_SUCCESS;
		}
		return BH_RUNNING;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_MOVE_TOWARDS_POSITION_BEHAVIOR__