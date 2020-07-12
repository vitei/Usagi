/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Returns SUCCESS if the target is at above the specified
	distance (squared).
*****************************************************************************/

#ifndef __USG_AI_TARGET_MAX_DISTANCE_BEHAVIOR__
#define __USG_AI_TARGET_MAX_DISTANCE_BEHAVIOR__

#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhTargetMaxDistance : public IBehavior<ContextType>
{
public:

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		const Vector3f& vPos = ctx.GetPosition();
		const Vector3f& vTarget = ctx.GetTarget().position;

		float fDistanceSq = vPos.GetSquaredXZDistanceFrom(vTarget);

		if(fDistanceSq > m_data.maxDistance)
		{
			return BH_SUCCESS;
		}

		return BH_RUNNING;
	}

	void SetData(const usg::ai::TargetMaxDistance& data)
	{
		m_data.maxDistance = data.maxDistance * data.maxDistance;
	}

private:
	usg::ai::TargetMaxDistance m_data;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_TARGET_MAX_DISTANCE_BEHAVIOR__