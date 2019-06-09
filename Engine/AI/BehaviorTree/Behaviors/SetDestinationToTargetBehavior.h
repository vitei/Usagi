/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Will set the destination position to the targets position.
*****************************************************************************/

#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_DESTINATION_TO_TARGET_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_DESTINATION_TO_TARGET_BEHAVIOR__

#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhSetDestinationToTarget : public IBehavior<ContextType>
{
public:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		const Vector3f& vTarget = ctx.GetTarget().position;
		ctx.Navigation().SetDestination(vTarget, m_data.usePathfinding);
		return BH_SUCCESS;
	}

	void SetData(const usg::ai::SetDestinationToTarget& data)
	{
		m_data.usePathfinding = data.usePathfinding;
	}

private:
	usg::ai::SetDestinationToTarget m_data;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_DESTINATION_TO_TARGET_BEHAVIOR__
