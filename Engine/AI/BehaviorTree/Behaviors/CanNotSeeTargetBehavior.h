/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Behavior for checking if the currenttarget is not visible
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_CAN_NOT_SEE_TARGET__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_CAN_NOT_SEE_TARGET__

#include "Engine/AI/BehaviorTree/Behavior.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhCanNotSeeTarget : public IBehavior<ContextType>
{
protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		if(!ctx.GetTarget().visible || ctx.GetTarget().uLineOfSight == 0)
		{
			return BH_SUCCESS;
		}

		return BH_RUNNING;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_CAN_NOT_SEE_TARGET__