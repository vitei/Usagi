/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Will turn the target so that the side is facing .
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_CIRCLE_AROUND_TARGET_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_CIRCLE_AROUND_TARGET_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhCircleAroundTarget : public IBehavior<ContextType>
{
public:
	bhCircleAroundTarget()
	{
		m_fDir = 0.0f;
		m_fTimer = 0;
		m_bCircleAround = false;
		m_fLastFlipTime = -10;
		m_bReverse = false;
	}

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		m_fTimer += fElapsed;

		usg::ai::NavigationWrapper& navigation = ctx.Navigation();
		const usg::ai::Target& target = ctx.GetTarget();

		const Vector3f& vPos = navigation.GetPosition();
		const Vector3f& vTarget = target.position;
		Vector3f vDir = target.normalizedDirToTarget;

		const float fDistanceToTargetSq = vTarget.GetSquaredXZDistanceFrom(vPos);
		if(fDistanceToTargetSq < m_data.minDistance)
		{
			m_bCircleAround = true;
		}

		if(fDistanceToTargetSq > m_data.maxDistance)
		{
			m_bCircleAround = false;
		}

		if(m_bCircleAround)
		{
			Vector3f vMoveDir = (vDir * 0.25f) * m_fDir;
			vDir = Vector3f(-vDir.z, 0.0f, vDir.x);
			vMoveDir += vDir*m_fDir;
			vMoveDir *= (m_bReverse ? -1.0f : 1.0f);
			const NavigationGrid* pNaviGrid = navigation.GetNavigationGrid();
			const bool bCanCircle = !pNaviGrid->LineTest(ToVector2f(vPos), ToVector2f(vPos + vMoveDir*4.5f));
			if (bCanCircle)
			{
				ctx.Navigation().AddToTargetDirection(m_bReverse ? -vMoveDir : vMoveDir);
				ctx.Navigation().SetMove(true);
			}
			else
			{
				if (m_fTimer >= m_fLastFlipTime + 0.5f)
				{
					m_fLastFlipTime = m_fTimer;
					m_bReverse = !m_bReverse;
				}
			}
			navigation.SetReverse(m_bReverse);
		}
		else
		{
			navigation.ApproachTarget(target);
			m_bCircleAround = false;
			m_bReverse = false;
		}
		return BH_RUNNING;
	}

	void SetData(const usg::ai::CircleAroundTarget& data)
	{
		m_data.maxDistance = data.maxDistance * data.maxDistance;
		m_data.minDistance = data.minDistance * data.minDistance;
	}

	virtual void Init(ContextType& ctx)
	{
		Randomize();
	}

private:
	void Randomize()
	{
		sint32 iRandom = usg::Math::RangedRandomSInt(0, 3);
		m_fDir = iRandom == 0 ? 1.0f : -1.0f;
	}

	usg::ai::CircleAroundTarget m_data;
	float m_fDir;
	float m_fTimer;
	float m_fLastFlipTime;
	bool m_bCircleAround;
	bool m_bReverse;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_CIRCLE_AROUND_TARGET_BEHAVIOR__