/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Check if the agent does NOT have a target. If so return SUCCESS.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_HAS_NO_TARGET_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_HAS_NO_TARGET_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class bhHasNoTarget : public IBehavior<ContextType>
{
public:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_SUCCESS;
		}

		return BH_RUNNING;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_HAS_NO_TARGET_BEHAVIOR__