/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Storage and arithmetic operations for three component vectors
//	Use Vector4f wherever possible, only use Vector3f
//	when storage space is at a premium, such as in vertices.
*****************************************************************************/
#ifndef _USG_VECTOR3F_H_
#define	_USG_VECTOR3F_H_

#include "Engine/Maths/MathUtil.h"
#include "Engine/Maths/Vector2f.h"

namespace usg{

class Matrix4x4;
class Matrix3x3;
class Quaternionf;

class Vector3f
{
	friend Vector3f 
	ScalarVectorMultiply(
		const Vector3f &multVector, float scalarValue );

	friend Vector3f
	ScalarVectorDivide(
		const Vector3f &divVector, float scalarValue );

public:
	static const Vector3f ZERO;
	static const Vector3f ONE;
	static const Vector3f X_AXIS;
	static const Vector3f Y_AXIS;
	static const Vector3f Z_AXIS;

	Vector3f() {}
	
	Vector3f( float ax, float ay, float az )
		{ Assign( ax, ay, az); }

	Vector3f( float a )
	{ Assign( a, a, a); }
	
	
	~Vector3f(){}

	void Clear() { Assign(0.0f, 0.0f, 0.0f); }

	void Assign( float ax, float ay, float az )
		{ x = ax; y = ay; z = az; }

	void operator *=( float scalarValue );
	void operator *=( const Vector3f& mulValue);
	void operator +=( const Vector3f &addVector3f );
	bool operator ==( const Vector3f &compVector3f ) const;
	bool operator !=( const Vector3f &compVector3f ) const;

	void operator -=( const Vector3f &subVector3f );
	void operator /=( float scalarValue );

	//Addition and subtraction
	Vector3f operator +( const Vector3f &addVector3f ) const;
	Vector3f operator -( const Vector3f &subVector3f ) const;
    
	Vector3f operator *( const Vector3f &addVector3f ) const;

	Vector3f operator*( const Matrix4x4 &rhs ) const;
	Vector3f operator*( const Matrix3x3 &rhs ) const;
	Vector3f operator*( const Quaternionf &rhs ) const;

	Vector3f operator-() const;

	float& operator[] (const int i)  { return m_xyz[i]; }
	const float& operator[] (const int i) const { return m_xyz[i]; }
	
	void Normalise();
	void NormaliseIfNZero(const Vector3f &vDefault = Vector3f(0.f, 0.0f, 0.0f));
	bool TryNormalise();
	void GetNormalised(Vector3f& out) const;
	Vector3f GetNormalised() const;
	Vector3f GetNormalisedIfNZero(const Vector3f &vDefault = ZERO) const;

	Vector3f Project(const Vector3f& norm) const;
	
	
	float Magnitude() const;
	float MagnitudeSquared() const;
	
	Vector3f ClampLength(float fLength);
	Vector2f v2() { return Vector2f(m_xyz[0], m_xyz[1]); }

	void AssignAbsolute(const Vector3f& vIn);

	void AssignMin(const Vector3f& v1, const Vector3f& v2);
	void AssignMax(const Vector3f& v1, const Vector3f& v2);
	void SphericalRandomVector(float radius);
	void DirectionalRandomVector(const Vector3f &dir, float radius);
	void VLerp(Vector3f a, Vector3f b, float frac);

	float GetDistanceFrom( const Vector3f &vec ) const;
	float GetXZDistanceFrom( const Vector3f &vec ) const;
	// To avoid the costly sqrt function when the actual value isn't that important
	float GetSquaredDistanceFrom(const Vector3f &vec) const;
	float GetSquaredXZDistanceFrom(const Vector3f &vec) const;
	Vector3f GetSign() const;
	
	static Vector3f RandomRange(const Vector3f& low, const Vector3f& high)
	{
		float x = Math::RangedRandom(low.x, high.x);
		float y = Math::RangedRandom(low.y, high.y);
		float z = Math::RangedRandom(low.z, high.z);
		return Vector3f(x, y, z);
	}

	static Vector3f RandomSign()
	{
		return Vector3f(Math::RandSign(), Math::RandSign(), Math::RandSign());
	}

	union
	{
		float m_xyz[3];
		struct
		{
			float x, y, z;
		};
	};
};


// Accommodates C style functions so that the scalar can be on either side of the equation
inline Vector3f ScalarVectorMultiply(
	const Vector3f &inV, float inScalar )
{
	return Vector3f(inScalar*inV.x, inScalar*inV.y, inScalar*inV.z);
}

inline Vector3f ScalarVectorDivide(
	const Vector3f &inV, float scalarValue )
{
	float		oneOverVal = 1.f/scalarValue;

	return Vector3f(oneOverVal*inV.x, oneOverVal*inV.y, oneOverVal*inV.z);
}


inline bool Vector3f::operator==(const Vector3f &compVector3f) const
{
	if(compVector3f.x == x)
	{
		if(compVector3f.y == y)
		{
			if(compVector3f.z == z)
			{
				return true;
			}
		}
	}
	return false;

}

inline bool Vector3f::operator!=(const Vector3f &compVector3f) const
{
	if(compVector3f.x == x)
	{
		if(compVector3f.y == y)
		{
			if(compVector3f.z == z)
			{
				return false;
			}
		}
	}
	return true;

}

inline Vector3f operator*( float scalarValue, const Vector3f &multVector )
{
	return ScalarVectorMultiply(multVector, scalarValue);
}

inline Vector3f operator*( const Vector3f &multVector, float scalarValue )
{
	return ScalarVectorMultiply(multVector, scalarValue);
}


inline Vector3f operator/( const Vector3f &divVector, const float &scalarValue )
{
	return ScalarVectorDivide(divVector, scalarValue);
}

inline void Vector3f::operator *=( float scalarValue )
{
	x *= scalarValue;
	y *= scalarValue;
	z *= scalarValue;
}

inline void Vector3f::operator *=(const Vector3f& mulValue)
{
	*this = *this * mulValue;
}

inline Vector3f Vector3f::GetSign() const
{
	Vector3f vOut;
	vOut.x = Math::Sign(x);
	vOut.y = Math::Sign(y);
	vOut.z = Math::Sign(z);

	return vOut;
}

inline void Vector3f::operator /=( float scalarValue )
{
	x /= scalarValue;
	y /= scalarValue;
	z /= scalarValue;
}


inline Vector3f Vector3f::operator +( const Vector3f &in ) const
{
	return Vector3f(x+in.x, y+in.y, z+in.z);
}

inline Vector3f Vector3f::operator -( const Vector3f &in ) const
{
	return Vector3f(x-in.x, y-in.y, z-in.z);
}

inline Vector3f Vector3f::operator *( const Vector3f &in ) const
{
	return Vector3f(x*in.x, y*in.y, z*in.z);
}


inline void Vector3f::operator +=( const Vector3f &addVector3f )
{
	x += addVector3f.x;
	y += addVector3f.y;
	z += addVector3f.z;
}

inline void Vector3f::operator -=( const Vector3f &subVector3f )
{
	x -= subVector3f.x;
	y -= subVector3f.y;
	z -= subVector3f.z;
}


// TODO: Designed for efficeny, usage in code harder to read
inline void CrossProduct( const Vector3f &vec1, const Vector3f &vec2, Vector3f &out )
{
	out.x = ( vec1.y * vec2.z)-( vec1.z * vec2.y );
	out.y = ( vec1.z * vec2.x)-( vec1.x * vec2.z );
	out.z = ( vec1.x * vec2.y)-( vec1.y * vec2.x );
}

inline Vector3f CrossProduct( const Vector3f &vec1, const Vector3f &vec2 )
{
	Vector3f	out;
	out.x = ( vec1.y * vec2.z)-( vec1.z * vec2.y );
	out.y = ( vec1.z * vec2.x)-( vec1.x * vec2.z );
	out.z = ( vec1.x * vec2.y)-( vec1.y * vec2.x );
	return out;
}



inline float DotProduct( const Vector3f& vec1,const Vector3f& vec2 )
{
	return ((vec1.x * vec2.x)+(vec1.y * vec2.y)+(vec1.z * vec2.z));
}

inline float Vector3f::MagnitudeSquared() const
{
	return ( x*x ) + ( y*y ) + ( z*z );
}

inline void Vector3f::Normalise()
{
	float fMagSq = MagnitudeSquared();
	if(fMagSq > 0.0f)
	{
		float length = sqrtf(fMagSq);	    
		x /= length;
		y /= length;
		z /= length;
	}
	else
	{
		ASSERT(false);
	}
}

inline void Vector3f::NormaliseIfNZero(const Vector3f &vDefault)
{
	float fMagSq = MagnitudeSquared();
	if (fMagSq > 0.0f)
	{
		float length = sqrtf(fMagSq);
		x /= length;
		y /= length;
		z /= length;
	}
	else
	{
		*this = vDefault;
	}
}

inline bool Vector3f::TryNormalise()
{
	float fMagSq = MagnitudeSquared();
	if(fMagSq > 0.0f)
	{
		float length = sqrtf(fMagSq);
		x /= length;
		y /= length;
		z /= length;
		return true;
	}
	else
	{
		return false;
	}
}
inline void Vector3f::GetNormalised(Vector3f& out) const
{
	out = *this;
	out.Normalise();
}

inline Vector3f Vector3f::GetNormalised() const
{
	Vector3f out = *this;
	out.Normalise();
	return out;
}

inline Vector3f Vector3f::GetNormalisedIfNZero(const Vector3f& vDefault) const
{
	if(MagnitudeSquared() > Math::EPSILON)
	{
		Vector3f out = *this;
		out.Normalise();
		return out;
	}
	return vDefault;
}


inline float Vector3f::Magnitude() const
{
	return sqrtf( ( x*x )+ ( y*y )+ ( z*z ) );
}


inline Vector3f Vector3f::Project(const Vector3f& norm) const
{
	return *this - (DotProduct(*this, norm) * norm);
}



inline float Vector3f::GetDistanceFrom( const Vector3f &vec ) const
{
	const float fDx = x - vec.x;
	const float fDy = y - vec.y;
	const float fDz = z - vec.z;
	return sqrtf(fDx*fDx + fDy*fDy + fDz*fDz);
}

inline float Vector3f::GetXZDistanceFrom( const Vector3f &vec ) const
{
	const float fDx = x - vec.x;
	const float fDz = z - vec.z;
	return sqrtf(fDx*fDx + fDz*fDz);
}

inline float Vector3f::GetSquaredDistanceFrom( const Vector3f &vec ) const
{
	const float fDx = x - vec.x;
	const float fDy = y - vec.y;
	const float fDz = z - vec.z;
	return (fDx*fDx + fDy*fDy + fDz*fDz);
}

inline float Vector3f::GetSquaredXZDistanceFrom( const Vector3f &vec ) const
{
	const float fDx = x - vec.x;
	const float fDz = z - vec.z;
	return (fDx*fDx + fDz*fDz);
}

// Linearly interpolate between two vectors based on interpolation value t
inline void Lerp(const Vector3f &v1, const Vector3f &v2, Vector3f &vOut, float t)
{
	vOut = (v1 + ((v2 - v1) * t));
}

// Some occassions we won't have access to the destination vector
inline Vector3f Lerp(const Vector3f &v1, const Vector3f &v2, float t)
{
	return (v1 + ((v2 - v1) * t));
}


inline void Vector3f::AssignMin(const Vector3f& v1, const Vector3f& v2)
{
	x = v1.x < v2.x ? v1.x : v2.x;
	y = v1.y < v2.y ? v1.y : v2.y;
	z = v1.z < v2.z ? v1.z : v2.z;
}

inline void Vector3f::AssignMax(const Vector3f& v1, const Vector3f& v2)
{
	x = v1.x < v2.x ? v2.x : v1.x;
	y = v1.y < v2.y ? v2.y : v1.y;
	z = v1.z < v2.z ? v2.z : v1.z;
}

inline void Vector3f::AssignAbsolute(const Vector3f& vIn)
{
	x = fabsf(vIn.x);
	y = fabsf(vIn.y);
	z = fabsf(vIn.z);
}

inline Vector3f Vector3f::operator-() const
{
	Vector3f tmp( -x, -y, -z );
	return tmp;
}

inline Vector3f Vector3f::ClampLength(float fLength)
{
	float d = MagnitudeSquared();
	
	if (d > fLength*fLength)
	{
		Normalise();
		*this *= fLength;
	}
    
    return *this;
	
}

inline void Vector3f::SphericalRandomVector(float radius)
{
	x = Math::RangedRandom(-1, 1);
	y = Math::RangedRandom(-1, 1);
	z = Math::RangedRandom(-1, 1);
	TryNormalise();
	*this *= radius;
}

inline void Vector3f::DirectionalRandomVector(const Vector3f &dir, float radius)
{
	x = Math::RangedRandom(-1, 1) + dir.x;
	y = Math::RangedRandom(-1, 1) + dir.y;
	z = Math::RangedRandom(-1, 1) + dir.z;
	Normalise();
	*this *= radius;
}

inline void Vector3f::VLerp(Vector3f a, Vector3f b, float frac)
{
	x = Math::Lerp(a.x, b.x, frac);
	y = Math::Lerp(a.y, b.y, frac);
	z = Math::Lerp(a.z, b.z, frac);
}



}

#endif
