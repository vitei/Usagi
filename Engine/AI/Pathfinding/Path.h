/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: A collection of positions, consisting of a path.
 *****************************************************************************/

#ifndef __USG_AI_PATHFINDING_PATH__
#define __USG_AI_PATHFINDING_PATH__
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Maths/Vector3f.h"
#include "Engine/Maths/Vector2f.h"

namespace usg
{

namespace ai
{
	
class Waypoint;
class Path
{
public:
	Path() : 
		m_arraySize(0) {}
	
	static const uint32 MaxNumPositions = 64;

	// returns the next position and removes it from the list, call this member function when the agent has reached its destination and is ready to continue to the next
	Vector3f GetNext();

	Vector3f GetDestination() const;
	
	void Reset();
	
	void AddFirst(Waypoint* pWaypoint);
	void AddFirst(const Vector2f& vPos);
	void AddLast(Waypoint* pWaypoint);
	void AddLast(const Vector2f& vPos);

	const Vector2f* Begin() const {
		return m_array;
	}

	const Vector2f* End() const {
		return m_array + ARRAY_SIZE(m_array);
	}

	Vector2f* Begin() {
		return m_array;
	}

	Vector2f* Last() {
		ASSERT(m_arraySize > 0);
		return m_array + (m_arraySize - 1);
	}

	Vector2f* End() {
		return m_array + m_arraySize;
	}

	uint32 Count() const {
		return m_arraySize;
	}

	Vector2f GetPosition(int iIndex) const {
		ASSERT(iIndex >= 0 && iIndex < (int)m_arraySize);
		return m_array[iIndex];
	}

private:
	Vector2f m_array[usg::ai::Path::MaxNumPositions];
	uint32 m_arraySize;
};

}	//	namespace ai

}	//	namespace usg



#endif	//	__USG_AI_PATHFINDING_PATH__