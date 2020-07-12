/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Runs the child if the target is not visible.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_DECORATORS_CAN_NOT_SEE_TARGET_DECORATOR__
#define __USG_AI_BEHAVIOR_TREE_DECORATORS_CAN_NOT_SEE_TARGET_DECORATOR__

#include "Engine/AI/BehaviorTree/Decorator.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class dcCanNotSeeTarget : public IDecorator<ContextType>
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
			return this->GetChild().Tick(fElapsed, ctx);
		}

		return BH_FAILURE;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_DECORATORS_CAN_NOT_SEE_TARGET_DECORATOR__
