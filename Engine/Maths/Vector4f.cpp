/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "MathUtil.h"
#include "Matrix4x4.h"
#include "Vector4f.h"

namespace usg{

Vector4f Vector4f::operator*( const Matrix4x4 &rhs ) const
{
	Vector4f	result;

	// Perform the dot product with each row in the matrix
	result.x	= x*rhs._11 + y*rhs._21 + z*rhs._31 + w*rhs._41;
	result.y	= x*rhs._12 + y*rhs._22 + z*rhs._32 + w*rhs._42;
	result.z	= x*rhs._13 + y*rhs._23 + z*rhs._33 + w*rhs._43;
	result.w	= x*rhs._14 + y*rhs._24 + z*rhs._34 + w*rhs._44;

	return result;
}

Vector4f Vector4f::operator *(const Quaternionf& q) const
{
	return Vector4f(v3() * q, w);
}

}