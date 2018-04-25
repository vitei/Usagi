/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: Defines the distance between waypoints.
 *****************************************************************************/

#ifndef __USG_AI_LINK__
#define __USG_AI_LINK__

#include <stddef.h>

namespace usg
{

namespace ai
{

class Waypoint;
class Link
{
public:
	Link() : 
		m_pWaypoint(NULL),
		m_fDistance(0.0f){}
	~Link(){}

	Waypoint* GetWaypoint() const { return m_pWaypoint; }
	float Distance() const { return m_fDistance; }
	void Set(Waypoint* pWaypoint, float fDistance)
	{
		m_pWaypoint = pWaypoint;
		m_fDistance = fDistance;
	}

	void Reset()
	{
		m_pWaypoint = NULL;
		m_fDistance = 0.0f;
	}

private:
	Waypoint* m_pWaypoint;
	float m_fDistance;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_LINK__