/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Verify that the context does NOT have a valid destination.
*****************************************************************************/

#ifndef __USG_AI_BEHAVIOR_TREE_DECORATORS_HAS_NO_DESTINATION_DECORATOR__
#define __USG_AI_BEHAVIOR_TREE_DECORATORS_HAS_NO_DESTINATION_DECORATOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Decorator.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class dcHasNoDestination : public IDecorator<ContextType>
{
protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.Navigation().HasDestination())
		{
			return this->GetChild().Tick(fElapsed, ctx);
		}

		return BH_FAILURE;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_DECORATORS_HAS_NO_DESTINATION_DECORATOR__
