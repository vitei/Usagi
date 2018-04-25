/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "Engine/Graphics/RenderConsts.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)

namespace usg {


#define SCALE_PER 100
#define Scale(s) ((s*SCALE_PER)/100)


void Display_ps::Initialise(usg::GFXDevice* pDevice, WindHndl hndl)
{
	m_hwnd = hndl;

	m_uID		= 0;
   
    bool bFullscreen = false;
  	m_hdc = ::GetDC(hndl);

	RECT dim;
	GetClientRect(hndl,&dim);

	m_uWidth = dim.right - dim.left;
	m_uHeight = dim.bottom - dim.top;
	m_bDirty = false;
}


void Display_ps::Transfer(RenderTarget* pTarget)
{
	const Viewport& srcViewport = pTarget->GetViewport();
	GFXBounds src = { 0, 0, srcViewport.GetWidth(), srcViewport.GetHeight() };
	GFXBounds dst = { 0, 0, (int)m_uWidth, (int)m_uHeight };

	TransferRect(pTarget, src, dst);
}

void Display_ps::TransferRect(RenderTarget* pTarget, const GFXBounds& src, const GFXBounds& dst)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, pTarget->GetPlatform().GetOGLFBO());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(src.x, src.y, src.x + src.width, src.y + src.height,
		dst.x, dst.y, dst.x + dst.width, dst.y + dst.height,
		GL_COLOR_BUFFER_BIT,
		dst.width == src.width ? GL_NEAREST : GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	m_bDirty = true;
}


void Display_ps::Present()
{
    if(m_uID==1)
    {
//        m_sContext.PerformBufferSwap(&g_window);
      // m_sContext.Deactivate();
    }
}

void Display_ps::SwapBuffers(GFXDevice* pDevice)
{
	if (!m_bDirty)
		return;

	static bool bValid = false;
	if (bValid)
	{
		pDevice->GetPlatform().GetOGLContext().PerformBufferSwap(m_hdc);
	}
	else
	{
		bValid = true;
	}

	m_bDirty = false;
}


void Display_ps::ScreenShot(const char* szFileName)
{
	BYTE* pixels = (BYTE*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_DEBUG, 3 * m_uWidth * m_uHeight);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, m_uWidth, m_uHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	TGAFile fileOut;
	fileOut.SetData(pixels, CF_RGB_888, m_uWidth, m_uHeight);
	fileOut.Save(szFileName);

	mem::Free(MEMTYPE_STANDARD, pixels);
}


void Display_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;
}

void Display_ps::Resize(GFXDevice* pDevice)
{
    RECT dim;
    GetClientRect(m_hwnd, &dim);

    m_uWidth = dim.right - dim.left;
    m_uHeight = dim.bottom - dim.top;
}

}

