#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "DisplayTarget.h"



usg::DisplayTarget::DisplayTarget()
{

}

usg::DisplayTarget::~DisplayTarget()
{

}

void usg::DisplayTarget::InitForDisplay(GFXDevice* pDevice, usg::ResourceMgr* pResMgr, uint32 uDisplay /*= 0*/)
{
	m_uDisplayId = uDisplay;
	usg::Display* pDisplay = pDevice->GetDisplay(uDisplay);
	if(pDisplay)
	{ 
		uint32 uWidthTV, uHeightTV;
		pDisplay->GetDisplayDimensions(uWidthTV, uHeightTV, false);

		m_colorBuffer.Init(pDevice, uWidthTV, uHeightTV, ColorFormat::RGB_HDR); 
		m_renderTarget.Init(pDevice, &m_colorBuffer, nullptr, "DisplayTarget");

		usg::RenderTarget::RenderPassFlags flags;
		flags.uClearFlags = usg::RenderTarget::RT_FLAG_COLOR_0;
		flags.uShaderReadFlags = RenderTarget::RT_FLAG_COLOR_0;
		flags.uStoreFlags = RenderTarget::RT_FLAG_COLOR_0;


		m_renderTarget.InitRenderPass(pDevice, flags);

		m_blitImage.InitForDisplay(pDevice, pResMgr, uDisplay);
		m_blitImage.SetSourceTexture(pDevice, m_colorBuffer.GetTexture());

		m_pDisplay = pDisplay;
	}
}

void usg::DisplayTarget::Cleanup(GFXDevice* pDevice)
{
	m_renderTarget.Cleanup(pDevice);
	m_colorBuffer.Cleanup(pDevice);
	m_blitImage.Cleanup(pDevice);
}


void usg::DisplayTarget::StartDraw(GFXContext* pContext)
{
	pContext->SetRenderTarget(&m_renderTarget);
}


void usg::DisplayTarget::EndDraw(GFXContext* pContext)
{
	if(!m_pDisplay)
		return;
	pContext->RenderToDisplay(m_pDisplay, RenderTarget::RT_FLAG_COLOR);
	pContext->ApplyViewport(m_renderTarget.GetViewport());
	m_blitImage.Draw(pContext);
}

usg::RenderPassHndl usg::DisplayTarget::GetRenderPass() const
{
	return m_renderTarget.GetRenderPass();
}

void usg::DisplayTarget::NotifyResize(usg::GFXDevice* pDevice, uint32 uDisplay, uint32 uWidth, uint32 uHeight)
{
	m_colorBuffer.Resize(pDevice, uWidth, uHeight);
	m_renderTarget.Resize(pDevice);
}

