/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Memory/MemUtil.h"

// Vulkan has inverted y co-ordinates compared to OpenGL and the depth range is in 0 to 1 rather than -1 to 1
namespace usg
{


	void Matrix4x4::Perspective(float32 fFovY, float32 fAspect, float32 fZNear, float32 fZFar, bool bOrient)
	{
		float32 fYScale = Math::cotanf(fFovY / 2.f);
		float32 fXScale = fYScale / fAspect;

		MemSet(this, 0, sizeof(Matrix4x4));
		_11 = fXScale;
		_22 = -fYScale;

		_33 = (fZFar) / (fZFar - fZNear);
		_34 = 1.0f;
		_43 = -(fZNear*fZFar) / (fZFar - fZNear);
		_44 = 0.0f;
	}

	void Matrix4x4::PerspectiveRH(float32 fFovY, float32 fAspect, float32 fZNear, float32 fZFar, bool bOrient)
	{
		float32 fYScale = Math::cotanf(fFovY / 2.f);
		float32 fXScale = fYScale / fAspect;

		MemSet(this, 0, sizeof(Matrix4x4));
		_11 = fXScale;
		_22 = -fYScale;
		_33 = fZFar / (fZNear - fZFar);
		_34 = -1.0f;
		_43 = fZNear*fZFar / (fZNear - fZFar);
		_44 = 0.0f;
	}

	void Matrix4x4::Orthographic(float32 fLeft, float32 fRight, float32 fBottom, float32 fTop, float32 fZNear, float32 fZFar, bool bOrient)
	{
		LoadIdentity();
		_11 = 2.f / (fRight - fLeft);
		_22 = -(2.f / (fTop - fBottom));
		_33 = 1.f / (fZFar - fZNear);
		_41 = (fLeft + fRight) / (fLeft - fRight);
		_42 = -((fTop + fBottom) / (fBottom - fTop));
		_43 = -fZNear / (fZFar - fZNear);
		_44 = 1.f;
	}

	void Matrix4x4::OrthographicRH(float32 fLeft, float32 fRight, float32 fBottom, float32 fTop, float32 fZNear, float32 fZFar, bool bOrient)
	{
		// TODO: Confirm
		ASSERT(false);
		LoadIdentity();
		_11 = 2.f / (fRight - fLeft);
		_22 = 2.f / (fTop - fBottom);
		_33 = 1.f / (fZFar - fZNear);
		_41 = -(fLeft + fRight) / (fLeft - fRight);
		_42 = (fTop + fBottom) / (fBottom - fTop);
		_43 = -fZNear / (fZFar - fZNear);
		_44 = 1.f;
	}

	Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &rhs) const
	{
		Matrix4x4 tmpMatrix;

		for (uint32 i = 0; i<4; i++)
		{
			for (uint32 j = 0; j<4; j++)
			{
				tmpMatrix[i][j] = (M[i][0] * rhs.M[0][j]) + (M[i][1] * rhs.M[1][j]) + (M[i][2] * rhs.M[2][j]) + (M[i][3] * rhs.M[3][j]);
			}
		}
		return tmpMatrix;
	}

	/*
	void Matrix4x4::Orthographic(float32 fLeft, float32 fRight, float32 fBottom, float32 fTop, float32 fZNear, float32 fZFar, bool bOrient)
	{
	float32 fTmp;

	fTmp =  1.0f / (fRight - fLeft);
	_11 =  2.0f * fTmp;
	_21 =  0.0f;
	_31 =  0.0f;
	_41 = -(fRight + fLeft) * fTmp;

	fTmp     =  1.0f / (fTop - fBottom);
	_12 =  0.0f;
	_22 =  2.0f * fTmp;
	_32 =  0.0f;
	_42 = -(fTop + fBottom) * fTmp;

	_31 =  0.0f;
	_32 =  0.0f;

	fTmp = 1.0f / (fZFar - fZNear);

	#ifndef RIGHT_HAND_COORDIANTES
	_33 = -2.0f / (fZNear - fZFar);
	#else
	_33 = -2.0f * fTmp;
	#endif
	_43 = -(fZFar+fZNear) * fTmp;

	_14 =  0.0f;
	_24 =  0.0f;
	_34 =  0.0f;
	_44 =  1.0f;
	}
	*/

	// Below is the DirectX equivalent incase we need it
#if 0
	void Matrix4x4::Perspective(float32 fFovY, float32 fAspect, float32 fZNear, float32 fZFar, bool bOrient)
	{
		float32 fYScale = Math::cotanf(fFovY / 2.f);
		float32 fXScale = fYScale / fAspect;

		MemSet(this, 0, sizeof(Matrix4x4));
		_11 = fXScale;
		_22 = fYScale;
#if RIGHT_HAND_COORDIANTES
		_33 = fZFar / (fZNear - fZFar);
		_34 = -1.0f;
		_43 = fZNear*fZFar / (fZNear - fZFar);
		_44 = 0.0f;
#else
		_33 = fZFar / (fZFar - fZNear);
		_34 = 1.0f;
		_43 = -fZNear*fZFar / (fZFar - fZNear);
		_44 = 0.0f;
#endif
	}

	void Matrix4x4::Orthographic(float32 fLeft, float32 fRight, float32 fBottom, float32 fTop, float32 fZNear, float32 fZFar, bool bOrient)
	{
		LoadIdentity();
		_11 = 2.f / (fRight - fLeft);
		_22 = 2.f / (fTop - fBottom);
#if RIGHT_HAND_COORDIANTES
		_33 = 1.f / (fZNear - fZFar);
#else
		_33 = 1.f / (fZFar - fZNear);
#endif
		_41 = (fLeft + fRight) / (fLeft - fRight);
		_42 = (fTop + fBottom) / (fBottom - fTop);
		_43 = fZNear / (fZNear - fZFar);
		_44 = 1.f;
	}
#endif

}




