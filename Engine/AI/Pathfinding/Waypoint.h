/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: Used by pathfinding to guide agents in the world.
 *****************************************************************************/

#ifndef __USG_AI_WAYPOINT__
#define __USG_AI_WAYPOINT__


#include "Engine/Memory/FastPool.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/Graphics/Color.h"
#include "Link.h"

namespace usg
{

namespace ai
{

class Waypoint
{
public:
	Waypoint();
	Waypoint(const Vector2f& vPos);
	~Waypoint();

	void Reset();

	void SetParent(Waypoint* pWaypoint);
	Waypoint* GetParent() const;
	void SetPosition(const Vector2f& vPos) { m_vPos = vPos; }
	const Vector2f& GetPosition() const;

	uint32 GetNameHash() const
	{
		return m_uNameHash;
	}

	void SetNameHash(uint32 uNameHash)
	{
		m_uNameHash = uNameHash;
	}

	void SetG(float fG) { m_fG = fG; }
	void SetF(float fF) { m_fF = fF; }

	float G() const { return m_fG; }
	float F() const { return m_fF; }

	bool IsClosed() const { return m_bClosed; }
	bool IsOpen() const { return m_bOpen; }

	void SetClosed(bool bClosed) { m_bClosed = bClosed; }
	void SetOpen(bool bOpen) { m_bOpen = bOpen; }

	static const uint32 Size = 256;

	void AddLink(Waypoint* pWaypoint, float fDistance);
	void ClearLinks();

	FastPool<Link>::Iterator GetLinkIterator() { return m_links.Begin(); }

#ifdef DEBUG_BUILD
	Color m_debugDrawColor;
#endif
private:
	uint32 m_uNameHash;
	FastPool<Link> m_links;
	usg::Vector2f m_vPos;
	Waypoint* m_pParent;
	float m_fG;
	float m_fH;
	float m_fF;
	bool m_bOpen;
	bool m_bClosed;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_WAYPOINT__