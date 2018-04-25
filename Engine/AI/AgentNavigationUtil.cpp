/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
//	AgentNavigationUtil.cpp
#include "Engine/Common/Common.h"
#include "AgentNavigationUtil.h"
#include "AgentNavigation.h"
#include "Engine/AI/Pathfinding/Path.h"
#include "Engine/AI/Pathfinding/NavigationGrid.h"
#include "Engine/AI/AICommon.h"

namespace usg {

namespace ai
{

struct AgentNavigationComponent::Data
{
	usg::ai::Path	path;			//	pathfinding path (to destination)
	usg::ai::Path	targetApproachPath;
	uint8 uTargetPathUpdate;

	Vector3f vPos;					//	the position of this agent.
	Vector3f vForward;				//	the forward direction
	Vector3f vRight;				//	the right vector

	Vector3f vTargetPos;			//	target position, where the agent wants to move
	Vector3f vDestination;			//	the final destination, if using pathfinding
	float fMinSqrDistToDestination;  //  how close to destination you need to get in order to consider destination being reached
	Vector3f vDirToDestination;		//	Normalized direction to destination
	Vector3f vAvoidance;			//	The direction of avoidance

	Vector3f vTargetDir;			//	The direction to the target position. This is should be written to if the agent want to move.

	const usg::ai::NavigationGrid* pNavGrid;	//	used for pathfinding

	float fDistanceToDest;			//	Squared distance to the destination
	float fAgentRadius;				//  Radius of the agent's collider's bounding sphere

	bool bReverse;					//	Forces the agent to move backwards
	bool bMove;						//	Tellse the agent to move forward
	bool bIsUsingPathFinding;		//	If this agent should use pathfinding or not
	bool bIsApproachingWaypoint;

	bool bHasDestination;			//	The agent has a target destination where it wants to go
	bool bAvoid;					//	if the agent is going to try avoidance, or not
	bool bBlocked;					//	if the agent is currently unable to move to its desired target because of blocking areas

	uint8 uPathFailCounter;			//	increase this number if current path appears to be have become invalid (we can't go straight from current position to our next target point).
									//  after reaching a specific number, recompute path
};

void AgentNavigationUtil::Alloc(AgentNavigationComponent* pNav)
{
	if(!pNav->pData)
	{
		pNav->pData = vnew(ALLOC_OBJECT) AgentNavigationComponent::Data;
		pNav->pData->bHasDestination = false;
		pNav->pData->bIsUsingPathFinding = false;
		pNav->pData->bIsApproachingWaypoint = false;
		pNav->pData->bMove = false;
		pNav->pData->bAvoid = false;
		pNav->pData->bReverse = 0.0f;
		pNav->pData->vDestination = Vector3f(0.0f);
		pNav->pData->vTargetDir = Vector3f(0.0f);
		pNav->pData->vAvoidance = Vector3f(0.0f);
		pNav->pData->pNavGrid = NULL;
		pNav->pData->fMinSqrDistToDestination = 2.15f;
	}
}

void AgentNavigationUtil::Initialize(Components::AgentNavigationComponent* pNav, float fAgentRadius)
{
	pNav->pData->fAgentRadius = fAgentRadius;
}

void AgentNavigationUtil::Free(AgentNavigationComponent* pNav)
{
	if(pNav->pData)
	{
		vdelete pNav->pData;
		pNav->pData = NULL;
	}
}

const Vector3f& AgentNavigationUtil::GetPosition(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->vPos;
}

const Vector3f& AgentNavigationUtil::GetForward(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->vForward;
}

const Vector3f& AgentNavigationUtil::GetRight(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->vRight;
}

const Vector3f& AgentNavigationUtil::GetNextMoveDirection(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->vTargetDir;
}

float AgentNavigationUtil::GetAgentRadius(const Components::AgentNavigationComponent& nav)
{
	return nav.pData->fAgentRadius;
}

const Vector3f& AgentNavigationUtil::GetAvoidance(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->vAvoidance;
}

const Vector3f AgentNavigationUtil::GetDirectionToNextTarget(const Components::AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	const Vector3f vToTargetPos = ToVector3f(ToVector2f(nav.pData->vTargetPos - nav.pData->vPos));
	return vToTargetPos.GetNormalisedIfNZero();
}

const Vector3f& AgentNavigationUtil::GetNextTarget(const Components::AgentNavigationComponent& nav)
{
	return nav.pData->vTargetPos;
}

const Vector3f& AgentNavigationUtil::GetDirectionToDestination(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->vDirToDestination;
}


float AgentNavigationUtil::GetDistanceToDestination(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->fDistanceToDest;
}

void AgentNavigationUtil::ResetNavigation(AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	nav.pData->vTargetDir = Vector3f(0.0f);
	nav.pData->vAvoidance = Vector3f(0.0f);
	nav.pData->bReverse = false;
	nav.pData->bMove = false;
	nav.pData->bAvoid = false;
	nav.pData->bBlocked = false;
	nav.pData->bIsApproachingWaypoint = false;
	nav.pData->vDirToDestination = (nav.pData->vDestination - nav.pData->vPos).GetNormalisedIfNZero();
	nav.pData->fDistanceToDest = nav.pData->vPos.GetSquaredXZDistanceFrom(nav.pData->vDestination);
}

const Vector3f& AgentNavigationUtil::GetDestination(const Components::AgentNavigationComponent& nav)
{
	ASSERT(HasDestination(nav));
	return nav.pData->vDestination;
}

void AgentNavigationUtil::UpdateDestination(Components::AgentNavigationComponent& nav, const Vector3f& vPos)
{
	SetDestination(nav, vPos, nav.pData->bIsUsingPathFinding);
}

void AgentNavigationUtil::ClearDestination(AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	nav.pData->bHasDestination = false;
}

void AgentNavigationUtil::UpdateNavigation(AgentNavigationComponent& nav, const Vector3f& vPos, const Vector3f& vForward, const Vector3f& vRight)
{
	ASSERT(nav.pData != NULL);
	nav.pData->vPos = vPos;
	nav.pData->vForward = vForward;
	nav.pData->vRight = vRight;
}

void AgentNavigationUtil::AddToTargetDirection(AgentNavigationComponent& nav, const Vector3f& vDir)
{
	ASSERT(nav.pData != NULL);
	nav.pData->vTargetDir += vDir;
}

void AgentNavigationUtil::AddToAvoidance(AgentNavigationComponent& nav, const Vector3f& vAvoidance)
{
	ASSERT(nav.pData != NULL);
	nav.pData->vAvoidance += vAvoidance;
	nav.pData->bAvoid = true;
}

bool AgentNavigationUtil::SetDestination(AgentNavigationComponent& nav, const Vector3f& vDestination, bool bUsePathfinding)
{
	ASSERT(nav.pData != NULL);
	if(bUsePathfinding)
	{
		ASSERT(nav.pData->pNavGrid != NULL);
		int iResults = nav.pData->pNavGrid->FindPath(nav.pData->vPos, vDestination, nav.pData->path);
		if(iResults != usg::ai::NavigationGrid::NAVIGATION_SUCCESS)
		{
			DEBUG_PRINT("[AgentNavigationUtil::SetDestination] Pathfinding ERROR destination is withing a blocking area. Review block positions.\n");
		}

		nav.pData->vDestination = vDestination;
		nav.pData->bIsUsingPathFinding = true;
		nav.pData->uPathFailCounter = 0;
		if(!SetNextTargetPosition(nav))	//	Returns true when count is zero, meaning we have reached target destination
		{
			nav.pData->bHasDestination = true;
		}
	}
	else
	{
		nav.pData->vDestination = vDestination;
		nav.pData->vTargetPos = vDestination;
		nav.pData->bHasDestination = true;
		nav.pData->bIsUsingPathFinding = false;
	}

	return true;
}

void AgentNavigationUtil::SetMove(AgentNavigationComponent& nav, bool bMove)
{
	ASSERT(nav.pData != NULL);
	nav.pData->bMove = bMove;
}

void AgentNavigationUtil::SetNavigationGrid(Components::AgentNavigationComponent& nav, const NavigationGrid* pNavGrid)
{
	nav.pData->pNavGrid = pNavGrid;
}

const NavigationGrid* AgentNavigationUtil::GetNavigationGrid(const Components::AgentNavigationComponent& nav)
{
	return nav.pData->pNavGrid;
}

void AgentNavigationUtil::EnablePathfinding(Components::AgentNavigationComponent& nav)
{
	nav.pData->bIsUsingPathFinding = true;
}

void AgentNavigationUtil::SetReverse(AgentNavigationComponent& nav, bool bReverse)
{
	ASSERT(nav.pData != NULL);
	nav.pData->bReverse = bReverse;
}

bool AgentNavigationUtil::ReachedDestination(AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	const float fSqDist = nav.pData->vPos.GetSquaredXZDistanceFrom(nav.pData->vTargetPos);
	if(fSqDist < nav.pData->fMinSqrDistToDestination)
	{
		return SetNextTargetPosition(nav);
	}
	return false;
}

bool AgentNavigationUtil::IsUsingPathfinding(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->bIsUsingPathFinding;
}

void AgentNavigationUtil::SetBlocked(Components::AgentNavigationComponent& nav, bool bMove)
{
	nav.pData->bBlocked = bMove;
}

bool AgentNavigationUtil::IsBlocked(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->bBlocked;
}

bool AgentNavigationUtil::GetReverse(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->bReverse;
}

bool AgentNavigationUtil::IsMoving(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->bMove;
}

bool AgentNavigationUtil::IsAvoiding(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->bAvoid;
}

bool AgentNavigationUtil::ApproachDestination(Components::AgentNavigationComponent& nav)
{
	ASSERT(HasDestination(nav));
	if (ReachedDestination(nav))
	{
		return true;
	}
	else
	{
		AddToTargetDirection(nav, GetDirectionToNextTarget(nav));
		nav.pData->bIsApproachingWaypoint = true;
		SetMove(nav, true);
	}
	return false;
}

bool AgentNavigationUtil::IsApproachingWaypoint(const Components::AgentNavigationComponent& nav)
{
	return nav.pData->bIsApproachingWaypoint;
}

bool AgentNavigationUtil::ApproachTarget(Components::AgentNavigationComponent& nav, const ai::Target& target)
{
	const float fSqDist = nav.pData->vPos.GetSquaredXZDistanceFrom(target.position);

	if (IsUsingPathfinding(nav))
	{
		nav.pData->uTargetPathUpdate++;
		usg::ai::Path& path = nav.pData->targetApproachPath;
		const bool bNoPath = path.Count() == 0 || (nav.pData->uTargetPathUpdate % 20 == 0);
		bool bCreateNewPath = false;
		if (bNoPath)
		{
			bCreateNewPath = true;
		}
		else
		{
			const Vector2f& vPathEnd = nav.pData->targetApproachPath.GetPosition(nav.pData->targetApproachPath.Count() - 1);
			const float fDist = vPathEnd.GetSquaredDistanceFrom(usg::ai::ToVector2f(target.position));
			if (fDist > 10 * 10)
			{
				bCreateNewPath = true;
				nav.pData->uTargetPathUpdate = 0;
			}
		}
		if (bCreateNewPath)
		{
			int iResults = nav.pData->pNavGrid->FindPath(nav.pData->vPos, target.position, path);
			if (iResults != usg::ai::NavigationGrid::NAVIGATION_SUCCESS)
			{
				path.Reset();
			}
		}
		if (path.Count() > 0)
		{
			Vector2f vNextPos = path.GetPosition(0);
			const float fDist = vNextPos.GetSquaredDistanceFrom(usg::ai::ToVector2f(nav.pData->vPos));
			if (fDist < 16)
			{
				path.GetNext();
				if (path.Count() > 0)
				{
					vNextPos = path.GetPosition(0);
				}
			}
			const Vector3f vNormalizedDirToTarget = (usg::ai::ToVector3f(vNextPos) - nav.pData->vPos).GetNormalisedIfNZero();
			AddToTargetDirection(nav, vNormalizedDirToTarget);
		}
		else
		{
			AddToTargetDirection(nav, target.normalizedDirToTarget);
		}
	}
	else
	{
		AddToTargetDirection(nav, target.normalizedDirToTarget);
	}

	SetMove(nav, true);
	if (fSqDist < nav.pData->fMinSqrDistToDestination)
	{
		return true;
	}
	return false;
}

bool AgentNavigationUtil::ApproachPosition(Components::AgentNavigationComponent& nav, const Vector3f& vTargetPosition, float fPathRecomputationThreshold)
{
	const Vector3f& vNormalizedDirToTarget = (vTargetPosition - GetPosition(nav)).GetNormalisedIfNZero();
	return ApproachPosition(nav, vTargetPosition, vNormalizedDirToTarget, fPathRecomputationThreshold);
}

bool AgentNavigationUtil::ApproachPosition(Components::AgentNavigationComponent& nav, const Vector3f& vTargetPosition, const Vector3f& vNormalizedDirectionTarget, float fPathRecomputationThreshold)
{
	if (IsUsingPathfinding(nav))
	{
		if (!HasDestination(nav))
		{
			SetDestination(nav, vTargetPosition, true);
		}

		const Vector3f& vPos = GetPosition(nav);

		// Check if we need to generate a new path. This can happen if:
		// 1) Target has moved too far away from the end point of our current path.
		// 2) We unable to go straight to next waypoint. Perhaps a projectile has hit us and distrayed us from the path?
		bool bUpdatePath = false;
		const Vector3f& vDestination = GetDestination(nav);
		if (vDestination.GetSquaredXZDistanceFrom(vTargetPosition) > fPathRecomputationThreshold*fPathRecomputationThreshold)
		{
			nav.pData->uPathFailCounter++;
		}
		const Vector3f& vNextWayPointPosition = GetNextTarget(nav);
		const NavigationGrid* pGrid = nav.pData->pNavGrid;
		if (pGrid->LineTest(Vector2f(vPos.x, vPos.z), Vector2f(vNextWayPointPosition.x, vNextWayPointPosition.z)))
		{
			nav.pData->uPathFailCounter++;
		}
		if (nav.pData->uPathFailCounter >= 6)
		{
			UpdateDestination(nav, vTargetPosition);
		}
		return ApproachDestination(nav);
	}
	else
	{
		if (ReachedDestination(nav))
		{
			return true;
		}
		else
		{
			AddToTargetDirection(nav, vNormalizedDirectionTarget);
		}
		SetMove(nav, true);
		return false;
	}
}

bool AgentNavigationUtil::HasDestination(const AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->bHasDestination;
}

Path& AgentNavigationUtil::GetPath(Components::AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	return nav.pData->path;
}

bool AgentNavigationUtil::SetNextTargetPosition(AgentNavigationComponent& nav)
{
	ASSERT(nav.pData != NULL);
	if(nav.pData->path.Count() > 0)
	{
		nav.pData->vTargetPos = nav.pData->path.GetNext();
		return false;
	}
	//	When count is zero in our path then we have reached the requested destination
	nav.pData->bHasDestination = false;
	return true;
}

}	//	namespace ai

}	//	namespace usg
