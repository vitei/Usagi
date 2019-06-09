/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Math utility functions and constants
*****************************************************************************/
#pragma once

#ifndef USG_MATHS_MATH_UTIL_H
#define USG_MATHS_MATH_UTIL_H

#include API_HEADER(Engine/Maths, MathUtil_ps.h)

namespace usg{

namespace Math
{
	const float targetFPS = 60.0f;
	const float pi = 3.14159265f;
	const float pi_over_2 = 1.5707963267f;
	const float three_pi_over_2 = 4.71238898f;
	const float two_pi = 6.28318530f;
	const float one_over_2pi = 0.159154943f;

	const float piover180 = 0.0174532925f;

	const float EPSILON = 1e-5f;

	inline float32 cotanf(const float32 fVal) { return(1.f / tanf(fVal)); }

	uint32 Rand();
	uint32 RandMax();
	float RandSign();
	float RangedRandom(const float low, const float high);
	sint32 RangedRandomSInt(const sint32 low, const sint32 high);
	uint32 RangedRandomUInt(const uint32 low, const uint32 high);
	void SeedRand();

	template<class NumberType>
	NumberType Square(const NumberType number)
	{
		return( number * number );
	}

	inline float GetTargetFPS()
	{
		return targetFPS;
	}

	inline bool IsNaN(const float value)
	{
		return !(value == value);
	}

	inline float DegToRad(const float angle)
	{
		return ( piover180*angle );
	}

	inline float RadToDeg(const float angle)
	{
		return ( ( 180/pi ) * angle );	
	}

	inline bool IsEqual(const float a, const float b, const float fEpsilon = EPSILON )
	{
		return fabsf(b - a) < fEpsilon;
	}

	inline float Sign(const float f)
	{
		return f>0.0f ? 1.0f : -1.0f;
	}
	
	inline float SqrtSafe(const float f)
	{
		return (f > 0.0f) ? sqrtf(f) : f;
	}

	template <class VariableType>
	inline VariableType MatchLoopingValue(const VariableType a, const VariableType matchValue, const VariableType low, const VariableType high)
	{
		VariableType quarter = (VariableType)((high - low) / 4.0f);
		VariableType threeQuarter = high - quarter;
		quarter += low;
		VariableType difference = high - low;

		if (a < quarter && matchValue > threeQuarter)
		{
			return a + difference;
		}

		if (a > threeQuarter && matchValue < quarter)
		{
			return a - difference;
		}

		return a;
	}

	// TODO: Standard specialisations for int, float etc and for anything else use references
	template <class VariableType>
	inline VariableType Clamp(const VariableType x, const VariableType min, const VariableType max)
	{
		return (x<min  ? min : x>max ? max : x);
	}

	template <class VariableType>
	inline VariableType Clamp01(const VariableType x)
	{
		return (x<0 ? 0 : x>1 ? 1 : x);
	}

	inline float WrapValue(const float x, const float min, const float max)
	{
		float t = fmodf(x - min, max - min);
		return t < 0 ? t + max : t + min;
	}

	template <class VariableType>
	inline VariableType Lerp(const VariableType x, const VariableType y, const float frac)
	{
		return (x*(1.0f-frac))+(y*frac);
	}

	template <class VariableType>
	inline VariableType Abs(const VariableType x)
	{
		return x > 0 ? x : -x;
	}

	template <class VariableType>
	inline VariableType Max(const VariableType x, const VariableType y)
	{
		return ( x > y ? x : y);
	}

	template <class VariableType>
	inline VariableType Min(const VariableType x, const VariableType y)
	{
		return ( x < y ? x : y);
	}

	template <class VariableType>
	inline VariableType Max(VariableType x, VariableType y, VariableType z)
	{
		return Max( x > y ? x : y, z );
	}
	
	template <class VariableType>
	inline VariableType Min(const VariableType x, const VariableType y, const VariableType z)
	{
		return Min( x < y ? x : y, z );
	}	

	inline float GetLerpValue(float fMin, float fMax, float fValue)
	{
		float fDiff = fMax - fMin;
		return (fValue - fMin) / fDiff;
	}

	inline float RemapRange(float fMinInput, float fMaxInput, float fMinOutput, float fMaxOutput, float fInputValue)
	{
		float fLerp = GetLerpValue(fMinInput, fMaxInput, fInputValue);
		return Lerp(fMinOutput, fMaxOutput, fLerp);
	}
	
	inline float AccelerateToValue(const float currentValue, const float desiredValue, const float maxChange)
	{
		if( desiredValue > currentValue )
		{
			return Clamp(currentValue+maxChange, currentValue, desiredValue);
		}
		else
		{
			return Clamp(currentValue-maxChange, desiredValue, currentValue);
		}
	}

	template <class VariableType>
    inline VariableType Roundup(const VariableType x, const int base)
    {
		ASSERT(base && ((base & (base - 1)) == 0)); // assert that base is a power of two
        return ((x) + ((base)-1)) & ~((base)-1);
    }	
	
	inline	float	DegreesToRadians(const float deg)
	{
		return (deg * (Math::pi/180.0f));
	}
	
	inline	float	Slerp(const float t)
	{
		return (sinf(t*Math::pi - Math::pi/2.0f) + 1.0f) / 2.0f;
	}
	
	// return smoothed value between 0 and 1
	// 0 < t < 1
	inline float	Smooth(const float t)
	{
		return (cosf(t * Math::pi + Math::pi) + 1.0f) / 2.0f;
	}

	template<class VectorType>
	VectorType Hermite(const VectorType& p0, const VectorType& v0, const VectorType& p1, const VectorType& v1, const float t)
	{
		const float t3 = t*t*t;
		const float t2 = t*t;
		const float fH00 = 2 * t3 - 3 * t2 + 1;
		const float fH10 = t3 - 2 * t2 + t;
		const float fH01 = t2 * (3 - 2 * t);
		const float fH11 = t2 * (t - 1);
		return fH00 * p0 + fH10 * v0 + fH01 * p1 + fH11 * v1;
	}

	template<class VectorType>
	VectorType HermiteDerivative(const VectorType& p0, const VectorType& v0, const VectorType& p1, const VectorType& v1, const float t)
	{
		const float t2 = t*t;
		const float fH00 = 6 * t2 - 6 * t;
		const float fH10 = 3 * t2 - 4 * t + 1;
		const float fH01 = 6 * t - 6 * t2;
		const float fH11 = 3 * t2 - 2 * t;
		return fH00 * p0 + fH10 * v0 + fH01 * p1 + fH11 * v1;
	}

	template<class VariableType>
	VariableType InverseLerp(const VariableType a, const  VariableType b, const VariableType value)
	{
		return (value - a) / (b - a);
	}

	inline uint32 PowerOfTwo(uint32 x)
	{
		if (x < 1)
		{
			return 1;
		}
		--x;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		return x+1;
	}
}

} // namespace usagi

#endif // USG_MATHS_MATH_UTIL_H
