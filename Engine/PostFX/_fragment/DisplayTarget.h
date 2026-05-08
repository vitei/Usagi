/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A utility render target to replace direct screen rendering
//	on HDR monitors for simple scenes like a splash or FMV
*****************************************************************************/
#ifndef USG_POSTFX_DISPLAY_TARGET
#define USG_POSTFX_DISPLAY_TARGET
#include "Engine/PostFX/_fragment/BlitImage.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include "Engine/Graphics/Textures/ColorBuffer.h"




namespace usg {

class DisplayTarget
{
public:
	DisplayTarget();
	~DisplayTarget();

	void InitForDisplay(GFXDevice* pDevice, usg::ResourceMgr* pResMgr, uint32 uDisplay = 0);
	void Cleanup(GFXDevice* pDevice);
	void StartDraw(GFXContext* pContext);
	void EndDraw(GFXContext* pContext);
	usg::RenderPassHndl GetRenderPass() const;

	void NotifyResize(usg::GFXDevice* pDevice, uint32 uDisplay, uint32 uWidth, uint32 uHeight);
private:
	usg::BlitImage		m_blitImage;
	usg::RenderTarget	m_renderTarget;
	usg::ColorBuffer	m_colorBuffer;
	usg::Display*		m_pDisplay = nullptr;

	uint32				m_uDisplayId = 0;
};

}

#endif
