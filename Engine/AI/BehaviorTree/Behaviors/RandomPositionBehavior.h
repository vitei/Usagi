/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Will randomize a target position for this agent.
*****************************************************************************/
#ifndef __USG_AI_RANDOM_POSITION_BEHAVIOR__
#define __USG_AI_RANDOM_POSITION_BEHAVIOR__

#include "Engine/Maths/Vector3f.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhRandomPosition : public IBehavior<ContextType>
{
public:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		Vector3f vMax(m_data.maxX, 0.0f, m_data.maxZ);
		Vector3f vMin(m_data.minX, 0.0f, m_data.minZ);
		Vector3f vRandomPosition(0.0f);
		vRandomPosition.RangedRandomVector(vMin, vMax);
		ctx.Navigation().SetDestination(vRandomPosition, m_data.usePathfinding);
		return BH_SUCCESS;
	}

	void SetData(const usg::ai::RandomPosition& data)
	{
		m_data.maxX = data.maxX;
		m_data.maxZ = data.maxZ;
		m_data.minX = data.minX;
		m_data.minZ = data.minZ;
		m_data.usePathfinding = data.usePathfinding;
	}

private:
	usg::ai::RandomPosition m_data;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_RANDOM_POSITION_BEHAVIOR__