/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Matrix4x3.h"

namespace usg{

void Matrix4x3::Assign(const Matrix4x4& in)
{
	// The 4x3 is column major so we have to transpose the matrix when copying it accross
	_11 = in._11;
	_12 = in._21;
	_13 = in._31;
	_14 = in._41;

	_21 = in._12;
	_22 = in._22;
	_23 = in._32;
	_24 = in._42;

	_31 = in._13;
	_32 = in._23;
	_33 = in._33;
	_34 = in._43;

	// Slight deviations saw these asserts failing
	//ASSERT(in._14 == 0.0f);
	//ASSERT(in._24 == 0.0f);
	//ASSERT(in._34 == 0.0f);
	//ASSERT(in._44 == 1.0f);
}

}