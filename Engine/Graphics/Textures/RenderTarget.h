/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_RENDERTARGET_H
#define _USG_GRAPHICS_RENDERTARGET_H

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

	struct RenderPassFlags
	{
		RenderPassFlags();
		void Clear();
		uint32 uLoadFlags;			// Targets to load memory from
		uint32 uClearFlags;			// Targets to clear
		uint32 uStoreFlags;			// Targets to store for later use (either as RT or reading)
		uint32 uShaderReadFlags;	// Targets that will next be read from by a shader
		uint32 uTransferSrcFlags;	// Targets to be used for transfers at a later stage
	};

	void Init(usg::GFXDevice* pDevice, ColorBuffer* pColorBuffer, DepthStencilBuffer* pDepth, const char* szName);
	void InitMRT(usg::GFXDevice* pDevice, uint32 uColCount, ColorBuffer** ppColorBuffer, DepthStencilBuffer* pDepth, const char* szName);

	// Create the rneder pass
	RenderPassHndl InitRenderPass(GFXDevice* pDevice, const RenderPassFlags& flags, const RenderPassDecl::Dependency* dependencies = RenderPassDecl::ExternalColorDependenciesInAndOut(), uint32 uDependiences = 2);

	void Cleanup(GFXDevice* pDevice);
	void Resize(usg::GFXDevice* pDevice);
	bool IsValid() const { return m_pDepth || m_uTargetCount; }

	// FIXME: True mip size data from PS
	uint32 GetWidth(uint32 uMip = 0) const { return m_platform.GetWidth() >> uMip; }
	uint32 GetHeight(uint32 uMip = 0) const { return m_platform.GetHeight() >> uMip; }
	const TextureHndl& GetColorTexture(uint32 uTex = 0) const { return m_pColorBuffer[uTex]->GetTexture(); }
	TextureHndl GetDepthTexture() const;
	void SetClearColor(const Color &col, uint32 uTarget = 0);
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
	const RenderPassHndl& GetRenderPass() const { return m_renderPass; }

private:
	RenderTarget_ps	m_platform;

	RenderPassHndl			m_renderPass;
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
