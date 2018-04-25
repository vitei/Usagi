/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Rotates the agent towards the target if any.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_FACE_TARGET_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_FACE_TARGET_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhFaceTarget : public IBehavior<ContextType>
{
public:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		const Vector3f& vDir = ctx.GetTarget().normalizedDirToTarget;
		float fDot = DotProduct(vDir, ctx.Navigation().GetForward());
		float fAngle = acosf(fDot) * (180.0f / usg::Math::pi);

		if(fAngle < m_data.minAngle)
		{
			return BH_SUCCESS;
		}

		ctx.Navigation().AddToTargetDirection(vDir);

		return BH_RUNNING;
	}

	void SetData(const usg::ai::FaceTarget& data)
	{
		m_data.minAngle = data.minAngle;
	}

private:
	usg::ai::FaceTarget m_data;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_FACE_TARGET_BEHAVIOR__