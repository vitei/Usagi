/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
//	Description: Moving average template.
*****************************************************************************/

#ifndef USAGI_MA_H
#define USAGI_MA_H

#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"

namespace usg
{

	template<class VariableType, uint32 N>
	class MovingAverage
	{
		VariableType m_data[N];
		VariableType m_sum;
		uint32 m_uDataCount;
	public:
		MovingAverage() : m_sum(0), m_uDataCount(0)
		{

		}

		void Clear()
		{
			m_sum = 0;
			m_uDataCount = 0;
		}

		void Add(VariableType value)
		{
			m_sum += value;
			if (m_uDataCount >= N)
			{
				m_sum -= m_data[m_uDataCount % N];
			}
			m_data[m_uDataCount % N] = value;
			m_uDataCount++;
		}

		uint32 GetDataSize() const
		{
			return m_uDataCount;
		}

		VariableType Get() const
		{
			if (m_uDataCount == 0)
			{
				return 0;
			}
			const uint32 uCount = m_uDataCount < N ? m_uDataCount : N;
			return m_sum / VariableType(uCount);
		}

		VariableType Variance() const
		{
			if (GetDataSize() < 2)
			{
				return 0;
			}
			const VariableType fMean = Get();
			const uint32 uM = Math::Min(GetDataSize(), N);
			VariableType fSum = 0;
			for (uint32 i = 0; i < uM; i++)
			{
				fSum += (m_data[i] - fMean)*(m_data[i] - fMean);
			}
			return fSum / (uM - 1);
		}
	};

}

#endif