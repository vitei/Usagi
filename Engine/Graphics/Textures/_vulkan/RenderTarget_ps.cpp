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
	// Nothing much to do, all lives in the MRT
}

void RenderTarget_ps::CleanUp(GFXDevice* pDevice)
{

}


void RenderTarget_ps::SetClearColor(const Color& col)
{
    
}


void RenderTarget_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	// TODO: Implement
	ASSERT(false);
}


}

