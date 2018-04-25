/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
//	Description: Sets the destination to a position of choice.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_DESTINATION_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_DESTINATION_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
#include "Engine/Maths/Vector3f.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class bhSetDestination : public IBehavior<ContextType>
{
public:
	bhSetDestination()
	{
		m_fX = 0.0f;
		m_fZ = 0.0f;
	}

	void SetData(const usg::ai::SetDestination& data)
	{
		m_fX = data.x;
		m_fZ = data.z;
	}

protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		ctx.Navigation().SetDestination(Vector3f(m_fX, 0.0f, m_fZ), true);
		return BH_SUCCESS;
	}

	float m_fX;
	float m_fZ;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_DESTINATION_BEHAVIOR__
