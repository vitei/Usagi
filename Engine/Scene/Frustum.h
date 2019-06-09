/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Used to determine is an object is within a view frustum
*****************************************************************************/
#pragma once

#ifndef USG_FRUSTUM_H
#define USG_FRUSTUM_H

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Maths/Plane.h"

namespace usg{

class AABB;

class Frustum
{
public:
	Frustum(void);
	~Frustum(void);

	enum FrustumPlane
	{
		PLANE_RIGHT = 0,
		PLANE_LEFT,
		PLANE_BOTTOM,
		PLANE_TOP,
		PLANE_FAR,
		PLANE_NEAR,
		PLANE_COUNT
	};
	
	void SetUp(const Matrix4x4 &proj, const Matrix4x4 &view);
	void SetUpLR(const Matrix4x4 &projL, const Matrix4x4 &viewL, const Matrix4x4 &projR, const Matrix4x4 &viewR);
	void SetUp(const Matrix4x4 &viewProj);

	bool IsSphereInFrustum(const usg::Sphere &sphere) const;
	bool ArePointsInFrustum(const Vector4f* ppvPoints, uint32 uCount) const;
	PlaneClass ClassifySphere(const usg::Sphere &sphere) const;
	PlaneClass ClassifyBox(const AABB &box) const;
	const Plane& GetPlane(uint32 uIndex) const;

private:
	void InitFromMatrix(const Matrix4x4& mat);
	Plane		m_planes[PLANE_COUNT];
	Vector3f	m_absNormals[PLANE_COUNT];
};

inline PlaneClass Frustum::ClassifySphere(const usg::Sphere &sphere) const
{
	PlaneClass out = PC_IN_FRONT;
	for( uint32 i = 0; i < PLANE_COUNT; i++)
	{
		PlaneClass result =  m_planes[i].GetSpherePlaneClass(sphere);
		switch(result)
		{
			case PC_BEHIND:
				return PC_BEHIND;
			case PC_ON_PLANE:
				out = PC_ON_PLANE;
				break;
			default:
				break;
		}
	}
	return out;
}

inline bool Frustum::IsSphereInFrustum(const usg::Sphere &sphere) const
{
	for( uint32 i = 0; i < PLANE_COUNT; i++)
	{
		if( m_planes[i].GetSpherePlaneClass(sphere) == PC_BEHIND )
		{
			return false;
		}
	}
	// usg::Sphere is not behind any of the planes
	return true;	
}

} // namespace usg

#endif // USG_FRUSTUM_H

