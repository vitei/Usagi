/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Ticks the child if a target is present and it is less
//	than the given distance
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_DECORATORS_TARGET_MIN_DISTANCE_DECORATOR__
#define __USG_AI_BEHAVIOR_TREE_DECORATORS_TARGET_MIN_DISTANCE_DECORATOR__

#include "Engine/AI/BehaviorTree/Decorator.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class dcTargetMinDistance : public IDecorator<ContextType>
{
public:
	void SetData(const usg::ai::TargetMinDistance& data)
	{
		m_data.minDistance = data.minDistance * data.minDistance;
	}
protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		const Vector3f& vPos = ctx.GetPosition();
		const Vector3f& vTarget = ctx.GetTarget().position;

		const float fDistanceToTargetSq = vPos.GetSquaredXZDistanceFrom(vTarget);

		if(fDistanceToTargetSq < m_data.minDistance)
		{
			return this->GetChild().Tick(fElapsed, ctx);
		}

		return BH_FAILURE;
	}

	usg::ai::TargetMinDistance m_data;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_DECORATORS_TARGET_MIN_DISTANCE_DECORATOR__
