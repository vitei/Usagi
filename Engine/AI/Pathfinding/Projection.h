/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: Used to project two axis on each other.
 *****************************************************************************/

#ifndef __USG_AI_PROJECTION__
#define __USG_AI_PROJECTION__

namespace usg
{

namespace ai
{

class Projection
{
public:
	Projection(float fMin, float fMax) :
	m_fMin(fMin),
	m_fMax(fMax)
	{}
	
	~Projection(){}

	bool Overlap(const Projection& projection)
	{
		const float fMin = projection.m_fMin;
		const float fMax = projection.m_fMax;

		return !(m_fMin > fMax || fMin > m_fMax);
	}

private:
	float m_fMin;
	float m_fMax;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_PROJECTION__