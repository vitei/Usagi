/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Checks if health is more than a specific value
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_IS_HEALTH_ABOVE_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_IS_HEALTH_ABOVE_BEHAVIOR__

namespace usg
{

namespace ai
{

template <class ContextType>
class bhIsHealthAbove : public IBehavior<ContextType>
{
public:
	void SetData(const usg::ai::IsHealthAbove data)
	{
		m_data.health = data.health;
	}

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		const float fHealth = ctx.Health();

		if(fHealth <= 0.0f)
		{
			return BH_FAILURE;
		}

		if(fHealth > m_data.health)
		{
			return BH_SUCCESS;
		}

		return BH_RUNNING;
	}

private:
	usg::ai::IsHealthAbove m_data;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_IS_HEALTH_ABOVE_BEHAVIOR__
