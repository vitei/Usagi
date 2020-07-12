/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceMgr.h"
#include "DebugFont.h"

namespace usg {

DebugFont::DebugFont(void)
{

}

DebugFont::~DebugFont(void)
{
}


bool DebugFont::Load(GFXDevice* pDevice, ResourceMgr* pResMgr, const char* szName)
{
	// FIXME: Hardcoded, load from file
	m_uChrPerLine	= 32;
	m_fTexelWidth = 6.0f;
	m_fTexelHeight = 8.0f;

	m_fUVChrWidth	= (1.0f/256.0f)*m_fTexelWidth;
	m_fUVChrHeight	= (1.0f/32.0f) *m_fTexelHeight;
	m_fOffsetY		= 3.0f;

	m_pTexture = pResMgr->GetTexture(pDevice, szName);

	return true;
}

float DebugFont::GetStringWidth(const char* string, float height) const
{
	float totalWidth = 0.0f;
	float fMultiple = height/m_fUVChrHeight; 
	for(const char* c = string; (*c)!='\0'; c++)
	{
		totalWidth+= fMultiple*m_fUVChrWidth;
	}
	return totalWidth;
}

}

