/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Verifies that the agent has a target.
*****************************************************************************/

#ifndef __USG_AI_HAS_TARGET_BEHAVIOR__
#define __USG_AI_HAS_TARGET_BEHAVIOR__

#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhHasTarget : public IBehavior<ContextType>
{
public:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(ctx.HasTarget())
		{
			return BH_SUCCESS;
		}

		return BH_RUNNING;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_HAS_TARGET_BEHAVIOR__
