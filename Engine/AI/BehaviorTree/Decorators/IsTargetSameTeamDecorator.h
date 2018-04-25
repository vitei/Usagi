/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Decorator for checking if target is on the same team, or not.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_DECORATORS_IS_TARGET_SAME_TEAM_DECORATOR__
#define __USG_AI_BEHAVIOR_TREE_DECORATORS_IS_TARGET_SAME_TEAM_DECORATOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Decorator.h"
namespace usg
{
namespace ai
{
template <class ContextType>
class dcIsTargetSameTeam : public IDecorator<ContextType>
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
			return this->GetChild().Tick(fElapsed, ctx);
		}

		return BH_FAILURE;
	}

};

template <class ContextType>
class dcIsTargetNotSameTeam : public IDecorator<ContextType>
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
			return this->GetChild().Tick(fElapsed, ctx);
		}

		return BH_FAILURE;
	}

};
}	//	namespace ai
}	//	namespace usg
#endif	//	__USG_AI_BEHAVIOR_TREE_DECORATORS_IS_TARGET_SAME_TEAM_DECORATOR__
