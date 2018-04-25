/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Helper functions for ai navigation
*****************************************************************************/
#ifndef __USG_AI_AGENT_NAVIGATION_UTIL__
#define __USG_AI_AGENT_NAVIGATION_UTIL__
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector3f.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/AI/Targetting/Target.pb.h"

namespace usg
{

namespace ai
{

	class NavigationGrid;
	class Path;

namespace Components
{

struct AgentNavigationComponent;

}

class AgentNavigationUtil
{
public:
	static void Alloc(Components::AgentNavigationComponent* pNav);
	static void Free(Components::AgentNavigationComponent* pNav);
	static void Initialize(Components::AgentNavigationComponent* pNav, float fAgentRadius);
protected:
	static const Vector3f& GetPosition(const Components::AgentNavigationComponent& nav);
	static const Vector3f& GetForward(const Components::AgentNavigationComponent& nav);
	static const Vector3f& GetRight(const Components::AgentNavigationComponent& nav);
	static const Vector3f& GetDirectionToDestination(const Components::AgentNavigationComponent& nav);
	static const Vector3f GetDirectionToNextTarget(const Components::AgentNavigationComponent& nav);
	static const Vector3f& GetNextMoveDirection(const Components::AgentNavigationComponent& nav);
	static const Vector3f& GetNextTarget(const Components::AgentNavigationComponent& nav);
	static const Vector3f& GetAvoidance(const Components::AgentNavigationComponent& nav);
	static float GetDistanceToDestination(const Components::AgentNavigationComponent& nav);
	static const NavigationGrid* GetNavigationGrid(const Components::AgentNavigationComponent& nav);
	static float GetAgentRadius(const Components::AgentNavigationComponent& nav);

	//	This should be called every time before ticking a tree
	static void ResetNavigation(Components::AgentNavigationComponent& nav);

	// Change current destination. A new path is computed if using pathfinding.
	static void UpdateDestination(Components::AgentNavigationComponent& nav, const Vector3f& vPos);

	static void ClearDestination(Components::AgentNavigationComponent& nav);
	static const Vector3f& GetDestination(const Components::AgentNavigationComponent& nav);

	static void UpdateNavigation(Components::AgentNavigationComponent& nav, const Vector3f& vPos, const Vector3f& vForward, const Vector3f& vRight);
	static void AddToTargetDirection(Components::AgentNavigationComponent& nav, const Vector3f& vDir);
	static void AddToAvoidance(Components::AgentNavigationComponent& nav, const Vector3f& vAvoidance);
	static bool SetDestination(Components::AgentNavigationComponent& nav, const Vector3f& vDestination, bool bUsePathfinding);

	static void SetMove(Components::AgentNavigationComponent& nav, bool bMove);
	static void SetReverse(Components::AgentNavigationComponent& nav, bool bReverse);
	static void EnablePathfinding(Components::AgentNavigationComponent& nav);
	static void SetNavigationGrid(Components::AgentNavigationComponent& nav, const NavigationGrid* pNavGrid);

	static bool ReachedDestination(Components::AgentNavigationComponent& nav);

	static void SetBlocked(Components::AgentNavigationComponent& nav, bool bMove);
	static bool IsBlocked(const Components::AgentNavigationComponent& nav);
	static bool IsUsingPathfinding(const Components::AgentNavigationComponent& nav);
	static bool GetReverse(const AgentNavigationComponent& nav);
	static bool IsMoving(const Components::AgentNavigationComponent& nav);
	static bool IsAvoiding(const Components::AgentNavigationComponent& nav);
	static bool HasDestination(const Components::AgentNavigationComponent& nav);
	static Path& GetPath(Components::AgentNavigationComponent& nav);

	static bool IsApproachingWaypoint(const Components::AgentNavigationComponent& nav);

	// Approach target using pathfinding (if bPathFinding set to true for the agent). Returns true if target is reached.
	static bool ApproachTarget(Components::AgentNavigationComponent& nav, const ai::Target& target);

	// Approach target using pathfinding. Returns true if target is reached.
	static bool ApproachPosition(Components::AgentNavigationComponent& nav, const Vector3f& target, float fPathRecomputationThreshold);

	// Approach current destination using pathfinding (if bPathFinding set to true for the agent). Returns true if target is reached.
	static bool ApproachDestination(Components::AgentNavigationComponent& nav);
private:
	static bool SetNextTargetPosition(Components::AgentNavigationComponent& nav);	// Used in pathfinding to set the next position
	static bool ApproachPosition(Components::AgentNavigationComponent& nav, const Vector3f& target, const Vector3f& normalizedDirectionTarget, float fPathRecomputationThreshold);
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_AGENT_NAVIGATION_UTIL__
