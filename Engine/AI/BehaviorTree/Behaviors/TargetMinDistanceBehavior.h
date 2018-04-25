/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Returns SUCCESS if the target is at least below the specified 
	distance (squared).
*****************************************************************************/

#ifndef __USG_AI_TARGET_MIN_DISTANCE_BEHAVIOR__
#define __USG_AI_TARGET_MIN_DISTANCE_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhTargetMinDistance : public IBehavior<ContextType>
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

		if(fDistanceSq < m_data.minDistance)
		{
			return BH_SUCCESS;
		}

		return BH_RUNNING;
	}

	void SetData(const usg::ai::TargetMinDistance& data)
	{
		m_data.minDistance = data.minDistance * data.minDistance;
	}

private:
	usg::ai::TargetMinDistance m_data;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_TARGET_MIN_DISTANCE_BEHAVIOR__