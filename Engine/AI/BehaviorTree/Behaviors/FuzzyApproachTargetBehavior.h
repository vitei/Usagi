/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Randomizes minimum and maximum distance requirements for approaching
//	the target
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_FUZZY_APPROACH_TARGET_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_FUZZY_APPROACH_TARGET_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/Pathfinding/Path.h"
#include "Engine/Debug/Rendering/DebugRender.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class bhFuzzyApproachTarget : public IBehavior<ContextType>
{
public:
	bhFuzzyApproachTarget():
		m_fMinDistanceSq(0.0f),
		m_fMaxDistanceSq(0.0),
		m_bStopMoving(false),
		m_bUsePathFinding(false)
	{
	
	}

	void SetData(const usg::ai::FuzzyApproachTarget& data)
	{
		m_data.maxMaxDistance = data.maxMaxDistance;
		m_data.maxMinDistance = data.maxMinDistance;
		m_data.minMaxDistance = data.minMaxDistance;
		m_data.minMinDistance = data.minMinDistance;
		m_bUsePathFinding = data.has_usePathFinding ? data.usePathFinding : false;

		m_data.fOffsetX = data.has_fOffsetX ? data.fOffsetX : 0.0f;
		m_data.fOffsetY = data.has_fOffsetY ? data.fOffsetY : 0.0f;
		m_data.fOffsetZ = data.has_fOffsetZ ? data.fOffsetZ : 0.0f;
	}

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.HasTarget())
		{
			return BH_FAILURE;
		}

		const usg::ai::Target& target = ctx.GetTarget();
		usg::ai::NavigationWrapper& navigation = ctx.Navigation();
		const Vector3f& vPos = navigation.GetPosition();
		const bool bHasOffset = Math::Abs(m_data.fOffsetX) + Math::Abs(m_data.fOffsetY) + Math::Abs(m_data.fOffsetZ) > Math::EPSILON;
		const Vector3f vTarget = bHasOffset ? target.position + m_data.fOffsetZ * target.forward + m_data.fOffsetX * target.right : target.position;

		const float fDistanceToTargetSq = vPos.GetSquaredXZDistanceFrom(vTarget);
		if(fDistanceToTargetSq < m_fMinDistanceSq)
		{
			m_bStopMoving = true;
		}

		if(m_bStopMoving)
		{
			if(fDistanceToTargetSq > m_fMaxDistanceSq)
			{
				m_bStopMoving = false;
			}

			return BH_SUCCESS;
		}

		if (m_bUsePathFinding)
		{
			if (!navigation.IsUsingPathfinding())
			{
				navigation.EnablePathfinding();
			}
		}

		if (bHasOffset)
		{
			const NavigationGrid* pNaviGrid = navigation.GetNavigationGrid();
			// const bool bAlreadyInsideBlockingArea = !pNaviGrid->CanPlacePoint(ToVector2f(vPos));
			const bool bUnblocked = !pNaviGrid->LineTest(ToVector2f(vPos), ToVector2f(vPos + (vTarget - vPos).GetNormalisedIfNZero()*navigation.GetAgentRadius()*2.0f));
			if (bUnblocked)
			{
				m_fTimeStuck = 0;
			}
			else
			{
				m_fTimeStuck += fElapsed;
			}
			if (bUnblocked || m_fTimeStuck > 3.0f)
			{
				navigation.ApproachPosition(vTarget, 0.5f);
				if (!bUnblocked)
				{
					navigation.SetBlocked(true);
				}
			}
		}
		else {
			navigation.ApproachTarget(target);
		}
		return BH_RUNNING;
	}

	virtual void Init(ContextType& ctx)
	{
		m_fTimeStuck = 0;
		Randomize();
	}

	virtual void Shut(int iStatus)
	{
		Randomize();
	}

private:
	void Randomize()
	{
		const float fMinDistance = usg::Math::RangedRandom(m_data.minMinDistance, m_data.maxMinDistance);
		const float fMaxDistance = usg::Math::RangedRandom(m_data.minMaxDistance, m_data.maxMaxDistance);
		m_fMinDistanceSq = fMinDistance * fMinDistance;
		m_fMaxDistanceSq = fMaxDistance * fMaxDistance;
	}

	usg::ai::FuzzyApproachTarget m_data;
	float m_fMinDistanceSq;
	float m_fMaxDistanceSq;
	bool m_bStopMoving;
	bool m_bUsePathFinding;
	float m_fTimeStuck;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_FUZZY_APPROACH_TARGET_BEHAVIOR__