/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_RENDERTARGET_H
#define _USG_GRAPHICS_PC_RENDERTARGET_H
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Core/stl/vector.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class Viewport;
class DepthStencilBuffer;
class ColorBuffer;

class RenderTarget_ps
{
public:
	RenderTarget_ps();
	~RenderTarget_ps();

	void InitMRT(GFXDevice* pDevice, uint32 uCount, ColorBuffer** ppColorBuffers, DepthStencilBuffer* pDepth);
	void RenderPassUpdated(usg::GFXDevice* pDevice, const RenderPassHndl &renderPass);

	void CleanUp(GFXDevice* pDevice);
	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	void SetClearColor(const Color& col, uint32 uTarget);

	uint32 GetWidth() const { return m_uWidth; }
	uint32 GetHeight() const { return m_uHeight; }
	bool SaveToFile(const char* szFileName) { }

	const Viewport& GetViewport() const { return m_fullScreenVP; }
	void SetInUse(bool bInUse) {}

	void EndDraw() { }
	
	// Platform specific
	const VkClearValue* GetClearValues() const { return m_colorClearValues; }
	const VkFramebuffer& GetFrameBuffer() const { return m_framebuffer; }

private:
	// Color + depth for the clear
	VkFramebufferCreateInfo m_fbCreateInfo;
	vector<VkImageView>	m_imageViews;
	VkFramebuffer	m_framebuffer;
	VkClearValue	m_colorClearValues[MAX_COLOR_TARGETS+1];
	// For creating a render pass declaration used with sub passes
	RenderPassHndl	m_renderPass;
	Viewport		m_fullScreenVP;
	uint32			m_uCurrentImage;
	uint32			m_uWidth;
	uint32			m_uHeight;
};


}

#endif