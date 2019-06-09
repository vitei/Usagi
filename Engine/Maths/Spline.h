/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef Spline_h__
#define Spline_h__

#include "Engine/Maths/Vector3f.h"

#include "Engine/Maths/MathUtil.h"

namespace usg
{

	template<class VectorType, size_t MaxNumControlPoints>
	class Spline
	{
		size_t m_uNumControlPoints;

		void ComputeSegmentLengths();
		VectorType Sample(size_t uSegment, float t) const;
		VectorType Derivative(size_t uSegment, float t) const;
	public:
		void Setup(const VectorType* pControlPoints, const VectorType* pTangents, size_t uNumControlPoints) {
			ASSERT(uNumControlPoints <= MaxNumControlPoints);
			m_uNumControlPoints = uNumControlPoints;
			memcpy(m_vControlPoints, pControlPoints, sizeof(VectorType)*uNumControlPoints);
			memcpy(m_vTangents, pTangents, sizeof(VectorType)*uNumControlPoints);
			ComputeSegmentLengths();
		}

		Spline() : m_uNumControlPoints(0) {

		}

		size_t GetControlPointCount() const {
			return m_uNumControlPoints;
		}

		float GetTotalLength() const {
			float fLength = m_fSegmentLengths[0];
			for (uint32 uSegment = 1; uSegment < m_uNumControlPoints - 1; uSegment++)
			{
				fLength += m_fSegmentLengths[uSegment];
			}
			return fLength;
		}

		VectorType Sample(float t) const;
		size_t GetSegment(float t) const;
		VectorType Derivative(float t) const;

		VectorType m_vControlPoints[MaxNumControlPoints];
		VectorType m_vTangents[MaxNumControlPoints];
		float m_fSegmentLengths[MaxNumControlPoints - 1];
		float m_fSegmentBeginT[MaxNumControlPoints];
	};

	template<class VectorType, size_t MaxNumControlPoints>
	void Spline<VectorType, MaxNumControlPoints>::ComputeSegmentLengths()
	{
		const size_t uItersPerSegment = 8;

		float fSum = 0;
		for (size_t uSegment = 0; uSegment < m_uNumControlPoints - 1; uSegment++)
		{
			float fLength = 0;
			Vector3f vPrev = Sample(uSegment, 0.0f);
			for (size_t j = 1; j < uItersPerSegment; j++)
			{
				const float fT = j / (float)(uItersPerSegment - 1);
				const Vector3f vSample = Sample(uSegment, fT);
				fLength += vSample.GetDistanceFrom(vPrev);
				vPrev = vSample;
			}

			m_fSegmentLengths[uSegment] = fLength;
			m_fSegmentBeginT[uSegment] = fSum;
			fSum += fLength;
		}
		for (size_t i = 0; i < m_uNumControlPoints - 1; i++)
		{
			m_fSegmentBeginT[i] /= fSum;
		}
		m_fSegmentBeginT[m_uNumControlPoints - 1] = 1.0f;
	}

	template<class VectorType, size_t MaxNumControlPoints>
	size_t Spline<VectorType,MaxNumControlPoints>::GetSegment(float t) const
	{
		for (size_t i = 0; i < m_uNumControlPoints - 2; i++) {
			if (t >= m_fSegmentBeginT[i] && t < m_fSegmentBeginT[i + 1])
			{
				return i;
			}
		}
		return m_uNumControlPoints - 2;
	}

	template<class VectorType, size_t MaxNumControlPoints>
	VectorType Spline<VectorType, MaxNumControlPoints>::Sample(size_t uSegment, float fT) const
	{
		const Vector3f& vPos0 = m_vControlPoints[uSegment];
		const Vector3f& vPos1 = m_vControlPoints[uSegment + 1];
		const Vector3f& vTan0 = m_vTangents[uSegment];
		const Vector3f& vTan1 = m_vTangents[uSegment + 1];
		return Math::Hermite(vPos0, vTan0, vPos1, vTan1, fT);
	}

	template<class VectorType, size_t MaxNumControlPoints>
	VectorType Spline<VectorType, MaxNumControlPoints>::Sample(float t) const
	{
		const size_t uSegment = GetSegment(t);
		const float fCurrentSegmentT = Math::InverseLerp(m_fSegmentBeginT[uSegment], m_fSegmentBeginT[uSegment + 1], t);
		return Sample(uSegment, fCurrentSegmentT);
	}

	template<class VectorType, size_t MaxNumControlPoints>
	VectorType Spline<VectorType, MaxNumControlPoints>::Derivative(size_t uSegment, float fT) const
	{
		const Vector3f& vPos0 = m_vControlPoints[uSegment];
		const Vector3f& vPos1 = m_vControlPoints[uSegment + 1];
		const Vector3f& vTan0 = m_vTangents[uSegment];
		const Vector3f& vTan1 = m_vTangents[uSegment + 1];
		return Math::HermiteDerivative(vPos0, vTan0, vPos1, vTan1, fT);
	}

	template<class VectorType, size_t MaxNumControlPoints>
	VectorType Spline<VectorType, MaxNumControlPoints>::Derivative(float t) const
	{
		const size_t uSegment = GetSegment(t);
		const float fCurrentSegmentT = Math::InverseLerp(m_fSegmentBeginT[uSegment], m_fSegmentBeginT[uSegment + 1], t);
		return Derivative(uSegment, fCurrentSegmentT);
	}
}

namespace spline {

inline float calcHermite( float frame,
				   float key0frame, float key0value, float key0slope,
				   float key1frame, float key1value, float key1slope ) {
	float32 t1 = frame - key0frame;
	float32 t2 = 1.0f / ( key1frame - key0frame );
	float32 v0 = key0value;
	float32 v1 = key1value;
	float32 s0 = key0slope;
	float32 s1 = key1slope;

	float32 t1_squared_t2 = t1 * t1 * t2;
	float32 t1_squared_t2_squared = t1_squared_t2 * t2;
	float32 t1_cubed_t2_squared = t1 * t1_squared_t2_squared;
	float32 t1_cubed_t2_cubed = t1_cubed_t2_squared * t2;

	return v0 * ( 2.0f * t1_cubed_t2_cubed - 3.0f * t1_squared_t2_squared + 1.0f )
		+ v1 * ( -2.0f * t1_cubed_t2_cubed + 3.0f * t1_squared_t2_squared )
		+ s0 * ( t1_cubed_t2_squared - 2.0f * t1_squared_t2 + t1 )
		+ s1 * ( t1_cubed_t2_squared - t1_squared_t2 );
}

inline float calcLinear( float frame,
				  float key0frame, float key0value, float key1frame, float key1value ) {
	float tangent = ( key1value - key0value ) / ( key1frame - key0frame );
	return tangent * ( frame - key0frame ) + key0value;
}

}

#endif // Spline_h__
