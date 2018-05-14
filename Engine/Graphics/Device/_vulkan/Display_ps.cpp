/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Core/_win/WinUtil.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "Engine/Core/stl/vector.h"

extern bool	 g_bFullScreen;
extern uint32 g_uWindowWidth;
extern uint32 g_uWindowHeight;

namespace usg {


#define SCALE_PER 100
#define Scale(s) ((s*SCALE_PER)/100)


Display_ps::Display_ps()
{
	m_swapChain = VK_NULL_HANDLE;
	m_uActiveImage = 0;
	m_uSwapChainImageCount = 0;
}

void Display_ps::CleanUp(usg::GFXDevice* pDevice)
{
	if (m_swapChain != VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < m_uSwapChainImageCount; i++)
		{
			// FIXME: Move to cleanup function
			vkDestroyImageView(pDevice->GetPlatform().GetVKDevice(), m_pSwapchainImageViews[i], pDevice->GetPlatform().GetAllocCallbacks());
		}
		vdelete[] m_pSwapchainImages;
		vdelete[] m_pSwapchainImageViews;
	}
	m_uSwapChainImageCount = 0;
}

Display_ps::~Display_ps()
{
	// Ensure we've already performed the cleanup
	ASSERT(m_uSwapChainImageCount == 0);
}

void Display_ps::Initialise(usg::GFXDevice* pDevice, WindHndl hndl)
{
	GFXDevice_ps& devicePS = pDevice->GetPlatform();
	m_hwnd = hndl;

	m_uID = 0;

	bool bFullscreen = false;
	m_hdc = ::GetDC(hndl);

	RECT dim;
	GetClientRect(hndl, &dim);

	m_uWidth = dim.right - dim.left;
	m_uHeight = dim.bottom - dim.top;


	VkResult res;

	// BEGIN PC SPECIFIC CODE
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = WINUTIL::GetInstanceHndl();
	createInfo.hwnd = m_hwnd;
	res = vkCreateWin32SurfaceKHR(devicePS.GetVKInstance(), &createInfo, devicePS.GetAllocCallbacks(), &m_surface);
	// END PC SPECIFIC CODE

	ASSERT(res == VK_SUCCESS);

	// Iterate over each queue to learn whether it supports presenting:
	vector<VkBool32> supportsPresent;
	supportsPresent.resize(devicePS.GetQueueFamilyCount());
	for (uint32_t i = 0; i < devicePS.GetQueueFamilyCount(); i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(devicePS.GetGPU(0), i, m_surface,	&supportsPresent[i]);
	}

	// Search for a graphics queue and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < devicePS.GetQueueFamilyCount(); i++)
	{
		if ((devicePS.GetQueueProperties(i)->queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				graphicsQueueNodeIndex = i;
				break;
			}
		}
	}

	// Generate error if could not find a queue that supports both a graphics
	// and present
	if (graphicsQueueNodeIndex == UINT32_MAX)
	{
		ASSERT_MSG(false, "Could not find a queue that supports both graphics and present\n");
		return;
	}


	// Get the list of VkFormats that are supported:
	uint32_t formatCount;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(devicePS.GetGPU(0), m_surface, &formatCount, NULL);
	ASSERT(res == VK_SUCCESS);
	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(devicePS.GetGPU(0), m_surface, &formatCount, surfFormats);
	ASSERT(res == VK_SUCCESS);
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	VkFormat eFormat;
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		eFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		ASSERT(formatCount >= 1);
		eFormat = surfFormats[0].format;
	}

	VkSurfaceCapabilitiesKHR surfCapabilities;

	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devicePS.GetGPU(0), m_surface, &surfCapabilities);
	ASSERT(res == VK_SUCCESS);

	uint32_t presentModeCount;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(devicePS.GetGPU(0), m_surface,	&presentModeCount, NULL);
	ASSERT(res == VK_SUCCESS);
	vector<VkPresentModeKHR> presentModes;
	presentModes.resize(presentModeCount);

	res = vkGetPhysicalDeviceSurfacePresentModesKHR(devicePS.GetGPU(0), m_surface, &presentModeCount, presentModes.data());
	ASSERT(res == VK_SUCCESS);

	VkExtent2D swapChainExtent;
	// width and height are either both -1, or both not -1.
	if (surfCapabilities.currentExtent.width == (uint32_t)-1)
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapChainExtent.width = m_uWidth;
		swapChainExtent.height = m_uHeight;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapChainExtent = surfCapabilities.currentExtent;
	}

	// If mailbox mode is available, use it, as is the lowest-latency non-
	// tearing mode.  If not, try IMMEDIATE which will usually be available,
	// and is fastest (though it tears).  If not, fall back to FIFO which is
	// always available.
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < presentModeCount; i++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount + 1;
	if ((surfCapabilities.maxImageCount > 0) &&	(desiredNumberOfSwapChainImages > surfCapabilities.maxImageCount))
	{
		// Application must settle for fewer images than desired:
		desiredNumberOfSwapChainImages = surfCapabilities.maxImageCount;
	}

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfCapabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swap_chain = {};
	swap_chain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain.pNext = NULL;
	swap_chain.surface = m_surface;
	swap_chain.minImageCount = desiredNumberOfSwapChainImages;
	swap_chain.imageFormat = eFormat;
	swap_chain.imageExtent.width = swapChainExtent.width;
	swap_chain.imageExtent.height = swapChainExtent.height;
	swap_chain.preTransform = preTransform;
	swap_chain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swap_chain.imageArrayLayers = 1;
	swap_chain.presentMode = swapchainPresentMode;
	swap_chain.oldSwapchain = NULL;
	swap_chain.clipped = true;
	swap_chain.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swap_chain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swap_chain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swap_chain.queueFamilyIndexCount = 0;
	swap_chain.pQueueFamilyIndices = NULL;

	res = vkCreateSwapchainKHR(devicePS.GetVKDevice(), &swap_chain, devicePS.GetAllocCallbacks(), &m_swapChain);
	ASSERT(res == VK_SUCCESS);

	// Get the 
	res = vkGetSwapchainImagesKHR(devicePS.GetVKDevice(), m_swapChain, &m_uSwapChainImageCount, NULL);

	m_pSwapchainImages = vnew(ALLOC_GFX_RENDER_TARGET) VkImage[m_uSwapChainImageCount];
	m_pSwapchainImageViews = vnew(ALLOC_GFX_RENDER_TARGET) VkImageView[m_uSwapChainImageCount];
	ASSERT(m_pSwapchainImages!=NULL);
	res = vkGetSwapchainImagesKHR(devicePS.GetVKDevice(), m_swapChain, &m_uSwapChainImageCount, m_pSwapchainImages);
	ASSERT(res == VK_SUCCESS);


	for (uint32_t i = 0; i < m_uSwapChainImageCount; i++)
	{
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = NULL;
		color_image_view.format = eFormat;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.flags = 0;

		color_image_view.image = m_pSwapchainImages[i];

		res = vkCreateImageView(devicePS.GetVKDevice(), &color_image_view, devicePS.GetAllocCallbacks(), &m_pSwapchainImageViews[i]);
		ASSERT(res == VK_SUCCESS);
	}

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;
	semaphoreCreateInfo.flags = 0;

	res = vkCreateSemaphore(devicePS.GetVKDevice(), &semaphoreCreateInfo, nullptr, &m_presentComplete);
	ASSERT(res == VK_SUCCESS);
}

void Display_ps::Use(usg::GFXDevice* pDevice)
{
	vkAcquireNextImageKHR(pDevice->GetPlatform().GetVKDevice(), m_swapChain, UINT64_MAX, m_presentComplete, (VkFence)nullptr, &m_uActiveImage);
}


void Display_ps::Transfer(GFXContext* pContext, RenderTarget* pTarget)
{
	TransferRect(pContext, pTarget, 0, 0, m_uWidth, m_uHeight);
}

void Display_ps::TransferRect(GFXContext* pContext, RenderTarget* pTarget, uint32 uX, uint32 uY, uint32 uWidth, uint32 uHeight)
{
	VkImageCopy ic = {};
	ic.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ic.srcSubresource.mipLevel = 0;
	ic.srcSubresource.baseArrayLayer = 0;
	ic.srcSubresource.layerCount = 1;
	ic.srcOffset.x = 0;
	ic.srcOffset.y = 0;
	ic.srcOffset.z = 0;
	ic.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ic.dstSubresource.mipLevel = 0;
	ic.dstSubresource.baseArrayLayer = 0;
	ic.dstSubresource.layerCount = 1;
	ic.dstOffset.x = 0;
	ic.dstOffset.y = 0;
	ic.dstOffset.z = 0;
	ic.extent.width = m_uWidth;
	ic.extent.height = m_uHeight;
	ic.extent.depth = 1;

	const Texture_ps& tex = pTarget->GetColorTexture(0)->GetPlatform();
	VkImage srcImage = tex.GetImage();
	vkCmdCopyImage(pContext->GetPlatform().GetVkCmdBuffer(), srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_pSwapchainImages[m_uActiveImage], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ic);

	pContext->GetPlatform().ImageBarrier(m_pSwapchainImages[m_uActiveImage], VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	pContext->GetPlatform().ImageBarrier(srcImage, VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
}



bool Display_ps::GetActualDimensions(uint32 & xOut, uint32 & yOut, bool bOrient)
{
    xOut = m_uWidth;
    yOut = m_uHeight;
    
	return true;
}

bool Display_ps::GetDisplayDimensions(uint32 & xOut, uint32 & yOut, bool bOrient)
{
    xOut = m_uWidth;
    yOut = m_uHeight;
	return true;
}


void Display_ps::Present()
{

}

void Display_ps::SwapBuffers(GFXDevice* pDevice)
{

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapChain;
	presentInfo.pImageIndices = &m_uActiveImage;
	// Check if a wait semaphore has been specified to wait for before presenting the image
	if (m_presentComplete != VK_NULL_HANDLE)
	{
		presentInfo.pWaitSemaphores = &m_presentComplete;
		presentInfo.waitSemaphoreCount = 1;
	}
	VkResult res = vkQueuePresentKHR(pDevice->GetPlatform().GetQueue(), &presentInfo);
	ASSERT(res == VK_SUCCESS);
}

void Display_ps::ScreenShot(const char* szFileName)
{

}

}

