/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
//	NavigationGrid.cpp
#include "Engine/Common/Common.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "NavigationGrid.h"
#include "Path.h"
#include <float.h>

namespace usg
{
namespace ai
{

NavigationGrid::NavigationGrid() :
	m_uBlockingAreaUIDCounter(0),
	m_blocks(NavigationGrid::MaxNumBlocks),
	m_waypoints(NavigationGrid::MaxNumWaypoints)
{
}

NavigationGrid::~NavigationGrid()
{
	m_blocks.Clear();
	m_waypoints.Clear();
}

#ifdef DEBUG_BUILD
bool NavigationGrid::CheckIntegrity()
{
	for (FastPool<Waypoint>::Iterator it = m_waypoints.Begin(); !it.IsEnd(); ++it)
	{
		const Waypoint* pWaypoint = (*it);
		if (!CanPlacePoint(pWaypoint->GetPosition()))
		{
			DEBUG_PRINT("Waypoint at (%f,%f) is inside blocking area.\n",pWaypoint->GetPosition().x,pWaypoint->GetPosition().y);
			return false;
		}
	}
	return true;
}
#endif

bool NavigationGrid::CreateWaypoint(const Vector3f& vPos, uint32 uNameHash)
{
	return CreateWaypoint(Vector2f(vPos.x, vPos.z), uNameHash);
}

const Waypoint* NavigationGrid::GetWaypoint(uint32 uNameHash) const
{
	for (FastPool<Waypoint>::Iterator it = m_waypoints.Begin(); !it.IsEnd(); ++it)
	{
		const Waypoint* pWaypoint = *it;
		if (pWaypoint->GetNameHash() == uNameHash)
		{
			return pWaypoint;
		}
	}
	return NULL;
}

bool NavigationGrid::CreateWaypoint(const Vector2f& vPos, uint32 uNameHash)
{
	Waypoint* pWaypoint = m_waypoints.Alloc();
	pWaypoint->SetPosition(vPos);
	pWaypoint->SetNameHash(uNameHash);
	return true;
}

Waypoint* NavigationGrid::GetClosestWaypointAccessibleFrom(const Vector2f& vPos) const
{
	Waypoint* pWaypoint = NULL;
	Line line;
	line.m_vFrom = vPos;
	float fMinSqrDist = FLT_MAX;
	for (FastPool<Waypoint>::Iterator it = m_waypoints.Begin(); !it.IsEnd(); ++it)
	{
		const Vector2f& vWaypointPosition = (*it)->GetPosition();
		const float fSqrDist = vPos.GetSquaredDistanceFrom(vWaypointPosition);
		if (fSqrDist < fMinSqrDist)
		{
			line.m_vTo = vWaypointPosition;
			if (!LineTest(line))
			{
				fMinSqrDist = fSqrDist;
				pWaypoint = (*it);
			}
		}
	}
	return pWaypoint;
}

Waypoint* NavigationGrid::GetClosestWaypoint(const Vector2f& vPos) const
{
	Waypoint* pWaypoint = NULL;
	float fMinSqrDist = FLT_MAX;
	for(FastPool<Waypoint>::Iterator it = m_waypoints.Begin(); !it.IsEnd(); ++it)
	{
		const float fSqrDist = vPos.GetSquaredDistanceFrom((*it)->GetPosition());
		if(fSqrDist < fMinSqrDist)
		{
			fMinSqrDist = fSqrDist;
			pWaypoint = (*it);
		}
	}
	return pWaypoint;
}

int NavigationGrid::FindPath(const Vector3f& vStart, const Vector3f& vEnd, usg::ai::Path& path) const
{
	Vector2f vStartV2(vStart.x, vStart.z);
	Vector2f vEndV2(vEnd.x, vEnd.z);
	return FindPath(vStartV2, vEndV2, path);
}

int NavigationGrid::FindPath(const Vector2f& vStart, const Vector2f& vEnd, usg::ai::Path& path) const
{
	int rval = NAVIGATION_SUCCESS;
	path.Reset();

	Waypoint start(vStart);
	Waypoint end(vEnd);

	// An important optimization: if you can go straight to target, construct a minimal path:
	if (!LineTest(usg::ai::Line(vStart, vEnd)))
	{
		path.AddLast(vEnd);
		return NAVIGATION_SUCCESS;
	}

	// Test if start/end point is inside blocking area. If yes, go to nearest waypoint as a fallback.
	Waypoint* pClosestToStart = GetClosestWaypointAccessibleFrom(vStart);
	if (pClosestToStart == NULL) pClosestToStart = GetClosestWaypoint(vStart);
	if (!pClosestToStart)
	{
		path.AddLast(vEnd);
		return NAVIGATION_START_BLOCKED;
	}
	Waypoint* pClosestToEnd = GetClosestWaypointAccessibleFrom(vEnd);
	if (pClosestToEnd == NULL) pClosestToEnd = GetClosestWaypoint(vEnd);
	if (!CanPlacePoint(vEnd))
	{
		if (pClosestToEnd != NULL)
		{
			const float fDistToToTarget = vStart.GetSquaredDistanceFrom(vEnd);
			const float fDistToToClosestWaypoint = vStart.GetSquaredDistanceFrom(pClosestToEnd->GetPosition());
			if (fDistToToClosestWaypoint > fDistToToTarget)
			{
				// At least going to nearest waypoint takes us closer to target, so let
				path.AddLast(vEnd);
				return NAVIGATION_END_BLOCKED;
			}
		}
		else
		{
			path.AddLast(vEnd);
			return NAVIGATION_END_BLOCKED;
		}
	}
	FindPath(pClosestToStart, pClosestToEnd, path);

	path.AddFirst(&start);
	path.AddLast(&end);

	SubdividePath(path);
	SubdividePath(path);

	PathVisibility(path);

	path.GetNext();	//	remove the first position because this is basically where we are standing right now, unless we changed it
	
	return rval;
}

void NavigationGrid::SubdividePath(usg::ai::Path& path) const
{
	usg::ai::Path res;
	Vector2f* pvPrev = NULL;
	for(Vector2f* it = path.Begin(); it != path.End(); ++it)
	{
		const Vector2f& vPos = (*it);
		if(pvPrev != NULL)
		{
			res.AddLast(Vector2f((pvPrev->x + vPos.x) * 0.5f, (pvPrev->y + vPos.y) * 0.5f));
		}
		res.AddLast(vPos);
		pvPrev = it;
	}

	path = res;
}

void NavigationGrid::PathVisibility(usg::ai::Path& path) const
{
	usg::ai::Path res;

	for (Vector2f* it = path.Begin(); it != path.End(); ++it)
	{
		res.AddLast(*it);
	}

	Vector2f* pvPrev = NULL;
	Vector2f* pvPrevPrev = NULL;
	Vector2f* pvCurrent = NULL;
	Vector2f* pvNext = NULL;
	Vector2f* pvLast = NULL;

	path.Reset();

	pvPrev = (res.Begin());
	pvCurrent = (res.Last());

	if (!LineTest(usg::ai::Line(*pvCurrent, *pvPrev)))
	{
		path.AddLast(*pvPrev);
		path.AddLast(*pvCurrent);
		return;
	}

	pvCurrent = res.Begin();
	path.AddLast(*pvCurrent);
	pvLast = (res.Last());
	Vector2f* nextIt = res.Begin();
	Vector2f* prevIt = res.Begin();
	int iT = 0;


	while (pvCurrent != NULL && nextIt != res.End())
	{
		++nextIt;
		pvNext = nextIt;

		if (!LineTest(usg::ai::Line(*pvCurrent, *pvNext)))
		{
			pvPrev = pvNext;
			prevIt = nextIt;
		}
		else
		{
			//	If the previous of the previous location is the same, then the subdivision probably slightly pushed it inside a block se lets just move on.
			if (pvPrev != pvPrevPrev)
			{
				path.AddLast(*pvPrev);
				pvPrevPrev = pvPrev;
				pvCurrent = pvPrev;
				nextIt = prevIt;
			}
			else
			{
				pvPrev = pvNext;
				prevIt = nextIt;
			}
		}

		if (pvNext == pvLast)
		{
			break;
		}
	}

	path.AddLast(*pvLast);
}

const BlockingArea& NavigationGrid::CreateBlock(const Vector3f& vPos, float fWidth, float fHeight, float fAngle, bool bAddWaypoints)
{
	Vector2f vPosV2(vPos.x, vPos.z);
	return CreateBlock(vPosV2, fWidth, fHeight, fAngle, bAddWaypoints);
}

const BlockingArea& NavigationGrid::CreateBlock(float fX, float fY, float fWidth, float fHeight, float fAngle, bool bAddWaypoints)
{
	return CreateBlock(Vector2f(fX, fY), fWidth, fHeight, fAngle, bAddWaypoints);
}

void NavigationGrid::RemoveBlock(uint32 uUID)
{
	for (FastPool<usg::ai::BlockingArea>::Iterator it = m_blocks.Begin(); !it.IsEnd(); ++it)
	{
		if ((*it)->uUID == uUID)
		{
			m_blocks.Free(*it);
			break;
		}
	}
}

const BlockingArea& NavigationGrid::CreateBlock(const Vector2f& vPos, float fWidth, float fHeight, float fAngle, bool bAddWaypoints)
{
	usg::ai::BlockingArea* pNewBlockingArea = m_blocks.Alloc();
	pNewBlockingArea->uUID = m_uBlockingAreaUIDCounter++;
	usg::ai::OBB* pOBB = &pNewBlockingArea->shape;
	pOBB->SetCenter(vPos);
	pOBB->SetScale(Vector2f(fWidth, fHeight));
	pOBB->SetRotation(fAngle);

	if(bAddWaypoints)
	{
		const uint32 uCount = pOBB->GetNumVerts();
		const Vector2f* pVerts = pOBB->GetVerts();
		const float fOffset = 2.5f;

		for(uint32 i = 0; i < uCount; i++)
		{
			Vector2f vPoint = vPos + pVerts[i];
			const Vector2f vDirToCenter = (vPoint - vPos).GetNormalised();
			vPoint += vDirToCenter * fOffset;
			CreateWaypoint(vPoint,0);
		}
	}

	return *pNewBlockingArea;
}

void NavigationGrid::FindPath(Waypoint* pStart, Waypoint* pEnd, usg::ai::Path& path) const
{
	if(!pStart || !pEnd)
	{
		return;
	}

	Reset();
	
	pStart->SetG(0.0f);
	pStart->SetF(0.0f);

	pStart->SetOpen(true);
	m_openList.AddToEnd(pStart);
	Waypoint* pCurrent = NULL;

	while(m_openList.GetSize() > 0)
	{
		float fDistance = FLT_MAX;

		for(List<Waypoint>::Iterator it = m_openList.Begin(); !it.IsEnd(); ++it)
		{
			Waypoint* pWp = (*it);
			if(pWp->F() < fDistance)
			{
				pCurrent = pWp;
				fDistance = pWp->F();
			}
		}

		if(pCurrent == pEnd)
		{
			while(pCurrent != NULL)
			{
#ifdef DEBUG_BUILD
				pCurrent->m_debugDrawColor = Color::Red;
#endif
				path.AddFirst(pCurrent);
				pCurrent = pCurrent->GetParent();				
			}

			return;
		}

		m_openList.Remove(pCurrent);
		pCurrent->SetClosed(true);
		pCurrent->SetOpen(false);

		for(FastPool<Link>::Iterator it = pCurrent->GetLinkIterator(); !it.IsEnd(); ++it)
		{
			Link* pLink = (*it);
			Waypoint* pLinkWp = pLink->GetWaypoint();
			if(!pLinkWp->IsClosed())
			{
				const float fTentativeG = pCurrent->G() + pLink->Distance();
				bool bIsBetter = false;

				if(!pLinkWp->IsOpen())
				{
					m_openList.AddToFront(pLinkWp);
					pLinkWp->SetOpen(true);
					pLinkWp->SetClosed(false);
					bIsBetter = true;
				}
				else
				{
					if(fTentativeG < pLinkWp->G())
					{
						bIsBetter = true;
					}
				}

				if(bIsBetter)
				{
					pLinkWp->SetParent(pCurrent);
					pLinkWp->SetG(fTentativeG);
					pLinkWp->SetF(pLinkWp->G() + pLinkWp->GetPosition().Distance(pEnd->GetPosition()));
				}
			}
		}
	}
	pCurrent = pEnd;

	while(pCurrent != NULL)
	{
		path.AddFirst(pCurrent);
#ifdef DEBUG_BUILD
		pCurrent->m_debugDrawColor = Color::Red;
#endif
		pCurrent = pCurrent->GetParent();
	}
}

void NavigationGrid::BuildLinks()
{
	for(FastPool<Waypoint>::Iterator it = m_waypoints.Begin(); !it.IsEnd(); ++it)
	{
		Waypoint* pCurrent = (*it);
		pCurrent->ClearLinks();
		const Vector2f vFrom = pCurrent->GetPosition();
		for(FastPool<Waypoint>::Iterator it2 = m_waypoints.Begin(); !it2.IsEnd(); ++it2)
		{
			Waypoint* pTarget = (*it2);
			if(pCurrent != pTarget)
			{
				const Vector2f vTo = pTarget->GetPosition();
				if(!LineTest(usg::ai::Line(vFrom, vTo)))
				{
					const float fDistance = vFrom.Distance(vTo);
					pCurrent->AddLink((pTarget), fDistance);
				}
			}
		}
	}
}

bool NavigationGrid::LineTest(const Vector2f& vFrom, const Vector2f& vTo) const
{
	Line line;
	line.m_vFrom = vFrom;
	line.m_vTo = vTo;
	return LineTest(line);
}

bool NavigationGrid::LineTest(const usg::ai::Line& line) const
{
	IShape* pShapes[32];
	uint32 uSize = GetOverlappingShapes(&pShapes[0], 32, line);

	for (uint32 i = 0; i < uSize; i++)
	{
		if (pShapes[i]->Intersects(line))
		{
			return true;
		}
	}

	return false;
}

bool NavigationGrid::CanPlacePoint(const Vector2f& vPoint) const
{
	const uint64 uPointMask = shape_details::ComputeGridMask(vPoint);
	for(FastPool<usg::ai::BlockingArea>::Iterator it = m_blocks.Begin(); !it.IsEnd(); ++it)
	{
		const bool bMaskFail = ((*it)->shape.GetGridMask() & uPointMask) == 0;
		if (!bMaskFail)
		{
			if ((*it)->shape.Intersects(vPoint))
			{
				return false;
			}
		}
	}
	return true;
}

uint32 NavigationGrid::GetOverlappingShapes(IShape** pShapes, uint32 uSize, usg::ai::Line line) const
{
	uint32 uCounter = 0;
	Vector2f hit;

	const uint64 uLineMask = shape_details::ComputeGridMask(line.m_vFrom, line.m_vTo);

	for(FastPool<usg::ai::BlockingArea>::Iterator it = m_blocks.Begin(); !it.IsEnd() && (uCounter < uSize); ++it)
	{
		const bool bMaskFail = ((*it)->shape.GetGridMask() & uLineMask) == 0;
		if (!bMaskFail)
		{
			if ((*it)->shape.Intersects(line, hit))
			{
				pShapes[uCounter++] = &(*it)->shape;
			}
		}
	}
	return uCounter;
}

const usg::ai::OBB* NavigationGrid::GetNearestOBB(const Vector3f& vPos) const
{
	return GetNearestOBB(Vector2f(vPos.x, vPos.z));
}

const usg::ai::OBB* NavigationGrid::GetNearestOBB(const Vector2f& vPos) const
{
	float fDistance = FLT_MAX;
	const usg::ai::OBB* pClosestOBB = NULL;
	for(FastPool<usg::ai::BlockingArea>::Iterator it = m_blocks.Begin(); !it.IsEnd(); ++it)
	{
		const usg::ai::OBB* pOBB = &(*it)->shape;
		const Vector2f& vOBB = pOBB->GetCenter();
		const float fDistanceToTargetSq = (vOBB - vPos).MagnitudeSquared();
		if(fDistanceToTargetSq < fDistance)
		{
			fDistance = fDistanceToTargetSq;
			pClosestOBB = pOBB;
		}
	}

	return pClosestOBB;
}

uint32 NavigationGrid::GetNearestOBBs(const Vector2f& vPos, usg::ai::OBB** pBuffer, uint32 uSize) const
{
	if (uSize == 0)
	{
		return 0;
	}
	float fDistance = FLT_MAX;

	for(FastPool<usg::ai::BlockingArea>::Iterator it = m_blocks.Begin(); !it.IsEnd(); ++it)
	{
		usg::ai::OBB* pOBB = &(*it)->shape;

		const float fDistanceToTargetSq = pOBB->GetSquaredDistanceToPoint(vPos);
		if(fDistanceToTargetSq < fDistance)
		{
			fDistance = fDistanceToTargetSq;
			utl::MoveElements(pBuffer, 1, 0, uSize - 1);
			pBuffer[0] = pOBB;
		}
		else
		{
			for(uint32 i = 1; i < uSize; i++)
			{
				if(pBuffer[i] == NULL)
				{
					pBuffer[i] = pOBB;
					break;
				}
				else if(fDistanceToTargetSq < pBuffer[i]->GetSquaredDistanceToPoint(vPos))
				{
					if(i < uSize - 2)
					{
						utl::MoveElements(pBuffer, i + 1, i, uSize - i - 1);
					}
					pBuffer[i] = pOBB;
					break;
				}
			}
		}
	}
	if (pBuffer[uSize - 1] != NULL)
	{
		// Super likely
		return uSize;
	}
	uint32 uCount = 0;
	for (uint32 i = 1; i < uSize; i++)
	{
		if (pBuffer[i] != NULL)
		{
			uCount++;
		}
	}
	return uCount;
}

uint32 NavigationGrid::GetNearestOBBs(const Vector3f& vPos, usg::ai::OBB** pBuffer, uint32 uSize) const
{
	return GetNearestOBBs(Vector2f(vPos.x, vPos.z), pBuffer, uSize);
}

const usg::ai::OBB* NavigationGrid::GetNearestIntersectingOBB(const Vector3f& vPos) const
{
	return GetNearestIntersectingOBB(Vector2f(vPos.x, vPos.z));
}

const usg::ai::OBB* NavigationGrid::GetNearestIntersectingOBB(const Vector2f& vPos) const
{

	const uint64 uPointMask = shape_details::ComputeGridMask(vPos);
	for(FastPool<usg::ai::BlockingArea>::Iterator it = m_blocks.Begin(); !it.IsEnd(); ++it)
	{
		const bool bMaskFail = ((*it)->shape.GetGridMask() & uPointMask) == 0;
		if (bMaskFail)
		{
			continue;
		}
		if((*it)->shape.Intersects(vPos))
		{
			return &(*it)->shape;
		}
	}
	return NULL;
}

void NavigationGrid::Reset() const
{
	for(FastPool<Waypoint>::Iterator it = m_waypoints.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->Reset();
	}
	m_openList.Clear();
}

void NavigationGrid::DebugDrawBlocks(Debug3D* pDebugRender) const
{
	ASSERT(pDebugRender != NULL);
	Color blue = Color::Blue;
	blue.m_fA = 0.5f;
	for(FastPool<usg::ai::BlockingArea>::Iterator it = m_blocks.Begin(); !it.IsEnd(); ++it)
	{
		const usg::ai::OBB* pOBB = &(*it)->shape;
		const Vector2f vPos = pOBB->GetCenter();
		const Vector2f vScale = pOBB->GetScale();
		const float fAngle = pOBB->GetAngle();
		usg::Matrix4x4 m;
		m.LoadIdentity();
		m.MakeRotateY(usg::Math::DegToRad(fAngle));
		m.Scale(vScale.x * 0.5f, 20.0f, vScale.y * 0.5f, 1.0f);
		m.SetPos(usg::Vector3f(vPos.x, 10.0f, vPos.y));

		pDebugRender->AddCube(m, blue);
	}
}

void NavigationGrid::DebugDrawWaypoints(Debug3D* pDebugRender) const
{
#ifdef DEBUG_BUILD
	ASSERT(pDebugRender != NULL);
	for(FastPool<usg::ai::Waypoint>::Iterator it = m_waypoints.Begin(); !it.IsEnd(); ++it)
	{
		usg::ai::Waypoint& wp = *(*it);
		const Vector2f& vPos = wp.GetPosition();
		usg::Matrix4x4 m;
		m.LoadIdentity();
		m.Scale(1.0f,1.0f,1.0f, 1.0f);
		m.SetPos(usg::Vector3f(vPos.x, 10.0f, vPos.y));
		Color green = wp.m_debugDrawColor;
		green.m_fA = 0.5f;
		pDebugRender->AddCube(m, green);
	}
#endif
}

void NavigationGrid::DebugDrawLinks(Debug3D* pDebugRender) const
{
	ASSERT(pDebugRender != NULL);
	Color red = Color::Red;
	red.m_fA = 0.5f;

	for(FastPool<usg::ai::Waypoint>::Iterator it = m_waypoints.Begin(); !it.IsEnd(); ++it)
	{
		usg::ai::Waypoint& wp = *(*it);
		const Vector2f& vPos = wp.GetPosition();
		FastPool<usg::ai::Link>::Iterator it2 = wp.GetLinkIterator();
		for(; !it2.IsEnd(); ++it2)
		{
			const Vector2f vV = (*it2)->GetWaypoint()->GetPosition();
			pDebugRender->AddLine(Vector3f(vPos.x, 20.0f, vPos.y), Vector3f(vV.x, 20.0f, vV.y), red, 1.0f);
		}
	}
}

bool NavigationGrid::GetNearestValidPoint(const IShape* pShape, const Vector2f& vPoint, const Vector2f& vOrigin, Vector2f& vOutPoint) const
{
	const IShape* pOBB = (pShape == NULL) ? GetNearestIntersectingOBB(vPoint) : pShape;
	if(pOBB != NULL)
	{
		Vector2f vIP1, vIP2;
		usg::ai::Line line;
		line.m_vFrom = vOrigin;
		line.m_vTo = vPoint;
		int iIntersectionCount = pOBB->Intersects(line, vIP1, vIP2);
		if (iIntersectionCount == 1)
		{
			// Great! Move slightly toward vOrigin from the intersection points
			vOutPoint = vIP1 + (vOrigin-vIP1)*0.001f;
			return true;
		}
	}
	return false;
}

}	//	namespace ai

}	//	namespace usg
