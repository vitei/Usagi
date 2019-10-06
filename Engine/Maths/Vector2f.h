/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Storage and arithmetic operations for two component vectors
*****************************************************************************/
#ifndef _USG_VECTOR2F_H_
#define _USG_VECTOR2F_H_

#include "Engine/Maths/MathUtil.h"

namespace usg{


class Vector2f
{
public:
	static const Vector2f ZERO;


	Vector2f(void){ x = y = 0.0f; }
	Vector2f(float32 inX, float32 inY){ x = inX; y = inY; }
	~Vector2f(void){}
	void Assign( float32 fX, float32 fY ) { x = fX; y = fY; }

	static void RotateAboutPoint( const Vector2f& origin, Vector2f& point, float radians )
	{
		const float s = sinf(radians);
		const float c = cosf(radians);

		point.x -= origin.x;
		point.y -= origin.y;

		const float newX = point.x * c - point.y * s;
		const float newY = point.x * s + point.y * c;

		point.x = newX + origin.x;
		point.y = newY + origin.y;
	}

	float DotProduct(const Vector2f& lhs) const;

	static float DotProduct(const Vector2f& lhs, const Vector2f& rhs)
	{
		return (lhs.x * rhs.x) + (lhs.y * rhs.y);
	}

	float Distance(const Vector2f& vV) const;

	void     operator+=( const Vector2f& vec );
	Vector2f operator+ ( const Vector2f& vec ) const;
	Vector2f operator- ( const Vector2f& rhs ) const;
	Vector2f operator* ( const Vector2f& vec ) const;
	Vector2f operator* ( float val ) const;
	bool     operator==( const Vector2f& vec ) const { return (vec.x == x && vec.y == y);  }
	Vector2f operator / ( const Vector2f& rhs ) const;
	Vector2f operator / (float rhs) const;
	Vector2f operator-() const
	{
		return Vector2f(-x, -y);
	}

	void Normalise();
	bool TryNormalise();
	void GetNormalised(Vector2f& out) const;
	Vector2f GetNormalised() const;
	Vector2f GetNormalisedIfNZero(const Vector2f &vDefault = ZERO) const;
	float GetSquaredDistanceFrom(const Vector2f& vec) const;

	float Magnitude() const
	{
		return sqrtf(MagnitudeSquared());
	}

	float MagnitudeSquared() const;

	bool IsEqual(const Vector2f& rhs, float fEpsilon = Math::EPSILON) const;

	float32 x;
	float32 y;
};


inline void Vector2f::operator +=( const Vector2f& vec )
{
	x += vec.x;
	y += vec.y;
}

inline Vector2f Vector2f::operator+( const Vector2f& vec ) const
{
	return Vector2f(x + vec.x, y + vec.y);
}

inline Vector2f Vector2f::operator / ( const Vector2f& rhs ) const
{
	return Vector2f(x / rhs.x, y / rhs.y);
}

inline Vector2f Vector2f::operator/ (float rhs) const
{
	return Vector2f(x / rhs, y / rhs);
}

inline Vector2f Vector2f::operator- ( const Vector2f& rhs ) const
{
	return Vector2f(x - rhs.x, y - rhs.y);
}

inline Vector2f Vector2f::operator*( const Vector2f& vec ) const
{
	return Vector2f(x * vec.x, y * vec.y);
}

inline Vector2f Vector2f::operator*( float val ) const
{
	return Vector2f(x * val, y * val);
}

inline bool Vector2f::IsEqual(const Vector2f& rhs, float fEpislon) const
{
	return( Math::IsEqual(x, rhs.x, fEpislon)
		&& Math::IsEqual(y, rhs.y, fEpislon) );
}

inline float Vector2f::DotProduct(const Vector2f& vV) const
{
	return (x * vV.x) + (y * vV.y);
}

inline float Vector2f::GetSquaredDistanceFrom(const Vector2f& vec) const
{
	const float fDx = x - vec.x;
	const float fDy = y - vec.y;
	return fDx*fDx + fDy*fDy;
}

inline float Vector2f::Distance(const Vector2f& vV) const
{
	return (float)sqrtf(powf(this->x - vV.x, 2.0f) + powf(this->y - vV.y, 2.0f));
}

inline float Vector2f::MagnitudeSquared() const
{
	return ( x*x ) + ( y*y );
}

inline void Vector2f::Normalise()
{
	float fMagSq = MagnitudeSquared();
	if(fMagSq > 0.0f)
	{
		float length = sqrtf(fMagSq);	    
		x /= length;
		y /= length;
	}
	else
	{
		ASSERT(false);
	}
}

inline bool Vector2f::TryNormalise()
{
	float fMagSq = MagnitudeSquared();
	if(fMagSq > 0.0f)
	{
		float length = sqrtf(fMagSq);
		x /= length;
		y /= length;
		return true;
	}
	else
	{
		return false;
	}
}
inline void Vector2f::GetNormalised(Vector2f& out) const
{
	out = *this;
	out.Normalise();
}

inline Vector2f Vector2f::GetNormalised() const
{
	Vector2f out = *this;
	out.Normalise();
	return out;
}

inline Vector2f Vector2f::GetNormalisedIfNZero(const Vector2f& vDefault) const
{
	if(MagnitudeSquared() > Math::EPSILON)
	{
		Vector2f out = *this;
		out.Normalise();
		return out;
	}
	return vDefault;
}

inline Vector2f Lerp(const Vector2f &v1, const Vector2f &v2, const Vector2f vLerp)
{
	return (v1 + ((v2 - v1) * vLerp));
}


}

#endif
