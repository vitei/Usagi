/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Storage and arithmetic operations a row major 4x3 matrix
//	For consistency between files please try confuse use of this class to
//	creating variables for shaders
*****************************************************************************/
#ifndef _USG_MATRIX4X3_H_
#define	_USG_MATRIX4X3_H_

#include "Engine/Memory/MemUtil.h"
#include "Matrix4x4.h"

namespace usg{

class Matrix4x3
{
public:
	Matrix4x3() {}
	Matrix4x3(const Matrix4x4& in);
	~ Matrix4x3() {}
	
	void Assign(const Matrix4x4& in);

	Matrix4x3& operator=(const Matrix4x4& in);
	Matrix4x3& operator=(const Matrix4x3& in);

	void Translate( const Vector4f &translation );

	union
	{
		float m[12];
		float M[3][4];
		struct
		{
			float32 _11, _12, _13, _14;
			float32 _21, _22, _23, _24;
			float32 _31, _32, _33, _34;
		};
	};

	const Vector4f vPos() const		{ return Vector4f(_14, _24, _34, 1.0f); }

	static inline const Matrix4x3& Identity();

private:
	static Matrix4x3 IdentityOnce();
};


inline Matrix4x3::Matrix4x3(const Matrix4x4& in)
{
	Assign(in);
}

inline Matrix4x3 Matrix4x3::IdentityOnce()
{
	Matrix4x3 tmp;
	MemSet(&tmp, 0, sizeof(Matrix4x3));
	tmp._11 = tmp._22 = tmp._33 = 1.0f;
	return tmp;
}

inline Matrix4x3& Matrix4x3::operator=(const Matrix4x4& in)
{
	Assign(in);
	return *this;
}

inline Matrix4x3& Matrix4x3::operator=(const Matrix4x3& in)
{
	for(int i=0; i<12; i++)
	{
		m[i] = in.m[i];
	}

	return *this;
}

inline const Matrix4x3& Matrix4x3::Identity()
{
	static Matrix4x3 identityMat = Matrix4x3::IdentityOnce();
	return identityMat;
}

inline void Matrix4x3::Translate( const Vector4f &translation )
{
	_14 += translation.x;
	_24 += translation.y;
	_34 += translation.z;
}

}

#endif
