/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Check if the target is on the same team, or not.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_IS_TARGET_SAME_TEAM_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_IS_TARGET_SAME_TEAM_BEHAVIOR__

#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/Targetting/Target.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhIsTargetSameTeam : public IBehavior<ContextType>
{
protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}
	
		if(ctx.GetTarget().team == ctx.Team())
		{
			return BH_SUCCESS;
		}

		return BH_RUNNING;
	}
};

template <class ContextType>
class bhIsTargetNotSameTeam : public IBehavior<ContextType>
{
protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}
	
		if(ctx.GetTarget().team != ctx.Team())
		{
			return BH_SUCCESS;
		}

		return BH_RUNNING;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_IS_TARGET_SAME_TEAM_BEHAVIOR__
