/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: Infinite space with blocks and waypoints where an agent can navigate.
 *****************************************************************************/

#ifndef __USG_AI_NAVIGATION_GRID__
#define __USG_AI_NAVIGATION_GRID__


#include "Engine/Memory/FastPool.h"
#include "Engine/Core/stl/list.h"
#include "Engine/Maths/Vector3f.h"
#include "Shape.h"
#include "Waypoint.h"
#include "BlockingArea.h"

namespace usg
{

class Debug3D;

namespace ai
{

class Path;
class NavigationGrid
{
public:
	static const uint32 MaxNumBlocks = 256;
	static const uint32 MaxNumWaypoints = 256;

	NavigationGrid();
	virtual ~NavigationGrid();

	bool CreateWaypoint(const Vector2f& vPos, uint32 uNameHash);
	bool CreateWaypoint(const Vector3f& vPos, uint32 uNameHash);
	Waypoint* GetClosestWaypoint(const Vector2f& vPos) const;
	Waypoint* GetClosestWaypointAccessibleFrom(const Vector2f& vPos) const;
	const usg::ai::OBB* GetNearestOBB(const Vector3f& vPos) const;
	const usg::ai::OBB* GetNearestOBB(const Vector2f& vPos) const;
	const Waypoint* GetWaypoint(uint32 uNameHash) const;

	uint32 GetNearestOBBs(const Vector2f& vPos, usg::ai::OBB** pBuffer, uint32 uSize) const;
	uint32 GetNearestOBBs(const Vector3f& vPos, usg::ai::OBB** pBuffer, uint32 uSize) const;

	const usg::ai::OBB* GetNearestIntersectingOBB(const Vector3f& vPos) const;
	const usg::ai::OBB* GetNearestIntersectingOBB(const Vector2f& vPos) const;

	enum 
	{
		NAVIGATION_SUCCESS = 0,
		NAVIGATION_START_BLOCKED = -1,
		NAVIGATION_END_BLOCKED = -2,
	};

	// If return value is any other than 0 then somethign wrong has happened, use GetError() to find out what
	int FindPath(const Vector3f& vStart, const Vector3f& vEnd, usg::ai::Path& path) const;
	// If return value is any other than 0 then somethign wrong has happened, use GetError() to find out what
	int FindPath(const Vector2f& vStart, const Vector2f& vEnd, usg::ai::Path& path) const;

	void BuildLinks();

	// Returns true if the line intersects any of the blocks
	bool LineTest(const usg::ai::Line& line) const;
	bool LineTest(const Vector2f& vFrom, const Vector2f& vTo) const;
	bool CanPlacePoint(const Vector2f& vPoint) const;
	uint32 GetOverlappingShapes(IShape** pShapes, uint32 uSize, usg::ai::Line) const;

	const BlockingArea& CreateBlock(const Vector3f& vPos, float fWidth, float fHeight, float fAngle, bool bAddWaypoints);
	const BlockingArea& CreateBlock(float fX, float fY, float fWidth, float fHeight, float fAngle, bool bAddWaypoints);
	const BlockingArea& CreateBlock(const Vector2f& vPos, float fWidth, float fHeight, float fAngle, bool bAddWaypoints);

	void RemoveBlock(uint32 uUID);

	void DebugDrawBlocks(Debug3D* pDebugRender) const;
	void DebugDrawWaypoints(Debug3D* pDebugRender) const;
	void DebugDrawLinks(Debug3D* pDebugRender) const;

	bool GetNearestValidPoint(const IShape* pShape, const Vector2f& vPoint, const Vector2f& vOrigin, Vector2f& vOutPoint) const;
#ifdef DEBUG_BUILD
	// Return false if something is wrong with the grid (ie. waypoint defined inside a blocking area).
	bool CheckIntegrity();
#endif
private:
	void FindPath(Waypoint* pStart, Waypoint* pEnd, usg::ai::Path& path) const;
	void SubdividePath(usg::ai::Path& path) const;
	void PathVisibility(usg::ai::Path& path) const;
	void Reset() const;

	uint32 m_uBlockingAreaUIDCounter;

	FastPool<usg::ai::BlockingArea> m_blocks;
	FastPool<usg::ai::Waypoint> m_waypoints;
	mutable list<usg::ai::Waypoint*> m_openList;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_NAVIGATION_GRID__