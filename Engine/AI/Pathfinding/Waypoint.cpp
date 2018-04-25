/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
//	Waypoint.cpp
#include "Engine/Common/Common.h"
#include "Waypoint.h"
#include <float.h>

namespace usg
{

namespace ai
{

Waypoint::Waypoint():
	m_links(32, true)
{
#ifdef DEBUG_BUILD
	m_debugDrawColor = Color::Green;
#endif	
}

Waypoint::Waypoint(const Vector2f& vPos):
	m_links(32, true)
{
	m_vPos = vPos;
}

Waypoint::~Waypoint()
{

}

void Waypoint::ClearLinks()
{
	m_links.Clear();
}

void Waypoint::AddLink(Waypoint* pWaypoint, float fDistance)
{
	Link* pLink = m_links.Alloc();
	pLink->Set(pWaypoint, fDistance);
}

void Waypoint::Reset()
{
	m_fG = 0.0f;
	m_fH = 0.0f;
	m_fF = 0.0f;
	m_pParent = NULL;
	m_bOpen = false;
	m_bClosed = false;
#ifdef DEBUG_BUILD
	m_debugDrawColor = Color::Green;
#endif
}

void Waypoint::SetParent(Waypoint* pParent)
{
	m_pParent = pParent;
}

Waypoint* Waypoint::GetParent() const
{
	return m_pParent;
}

const Vector2f& Waypoint::GetPosition() const
{
	return m_vPos;
}

}	//	namespace ai

}	//	namespace usg