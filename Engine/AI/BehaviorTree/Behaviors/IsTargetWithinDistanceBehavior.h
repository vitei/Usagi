/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Checks if the target is withing a certain (squared) distance.
*****************************************************************************/

#ifndef __USG_AI_IS_TARGET_WITHIN_DISTANCE_BEHAVIOR__
#define __USG_AI_IS_TARGET_WITHIN_DISTANCE_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class bhIsTargetWithinDistance : public IBehavior<ContextType>
{
public:

	void SetData(const usg::ai::IsTargetWithinDistance& data)
	{
		m_data.max = data.max * data.max;
		m_data.min = data.min * data.min;
	}

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		float fDistanceToTargetSq = ctx.GetPosition().GetSquaredXZDistanceFrom(ctx.GetTarget().position);
	
		if(fDistanceToTargetSq > m_data.min && fDistanceToTargetSq < m_data.max)
		{
			return BH_SUCCESS;
		}
	
		return BH_RUNNING;
	}

private:
	usg::ai::IsTargetWithinDistance m_data;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_IS_TARGET_WITHIN_DISTANCE_BEHAVIOR__
