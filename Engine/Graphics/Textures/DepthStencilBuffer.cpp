/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "DepthStencilBuffer.h"

namespace usg
{

	DepthStencilBuffer::DepthStencilBuffer()
	{
		m_uRefCount = 0;
		m_uSlices = 1;
	}

	DepthStencilBuffer::~DepthStencilBuffer()
	{

	}

	void DepthStencilBuffer::Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
	{
		m_uFlags = uFlags;
		m_eSamples = eSamples;
		m_eFormat = eFormat;
		m_platform.Init(pDevice, uWidth, uHeight, eFormat, eSamples, uFlags);
	}

	void DepthStencilBuffer::IncRef()
	{
		if (m_uRefCount == 0)
		{
			m_platform.SetActive(true);
		}
		m_uRefCount++;
	}

	void DepthStencilBuffer::DecRef()
	{
		ASSERT(m_uRefCount > 0);
		m_uRefCount--;
		if (m_uRefCount == 0)
		{
			m_platform.SetActive(false);
		}
	}


	// TODO: When working support on the WiiU via a copy / resolve command
	void DepthStencilBuffer::InitArray(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uSlices, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
	{
		m_uSlices = uSlices;
		m_uFlags = uFlags;
		m_eSamples = eSamples;
		m_eFormat = eFormat;
		m_platform.InitArray(pDevice, uWidth, uHeight, uSlices, eFormat, eSamples, uFlags);
	}

	void DepthStencilBuffer::InitCube(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
	{
		m_uSlices = 6;
		m_uFlags = uFlags;
		m_eSamples = eSamples;
		m_eFormat = eFormat;
		m_platform.InitCube(pDevice, uWidth, uHeight, eFormat, eSamples, uFlags);
	}

	void DepthStencilBuffer::CleanUp(GFXDevice* pDevice)
	{
		m_platform.CleanUp(pDevice);
	}


	void DepthStencilBuffer::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
	{
		m_platform.Resize(pDevice, uWidth, uHeight);
	}

}
