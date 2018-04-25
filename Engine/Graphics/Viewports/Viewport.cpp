/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Viewport.h"

namespace usg {

const float32 VIEWPORT_fFarClip = 1000.0f;

Viewport::~Viewport(void)
{
}


void Viewport::InitViewport(uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight)
{
	SetPos(uLeft, uBottom);
	SetSize(uWidth, uHeight);

	m_bIsSet = true;
}

void Viewport::SetPos(uint32 uLeft, uint32 uBottom)
{
	m_uBottom	= uBottom;
	m_uLeft		= uLeft;
}

void Viewport::SetSize(uint32 uWidth, uint32 uHeight)
{
	m_uWidth	= uWidth;
	m_uHeight	= uHeight;

	m_fAspect = ((float32)(uWidth)) / (uHeight);
}

}
