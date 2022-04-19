/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix3x3.h"
#include "Quaternionf.h"

namespace usg{

float ReciprocalSqrt( float x )
{ 
	long i; 
	float y, r; 

	y = x * 0.5f; 
	i = *(long *)( &x ); 
	i = 0x5f3759df - ( i >> 1 ); 
	r = *(float *)( &i ); 
	r = r * ( 1.5f - r * r * y ); 
	return r;
}

void Quaternionf::GetAngleAxis(Vector3f& vAxis, float& fAngle) const
{
	const Quaternionf& qRot = *this;
	ASSERT(fabsf(qRot.Magnitude()-1.0f) < Math::EPSILON);
	const float qWSqr = qRot.w*qRot.w;
	if (qWSqr > 1 - Math::EPSILON)
	{
		fAngle = 0;
		vAxis.x = 0;
		vAxis.y = 0;
		vAxis.z = 1;
		return;
	}
	fAngle = 2 * acosf(qRot.w);
	const float fSqrtDivisor = 1.0f / sqrtf(1 - qWSqr);
	vAxis.x = qRot.x * fSqrtDivisor;
	vAxis.y = qRot.y * fSqrtDivisor;
	vAxis.z = qRot.z * fSqrtDivisor;
}

void Quaternionf::MakeVectorRotation(const Vector3f& vFrom, const Vector3f& vTo)
{
	const Vector3f vCross = CrossProduct(vFrom, vTo );
	const float32 fDotAddOne = DotProduct( vFrom, vTo ) + 1.f;

	if ( fDotAddOne <= Math::EPSILON )
	{
		x = 1.f;
		y = 0.f;
		z = 0.f;
		w = 0.f;
		return;
	}

	const float32 s = sqrtf( fDotAddOne * 2.f );
	const float32 oos = 1.f / s;

	x = vCross.x * oos;
	y = vCross.y * oos;
	z = vCross.z * oos;
	w = s * 0.5f;
}

Quaternionf Quaternionf::GetNormalised() const
{
	Quaternionf qOut = *this;
	qOut.Normalise();
	return qOut;
}

void Quaternionf::FillFromMatrix(const Matrix3x3& mat)
{
	float trace = mat.M[0][0] + mat.M[1][1] + mat.M[2][2]; 
	if (trace > 0)
	{
		float s = 0.5f / sqrtf(trace + 1.0f);
		w = 0.25f / s;
		x = (mat.M[1][2] - mat.M[2][1]) * s;
		y = (mat.M[2][0] - mat.M[0][2]) * s;
		z = (mat.M[0][1] - mat.M[1][0]) * s;
	}
	else
	{
		if (mat.M[0][0] > mat.M[1][1] && mat.M[0][0] > mat.M[2][2]) {
			float s = 2.0f * sqrtf(1.0f + mat.M[0][0] - mat.M[1][1] - mat.M[2][2]);
			w = (mat.M[1][2] - mat.M[2][1]) / s;
			x = 0.25f * s;
			y = (mat.M[1][0] + mat.M[0][1]) / s;
			z = (mat.M[2][0] + mat.M[0][2]) / s;
		}
		else if (mat.M[1][1] > mat.M[2][2]) {
			float s = 2.0f * sqrtf(1.0f + mat.M[1][1] - mat.M[0][0] - mat.M[2][2]);
			w = (mat.M[2][0] - mat.M[0][2]) / s;
			x = (mat.M[1][0] + mat.M[0][1]) / s;
			y = 0.25f * s;
			z = (mat.M[2][1] + mat.M[1][2]) / s;
		}
		else {
			float s = 2.0f * sqrtf(1.0f + mat.M[2][2] - mat.M[0][0] - mat.M[1][1]);
			w = (mat.M[0][1] - mat.M[1][0]) / s;
			x = (mat.M[2][0] + mat.M[0][2]) / s;
			y = (mat.M[2][1] + mat.M[1][2]) / s;
			z = 0.25f * s;
		}
	}

	// Getting slight rounding errors through
	Normalise();
}

void Quaternionf::FillFromMatrix(const Matrix4x4 &mat)
{
	float trace = mat.M[0][0] + mat.M[1][1] + mat.M[2][2];
	if (trace > 0)
	{
		float s = 0.5f / sqrtf(trace + 1.0f);
		w = 0.25f / s;
		x = (mat.M[1][2] - mat.M[2][1]) * s;
		y = (mat.M[2][0] - mat.M[0][2]) * s;
		z = (mat.M[0][1] - mat.M[1][0]) * s;
	}
	else
	{
		if (mat.M[0][0] > mat.M[1][1] && mat.M[0][0] > mat.M[2][2]) {
			float s = 2.0f * sqrtf(1.0f + mat.M[0][0] - mat.M[1][1] - mat.M[2][2]);
			w = (mat.M[1][2] - mat.M[2][1]) / s;
			x = 0.25f * s;
			y = (mat.M[1][0] + mat.M[0][1]) / s;
			z = (mat.M[2][0] + mat.M[0][2]) / s;
		}
		else if (mat.M[1][1] > mat.M[2][2]) {
			float s = 2.0f * sqrtf(1.0f + mat.M[1][1] - mat.M[0][0] - mat.M[2][2]);
			w = (mat.M[2][0] - mat.M[0][2]) / s;
			x = (mat.M[1][0] + mat.M[0][1]) / s;
			y = 0.25f * s;
			z = (mat.M[2][1] + mat.M[1][2]) / s;
		}
		else {
			float s = 2.0f * sqrtf(1.0f + mat.M[2][2] - mat.M[0][0] - mat.M[1][1]);
			w = (mat.M[0][1] - mat.M[1][0]) / s;
			x = (mat.M[2][0] + mat.M[0][2]) / s;
			y = (mat.M[2][1] + mat.M[1][2]) / s;
			z = 0.25f * s;
		}
	}
	// Getting slight rounding errors through
	Normalise();
}

void Quaternionf::SetFromEuler(float yaw, float pitch, float roll)
{
	pitch = (float)(pitch);
	yaw	  = (float)(yaw);
	roll  = (float)(roll);

	float cosP = cosf(pitch / 2.0f);
	float sinP = sinf(pitch / 2.0f);

	float cosY = cosf(yaw   / 2.0f);
	float sinY = sinf(yaw   / 2.0f);

	float cosR = cosf(roll  / 2.0f);
	float sinR = sinf(roll  / 2.0f);

	float cosPcosY = cosP*cosY;
	float sinPsinY = sinP*sinY;
	float sinPcosY = sinP*cosY;
	float cosPsinY = cosP*sinY;

	x = cosR*sinPcosY + sinR*cosPsinY;
	y= cosR*cosPsinY - sinR*sinPcosY;
	z= sinR*cosPcosY - cosR*sinPsinY;
	w = cosR*cosPcosY + sinR*sinPsinY;

}

Quaternionf Slerp(Quaternionf q0, Quaternionf q1, float t)
{
	float fCosAngle = DotProduct(q0, q1);
	// If the dot product is negative then the angle between the two rotations
	// is greater than 90 and therefore rotating the opposite direction is shorter
	if (fCosAngle < 0.0f)
	{
		q0 = -q0;
		fCosAngle = -fCosAngle;
	}

	// If the float angle between them is too close to 0 then default to
	// linear interpolation as Slerp does not handle such interpolations well
	if (fCosAngle > 0.9999f)
	{
		return Lerp(q0, q1, t);
	}

	const float fOmega = acosf(fCosAngle);
	const float fSinO = sinf(fOmega);
	const float fScale0 = sinf((1.0f - t) * fOmega) / fSinO;
	const float fScale1 = sinf(t * fOmega) / fSinO;
	return  (fScale0 * q0) + (fScale1 * q1);
}

}
