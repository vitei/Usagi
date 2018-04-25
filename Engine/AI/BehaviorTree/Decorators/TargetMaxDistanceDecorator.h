/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Ticks the child if a target is present and it is at least more 
//	than the given distance
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_DECORATORS_TARGET_MAX_DISTANCE_DECORATOR__
#define __USG_AI_BEHAVIOR_TREE_DECORATORS_TARGET_MAX_DISTANCE_DECORATOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Decorator.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class dcTargetMaxDistance : public IDecorator<ContextType>
{
public:
	void SetData(const usg::ai::TargetMaxDistance& data)
	{
		m_data.maxDistance = data.maxDistance * data.maxDistance;
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

		if(fDistanceToTargetSq > m_data.maxDistance)
		{
			return this->GetChild().Tick(fElapsed, ctx);
		}

		return BH_FAILURE;
	}

	usg::ai::TargetMaxDistance m_data;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_DECORATORS_TARGET_MAX_DISTANCE_DECORATOR__
