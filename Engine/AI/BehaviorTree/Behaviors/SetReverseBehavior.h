/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Sets the navigation context to reverse.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_REVERSE_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_REVERSE_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhSetReverse : public IBehavior<ContextType>
{
public:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		ctx.Navigation().SetReverse(false);
		return BH_SUCCESS;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_REVERSE_BEHAVIOR__