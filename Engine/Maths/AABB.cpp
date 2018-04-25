/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/AABB.h"
#include <limits>

namespace usg{

void AABB::GetCentreRadii( Vector3f &centre, Vector3f &radii ) const
{
	centre = m_centre;
	radii = m_radii;
}

void AABB::Invalidate()
{
	m_min = usg::Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	m_max = usg::Vector3f(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
}

void AABB::Apply(const Vector3f& v)
{
	bool bDirty = false;
	if (v.x > m_max.x)
	{
		bDirty = true;
		m_max.x = v.x;
	}
	if (v.x < m_min.x)
	{
		bDirty = true;
		m_min.x = v.x;
	}

	if (v.y > m_max.y)
	{
		bDirty = true;
		m_max.y = v.y;
	}
	if (v.y < m_min.y)
	{
		bDirty = true;
		m_min.y = v.y;
	}

	if (v.z > m_max.z)
	{
		bDirty = true;
		m_max.z = v.z;
	}
	if (v.z < m_min.z)
	{
		bDirty = true;
		m_min.z = v.z;
	}
	if (bDirty)
	{
		SetMinMax(m_min, m_max);
	}
}

void AABB::GetCorners( BoxCorners &corners ) const
{
	for( uint32 i=0; i<8; i++ )
	{
		corners.verts[i] = m_centre;
	}

	corners.verts[0].x -= m_radii.x;
	corners.verts[1].x -= m_radii.x;
	corners.verts[2].x -= m_radii.x;
	corners.verts[3].x -= m_radii.x;
	corners.verts[4].x += m_radii.x;
	corners.verts[5].x += m_radii.x;
	corners.verts[6].x += m_radii.x;
	corners.verts[7].x += m_radii.x;

	corners.verts[0].y -= m_radii.y;
	corners.verts[1].y -= m_radii.y;
	corners.verts[2].y += m_radii.y;
	corners.verts[3].y += m_radii.y;
	corners.verts[4].y -= m_radii.y;
	corners.verts[5].y -= m_radii.y;
	corners.verts[6].y += m_radii.y;
	corners.verts[7].y += m_radii.y;

	corners.verts[0].z -= m_radii.z;
	corners.verts[1].z += m_radii.z;
	corners.verts[2].z -= m_radii.z;
	corners.verts[3].z += m_radii.z;
	corners.verts[4].z -= m_radii.z;
	corners.verts[5].z += m_radii.z;
	corners.verts[6].z -= m_radii.z;
	corners.verts[7].z += m_radii.z;

}



bool AABB::CollisionX(const AABB& b)
{
	// Check 1D distance from centre points and compare with radii of same axis
	if( fabs( m_centre.x - b.m_centre.x ) > ( m_radii.x + b.m_radii.x ) )
	{
		return false;
	}
	if( fabs( m_centre.y - b.m_centre.y ) > ( m_radii.y + b.m_radii.y ) )
	{
		return false;
	}
	if( fabs( m_centre.z - b.m_centre.z ) > ( m_radii.z + b.m_radii.z ) )
	{
		return false;
	}

	// All distances are within range
	return true;
}

void AABB::TransformBounds(const Matrix4x4& bounds) const
{
	// Implement me
	ASSERT(false);
}

void AABB::SetMinMax( const Vector3f &vMin, const Vector3f &vMax )
{
	Vector3f vRadii = (vMax - vMin) * 0.5f;
	m_centre = vMin + vRadii;
	m_radii = vRadii;
	m_min = vMin;
	m_max = vMax;
}


}
