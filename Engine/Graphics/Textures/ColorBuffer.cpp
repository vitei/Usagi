/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "ColorBuffer.h"

namespace usg {

ColorBuffer::ColorBuffer()
{
	m_uWidth = 0;
	m_uHeight = 0;
	m_eType = TYPE_INVALID;
	m_eSampleCount = SAMPLE_COUNT_1_BIT;
	m_uRefCount = 0;
	m_uMipmaps	= 1;
}

ColorBuffer::~ColorBuffer()
{

}


void ColorBuffer::Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, ColorFormat eFormat, SampleCount eSamples, uint32 uFlags, uint32 uRTLoc, uint32 uMipCount)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	m_eType = TYPE_2D;
	m_uFlags = uFlags;
	m_uRTLoc = uRTLoc;
	m_uMipmaps = uMipCount;
	m_uSlices = 1;
	m_eSampleCount = eSamples;
	m_eFormat = eFormat;

	m_platform.Init(pDevice, uWidth, uHeight, eFormat, eSamples, uFlags, uRTLoc, uMipCount);
}

void ColorBuffer::InitCube(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uSlices, ColorFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	m_eType = TYPE_CUBE;
	m_uFlags = uFlags;
	m_uRTLoc = 0;	// TODO: We may want multiple cube RTS? Doubt it
	m_uMipmaps = 1;
	m_uSlices = uSlices;
	m_eSampleCount = eSamples;
	m_eFormat = eFormat;

	ASSERT(uSlices <= 6);
	m_platform.InitArray(pDevice, 0, uWidth, uHeight, uSlices, eFormat, eSamples, uFlags);
}

void ColorBuffer::CleanUp(GFXDevice* pDevice)
{
	m_platform.CleanUp(pDevice);
}

void ColorBuffer::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	if (uWidth != m_uWidth || uHeight != m_uHeight)
	{
		m_uWidth = uWidth;
		m_uHeight = uHeight;

		m_platform.Resize(pDevice, uWidth, uHeight);
	}
}

void ColorBuffer::IncRef()
{
	if (m_uRefCount == 0)
	{
		m_platform.SetActive(true);
	}
	m_uRefCount++;
}

void ColorBuffer::DecRef()
{
	ASSERT(m_uRefCount > 0);
	m_uRefCount--;
	if (m_uRefCount == 0)
	{
		m_platform.SetActive(false);
	}
}

}
