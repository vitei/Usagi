/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Clears the path and sets destination to false.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_CLEAR_DESTINATION_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_CLEAR_DESTINATION_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhClearDestination : public IBehavior<ContextType>
{
public:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.Navigation.HasDestination())
		{
			return BH_FAILURE;
		}

		ctx.Navigation().ClearDestination();
		return BH_SUCCESS;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_CLEAR_DESTINATION_BEHAVIOR__
