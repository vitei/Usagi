/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_MATHS_AABB_H
#define USG_MATHS_AABB_H


#include "Engine/Maths/Vector3f.h"
#include "Engine/Maths/Sphere.h"

namespace usg {

class AABB
{
public:
	AABB() {}
	~AABB() {}

	explicit AABB(const Vector3f& min, const Vector3f& max)
	{
		SetMinMax(min, max);
	}

	struct BoxCorners
	{
		Vector3f	verts[8];
	};

	void SetMinMax( const Vector3f &min, const Vector3f &max );
	void SetCentreRadii( const Vector3f &centre, const Vector3f &radii ) { m_centre = centre; m_radii = radii; UpdateMinMax(); }
	void SetPos( const Vector3f &pos ) { m_centre = pos; UpdateMinMax(); }
	void SetRadii( float x, float y, float z ) { m_radii.Assign(x,y,z); UpdateMinMax(); }
	void SetRadii(const Vector3f& vRadii) { m_radii = vRadii;  UpdateMinMax(); }
	
	
	void Invalidate();
	void Apply(const Vector3f& v);
	
	void GetCentreRadii( Vector3f &centre, Vector3f &radii ) const;
	void GetCorners( BoxCorners &corners ) const;
	const Vector3f& GetPos() const { return m_centre; }
	const Vector3f& GetRadii() const { return m_radii; }
	const Vector3f& GetMin() const { return m_min; }
	const Vector3f& GetMax() const { return m_max; }
	virtual bool CollisionX(const AABB &box);
	bool InBox(const Vector3f& point) const;
	bool IntersectBox(const usg::Sphere& sphere) const;
	bool ContainedInBox(const usg::Sphere& sphere) const;
	void TransformBounds(const Matrix4x4& bounds) const;
private:
	void UpdateMinMax() { m_min = m_centre - m_radii; m_max = m_centre + m_radii; }

	// Center point
	Vector3f	m_centre;
	// Radius in each axis
	Vector3f	m_radii;
	
	Vector3f	m_min,m_max;
};


inline void Lerp(const AABB &box1, const AABB &box2, AABB &boxOut, float t)
{
	// FIXME: Stupid and slow
	Vector3f vPos1;
	Vector3f vPos2;
	Vector3f vRadius1;
	Vector3f vRadius2;

	box1.GetCentreRadii(vPos1, vRadius1);
	box2.GetCentreRadii(vPos2, vRadius2);

	Lerp( vPos1, vPos2, vPos1, t );
	Lerp( vRadius1, vRadius2, vRadius1, t );

	boxOut.SetCentreRadii(vPos1, vRadius1);
}

inline bool AABB::IntersectBox(const usg::Sphere& sphere) const
{
	Vector3f offset = m_centre - sphere.GetPos();
	Vector3f radius = m_radii + Vector3f(sphere.GetRadius(), sphere.GetRadius(), sphere.GetRadius());

	uint32 uClip = 0;
	if (offset.x > radius.x) 
	{
		uClip |= 1<<0;
	}
	else if (offset.x < -radius.x)
	{
		uClip |= 1<<1;
	}
		
	if (offset.z > radius.z) 
	{
		uClip |= 1<<2;
	}
	else if (offset.z < -radius.z)
	{
		uClip |= 1<<3;
	}

	if (offset.y > m_radii.y) 
	{
		uClip |= 1<<4;
	}
	else if (offset.y < -m_radii.y)
	{
		uClip |= 1<<5;
	}

	return uClip == 0;
}

inline bool AABB::ContainedInBox(const usg::Sphere& sphere) const
{
	Vector3f offset = m_centre - sphere.GetPos();
	Vector3f radius = m_radii - Vector3f(sphere.GetRadius(), sphere.GetRadius(), sphere.GetRadius());

	uint32 uClip = 0;
	if (offset.x > radius.x) 
	{
		uClip |= 1<<0;
	}
	else if (offset.x < -radius.x)
	{
		uClip |= 1<<1;
	}
		
	if (offset.z > radius.z) 
	{
		uClip |= 1<<2;
	}
	else if (offset.z < -radius.z)
	{
		uClip |= 1<<3;
	}

	if (offset.y > m_radii.y) 
	{
		uClip |= 1<<4;
	}
	else if (offset.y < -m_radii.y)
	{
		uClip |= 1<<5;
	}


	return uClip == 0;
}

inline bool AABB::InBox(const Vector3f& point) const
{
	Vector3f offset = m_centre - point;

	uint32 uClip = 0;
	if (offset.x > m_radii.x) 
	{
		uClip |= 1<<0;
	}
	else if (offset.x < -m_radii.x)
	{
		uClip |= 1<<1;
	}
		
	if (offset.z > m_radii.z) 
	{
		uClip |= 1<<2;
	}
	else if (offset.z < -m_radii.z)
	{
		uClip |= 1<<3;
	}

	if (offset.y > m_radii.y) 
	{
		uClip |= 1<<4;
	}
	else if (offset.y < -m_radii.y)
	{
		uClip |= 1<<5;
	}

	return uClip == 0;
}

} // namespace usg

#endif // USG_MATHS_AABB_H
