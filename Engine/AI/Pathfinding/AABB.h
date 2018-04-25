/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: 2D Axis aligned bounding box, used by OBB for initial sight test
 *****************************************************************************/

#ifndef __USG_AI_AABB__
#define __USG_AI_AABB__

#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector2f.h"

namespace usg
{

namespace ai
{

struct AABB
{
	AABB():
		m_vPos(0.0f, 0.0f),
		m_vSize(0.0f, 0.0f)
	{}

	AABB(const usg::Vector2f& vPos, const usg::Vector2f& vSize):
		m_vPos(vPos),
		m_vSize(vSize)
	{}

	~AABB()
	{}

	void Set(const usg::Vector2f& vPos, const Vector2f& vSize)
	{
		m_vPos = vPos;
		m_vSize = vSize;
	}

	bool Intersects(float fX, float fY) const
	{
		const usg::Vector2f vPoint(fX, fY);
		usg::Vector2f vExtents = GetExtents();

		if(vPoint.x < (m_vPos.x - vExtents.x)) { return false; }
		if(vPoint.x > (m_vPos.x + vExtents.x)) { return false; }
		if(vPoint.y < (m_vPos.y - vExtents.y)) { return false; }
		if(vPoint.y > (m_vPos.y + vExtents.y)) { return false; }

		return true;
	}

	bool Intersects(const Vector2f& vPoint) const
	{
		usg::Vector2f vExtents = GetExtents();

		if(vPoint.x < (m_vPos.x - vExtents.x)) { return false; }
		if(vPoint.x > (m_vPos.x + vExtents.x)) { return false; }
		if(vPoint.y < (m_vPos.y - vExtents.y)) { return false; }
		if(vPoint.y > (m_vPos.y + vExtents.y)) { return false; }

		return true;
	}

	bool Intersects(const AABB& aabb) const
	{
		const usg::Vector2f vPoint(aabb.m_vPos);
		const usg::Vector2f vPointExtents = aabb.GetExtents();
		const usg::Vector2f vExtents = GetExtents();

		if((vPoint.x + vPointExtents.x) < (m_vPos.x - vExtents.x)) { return false; }
		if((vPoint.x - vPointExtents.x) > (m_vPos.x + vExtents.x)) { return false; }
		if((vPoint.y + vPointExtents.y) < (m_vPos.y - vExtents.y)) { return false; }
		if((vPoint.y - vPointExtents.y) > (m_vPos.y + vExtents.y)) { return false; }

		return true;
	}

	usg::Vector2f GetExtents() const
	{
		return m_vSize * 0.5f;
	}

	usg::Vector2f m_vPos;
	usg::Vector2f m_vSize;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_AABB__