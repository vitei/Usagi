/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Storage and arithmetic operations for four component vectors
// Used for lights which do not accept floats and in matrix maths
*****************************************************************************/
#ifndef _USG_VECTOR4F
#define	_USG_VECTOR4F

#include "Vector3f.h"
#include "Vector2f.h"

namespace usg{

class Quaternionf;

class Vector4f
{
	friend Vector4f 
	ScalarVectorMultiply(
		const Vector4f &multVector, float scalarValue );

	friend Vector4f
	ScalarVectorDivide(
		const Vector4f &divVector, float scalarValue );

public:
	Vector4f() {};
	~Vector4f() {};

	void Clear();
	Vector4f(float v)  { x = v; y=v; z=v; w=v; }
	Vector4f(float inX, float inY, float inZ, float inW) { x = inX; y=inY; z=inZ; w=inW; }
	Vector4f(const Vector3f& xyz, float inW);
	
	const float* xyzw() const { return m_xyzw; }
	void Assign(float inX, float inY, float inZ, float inW);
	void Assign(const Vector3f& xyz, float inW);

	void operator *=( const float &scalarValue );
	void operator *=( const Vector4f &vScaleVector );
	void operator +=( const Vector4f &addVector );
	void operator /=( const float scalarValue );
	void operator /=( const Vector4f& divVector );
	void operator -=( const Vector4f& subVector );
	bool operator ==( const Vector4f &compVector3f ) const;
	Vector4f operator-() const;

	Vector4f operator*( const Matrix4x4 &rhs ) const;
	
	float DotProduct( const Vector4f &vec1, const Vector4f &vec2 );

	//Addition and subtraction
	Vector4f operator +( const Vector4f &addVector ) const;
	Vector4f operator -( const Vector4f &subVector ) const;
	Vector4f operator *( const Vector4f &scaleVector ) const;
	Vector4f operator /( const Vector4f &scaleVector ) const;

	Vector4f operator * (const Quaternionf& rhs) const;

	float MagnitudeSq() const;
	float Magnitude() const;
	const Vector4f& Normalise();
	const Vector4f& NormaliseIfNZero(const Vector4f &vZeroValue);
	void Translate( float x, float y, float z );
	void GetNormalised(Vector4f& out) const;
	Vector4f GetNormalised() const;
	Vector4f GetFloor() const;

	float GetDistanceFrom( const Vector4f &vec ) const;
	float GetSquaredDistanceFrom( const Vector4f &vec ) const;

	
	//const Vector3f& v3() const { return (Vector3f&)*(&x); };
	//const Vector2f& v2() const { return (Vector2f&)*(&x); };

	Vector3f v3() const { return Vector3f(x,y,z); };
	Vector2f v2() const { return Vector2f(x,y); };
	
	
	union
	{
		float m_xyzw[4];
		
		struct 
		{
			float x, y, z, w;
		};
	};
};

static const Vector4f V4F_ZERO(0.0f, 0.0f, 0.0f, 0.0f);
static const Vector4f V4F_X_AXIS(1.0f, 0.0f, 0.0f, 0.0f);
static const Vector4f V4F_Y_AXIS(0.0f, 1.0f, 0.0f, 0.0f);
static const Vector4f V4F_Z_AXIS(0.0f, 0.0f, 1.0f, 0.0f);

// Accommodates C style functions so that the scalar can be on either side of the equation
inline Vector4f ScalarVectorMultiply(const Vector4f &inV, float inScalar )
{
//	ASSERT(inV.w == 0.0f);
	return Vector4f(inScalar*inV.x, inScalar*inV.y, inScalar*inV.z, inV.w);
}

inline Vector4f ScalarVectorDivide(const Vector4f &inV, float scalarValue)
{
	ASSERT(inV.w == 0.0f);

	float		oneOverVal = 1.f/scalarValue;
	return Vector4f(oneOverVal*inV.x, oneOverVal*inV.y, oneOverVal*inV.z, 0.0f);
}

inline Vector4f operator*( float scalarValue, const Vector4f &multVector )
{
	return ScalarVectorMultiply(multVector, scalarValue);
}

inline Vector4f operator*( const Vector4f &multVector, float scalarValue )
{
	return ScalarVectorMultiply(multVector, scalarValue);
}

inline void Vector4f::Assign(float inX, float inY, float inZ, float inW)
{
	x = inX;
	y = inY;
	z = inZ;
	w = inW;
}

inline void Vector4f::Assign(const Vector3f& xyz, float inW)
{
	Assign(xyz.x, xyz.y, xyz.z, inW);
}


inline Vector4f::Vector4f(const Vector3f& xyz, float inW)
{
	Assign(xyz, inW);
}


 inline void Vector4f::Clear()
 {
 	Assign(0.0f, 0.0f, 0.0f, 0.0f);
 }


inline void Vector4f::operator *=( const float &scalarValue )
{
	x *= scalarValue;
	y *= scalarValue;
	z *= scalarValue;
}

inline void Vector4f::operator *=( const Vector4f &vScaleVector )
{
	x *= vScaleVector.x;
	y *= vScaleVector.y;
	z *= vScaleVector.z;
	w *= vScaleVector.w;
}

inline Vector4f Vector4f::operator +( const Vector4f &in ) const
{
//	ASSERT((in.w == 0.0f) || (w == 0.0f));
	return Vector4f(x+in.x, y+in.y, z+in.z, w+in.w);
}


inline void Vector4f::operator +=( const Vector4f &in )
{
//	ASSERT((in.w == 0.0f));
	x += in.x;
	y += in.y;
	z += in.z;
	//w += addVector4f.w;
}

inline void Vector4f::operator -=( const Vector4f &in )
{
//	ASSERT(in.w == 0.0f || w==1.0f && in.w == 1.0f);
	x -= in.x;
	y -= in.y;
	z -= in.z;
	w -= in.w;
}


inline Vector4f Vector4f::operator -( const Vector4f &in ) const
{
	//ASSERT(in.w == 0.0f || in.w == 1.0f && w==1.0f);
	return Vector4f(x-in.x, y-in.y, z-in.z, w-in.w);
}

inline Vector4f Vector4f::operator *( const Vector4f &scaleVector ) const
{
	return Vector4f(x*scaleVector.x, y*scaleVector.y, z*scaleVector.z, w*scaleVector.w);
}

inline Vector4f Vector4f::operator /( const Vector4f &scaleVector ) const
{
	return Vector4f(x/scaleVector.x, y/scaleVector.y, z/scaleVector.z, w/scaleVector.w);	
}

// TODO: Designed for efficeny, usage in code harder to read
inline void CrossProduct( const Vector4f &vec1, const Vector4f &vec2, Vector4f &out )
{
	//ASSERT(vec1.w == 0.0f && vec2.w == 0.0f);
	out.x = ( vec1.y * vec2.z)-( vec1.z * vec2.y );
	out.y = ( vec1.z * vec2.x)-( vec1.x * vec2.z );
	out.z = ( vec1.x * vec2.y)-( vec1.y * vec2.x );
	out.w = 0.0f;	// Makes no sense to have positional cross product
}

inline Vector4f CrossProduct( const Vector4f &vec1, const Vector4f &vec2 )
{
	Vector4f	out;
//	ASSERT(vec1.w == 0.0f && vec2.w == 0.0f);

	out.x = ( vec1.y * vec2.z)-( vec1.z * vec2.y );
	out.y = ( vec1.z * vec2.x)-( vec1.x * vec2.z );
	out.z = ( vec1.x * vec2.y)-( vec1.y * vec2.x );
	out.w = 0.0f;
	return out;
}

inline Vector4f Vec4fMin( const Vector4f &vec1, const Vector4f &vec2 )
{
	Vector4f vOut;
	vOut.x = vec1.x < vec2.x ? vec1.x : vec2.x;
	vOut.y = vec1.y < vec2.y ? vec1.y : vec2.y;
	vOut.z = vec1.z < vec2.z ? vec1.z : vec2.z;
	vOut.w = vec1.w < vec2.w ? vec1.w : vec2.w;

	return vOut;
}

inline Vector4f Vec4fMax( const Vector4f &vec1, const Vector4f &vec2 )
{
	Vector4f vOut;
	vOut.x = vec1.x > vec2.x ? vec1.x : vec2.x;
	vOut.y = vec1.y > vec2.y ? vec1.y : vec2.y;
	vOut.z = vec1.z > vec2.z ? vec1.z : vec2.z;
	vOut.w = vec1.w > vec2.w ? vec1.w : vec2.w;

	return vOut;
}

inline Vector4f CrossProduct4D( const Vector4f &vec1, const Vector4f &vec2, const Vector4f &vec3)
{
	Vector4f vReturn;
	vReturn.x = vec1.y * (vec2.z * vec3.w - vec3.z * vec2.w) - vec1.z * (vec2.y * vec3.w - vec3.y * vec2.w) + vec1.w * (vec2.y * vec3.z - vec2.z *vec3.y);
	vReturn.y = -(vec1.x * (vec2.z * vec3.w - vec3.z * vec2.w) - vec1.z * (vec2.x * vec3.w - vec3.x * vec2.w) + vec1.w * (vec2.x * vec3.z - vec3.x * vec2.z));
	vReturn.z = vec1.x * (vec2.y * vec3.w - vec3.y * vec2.w) - vec1.y * (vec2.x *vec3.w - vec3.x * vec2.w) + vec1.w * (vec2.x * vec3.y - vec3.x * vec2.y);
	vReturn.w = -(vec1.x * (vec2.y * vec3.z - vec3.y * vec2.z) - vec1.y * (vec2.x * vec3.z - vec3.x *vec2.z) + vec1.z * (vec2.x * vec3.y - vec3.x * vec2.y));

	return vReturn;
}

inline void Vector4f::operator /=( float scalarValue )
{
	x /= scalarValue;
	y /= scalarValue;
	z /= scalarValue;
	w /= scalarValue;
}

inline void Vector4f::operator /=( const Vector4f& divVector )
{
	x /= divVector.x;
	y /= divVector.y;
	z /= divVector.z;
	w /= divVector.w;
}

inline float DotProduct( const Vector4f& vec1, const Vector4f& vec2 )
{
	ASSERT(vec1.w == 0.0f || vec2.w == 0.0f);
	return
		( ( vec1.x * vec2.x )
		+ ( vec1.y * vec2.y )
		+ ( vec1.z * vec2.z )
		+ ( vec1.w * vec2.w ) );
}

inline float Vector4f::Magnitude() const
{
	return sqrtf( ( x*x )+ ( y*y )+ ( z*z ) );
}


inline float Vector4f::MagnitudeSq() const
{
	return ( ( x*x )+ ( y*y )+ ( z*z ) );
}

inline const Vector4f& Vector4f::Normalise()
{
	//ASSERT(w==0.0f);
	if(x!= 0.0f || y!=0.0f || z!=0.0f)
	{
		float length = Magnitude();	    
		x /= length;
		y /= length;
		z /= length;
	}

	return *this;
}

inline const Vector4f& Vector4f::NormaliseIfNZero(const Vector4f& vFailValue)
{
	if(x!= 0.0f || y!=0.0f || z!=0.0f)
	{
		float length = Magnitude();	    
		x /= length;
		y /= length;
		z /= length;
	}
	else
	{
		*this = vFailValue;
	}

	return *this;
}


inline void Vector4f::GetNormalised(Vector4f& out) const
{
	out = *this;
	out.Normalise();
}

inline Vector4f Vector4f::GetNormalised() const
{
	Vector4f out = *this;
	out.Normalise();
	return out;
}

inline Vector4f Vector4f::GetFloor() const
{
	Vector4f out;
	out.x = floorf(x);
	out.y = floorf(y);
	out.z = floorf(z);
	out.w = floorf(w);
	return out;
}

inline void Vector4f::Translate( float x, float y, float z )
{
	m_xyzw[0] += x;
	m_xyzw[1] += y;
	m_xyzw[2] += z;
}


inline float Vector4f::GetDistanceFrom( const Vector4f &vec ) const
{
	Vector4f tmpVec = *this - vec;
	return tmpVec.Magnitude();
}

inline float Vector4f::GetSquaredDistanceFrom( const Vector4f &vec ) const
{
	ASSERT( vec.w == 1.0f && w == 1.0f);
	float xS = vec.x - x;
	float yS = vec.y - y;
	float zS = vec.z - z;
	
	return ( (xS*xS) + (yS*yS) + (zS*zS) );
}

inline Vector4f Vector4f::operator-() const
{
	Vector4f tmp( -x, -y, -z, w );
	return tmp;
}

// Linearly interpolate between two vectors based on interpolation value t
inline void Lerp(const Vector4f &v1, const Vector4f &v2, Vector4f &vOut, float t)
{
	vOut = (v1 + ((v2 - v1) * t));
}

// Some occassions we won't have access to the destination vector
inline Vector4f Lerp(const Vector4f &v1, const Vector4f &v2, float t)
{
	return (v1 + ((v2 - v1) * t));
}

}

#endif
