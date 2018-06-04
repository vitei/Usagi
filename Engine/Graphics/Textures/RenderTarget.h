/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_RENDERTARGET_H
#define _USG_GRAPHICS_RENDERTARGET_H
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Textures/ColorBuffer.h"
#include API_HEADER(Engine/Graphics/Textures, RenderTarget_ps.h)

namespace usg {

class RenderTarget
{
public:
	RenderTarget();
	~RenderTarget();

	enum RTFlags
	{
		RT_FLAG_COLOR_0	= (1<<0),
		RT_FLAG_COLOR_1	= (1<<1),
		RT_FLAG_COLOR_2	= (1<<2),
		RT_FLAG_COLOR_3	= (1<<3),
		RT_FLAG_COLOR_4	= (1<<4),
		RT_FLAG_COLOR_5	= (1<<5),
		RT_FLAG_COLOR_6	= (1<<6),
		RT_FLAG_COLOR_7	= (1<<7),
		RT_FLAG_COLOR	= (RT_FLAG_COLOR_0|RT_FLAG_COLOR_1|RT_FLAG_COLOR_2|RT_FLAG_COLOR_4),
		RT_FLAG_DEPTH	= (1<<9),
		RT_FLAG_STENCIL	= (1<<10),
		RT_FLAG_DS		= (RT_FLAG_DEPTH|RT_FLAG_STENCIL)
	};

	// Fixme, why are we passing in the dimensions again?
	void Init(usg::GFXDevice* pDevice, ColorBuffer* pColorBuffer, DepthStencilBuffer* pDepth = NULL);
	void InitMRT(usg::GFXDevice* pDevice, uint32 uColCount, ColorBuffer** ppColorBuffer, DepthStencilBuffer* pDepth = NULL);
	void CleanUp(GFXDevice* pDevice);
	void Resize(usg::GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	bool IsValid() const { return m_pDepth || m_uTargetCount; }

	uint32 GetWidth() const { return m_platform.GetWidth(); }
	uint32 GetHeight() const { return m_platform.GetHeight(); }
	const TextureHndl& GetColorTexture(uint32 uTex = 0) const { return m_pColorBuffer[uTex]->GetTexture(); }
	TextureHndl GetDepthTexture() const;
	void SetClearColor(const Color &col, uint32 uTarget = 0) { m_clearColor[uTarget] = col; m_platform.SetClearColor(col, uTarget); }
	const Color& GetClearColor(uint32 uTarget=0) { return m_clearColor[uTarget]; }
	bool SaveToFile(const char* szFileName) { return m_platform.SaveToFile(szFileName);  }

	RenderTarget_ps& GetPlatform() { return m_platform; }
	const RenderTarget_ps& GetPlatform() const { return m_platform; }
	const Viewport& GetViewport() const { return m_platform.GetViewport(); }

	const ColorBuffer* 			GetColorBuffer(uint32 uTarget = 0) const { return m_pColorBuffer[uTarget]; }
	ColorBuffer* 				GetColorBuffer(uint32 uTarget = 0) { return m_pColorBuffer[uTarget]; }
	const DepthStencilBuffer*	GetDepthStencilBuffer() const { return m_pDepth; }
	DepthStencilBuffer*			GetDepthStencilBuffer() { return m_pDepth; }

	bool ConfirmCompataible(ColorBuffer* pColorBuffer, DepthStencilBuffer* pDepth);
	uint32 GetTargetCount() const { return m_uTargetCount; }
	uint32 GetTargetMask() const { return m_uTargetMask; }

	// Helper function for creating the standard single pass render passes
	RenderPassHndl CreateRenderPass(GFXDevice* pDevice, uint32 uLoadFlags, uint32 uClearFlags, uint32 uStoreFlags, 
		const RenderPassDecl::Dependency* dependencies = RenderPassDecl::ExternalColorDependenciesInAndOut(), uint32 uDependiences = 2);

private:
	RenderTarget_ps	m_platform;

	ColorBuffer*			m_pColorBuffer[MAX_COLOR_TARGETS];
	DepthStencilBuffer*		m_pDepth;

	Color					m_clearColor[MAX_COLOR_TARGETS];
	float					m_clearDepth;
	uint8					m_clearStencil;
	uint32					m_uTargetCount;
	uint32					m_uTargetMask;
};


}

#endif
