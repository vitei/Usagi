/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Pack unit vectors and unit quaternions into integers applying
//  the fact that X^2 + Y^2 + Z^2 = 1.
*****************************************************************************/
#ifndef USAGI_NETWORK_VECTOR_PACK_H
#define USAGI_NETWORK_VECTOR_PACK_H

#include "Engine/Maths/Vector3f.h"
#include "Engine/Maths/Quaternionf.h"

namespace usg
{

	template<typename IntegerType, typename FloatType, typename VectorType, size_t TotalBits>
	IntegerType PackUnitVector(VectorType v)
	{
		// static_assert(TotalBits % 2 == 0 && TotalBits >= 6 && TotalBits <= 64 && TotalBits <= sizeof(IntegerType) * 8, "Incorrect TotalBits.");

		IntegerType r = 0;

		// First handle sign bits. Sign bit will be 1 if the component is >= 0. Also make the components positive now, if they were negative.
		r |= v.x >= 0 ? ((IntegerType)1 << ((IntegerType)TotalBits - 0 - 1)) : (v.x = -v.x, 0);
		r |= v.y >= 0 ? ((IntegerType)1 << ((IntegerType)TotalBits - 1 - 1)) : (v.y = -v.y, 0);
		r |= v.z >= 0 ? ((IntegerType)1 << ((IntegerType)TotalBits - 2 - 1)) : (v.z = -v.z, 0);

		// Project the vector to the plane that passes through standard basis vectors
		const FloatType s = v.x + v.y + v.z;
		const FloatType M = 1 / s;
		v.x *= M;
		v.y *= M;
		v.z *= M;

		enum 
		{
			XBitCount = (TotalBits - 3) / 2 + 1,
			YBitCount = (TotalBits - 3) / 2,
			XRange = (1 << XBitCount) - 1,
			YRange = (1 << (XBitCount - 1)),
		};


		// static_assert(XBitCount + YBitCount == TotalBits - 3, "");

		IntegerType ix = (IntegerType)(((FloatType)(XRange - 1))*v.x + 0.49f);
		IntegerType iy = (IntegerType)(((FloatType)(XRange - 1))*v.y + 0.49f);

		ASSERT(ix <= XRange);
		ASSERT(iy <= XRange);

		if (iy >= (1 << XBitCount) / 2) 
		{
			ix = XRange - ix;
			iy = XRange - iy;
		}

		ASSERT(ix <= XRange);
		ASSERT(iy <= YRange);

		r |= (ix << (YBitCount)) | iy;

		return r;
	}

	template<typename VectorType, typename FloatType, typename IntegerType, size_t TotalBits>
	VectorType UnpackUnitVector(IntegerType vi)
	{
		const IntegerType totalBits = (IntegerType)TotalBits;
		const IntegerType one = (IntegerType)1;
		VectorType v ( (vi & (one << (totalBits - 0 - 1))) != 0 ? 1.0f : -1.0f,(vi & (one << (totalBits - 1 - 1))) != 0 ? 1.0f : -1.0f,(vi & (one << (totalBits - 2 - 1))) != 0 ? 1.0f : -1.0f );
		enum
		{
			XBitCount = (TotalBits - 3) / 2 + 1,
			YBitCount = (TotalBits - 3) / 2,
			XRange = (1 << XBitCount) - 1,
			YRange = (1 << (XBitCount - 1)) - 1
		};

		IntegerType XBits = (vi&(XRange << YBitCount)) >> YBitCount;
		IntegerType YBits = vi&YRange;

		if (XBits + YBits > XRange)
		{
			XBits = XRange - XBits;
			YBits = XRange - YBits;
		}

		v.x *= XBits / (FloatType)XRange;
		v.y *= YBits / (FloatType)XRange;
		v.z *= 1 - fabsf(v.x) - fabsf(v.y);

		const FloatType M = 1 / sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
		v.x *= M;
		v.y *= M;
		v.z *= M;

		return v;
	}

	template<typename IntegerType>
	usg::Vector3f UnpackUnitVector(IntegerType vi)
	{
		enum
		{
			TotalBits = sizeof(IntegerType) * 8
		};
		return UnpackUnitVector<usg::Vector3f, float, IntegerType, TotalBits>(vi);
	}

	template<typename IntegerType>
	IntegerType PackUnitVector(const usg::Vector3f& v)
	{
		enum
		{
			TotalBits = sizeof(IntegerType) * 8
		};
		return PackUnitVector<IntegerType, float, usg::Vector3f, TotalBits>(v);
	}

	template<class IntegerType>
	IntegerType PackUnitQuaternion(const Quaternionf& q)
	{
		ASSERT(fabsf(q.MagnitudeSquared() - 1) < 1 / 1000.0f);
		const size_t intSizeBits = sizeof(IntegerType) * 8;
		const size_t bitsPerComponent = intSizeBits / 3;
		const size_t remainingBits = intSizeBits - bitsPerComponent * 3;
		const size_t xBits = bitsPerComponent;
		const size_t yBits = bitsPerComponent + (remainingBits - 1);
		const size_t zBits = bitsPerComponent;
		const size_t signBitShift = xBits + yBits + zBits;
		const float maxValX = 1 << xBits;
		const float maxValY = 1 << yBits;
		const float maxValZ = 1 << zBits;
		const IntegerType iX = (IntegerType)((q.x + 1) / 2 * maxValX + 1 / 2.0f);
		const IntegerType iY = (IntegerType)((q.y + 1) / 2 * maxValY + 1 / 2.0f);
		const IntegerType iZ = (IntegerType)((q.z + 1) / 2 * maxValZ + 1 / 2.0f);
		const IntegerType signBit = q.w < 0 ? 0 : 1;
		return iX | (iY << xBits) | (iZ << (xBits + yBits)) | (signBit << signBitShift);
	}

	template<class IntegerType>
	Quaternionf UnpackUnitQuaternion(IntegerType qi)
	{
		const size_t intSizeBits = sizeof(IntegerType) * 8;
		const size_t bitsPerComponent = intSizeBits / 3;
		const size_t remainingBits = intSizeBits - bitsPerComponent * 3;
		const size_t xBits = bitsPerComponent;
		const size_t yBits = bitsPerComponent + (remainingBits - 1);
		const size_t zBits = bitsPerComponent;
		const size_t signBitShift = xBits + yBits + zBits;
		const float maxValX = 1 << xBits;
		const float maxValY = 1 << yBits;
		const float maxValZ = 1 << zBits;
		const IntegerType ix = qi&((1 << xBits) - 1);
		const IntegerType iy = (qi >> xBits)&((1 << yBits) - 1);
		const IntegerType iz = (qi >> (xBits + yBits))&((1 << zBits) - 1);
		const bool wIsNonNegative = (qi&(((IntegerType)1) << signBitShift)) != 0;
		const float x = -1 + 2 * ix / maxValX;
		const float y = -1 + 2 * iy / maxValY;
		const float z = -1 + 2 * iz / maxValZ;
		const float ww = 1 - x*x - y*y - z*z;
		const float w = ww < 0 ? 0 : (sqrtf(ww)*(wIsNonNegative ? 1 : -1));
		Quaternionf qR = Quaternionf(x, y, z, w);
		qR.Normalise();
		return qR;
	}

	template<typename IntegerType>
	inline IntegerType RangedFloatToInt(float fValue, const float fMin, const float fMax)
	{
		enum
		{
			IntegerType_Size_Bits = sizeof(IntegerType) * 8,
			IntegerType_Max_Val = 1 << (IntegerType_Size_Bits - 1)
		};
		return (IntegerType)((Math::Clamp(fValue, fMin, fMax) - fMin) / (fMax - fMin)*(float)IntegerType_Max_Val);
	}

	template<typename IntegerType>
	inline float IntToRangedFloat(IntegerType uValue, const float fMin, const float fMax)
	{
		enum
		{
			IntegerType_Size_Bits = sizeof(IntegerType) * 8,
			IntegerType_Max_Val = 1 << (IntegerType_Size_Bits - 1)
		};
		return fMin + uValue / (float)IntegerType_Max_Val*(fMax - fMin);
	}

}

#endif