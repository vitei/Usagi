/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "MathUtil.h"
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Quaternionf.h"
#include "Vector3f.h"

namespace usg{

const Vector3f Vector3f::ZERO(0.0f, 0.0f, 0.0f);
const Vector3f Vector3f::ONE(1.0f, 1.0f, 1.0f);
const Vector3f Vector3f::X_AXIS(1.0f, 0.0f, 0.0f);
const Vector3f Vector3f::Y_AXIS(0.0f, 1.0f, 0.0f);
const Vector3f Vector3f::Z_AXIS(0.0f, 0.0f, 1.0f);


Vector3f Vector3f::operator*( const Matrix4x4 &rhs ) const
{
	Vector3f	result;

	// Perform the dot product with each row in the matrix
	// Treats as a 4 component vector with w at 1.0f
	result.x	= x*rhs._11 + y*rhs._21 + z*rhs._31 + rhs._41;
	result.y	= x*rhs._12 + y*rhs._22 + z*rhs._32 + rhs._42;
	result.z	= x*rhs._13 + y*rhs._23 + z*rhs._33 + rhs._43;
	//float w	= in.x*_14 + in.y*_24 + in.z*_34 + _44;

	return result; // /w;
}

Vector3f Vector3f::operator*( const Matrix3x3 &rhs ) const
{
	Vector3f	result;

	result.x	= x*rhs._11 + y*rhs._21 + z*rhs._31;
	result.y	= x*rhs._12 + y*rhs._22 + z*rhs._32;
	result.z	= x*rhs._13 + y*rhs._23 + z*rhs._33;

	return result;
}

Vector3f Vector3f::operator *(const Quaternionf& q) const
{
	Vector3f u(q.x, q.y, q.z);

	// Extract the scalar part of the quaternion
	float s = q.w;

	// Do the math
	return Vector3f(2.0f * DotProduct(u, *this) * u
		+ (s*s - DotProduct(u, u)) * *this
		+ 2.0f * s * CrossProduct(u, *this) );
}


Vector3f Vector3f::RandomPointInSphere(float fMaxRadius)
{
	Vector3f vOut = Vector3f::ZERO;

	float fTheta = Math::RangedRandom(0.0f, Math::two_pi);
	float fPhi = acosf(Math::RangedRandom(0.0f, 2.0f) - 1.0f);
	float fRadius = cbrtf(Math::RangedRandom(0.0f, 1.0f)) * fMaxRadius;
	float fSinTheta = sinf(fTheta);
	float fCosTheta = cosf(fTheta);
	float fSinPhi = sinf(fPhi);
	float fCosPhi = cosf(fPhi);
	vOut.x = fRadius * fSinPhi * fCosTheta;
	vOut.y = fRadius * fSinPhi * fSinTheta;
	vOut.z = fRadius * fCosPhi;
	return vOut;
}


}