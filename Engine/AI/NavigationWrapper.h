/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Wrapper class for AgentNavigationComponent used in contexts by
//	behavior trees
*****************************************************************************/
#ifndef __USG_AI_NAVIGATION_WRAPPER__
#define __USG_AI_NAVIGATION_WRAPPER__

#include "Engine/AI/AgentNavigationUtil.h"
#include "Engine/AI/Pathfinding/NavigationGrid.h"
namespace usg
{

namespace ai
{

class NavigationWrapper : public AgentNavigationUtil
{
	typedef AgentNavigationUtil Inherited;
public:
	NavigationWrapper(Components::AgentNavigationComponent& nav, const usg::ai::NavigationGrid* pNavGrid);

	bool HasDestination() const { return Inherited::HasDestination(m_nav); }
	void SetDestination(const Vector3f& vDestination, bool bUsePathfinding) { Inherited::SetDestination(m_nav, vDestination, bUsePathfinding); }
	void ClearDestination() { Inherited::ClearDestination(m_nav); }
	const Vector3f& GetDestination() const
	{
		return Inherited::GetDestination(m_nav);
	}
	void UpdateDestination(const Vector3f& vDestination)
	{
		Inherited::UpdateDestination(m_nav, vDestination);
	}

	const Vector3f& GetPosition() const { return Inherited::GetPosition(m_nav); }
	const Vector3f& GetForward() const { return Inherited::GetForward(m_nav); }
	const Vector3f& GetRight() const { return Inherited::GetRight(m_nav); }
	const Vector3f& GetNextMoveDirection() const { return Inherited::GetNextMoveDirection(m_nav); }
	const Vector3f GetDirectionToNextTarget() const { return Inherited::GetDirectionToNextTarget(m_nav); }
	const Vector3f& GetNextTarget() const { return Inherited::GetNextTarget(m_nav); }
	const Vector3f& GetDirectionToDestination() const { return Inherited::GetDirectionToDestination(m_nav); }
	const Vector3f& GetAvoidance() const { return Inherited::GetAvoidance(m_nav); }
	float GetDistanceToDestination() const { return Inherited::GetDistanceToDestination(m_nav); }

	void AddToTargetDirection(const Vector3f& vDir) { Inherited::AddToTargetDirection(m_nav, vDir); }
	void AddToAvoidance(const Vector3f& vDir) { Inherited::AddToAvoidance(m_nav, vDir); }

	bool GetReverse() const
	{
		return Inherited::GetReverse(m_nav);
	}

	bool IsBlocked() const
	{
		return Inherited::IsBlocked(m_nav);
	}

	void SetBlocked(bool value)
	{
		Inherited::SetBlocked(m_nav, value);
	}

	float GetAgentRadius() const
	{
		return Inherited::GetAgentRadius(m_nav);
	}

	bool ApproachTarget(const ai::Target& target)
	{
		return Inherited::ApproachTarget(m_nav, target);
	}

	bool ApproachPosition(const Vector3f& vTargetPosition, float fPathRecomputationThreshold)
	{
		return Inherited::ApproachPosition(m_nav, vTargetPosition, fPathRecomputationThreshold);
	}

	bool IsApproachingWaypoint() const
	{
		return Inherited::IsApproachingWaypoint(m_nav);
	}

	bool ApproachDestination()
	{
		return Inherited::ApproachDestination(m_nav);
	}

	bool ReachedDestination() { return Inherited::ReachedDestination(m_nav); }

	void SetMove(bool bMove) { Inherited::SetMove(m_nav, bMove); }
	void SetReverse(bool bReverse) { Inherited::SetReverse(m_nav, bReverse); }

	void EnablePathfinding()
	{
		Inherited::EnablePathfinding(m_nav);
	}

	bool IsUsingPathfinding() const { return Inherited::IsUsingPathfinding(m_nav); }
	bool IsMoving() const { return Inherited::IsMoving(m_nav); }
	bool IsAvoiding() const { return Inherited::IsAvoiding(m_nav); }

	const usg::ai::NavigationGrid* GetNavigationGrid() const { return Inherited::GetNavigationGrid(m_nav); }

	usg::ai::Path& GetPath()
	{
		ASSERT(IsUsingPathfinding());
		return Inherited::GetPath(m_nav);
	}

	const usg::ai::Path& GetPath() const
	{
		ASSERT(IsUsingPathfinding());
		return Inherited::GetPath(m_nav);
	}

private:
	Components::AgentNavigationComponent& m_nav;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_NAVIGATION_WRAPPER__
