/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Storage and arithmetic operations a row major 3x3 matrix
*****************************************************************************/
#ifndef _USG_MATRIX3X3_H_
#define	_USG_MATRIX3X3_H_

#include "Engine/Memory/MemUtil.h"
#include "Vector3f.h"
#include "Vector4f.h"

namespace usg{


class Matrix3x3
{
public:
	Matrix3x3() { }
	Matrix3x3(	float r11, float r12, float r13,
				float r21, float r22, float r23,
				float r31, float r32, float r33 );
	Matrix3x3(const Matrix4x4& rhs) { *this = rhs; }
	~ Matrix3x3() {}

	void LoadIdentity();

	void Transpose();
	void Orthonormalize();

	const Vector3f vRight() const	{ return Vector3f(_11, _12, _13); }
	const Vector3f vUp() const		{ return Vector3f(_21, _22, _23); }
	const Vector3f vFace() const	{ return Vector3f(_31, _32, _33); }

	Matrix3x3 operator*( const Matrix3x3 &rhs ) const;
	const Matrix3x3& operator=( const Matrix3x3 &rhs );
	const Matrix3x3& operator=(const Matrix4x4 &rhs);
	Matrix3x3 operator*( const Matrix4x4 &rhs ) const;
	void operator = (const Quaternionf &quat);

	void SetRight(const Vector3f & vRight) { _11 = vRight.x; _12 = vRight.y; _13 = vRight.z; }
	void SetUp(const Vector3f & vUp) { _21 = vUp.x; _22 = vUp.y; _23 = vUp.z; }
	void SetFace(const Vector3f & vFace) { _31 = vFace.x; _32 = vFace.y; _33 = vFace.z; }

	union
	{
		float M[3][3];
		float m[12];
		struct
		{
			float32 _11, _12, _13;
			float32 _21, _22, _23;
			float32 _31, _32, _33;
		};
	};

	static Matrix3x3 IdentityOnce();
	static const Matrix3x3& Identity();

private:
};

inline Matrix3x3 Matrix3x3::IdentityOnce()
{
	Matrix3x3 tmp;
	MemSet(&tmp, 0, sizeof(Matrix3x3));
	tmp._11 = tmp._22 = tmp._33 = 1.0f;
	return tmp;
}


inline const Matrix3x3& Matrix3x3::Identity()
{
	static Matrix3x3 identityMat = Matrix3x3::IdentityOnce();
	return identityMat;
}

inline Matrix3x3 Matrix3x3::operator*( const Matrix3x3 &rhs ) const
{
	Matrix3x3 tmpMatrix;

	for(uint32 i=0; i<3; i++)
	{
		for(uint32 j=0; j<3; j++)
		{
			tmpMatrix.M[i][j] = (M[i][0] * rhs.M[0][j]) + (M[i][1] * rhs.M[1][j]) + (M[i][2] * rhs.M[2][j]);
		}
	}
	return tmpMatrix;
}


inline const Matrix3x3& Matrix3x3::operator=( const Matrix3x3 &rhs )
{
	for(uint32 i=0; i<12; i++)
	{
		m[i] = rhs.m[i];
	}
	return *this;
}

}

#endif
