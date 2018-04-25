#include "Engine/Common/Common.h"
#include "EmitterModifier.h"

bool EmitterModifier::CompareUnitVector(usg::Vector3f &vInOut, const usg::Vector3f newValue, const usg::Vector3f vSafetyValue)
{
	uint32 diffIndex = 0;
	uint32 diffCount = 0;
	for(uint32 i=0; i<3; i++)
	{
		if(vInOut.m_xyz[i] != newValue.m_xyz[i])
		{
			diffCount++;
			diffIndex = i;
		}
	}

	if(diffCount == 0)
	{
		return false;
	}
	if(diffCount > 1)
	{
		vInOut = newValue.GetNormalisedIfNZero(vSafetyValue);
		return true;
	}
	else
	{
		// Only one parameter has changed
		float fSize = newValue.Magnitude();
		// The value that can not be changed
		float fHardSize = newValue.m_xyz[diffIndex];
		fHardSize = fHardSize * fHardSize;
		float fRemainder = 1.0f-fHardSize;
		// The last two values are sqaures of the remainder
		uint32 uIndexA = diffIndex == 0 ? 1 : 0;	// The larger value
		uint32 uIndexB = 0;	// The smaller value
		for(uint32 i=0; i<3; i++)
		{
			if(i != uIndexA && i!=diffIndex)
			{
				uIndexB = i;
			}
		}

		if(fabsf(newValue.m_xyz[uIndexA]) < fabsf(newValue.m_xyz[uIndexB]))
		{
			uint32 uTmp = uIndexA;
			uIndexA = uIndexB;
			uIndexB = uTmp;
		}


		float fFracB = 0.5f;
		float fFracA = 0.5f;
		if(newValue.m_xyz[uIndexA] != 0.0f && newValue.m_xyz[uIndexB] != 0.0f )
		{
			float fFracB = newValue.m_xyz[uIndexB]/ newValue.m_xyz[uIndexA];
			float fFracA = 1.0f - fFracB;
		}

		vInOut.m_xyz[diffIndex] = newValue.m_xyz[diffIndex];
		vInOut.m_xyz[uIndexA] = sqrt(fRemainder) * fFracA; 
		vInOut.m_xyz[uIndexB] = sqrt(fRemainder) * fFracB;

		vInOut.GetNormalisedIfNZero(vSafetyValue);

		return true;
	}
}