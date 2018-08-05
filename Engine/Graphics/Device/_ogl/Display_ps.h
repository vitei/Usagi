#/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Represents a final destination screen, or window (not a viewport)
*****************************************************************************/
#ifndef _USG_GRAPHICS_DISPLAY_PC_H_
#define _USG_GRAPHICS_DISPLAY_PC_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLContext.h)

namespace usg {

struct DisplaySettings;
struct GFXBounds;
class IHeadMountedDisplay;

class Display_ps
{
public:
	Display_ps() {}
	~Display_ps() {}

	void Initialise(usg::GFXDevice* pDevice, WindHndl hndl);
	void CleanUp(usg::GFXDevice* pDevice) {}
	void Transfer(RenderTarget* pTarget);
	void TransferRect(RenderTarget* pTarget, const GFXBounds& srcBounds, const GFXBounds& dstBounds);
	void Present();
	RenderPassHndl& GetRenderPass() { return m_directRenderPass; }
	bool GetActualDimensions(uint32 &xOut, uint32 &yOut, bool bOrient) { xOut = m_uWidth; yOut = m_uHeight; return true; }
	bool GetDisplayDimensions(uint32 &xOut, uint32 &yOut, bool bOrient) { xOut = m_uWidth; yOut = m_uHeight; return true; }
	void ScreenShot(const char* szFileName);
	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
    void Resize(GFXDevice* pDevice);
    void Minimized(usg::GFXDevice* pDevice) {}
	void SetDirty() { m_bDirty = true; }
	
	//RenderTarget &GetRenderTarget() { return m_renderTarget; }

	void SwapBuffers(GFXDevice* pDevice);

	WindHndl GetWindowHndl() { return m_hwnd;  }

private:
	PRIVATIZE_COPY(Display_ps)

	static OpenGLContext	m_sContext;

	usg::RenderPassHndl	m_directRenderPass;
	HDC				m_hdc;
	WindHndl		m_hwnd;
	uint32			m_uID;
	uint32			m_uWidth;
	uint32			m_uHeight;
	bool			m_bDirty;
};

}

#endif