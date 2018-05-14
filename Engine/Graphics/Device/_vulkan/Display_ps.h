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

namespace usg {

class Display_ps
{
public:
	Display_ps();
	~Display_ps();

	void Initialise(usg::GFXDevice* pDevice, WindHndl hndl);
	void CleanUp(usg::GFXDevice* pDevice);
	void Use(usg::GFXDevice* pDevice);

	void Present();
	bool GetActualDimensions(uint32 &xOut, uint32 &yOut, bool bOrient);
	bool GetDisplayDimensions(uint32 &xOut, uint32 &yOut, bool bOrient);
	void ScreenShot(const char* szFileName);

	void Resize(usg::GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
    void Resize(usg::GFXDevice* pDevice);	
	
	// PS
	void Transfer(GFXContext* pContext, RenderTarget* pTarget);
	void TransferRect(GFXContext* pContext, RenderTarget* pTarget, const GFXBounds& srcBounds, const GFXBounds& dstBounds);
	void SwapBuffers(GFXDevice* pDevice);

private:
	PRIVATIZE_COPY(Display_ps)

	HWND			m_hwnd;
	HDC				m_hdc;
	VkImage*		m_pSwapchainImages;
	VkImageView*	m_pSwapchainImageViews;
	VkSurfaceKHR	m_surface;
	VkSwapchainKHR	m_swapChain;
	VkSemaphore		m_presentComplete;
	uint32			m_uSwapChainImageCount;
	uint32			m_uID;
	uint32			m_uWidth;
	uint32			m_uHeight;
	uint32			m_uActiveImage;
};

}

#endif