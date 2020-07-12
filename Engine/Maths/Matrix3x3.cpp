/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Quaternionf.h"

namespace usg{


Matrix3x3::Matrix3x3(
	float r11, float r12, float r13,
	float r21, float r22, float r23,
	float r31, float r32, float r33)
{
	_11 = r11;
	_12 = r12;
	_13 = r13;

	_21 = r21;
	_22 = r22;
	_23 = r23;

	_31 = r31;
	_32 = r32;
	_33 = r33;
}

Matrix3x3 Matrix3x3::operator*( const Matrix4x4 &rhs ) const
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

void Matrix3x3::LoadIdentity(void)
{
	MemSet(&_11, 0, sizeof(Matrix3x3));
	_11 = _22 = _33 = 1.0f;
}

void Matrix3x3::Transpose()
{
	Matrix3x3 mTmpMatrix = *this;
	for (uint32 i = 0; i < 3; i++)
	{
		for (uint32 j = 0; j < 3; j++)
		{
			M[i][j] = mTmpMatrix.M[j][i];
		}
	}
}

void Matrix3x3::Orthonormalize()
{
	usg::Vector3f vUpVec = vUp().GetNormalised();
	usg::Vector3f vForward = vFace().GetNormalised();

	usg::Vector3f vRight = CrossProduct(vUpVec, vForward);

	usg::Vector3f vUp = CrossProduct(vForward, vRight);

	_11 = vRight.x;
	_12 = vRight.y;
	_13 = vRight.z;

	_21 = vUp.x;
	_22 = vUp.y;
	_23 = vUp.z;

	_31 = vForward.x;
	_32 = vForward.y;
	_33 = vForward.z;
}

const Matrix3x3& Matrix3x3::operator=(const Matrix4x4 &rhs)
{
	for (uint32 i = 0; i < 3; i++)
	{
		for (uint32 j = 0; j < 3; j++)
		{
			M[i][j] = rhs.M[i][j];
		}
	}
	return *this;
}


void Matrix3x3::operator = (const Quaternionf &q)
{	 
	M[0][0] = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	M[0][1] = 2.0f * (q.x *q.y + q.z * q.w);
	M[0][2] = 2.0f * (q.x * q.z - q.y * q.w);
	M[1][0] = 2.0f * (q.x * q.y - q.z * q.w);
	M[1][1] = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
	M[1][2] = 2.0f * (q.y *q.z + q.x *q.w);
	M[2][0] = 2.0f * (q.x * q.z + q.y * q.w);
	M[2][1] = 2.0f * (q.y *q.z - q.x *q.w);
	M[2][2] = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
}

}
