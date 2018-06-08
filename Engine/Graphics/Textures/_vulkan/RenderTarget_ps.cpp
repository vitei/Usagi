/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Textures/ColorBuffer.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)
#include API_HEADER(Engine/Graphics/Textures, RenderTarget_ps.h)

namespace usg {


RenderTarget_ps::RenderTarget_ps()
{
	
}

RenderTarget_ps::~RenderTarget_ps()
{

}


void RenderTarget_ps::InitMRT(GFXDevice* pDevice, uint32 uColorCount, ColorBuffer** ppColorBuffers, DepthStencilBuffer* pDepth)
{
	MemClear(&m_colorClearValues, sizeof(m_colorClearValues));

	m_dsClearValue.depthStencil.depth = 1.0f;
	m_dsClearValue.depthStencil.stencil = 0;

}

void RenderTarget_ps::CleanUp(GFXDevice* pDevice)
{

}


void RenderTarget_ps::SetClearColor(const Color& col, uint32 uTarget)
{
	m_colorClearValues[uTarget].color.float32[0] = col.r();
	m_colorClearValues[uTarget].color.float32[1] = col.g();
	m_colorClearValues[uTarget].color.float32[2] = col.b();
	m_colorClearValues[uTarget].color.float32[3] = col.a();
}


void RenderTarget_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	// TODO: Implement
	ASSERT(false);
}


}

