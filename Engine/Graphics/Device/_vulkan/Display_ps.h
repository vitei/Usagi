#/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Represents a final destination screen, or window (not a viewport)
*****************************************************************************/
#ifndef _USG_GRAPHICS_DISPLAY_PC_H_
#define _USG_GRAPHICS_DISPLAY_PC_H_

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
	void SetAsTarget(VkCommandBuffer& cmd);

	void Present();
	bool GetActualDimensions(uint32 &xOut, uint32 &yOut, bool bOrient);
	bool GetDisplayDimensions(uint32 &xOut, uint32 &yOut, bool bOrient);
	void ScreenShot(const char* szFileName);
	const RenderPassHndl& GetRenderPass() { return m_directRenderPass; }

	void Resize(usg::GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
    void Resize(usg::GFXDevice* pDevice);	
	void Minimized(usg::GFXDevice* pDevice);
	
	// PS
	void Transfer(GFXContext* pContext, RenderTarget* pTarget);
	void TransferRect(GFXContext* pContext, RenderTarget* pTarget, const GFXBounds& srcBounds, const GFXBounds& dstBounds);
	void SwapBuffers(GFXDevice* pDevice);
	VkSemaphore& GetImageAcquired() { return m_imageAcquired; }
	VkImage GetActiveImage() const { return m_pSwapchainImages[m_uActiveImage]; }

private:
	PRIVATIZE_COPY(Display_ps)

	void InitFrameBuffers(GFXDevice* pDevice);
	void DestroySwapChain(GFXDevice* pDevice);
	void RecreateSwapChain(GFXDevice* pDevice);
	void CreateSwapChain(GFXDevice* pDevice);
	void CreateSwapChainImageViews(GFXDevice* pDevice);

	enum 
	{
		SWAP_BUFFER_COUNT = 2
	};

	usg::RenderPassHndl	m_directRenderPass;
	usg::RenderPassHndl	m_postCopyRenderPass;
	VkFramebuffer*		m_pFramebuffers;
	VkFramebuffer*		m_pFramebuffersNoCopy;
	HWND				m_hwnd;
	HDC					m_hdc;
	VkImage*			m_pSwapchainImages;
	VkImageView*		m_pSwapchainImageViews;
	VkSurfaceKHR		m_surface;
	VkSwapchainKHR		m_swapChain;
	VkSemaphore			m_imageAcquired;
	VkFormat			m_swapChainImageFormat;
	uint32				m_uSwapChainImageCount;
	uint32				m_uID;
	uint32				m_uWidth;
	uint32				m_uHeight;
	uint32				m_uActiveImage;
	bool				m_bWindowResized;
	bool				m_bRTShouldLoad;
};

inline bool Display_ps::GetActualDimensions(uint32 & xOut, uint32 & yOut, bool bOrient)
{
	xOut = m_uWidth;
	yOut = m_uHeight;

	return true;
}

inline bool Display_ps::GetDisplayDimensions(uint32 & xOut, uint32 & yOut, bool bOrient)
{
	xOut = m_uWidth;
	yOut = m_uHeight;
	return true;
}

}

#endif