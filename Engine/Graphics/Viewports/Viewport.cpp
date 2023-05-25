/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Viewport.h"

namespace usg {


Viewport::~Viewport(void)
{
}


void Viewport::InitViewport(uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight)
{
	SetPos(uLeft, uBottom);
	SetSize(uWidth, uHeight);

	m_bIsSet = true;
}


void Viewport::InitViewportAspect(uint32 uWidth, uint32 uHeight, float fTargAspect)
{
	float fWidth = (float)uWidth;
	float fHeight = (float)uHeight;
	float fOffset = 0.0f;

	if (fWidth > (fHeight * fTargAspect))
	{
		fWidth = fHeight * fTargAspect;
		fOffset = ((float)uWidth - fWidth) /2.f;
		SetPos((uint32)fOffset, 0);
	}
	else
	{
		fHeight = fWidth * (1.0f/fTargAspect);
		fOffset = ((float)uHeight - fHeight) / 2.f;
		SetPos(0, (uint32)fOffset);
	}

	SetSize(uint32(fWidth), uint32(fHeight));

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
