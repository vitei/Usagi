/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Fog.h"
#include <float.h>

namespace usg {

Fog::Fog() :
m_bActive(false),
m_eType(FOG_TYPE_LINEAR),
m_color(0.0f, 0.0f, 0.0f, 0.0f),
m_fDensity(0.0f),
m_fMinDepth(0.0f),
m_fMaxDepth(0.0f)
{
	
}

Fog::~Fog()
{

}

void Fog::SetActive(bool bActive)
{
	if(m_bActive!=bActive)
	{
		m_bActive = bActive;
	}
}

void Fog::SetType(FogType eType)
{
	if(eType!=m_eType)
	{
		m_eType = eType;
	}
}


void Fog::SetMinDepth(float depth)
{
	if(m_fMinDepth!=depth)
	{
		m_fMinDepth = depth;
	}
}

void Fog::SetMaxDepth(float depth)
{
	if(m_fMaxDepth!=depth)
	{
		m_fMaxDepth = depth;
	}
}


}