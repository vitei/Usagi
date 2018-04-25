/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
//	Path.cpp
#include "Engine/Common/Common.h"
#include "Engine/AI/AICommon.h"
#include "Path.h"
#include "Waypoint.h"

namespace usg
{

namespace ai
{

Vector3f Path::GetNext()
{
	ASSERT(m_arraySize > 0);
	Vector3f vPos = ToVector3f(m_array[0]);
	for (uint32 i = 0; i < m_arraySize-1; i++) {
		m_array[i] = m_array[i + 1];
	}
	m_arraySize--;
	return vPos;
}

Vector3f Path::GetDestination() const
{
	const Vector2f vV = m_array[m_arraySize - 1];
	return Vector3f(vV.x, 0.0f, vV.y);
}

void Path::Reset()
{
	m_arraySize = 0;
}

void Path::AddFirst(Waypoint* pWaypoint)
{
	AddFirst(pWaypoint->GetPosition());
}

void Path::AddLast(Waypoint* pWaypoint)
{
	AddLast(pWaypoint->GetPosition());
}

void Path::AddFirst(const Vector2f& vPos)
{
	ASSERT(m_arraySize < MaxNumPositions);
	m_arraySize++;
	for (uint32 i = m_arraySize - 1; i >= 1; i--)
	{
		m_array[i] = m_array[i - 1];
	}
	m_array[0] = vPos;
}

void Path::AddLast(const Vector2f& vPos)
{
	ASSERT(m_arraySize < MaxNumPositions);
	m_array[m_arraySize++] = vPos;
}

}	//	namespace ai

}	//	namespace usg