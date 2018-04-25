/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_MATH_SPHERE_H
#define USG_MATH_SPHERE_H

#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector3f.h"

namespace usg{

class Sphere
{
public:
	explicit Sphere(const float radius) : m_pos(0.0f, 0.0f, 0.0f), m_radius(radius) {}
	explicit Sphere(const Vector3f& pos, const float radius) : m_pos(pos), m_radius(radius) {}
	Sphere() : m_pos(0.0f, 0.0f, 0.0f), m_radius(0.0f) {}
	~Sphere(void) {}
	bool Intersect(const Sphere& sphere) const;
	void SetPos(const Vector3f& pos) { m_pos = pos; }
	void SetRadius(const float fRadius) { m_radius = fRadius; }
	virtual const Vector3f& GetPos()const { return m_pos; }
	float GetRadius() const { return m_radius; }
private:
	Vector3f	m_pos;
	float32		m_radius;
};

inline bool usg::Sphere::Intersect(const usg::Sphere& sphere) const
{
	float fDist = m_radius + sphere.GetRadius();
	if (sphere.GetPos().GetSquaredDistanceFrom(m_pos) <= (fDist * fDist))
	{
		return true;
	}
	return false;
}

} // namespace usg

#endif // USG_MATH_SPHERE_H
