/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An infinitely extending physical plane defined by its normal
//	and a point on it
*****************************************************************************/
#pragma once

#ifndef USG_MATHS_PLANE_H
#define USG_MATHS_PLANE_H

#include "Engine/Maths/Vector3f.h"
#include "Engine/Maths/Vector4f.h"
#include "Engine/Maths/Sphere.h"

namespace usg {

const float KF_EPSILON = 1.0f/32.0f;



enum PlaneClass
{
	PC_IN_FRONT = 0,
	PC_BEHIND,
	PC_ON_PLANE
};


class Plane
{
public:
	Plane();
	~Plane();

	void Set( const Vector3f &p1, const Vector3f &p2, const Vector3f &p3 );
	void Set( const Vector3f &normal, float32 distance );
	void Set( float32 a, float32 b, float32 c, float32 d );
	// Get the classification of a point relative to plane (in front, behind, on)
	PlaneClass GetPointPlaneClass(const Vector3f &point) const;
	PlaneClass GetSpherePlaneClass(const usg::Sphere &sphere) const;
	Vector4f ReflectPoint(const Vector4f &point) const;
	
	bool GetLineIntersectionPoint(const Vector3f& vLineStart, const Vector3f& vLineEnd, Vector3f& vPointOut) const;
	const Vector3f& GetNormal() const { return m_normal; }
	float GetDistance() const { return m_fDistance; }

	float SignedDistance(const Vector3f &v) const;
	Vector4f GetNormalAndDistanceV4() const { return Vector4f(m_normal, m_fDistance); }
	
private:
	// Normal of the plane
	Vector3f	m_normal;
	// Distance of an point on the plane to the origin
	float32		m_fDistance;
	// normal = a, b, c and distance d of plane equation
};


inline void Plane::Set(const Vector3f &p1, const Vector3f &p2, const Vector3f &p3)
{
	CrossProduct( p2-p1, p3-p1, m_normal );
	m_normal.Normalise();
	m_fDistance = DotProduct(m_normal, p1);
}


inline void Plane::Set( const Vector3f &normal, float distance )
{
	m_normal = normal;
	m_fDistance = distance;
}

inline void Plane::Set( float32 a, float32 b, float32 c, float32 d )
{
	m_normal.Assign(a, b, c);
	float32 fMag = m_normal.Magnitude();
	m_normal /= fMag;
	d /= fMag;
	m_fDistance = d;
}

inline float Plane::SignedDistance(const Vector3f &v) const
{
	return (DotProduct(m_normal, v) + m_fDistance);
}

} // namespace usagi

#endif // USG_MATHS_PLANE_H
