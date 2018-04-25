/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Run the child if the AI is moving.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_DECORATORS_IS_MOVING_DECORATOR__
#define __USG_AI_BEHAVIOR_TREE_DECORATORS_IS_MOVING_DECORATOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Decorator.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class dcIsMoving : public IDecorator<ContextType>
{
public:

protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(ctx.IsMoving())
		{
			return this->GetChild().Tick(fElapsed, ctx);
		}

		return BH_FAILURE;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_DECORATORS_IS_MOVING_DECORATOR__
