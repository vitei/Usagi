/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Math class for Quaternions, for the uniniated thats funky 4D
//  math with a talent for rotation operations
*****************************************************************************/
#ifndef _USG_QUATERNION_H_
#define	_USG_QUATERNION_H_

#include "Vector3f.h"
#include "Matrix4x4.h"

namespace usg{

class Matrix4x4;
class Matrix3x3;

class Quaternionf
{
public:
	Quaternionf(void) { Identity(); }
	Quaternionf(const Matrix4x4& mat) { FillFromMatrix(mat); }
	Quaternionf(const Matrix3x3& mat) { FillFromMatrix(mat); }
	Quaternionf( float inX, float inY, float inZ, float inW )
		{ Assign( inX, inY, inZ, inW ); }
	Quaternionf( const Vector3f& v, float inW );

	~Quaternionf(void) {}

	void Assign( float inX, float inY, float inZ, float inW )
		{	x = inX; y = inY; z = inZ; w = inW; }

	void Identity() { Assign(0.0f, 0.0f, 0.0f, 1.0f); }

	Quaternionf operator* ( const Quaternionf &q ) const;	
	void operator *= (const Quaternionf &q );
	const Quaternionf& operator = (const Matrix4x4 &mat) { FillFromMatrix(mat); return *this; }
	const Quaternionf& operator = (const Matrix3x3 &mat) { FillFromMatrix(mat); return *this; }

	// Subtraction and addition
	Quaternionf operator +( const Quaternionf &addQuart ) const;
	Quaternionf operator -( const Quaternionf &subQuart ) const;
	const Quaternionf& operator +=(const Quaternionf &addQuart);
	bool operator ==(const Quaternionf& rhs) const { return rhs.x == x && rhs.y == y &&	rhs.z == z && rhs.w == w; }
	bool operator !=(const Quaternionf& rhs) const { return !(*this == rhs); }
	// Return negated version of this quaternion
	const Quaternionf operator -() const;
	
	//Return the conjugate of the quaternion
	const Quaternionf operator~() const;
	void SetFromEuler(float yaw, float pitch, float roll);
	void MakeVectorRotation(const Vector3f& from, const Vector3f& to);

	// Calculates the W component from x, y, and z with the assumption that
	// this should be a unit quaternion
	void CalculateW();

	float Magnitude() const;
	float MagnitudeSquared() const;
	inline void Normalise();
	void FillFromMatrix(const Matrix4x4& mat);
	void FillFromMatrix(const Matrix3x3& mat);

	// Helper functions, don't rely on these when you've a lot of movement on multiple axes
	float GetYaw() const { return asinf(-2*(x*z - w*y)); }
	float GetPitch() const { return atan2f(2*(y*z + w*x), w*w - x*x - y*y + z*z); }
	float GetRoll() const { return atan2f(2*(x*y + w*z), w*w + x*x - y*y - z*z); }

	void GetAngleAxis(Vector3f& vAxis, float& fAngle) const;

	Quaternionf GetNormalised() const;

	float x, y, z;
	float w;		// Angle of rotation	
private:

};


inline Quaternionf::Quaternionf( const Vector3f& inAxis, float inAngle )
{
    float result = sinf( inAngle / 2.0f );
	
    x = inAxis.x * result;
    y = inAxis.y * result;
    z = inAxis.z * result;
	
    w = cosf( inAngle / 2.0f );
	
	Normalise();
}




// Multiplication, quaternion multiplication is defined as q * r = [qwrw -qvrv, (qv x rv, + rwqv + qwrv ) ]
// Multiplication of quaternions is non commutative and represents adding the rotations together as with a rotation matrix
inline void Quaternionf::operator *= (const Quaternionf &q )
{
	float tmpX, tmpY, tmpZ, tmpW;

	tmpW = w*q.w - x*q.x - y*q.y - z*q.z;
	tmpX = w*q.x + x*q.w + y*q.z - z*q.y;
	tmpY = w*q.y + y*q.w + z*q.x - x*q.z;
	tmpZ = w*q.z + z*q.w + x*q.y - y*q.x;

	x = tmpX;
	y = tmpY;
	z = tmpZ;
	w = tmpW;
}


inline Quaternionf Quaternionf::operator * (const Quaternionf &q) const
{
	Quaternionf out;

	out.w = w*q.w - x*q.x - y*q.y - z*q.z;
	out.x = w*q.x + x*q.w + y*q.z - z*q.y;
	out.y = w*q.y + y*q.w + z*q.x - x*q.z;
	out.z = w*q.z + z*q.w + x*q.y - y*q.x;

	return out;
}

inline Quaternionf Quaternionf::operator +( const Quaternionf &addQuart ) const
{
	Quaternionf tempQuart;

	tempQuart.x = x + addQuart.x;
	tempQuart.y = y + addQuart.y;
	tempQuart.z = z + addQuart.z;
	tempQuart.w = w + addQuart.w;

	return tempQuart;
}

inline const Quaternionf& Quaternionf::operator+= (const Quaternionf& addQuart)
{
	x += addQuart.x;
	y += addQuart.y;
	z += addQuart.z;
	w += addQuart.w;

	return *this;
}

inline Quaternionf Quaternionf::operator -( const Quaternionf &subQuart ) const
{
	Quaternionf tempQuart;

	tempQuart.x = x - subQuart.x;
	tempQuart.y = y - subQuart.y;
	tempQuart.z = z - subQuart.z;
	tempQuart.w = w - subQuart.w;

	return tempQuart;
}


// Return negated version of this quaternion
inline const Quaternionf Quaternionf::operator -() const
{
	return Quaternionf( -x, -y, -z, -w );
}

// Return the conjugate of the quaternion if q = [n,v] then ~q = [n, -v]
// The conjugate of a unit quaternion is its inverse
inline const Quaternionf Quaternionf::operator~() const
{
	// Negate the vector component of the vector component
	return Quaternionf(-x, -y, -z, w);
}

// Calculate W for a normalised quaternion, together the components
// should have a straight line length through 4D space of 1
inline void Quaternionf::CalculateW()
{
	float t = 1.0f - (x * x) - (y * y) - (z * z);

	w = t < 0.0f ? 0.0f : -sqrtf(t);
}

inline float Quaternionf::Magnitude() const
{
	return sqrtf( x*x + y*y + z*z + w*w );
}

inline float Quaternionf::MagnitudeSquared() const
{
	return x*x + y*y + z*z + w*w;
}

inline void Quaternionf::Normalise()
{
	if( x!= 0.0f || y!=0.0f || z!=0.0f || w!=0.0f)
	{
		float length = Magnitude();
		x /= length;
		y /= length;
		z /= length;
		w /= length;
	}
}


// Static functions

inline Quaternionf ScalarQuaternionMultiply( const Quaternionf &q, const float &scalar )
{
	return Quaternionf( q.x*scalar, q.y*scalar, q.z*scalar, q.w*scalar );
}

inline Quaternionf	ScalarQuaternionDivide( const Quaternionf &q, const float &scalar )
{
	// First calculate the inverse of the scalar as multiplications are faster
	float invScalar = 1.0f/scalar;
	return Quaternionf( q.x*invScalar, q.y*invScalar, q.z*invScalar, q.w*invScalar );
}

inline Quaternionf operator*( const float &scalar, const Quaternionf &q )
{
	return ScalarQuaternionMultiply(q, scalar);
}

inline Quaternionf operator*( const Quaternionf &q, const float &scalar )
{
	return ScalarQuaternionMultiply(q, scalar);
}


inline Quaternionf operator/( const Quaternionf &q, const float &scalar )
{
	return ScalarQuaternionDivide(q, scalar);
}


inline float DotProduct( const Quaternionf& q1,const Quaternionf& q2 )
{
	return
		( ( q1.x * q2.x ) + ( q1.y * q2.y ) + ( q1.z * q2.z ) + (q1.w * q2. w ) );
}

// Rotates a vector point by a quaternion p'=q(p)(~q) where ~q is the conjugate
inline Vector3f RotatePoint(const Quaternionf &q, const Vector3f &p)
{
	// Perform quaternion multiplication
	Quaternionf qTemp = q * Quaternionf(p.x, p.y, p.z, 0.0) * ~q;

	return(Vector3f(qTemp.x, qTemp.y, qTemp.z));
}

// Linearly interpolate between two quarternions based upon an interpolation value t
// (t will generally be (time elapsed since q)/ (time from q to p)
inline Quaternionf Lerp(Quaternionf q, Quaternionf p, float t)
{
	Quaternionf out = q*(1 - t) + p*t;
	// Interpolated result will no longer be of unit length
	out.Normalise();
	return out;
}


Quaternionf Slerp(Quaternionf q0, Quaternionf q1, float t);

}

#endif
