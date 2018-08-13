/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_RENDERTARGET_H
#define _USG_GRAPHICS_PC_RENDERTARGET_H
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Device/RenderState.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class Viewport;
class DepthStencilBuffer;
class ColorBuffer;

class RenderTarget_ps
{
public:
	RenderTarget_ps();
	~RenderTarget_ps();

	void InitMRT(usg::GFXDevice* pDevice, uint32 uCount, ColorBuffer** ppColorBuffers, DepthStencilBuffer* pDepth);
	void CleanUp(GFXDevice* pDevice);
	void Resize(GFXDevice* pDevice, uint32 uCount, ColorBuffer** ppColorBuffers, DepthStencilBuffer* pDepth);
	void RenderPassUpdated(usg::GFXDevice* pDevice, const RenderPassHndl &renderPass) {}
	GLuint GetTexture(uint32 uCount = 0) const;
	void SetClearColor(const Color& col, uint32 uTarget);

	// For split screen rendering etc, fast copy to final rendertarget
	void BlitMSToScreen(const Viewport* pViewport, int fOffsetX=0, int fOffsetY=0);
	bool SaveToFile(const char* szFileName);

	const TextureHndl& GetColorTexture(uint32 uTex=0) const;
	const TextureHndl& GetDepthTexture() const;
	uint32 GetWidth() const;
	uint32 GetHeight() const;

	const DepthStencilBuffer* GetDepthTarget() { return m_pDepthTarget; }

	GLuint GetOGLFBO() const { return m_FBO; }
	GLuint GetLayerFBO(uint32 uLayer) const;
	uint32 GetTargetCount() const { return m_uTargets; }
	const GLenum* GetBindings() const { return m_bindings; }
	const Viewport& GetViewport() const { return m_fullScreenVP; }

	void EndDraw() const;

private:
	void Reset();
	DepthStencilBuffer*		m_pDepthTarget;

	enum
	{
		MAX_LAYERS = 6
	};

	Viewport	m_fullScreenVP;
	uint32		m_uTargets;
	GLuint		m_FBO;
	GLuint		m_layerFBO[MAX_LAYERS];
	uint32		m_uSlices;

	ColorBuffer*	m_pColorBuffers[MAX_COLOR_TARGETS];

	// Multisampled render targets require a copy before they can be used as input
	// textures (currently not supported)
	GLenum	m_bindings[MAX_COLOR_TARGETS];

	static  GLuint s_activeFBO;
};

inline GLuint RenderTarget_ps::GetLayerFBO(uint32 uLayer) const
{
	if(m_uSlices > 1)
	{
		ASSERT(uLayer < m_uSlices);
		return m_layerFBO[uLayer];
	}
	else
	{
		return m_FBO;
	}
}


}

#endif