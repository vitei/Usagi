/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_PHYSICS_COLLISION_DETECTION_H
#define USG_PHYSICS_COLLISION_DETECTION_H

#include "Engine/Maths/Sphere.h"
#include "Engine/Maths/AABB.h"

namespace usg{

inline bool DetectCollision(const usg::Sphere &sphere, const AABB &box)
{
	float fRadius = sphere.GetRadius();
	Vector3f vMax(fRadius, fRadius, fRadius);
	Vector3f vMin;
		
	vMin = (box.GetPos() - box.GetRadii()) - vMax;
	vMax = (box.GetPos() + box.GetRadii()) + vMax;

	const Vector3f &spherePos = sphere.GetPos();
	
	if(spherePos.x >= vMin.x && spherePos.x <= vMax.x
		&& spherePos.y >= vMin.y && spherePos.y <= vMax.y
		&& spherePos.z >= vMin.z && spherePos.z <= vMax.z )
	{
		return true;
	}
	else
	{
		return false;
	}

}

inline bool DetectCollision(const Vector3f& vSphereCenter, float fSphereRadius, const Vector3f& vLineStart, const Vector3f& vLineEnd)
{
	const Vector3f& vDirToSphere = vSphereCenter - vLineStart;
	const Vector3f& vLineDir = vLineEnd - vLineStart;
	const float fRadiusSquared = fSphereRadius*fSphereRadius;
	const float fLineDirSqrMagnitude = vLineDir.MagnitudeSquared();
	const float fDirToSphereSqrMagnitude = vDirToSphere.MagnitudeSquared();

	const float fDot = DotProduct(vDirToSphere, vLineDir);
	const float d = fDirToSphereSqrMagnitude - fDot*fDot / fLineDirSqrMagnitude;

	if (d <= fRadiusSquared)
	{
		if (fDirToSphereSqrMagnitude <= fRadiusSquared || vLineEnd.GetSquaredDistanceFrom(vSphereCenter) <= fRadiusSquared)
		{
			return true;
		}
		return (fDot >= 0 && fDot <= fLineDirSqrMagnitude);
	}
	return false;
}

inline bool DetectCollision(const usg::Sphere &sphere1, const usg::Sphere &sphere2)
{
	float distance = sphere1.GetPos().GetDistanceFrom(sphere2.GetPos());
	if(distance < (sphere1.GetRadius() + sphere2.GetRadius()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

Vector3f ClosestPointOnTriangleToPoint(const Vector3f& vPoint, const Vector3f& vA, const Vector3f& vB, const Vector3f& vC);

inline bool TestSphereTriangle(const usg::Sphere& sphere, const Vector3f& vA, const Vector3f& vB, const Vector3f& vC, Vector3f& point)
{
	point = ClosestPointOnTriangleToPoint(sphere.GetPos(), vA, vB, vC);

	Vector3f vDir = point - sphere.GetPos();
	return DotProduct(vDir, vDir) <= (sphere.GetRadius() * sphere.GetRadius());
}


} // namespace usg

#endif  // USG_PHYSICS_COLLISION_DETECTION_H
