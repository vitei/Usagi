/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Quaternionf.h"

namespace usg{


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
