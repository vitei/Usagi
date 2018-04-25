/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "RenderTarget.h"

namespace usg {

	RenderTarget::RenderTarget()
	{
		m_clearDepth = 1.0f;
		m_clearStencil = 0;
		m_pDepth = NULL;
		m_uTargetCount = 0;
		MemClear(m_pColorBuffer, sizeof(ColorBuffer*)*MAX_RENDER_TARGETS);
	}

	RenderTarget::~RenderTarget()
	{
	}


	void RenderTarget::Init(usg::GFXDevice* pDevice, ColorBuffer* pColorBuffer, DepthStencilBuffer* pDepth)
	{
		uint32 uColCount = pColorBuffer != NULL ? 1 : 0;
		InitMRT(pDevice, uColCount, &pColorBuffer, pDepth);
	}

	void RenderTarget::InitMRT(usg::GFXDevice* pDevice, uint32 uColCount, ColorBuffer** ppColorBuffer, DepthStencilBuffer* pDepth)
	{
		m_uTargetMask = 0;
		m_uTargetCount = uColCount;
		ASSERT(uColCount <= MAX_RENDER_TARGETS);
		for (uint32 i = 0; i < uColCount; i++)
		{
			m_pColorBuffer[i] = ppColorBuffer[i];
			m_uTargetMask |= (1 << ppColorBuffer[i]->GetRTLoc());
		}
		m_pDepth = pDepth;

		ASSERT(ConfirmCompataible(ppColorBuffer[0], pDepth));

		m_platform.InitMRT(pDevice, uColCount, ppColorBuffer, pDepth);
	}

	void RenderTarget::CleanUp(GFXDevice* pDevice)
	{
		m_platform.CleanUp(pDevice);
	}

	void RenderTarget::Resize(usg::GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
	{
		m_platform.Resize(pDevice, uWidth, uHeight);
	}


	bool RenderTarget::ConfirmCompataible(ColorBuffer* pColorBuffer, DepthStencilBuffer* pDepth)
	{
		if (!pDepth || !pColorBuffer)
			return true;

		if (pColorBuffer->GetSampleCount() != pDepth->GetSampleCount())
			return false;

		if (pColorBuffer->GetWidth() != pDepth->GetWidth())
			return false;

		if (pColorBuffer->GetHeight() != pDepth->GetHeight())
			return false;

		return true;
	}

	TextureHndl RenderTarget::GetDepthTexture() const
	{
		return m_pDepth ? m_pDepth->GetTexture() : TextureHndl(NULL);
	}

}