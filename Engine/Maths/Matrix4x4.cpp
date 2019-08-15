/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Storage and arithmetic operations a column major 4x4 matrix
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Plane.h"
#include "Engine/Memory/MemUtil.h"
#include "MathUtil.h"
#include "Quaternionf.h"
#include "Matrix4x3.h"
#include "Matrix4x4.h"
#include "Matrix3x3.h"


namespace usg{

Matrix4x4::Matrix4x4(	float r11, float r12, float r13, float r14,
				float r21, float r22, float r23, float r24,
				float r31, float r32, float r33, float r34,
				float r41, float r42, float r43, float r44 )
{
	_11 = r11;
	_12 = r12;
	_13 = r13;
	_14 = r14;

	_21 = r21;
	_22 = r22;
	_23 = r23;
	_24 = r24;

	_31 = r31;
	_32 = r32;
	_33 = r33;
	_34 = r34;

	_41 = r41;
	_42 = r42;
	_43 = r43;
	_44 = r44;
}


bool Matrix4x4::operator==(const Matrix4x4 &mat) const
{
	for(uint32 i=0; i<16; i++)
	{
		if(!Math::IsEqual(m[i], mat.m[i]))
			return false;
	}
	return true;
}

void Matrix4x4::LoadIdentity(void)
{
	MemSet(&_11, 0, sizeof(Matrix4x4));
	_11 = _22 = _33 = _44 = 1.0f;
}

void Matrix4x4::Clear()
{
	MemSet(&_11, 0, sizeof(Matrix4x4));
}

void Matrix4x4::Copy( const float* in )
{
	MemCpy(m, in, sizeof(float)*16);
}

// Build rotation matrices
void Matrix4x4::MakeRotateX(float x)
{
   float fSin, fCos;
   Math::SinCos(x, fSin, fCos);

   _22 =  fCos;
   _23 =  fSin;
   _32 = -fSin;
   _33 =  fCos;

   _11 = _44 = 1.0f;
   _12 = 0.0f; _13 = 0.0f; _14 = 0.0f; 
   _21 = 0.0f; _24 = 0.0f; _31 = 0.0f; 
   _34 = 0.0f; _41 = 0.0f; _42 = 0.0f; _43 = 0.0f;
}

void Matrix4x4::MakeRotateY(float y)
{
   float fSin, fCos;
   Math::SinCos(y, fSin, fCos);

	_22 = 1.0f; _44 = 1.0f;
   	_12 = 0.0f; _23 = 0.0f; _14 = 0.0f;
   	_21 = 0.0f; _24 = 0.0f; _32 = 0.0f;
   	_34 = 0.0f; _41 = 0.0f; _42 = 0.0f; _43 = 0.0f;

   _11 =  fCos;
   _13 = -fSin;
   _31 =  fSin;
   _33 =  fCos;
}


void Matrix4x4::MakeRotateZ(float z)
{
	float fSin, fCos;
  	Math::SinCos(z, fSin, fCos);

	_33 = 1.0f; _44 = 1.0f;
	_13 = 0.f; _14 = 0.f; _23 = 0.f; 
	_24 = 0.f; _31 = 0.f; _32 = 0.f;
	_34 = 0.f; _41 = 0.f; _42 = 0.f;
	_43 = 0.0f;

   _11  =  fCos;
   _12  =  fSin;
   _21  = -fSin;
   _22  =  fCos;   
}


void Matrix4x4::MakeRotateYPR(float yaw, float pitch, float roll)
{
	/*// TODO: Optimise me
	Matrix4x4 mYaw, mPitch, mRoll;
	mRoll.MakeRotateZ(roll);
	mPitch.MakeRotateX(pitch);
	mYaw.MakeRotateY(yaw);

	*this = mRoll * mPitch * mYaw;*/

	float fSRoll, fCRoll, fSPitch, fCPitch, fSYaw, fCYaw;
	Math::SinCos(roll, fSRoll, fCRoll);
	Math::SinCos(pitch, fSPitch, fCPitch);
	Math::SinCos(yaw, fSYaw, fCYaw);

	_11 = fSRoll * fSPitch * fSYaw + fCRoll * fCYaw;
	_12 = fSRoll * fCPitch;
	_13 = fSRoll * fSPitch * fCYaw - fCRoll * fSYaw;
	_14 = 0.0f;
	_21 = fCRoll * fSPitch * fSYaw - fSRoll * fCYaw;
	_22 = fCRoll * fCPitch;
	_23 = fCRoll * fSPitch * fCYaw + fSRoll * fSYaw;
	_24 = 0.0f;
	_31 = fCPitch * fSYaw;
	_32 = -fSPitch;
	_33 = fCPitch * fCYaw;
	_34 = 0.0f;

	_41 = 0.0f;
	_42 = 0.0f;
	_43 = 0.0f;
	_44 = 1.0f;

}


void Matrix4x4::Inverse()
{
	Matrix4x4 mTmp;
	GetInverse(mTmp);
	*this = mTmp;
}

float Matrix4x4::Determinant() const
{
    Vector4f v1(_11, _21, _31, _41);
	Vector4f v2(_12, _22, _32, _42);
	Vector4f v3(_13, _23, _33, _43);

	Vector4f minor = CrossProduct4D(v1, v2, v3);

	return - (_14 * minor.x + _24 * minor.y + _34 * minor.z + _44 * minor.w);
}

void Matrix4x4::Orthonormalize()
{
	usg::Matrix3x3 mRot = *this;
	mRot.Orthonormalize();

	for (uint32 i = 0; i < 3; i++)
	{
		for (uint32 j = 0; j < 3; j++)
		{
			M[i][j] = mRot.M[i][j];
		}
	}
}

void Matrix4x4::GetQuickInverse(Matrix4x4& out) const
{
	out._11 = _11;
	out._12 = _21;
	out._13 = _31;
	out._14 = 0;
	
	out._21 = _12;
	out._22 = _22;
	out._23 = _32;
	out._24 = 0;
	
	out._31 = _13;
	out._32 = _23;
	out._33 = _33;
	out._34 = 1;

	out._41 = 0;
	out._42 = 0;
	out._43 = 0;
	out._44 = 1;

	Vector3f p(_41, _42, _43);
	
	p = p * out;
	
	out._41 = -p.x;
	out._42 = -p.y;
	out._43 = -p.z;
	out._44 = 1.0f;
}
	
void Matrix4x4::GetInverse(Matrix4x4& out) const
{
	Vector4f v, vec[3];
	uint32 a, j;
	float fCofactor;

	float fDet = Determinant();

	ASSERT( fDet!=0.0f );
	if(fDet == 0.0f)
		return;
	
	for(uint32 i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			if (j != i )
			{
				a = j;
				if ( j > i )
				{
					a = a-1;
				}
				vec[a].x = m[(j*4)+0];
				vec[a].y = m[(j*4)+1];
				vec[a].z = m[(j*4)+2];
				vec[a].w = m[(j*4)+3];
			}
		}

		v = CrossProduct4D(vec[0], vec[1], vec[2]);
		for (j=0; j<4; j++)
		{
			switch(j)
			{
				case 0: fCofactor = v.x; break;
				case 1: fCofactor = v.y; break;
				case 2: fCofactor = v.z; break;
				case 3: fCofactor = v.w; break;
			}
			out.m[(j*4)+i] = powf(-1.0f, (float)i) * fCofactor / fDet;
		}
	}
}



void Matrix4x4::Transpose()
{
	Matrix4x4 tmpMat = (*this);
	_12 = tmpMat._21;
	_13 = tmpMat._31;
	_14 = tmpMat._41;

	_21 = tmpMat._12;
	_23 = tmpMat._32;
	_24 = tmpMat._42;

	_31 = tmpMat._13;
	_32 = tmpMat._23;
	_34 = tmpMat._43;

	_41 = tmpMat._14;
	_42 = tmpMat._24;
	_43 = tmpMat._34;
}

void Matrix4x4::GetTranspose(Matrix4x4 &out) const
{
	out._11 = _11;
	out._12 = _21;
	out._13 = _31;
	out._14 = _41;

	out._21 = _12;
	out._22 = _22;
	out._23 = _32;
	out._24 = _42;

	out._31 = _13;
	out._32 = _23;
	out._33 = _33;
	out._34 = _43;

	out._41 = _14;
	out._42 = _24;
	out._43 = _34;
	out._44 = _44;
}

Matrix4x4& Matrix4x4::operator=(const Matrix3x3& in)
{

	_11 = in._11;
	_12 = in._12;
	_13 = in._13;
	_14 = 0.0f;

	_21 = in._21;
	_22 = in._22;
	_23 = in._23;
	_24 = 0.0f;

	_31 = in._31;	
	_32 = in._32;
	_33 = in._33;
	_34 = 0.0f;

	_41 = 0.0f;
	_42 = 0.0f;
	_43 = 0.0f;
	_44 = 1.0f;


	return *this;
}

void Matrix4x4::MakeScale( float x, float y, float z )
{
	LoadIdentity();
	_11 = x;
	_22 = y;
	_33 = z;
}
void Matrix4x4::MakeScale( const Vector3f& vec )
{
	LoadIdentity();
	_11 = vec.x;
	_22 = vec.y;
	_33 = vec.z;	
}
void Matrix4x4::Scale( float x, float y, float z, float w )
{
    SetRight(vRight() * x);
    SetUp(vUp() * y);
    SetFace(vFace() * z);
    SetPos(vPos() * w);
}


Matrix4x4 Matrix4x4::operator + (const Matrix4x4 rhs) const
{
	Matrix4x4 tmpMatrix;

	for (uint32 i = 0; i < 4; i++)
	{
		for (uint32 j = 0; j < 4; j++)
		{
			tmpMatrix[i][j] =(M[i][j] + rhs.M[i][j]);
		}
	}
	return tmpMatrix;
}

Matrix4x4& Matrix4x4::operator += (const Matrix4x4 rhs)
{
	for (uint32 i = 0; i < 4; i++)
	{
		for (uint32 j = 0; j < 4; j++)
		{
			M[i][j] = (M[i][j] + rhs.M[i][j]);
		}
	}
	return *this;
}

Matrix4x4 Matrix4x4::operator * (float fRhs) const
{
	Matrix4x4 tmpMatrix;

	for (uint32 i = 0; i < 4; i++)
	{
		for (uint32 j = 0; j < 4; j++)
		{
			tmpMatrix[i][j] = (M[i][j] * fRhs);
		}
	}
	return tmpMatrix;
}

Matrix4x4& Matrix4x4::operator *= (float fRhs)
{
	for (uint32 i = 0; i < 4; i++)
	{
		for (uint32 j = 0; j < 4; j++)
		{
			M[i][j] *= fRhs;
		}
	}
	return *this;
}

Matrix4x4 Matrix4x4::operator *= ( Matrix4x4 rhs )
{
	*this = *this * rhs;
	
	return *this;
}

void Matrix4x4::operator = (const Quaternionf &q)
{	 
	M[0][0] = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	M[0][1] = 2.0f * (q.x *q.y + q.z * q.w);
	M[0][2] = 2.0f * (q.x * q.z - q.y * q.w);
	M[0][3] = 0.0f;
	M[1][0] = 2.0f * (q.x * q.y - q.z * q.w);
	M[1][1] = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
	M[1][2] = 2.0f * (q.y *q.z + q.x *q.w);
	M[1][3] = 0.0f;
	M[2][0] = 2.0f * (q.x * q.z + q.y * q.w);
	M[2][1] = 2.0f * (q.y *q.z - q.x *q.w);
	M[2][2] = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	M[2][3] = 0.0f;

	M[3][0] = 0.0f;
	M[3][1] = 0.0f;
	M[3][2] = 0.0f;
	M[3][3] = 1.0f;
}

void Matrix4x4::BuildModelFromCamera( const Matrix4x4 &matIn )
{
	CameraMatrix(matIn.vRight(), matIn.vUp(), matIn.vFace(), matIn.vPos());
}

void Matrix4x4::BuildCameraFromModel(const Matrix4x4 &matIn)
{
	CameraMatrix(matIn.vRight(), matIn.vUp(), matIn.vFace(), matIn.vPos());
}

void Matrix4x4::BuildCameraFromModel( const Matrix4x4 &matIn, const Vector4f &localEyePos )
{
	Vector4f eyePosWorld;
	ASSERT(localEyePos.w == 1.0f);
	eyePosWorld = localEyePos * matIn;
	CameraMatrix(matIn.vRight(), matIn.vUp(), matIn.vFace(), eyePosWorld);
}


void Matrix4x4::CameraMatrix(const Vector4f &vXAxis, const Vector4f &vCorrUp, const Vector4f &vZAxis, const Vector4f &vEyePos)
{
	_11 = vXAxis.x;
	_12 = vCorrUp.x;
	_13 = vZAxis.x;

	_21 = vXAxis.y;
	_22 = vCorrUp.y;
	_23 = vZAxis.y;

	_31 = vXAxis.z;
	_32 = vCorrUp.z;
	_33 = vZAxis.z;

	_41 = -DotProduct(vXAxis, vEyePos);
	_42 = -DotProduct(vCorrUp, vEyePos);
	_43 = -DotProduct(vZAxis, vEyePos);


	_14 = 0.0f;
	_24 = 0.0f;
	_34 = 0.0f;
	_44 = 1.0f;
} 



void Matrix4x4::ModelMatrix( const Vector4f &right, const Vector4f &up, const Vector4f &view, const Vector4f &pos )
{
	_11 = right.x;
	_12 = right.y;
	_13 = right.z;
	_14 = right.w;

	_21 = up.x;
	_22 = up.y;
	_23 = up.z;
	_24 = up.w;

	_31 = view.x;
	_32 = view.y;
	_33 = view.z;
	_34 = view.w;

	_41 = pos.x;
	_42 = pos.y;
	_43 = pos.z;
	_44 = pos.w;
}


void Matrix4x4::MakeRotate(float x, float y, float z)
{
	float cx, cy, cz, sx, sy, sz;

	cx = cosf(x);
	sx = sinf(x);
	cy = cosf(y);
	sy = sinf(y);
	cz = cosf(z);
	sz = sinf(z);
    
	_11 = cy * cz;
	_12 = cy * sz;
	_13 = -sy;
	_14 = 0.0f;
    
	_21 = (sx * sy * cz) - (cx * sz);
	_22 = (sx * sy * sz) + (cx * cz);
	_23 = sx * cy;
	_24 = 0.0f;
	
	_31 = (cx * sy * cz) + (sx * sz);
	_32 = (cx * sy * sz) - (sx * cz);
	_33 = cx * cy;
	_34 = 0.0f;

	_41 = 0.0f;
	_42 = 0.0f;
	_43 = 0.0f;
	_44 = 1.0f;
}


 void Matrix4x4::Billboard(const Vector4f &vcPos, const Vector4f &vcDir, const Vector4f &vcWorldUp)
{
   Vector4f vcUp, vcRight; 
   float fAngle=0.0f;

   fAngle = DotProduct(vcWorldUp, vcDir);

   vcUp = vcWorldUp - (vcDir * fAngle);
   vcUp.Normalise();

   CrossProduct(vcUp, vcDir, vcRight);

   _11 = vcRight.x; _21 = vcUp.x; _31 = vcDir.x;
   _12 = vcRight.y; _22 = vcUp.y; _32 = vcDir.y;
   _13 = vcRight.z; _23 = vcUp.z; _33 = vcDir.z;

   _41 = vcPos.x;
   _42 = vcPos.y;
   _43 = vcPos.z;
   
   _41=0.0f; _42=0.0f; _43=0.0f; _44=1.0f;
}


void Matrix4x4::MakeRotAboutAxis(const Vector4f &axis, float fRadians)
{
	ASSERT(axis.w == 0.0f);

	float fCos = cosf(fRadians);
	float fSin = sinf(fRadians);
	float fSum = 1.0f - fCos;

	_11 = (axis.x * axis.x) * fSum + fCos;
	_12 = (axis.x * axis.y) * fSum - (axis.z*fSin);
	_13 = (axis.x * axis.z) * fSum + (axis.y*fSin);

	_21 = (axis.y * axis.x) * fSum + (axis.z*fSin);
	_22 = (axis.y * axis.y) * fSum + fCos;
	_23 = (axis.y * axis.z) * fSum - (axis.x*fSin);

	_31 = (axis.z * axis.x) * fSum - (axis.y*fSin);
	_32 = (axis.z * axis.y) * fSum + (axis.x*fSin);
	_33 = (axis.z * axis.z) * fSum + fCos;

	_44 = 1.0f;

	_41 = _42 = _43 = _14 = _24 = _34 = 0.0f;
}


void Matrix4x4::MakeReflectedMatrix( const Plane &reflectPlane, Matrix4x4& out ) const
{
	Vector4f vReflectPos	= reflectPlane.ReflectPoint( vPos() );
	Vector4f vReflectRight	= vPos() + vRight();
	Vector4f vReflectUp		= vPos() + vUp();
	Vector4f vReflectFace	= vPos() + vFace();

	vReflectRight	= reflectPlane.ReflectPoint( vReflectRight );
	vReflectUp		= reflectPlane.ReflectPoint( vReflectUp );
	vReflectFace	= reflectPlane.ReflectPoint( vReflectFace );

	vReflectRight	= vReflectRight - vReflectPos;
	vReflectUp		= vReflectUp -vReflectPos;
	vReflectFace	= vReflectFace - vReflectPos;

	out.SetPos(vReflectPos);
	out.SetRight(vReflectRight);
	out.SetFace(vReflectFace);
	out.SetUp(vReflectUp);
}



void Matrix4x4::LookAtRH(const Vector3f& vEyePos, const Vector3f& vAt, const Vector3f& vUp)
{
	Vector3f vZAxis = vEyePos - vAt;
	vZAxis.Normalise();
	LookAtInt(vEyePos, vZAxis, vUp);
}

void Matrix4x4::LookAt(const Vector3f& vEyePos, const Vector3f& vAt, const Vector3f& vUp)
{
	Vector3f vZAxis = vAt - vEyePos;
	vZAxis.Normalise();
	LookAtInt(vEyePos, vZAxis, vUp);
}

void Matrix4x4::LookAtInt(const Vector3f& vEyePos, const Vector3f& vZAxis, const Vector3f& vUp)
{
	Vector3f vXAxis = CrossProduct(vUp, vZAxis);
	Vector3f vCorrUp = CrossProduct(vZAxis, vXAxis);
	vXAxis.Normalise();
	vCorrUp.Normalise();
	_11 = vXAxis.x;
	_12 = vCorrUp.x;
	_13 = vZAxis.x;

	_21 = vXAxis.y;
	_22 = vCorrUp.y;
	_23 = vZAxis.y;

	_31 = vXAxis.z;	
	_32 = vCorrUp.z;
	_33 = vZAxis.z;

	_41 = -DotProduct(vXAxis, vEyePos);
	_42 = -DotProduct(vCorrUp, vEyePos);
	_43 = -DotProduct(vZAxis, vEyePos);


	_14 = 0.0f;
	_24 = 0.0f;
	_34 = 0.0f;
	_44 = 1.0f;
}
	

void Matrix4x4::Assign(const Matrix4x3& in)
{
	// The 4x3 is column major so we have to transpose the matrix when copying it accross
	_11 = in._11;
	_12 = in._21;
	_13 = in._31;

	_21 = in._12;
	_22 = in._22;
	_23 = in._32;

	_31 = in._13;
	_32 = in._23;
	_33 = in._33;

	_41 = in._14;
	_42 = in._24;
	_43 = in._34;

	_14 = 0.f;
	_24 = 0.f;
	_34 = 0.f;
	_44 = 1.0f;
	// Slight deviations saw these asserts failing
	//ASSERT(in._14 == 0.0f);
	//ASSERT(in._24 == 0.0f);
	//ASSERT(in._34 == 0.0f);
	//ASSERT(in._44 == 1.0f);
}


}