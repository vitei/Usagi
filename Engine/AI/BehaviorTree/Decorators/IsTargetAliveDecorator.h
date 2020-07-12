/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Decorator for checking if the target is alive, or not
*****************************************************************************/
#ifndef __USG_AI_IS_TARGET_ALIVE_DECORATOR__
#define __USG_AI_IS_TARGET_ALIVE_DECORATOR__

#include "Engine/AI/BehaviorTree/Decorator.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class dcIsTargetAlive : public IDecorator<ContextType>
{
protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		if(ctx.GetTarget().health > 0.0f)
		{
			return this->GetChild().Tick(fElapsed, ctx);
		}

		return BH_FAILURE;
	}
};

template <class ContextType>
class dcIsTargetNotAlive : public IDecorator<ContextType>
{
protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		if(ctx.GetTarget().health <= 0.0f)
		{
			return this->GetChild().Tick(fElapsed, ctx);
		}

		return BH_FAILURE;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_IS_TARGET_ALIVE_DECORATOR__
