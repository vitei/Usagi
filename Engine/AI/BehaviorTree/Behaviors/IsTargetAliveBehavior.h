/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Verifies if the target is alive or not
*****************************************************************************/

#ifndef __USG_AI_IS_TARGET_ALIVE_BEHAVIOR__
#define __USG_AI_IS_TARGET_ALIVE_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class bhIsTargetAlive : public IBehavior<ContextType>
{
public:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		if(ctx.GetTarget().health > 0.0f)
		{
			return BH_SUCCESS;
		}

		return BH_RUNNING;
	}

	virtual void Shut(int iStatus) {}
};

template <class ContextType>
class bhIsTargetNotAlive : public IBehavior<ContextType>
{
public:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		if(!(ctx.GetTarget().health <= 0.0f))
		{
			return BH_SUCCESS;
		}

		return BH_RUNNING;
	}

	virtual void Shut(int iStatus) {}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_IS_TARGET_ALIVE_BEHAVIOR__