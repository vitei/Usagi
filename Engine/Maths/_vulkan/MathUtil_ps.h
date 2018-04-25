/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific optimised math functions
*****************************************************************************/
#ifndef _USG_MATHS_MATH_UTIL_PS_H_
#define	_USG_MATHS_MATH_UTIL_PS_H_

namespace usg
{
	namespace Math
	{
		inline void SinCos(const float32 fRad, float &fSinOut, float &fCosOut)
		{
			fSinOut = sinf(fRad);
			fCosOut = cosf(fRad);
		}
	}
}

#endif