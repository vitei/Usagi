/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An infinitely extending physical plane defined by its normal
//	and a point on it
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Sphere.h"
#include "Engine/Maths/Plane.h"

namespace usg{

Plane::Plane()
{
}

Plane::~Plane()
{
}


// TODO: Very good candidates for inlining, but all the includes are hinky
PlaneClass Plane::GetPointPlaneClass(const Vector3f &point) const
{
	float distance = DotProduct( point, m_normal ) + m_fDistance;
	// Point in front of plane
	if( distance < -KF_EPSILON ){
		return PC_BEHIND;
	}
	// Point behind plane
	if ( distance > KF_EPSILON ){
		return PC_IN_FRONT;
	}
	// Point is on plane
	return PC_ON_PLANE;
}


PlaneClass Plane::GetSpherePlaneClass(const Sphere &sphere) const
{
	float distance = DotProduct( sphere.GetPos(), m_normal ) + m_fDistance;
	// Point in front of plane
	if( distance < -sphere.GetRadius() )
	{
		return PC_BEHIND;
	}
	// Point behind plane
	if ( distance > sphere.GetRadius() )
	{
		return PC_IN_FRONT;
	}
	// Point is on plane
	return PC_ON_PLANE;
}

Vector4f Plane::ReflectPoint(const Vector4f& point) const
{
	ASSERT(point.w == 1.0f ); // <-- should use a 'close enough' value check here ... value could be 0.999999999f
	Vector3f vAPoint = m_normal * -m_fDistance;
	Vector3f PA = vAPoint - point.v3();
	Vector3f PA_proj_N = m_normal * DotProduct(PA,m_normal);

	return Vector4f( point.v3() + PA_proj_N * 2.0f, 1.0f );
}

}
