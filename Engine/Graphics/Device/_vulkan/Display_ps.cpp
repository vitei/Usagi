/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Core/_win/WinUtil.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Device/IHeadMountedDisplay.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Core/Modules/ModuleManager.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Device, RenderPass.h)
#include "Engine/Core/stl/vector.h"

extern bool	 g_bFullScreen;
extern uint32 g_uWindowWidth;
extern uint32 g_uWindowHeight;


static VkPresentModeKHR g_presentMapping[] = 
{
	VK_PRESENT_MODE_IMMEDIATE_KHR,		// VSYNC_MODE_IMMEDIATE
	VK_PRESENT_MODE_FIFO_KHR,			// VSYNC_MODE_QUEUE
	VK_PRESENT_MODE_FIFO_RELAXED_KHR,	// VSYNC_MODE_QUEUE_RELAXED
	VK_PRESENT_MODE_MAILBOX_KHR			// VSYNC_MODE_MAILBOX
};

namespace usg {


#define SCALE_PER 100
#define Scale(s) ((s*SCALE_PER)/100)


Display_ps::Display_ps()
{
	m_swapChain = VK_NULL_HANDLE;
	m_uActiveImage = 0;
	m_uSwapChainImageCount = 0;
	m_bWindowResized = false;
	m_bRTShouldLoad = false;
	m_bHDR = false;
	m_eVsync = VSYNC_MODE_MAILBOX;
}


void Display_ps::DestroySwapChain(GFXDevice* pDevice)
{
	if (m_swapChain != VK_NULL_HANDLE)
	{
		for (uint32 i = 0; i < m_uSwapChainImageCount; i++)
		{
			vkDestroyFramebuffer(pDevice->GetPlatform().GetVKDevice(), m_pFramebuffers[i], nullptr);
			vkDestroyFramebuffer(pDevice->GetPlatform().GetVKDevice(), m_pFramebuffersNoCopy[i], nullptr);
			m_pFramebuffers[i] = VK_NULL_HANDLE;
			m_pFramebuffersNoCopy[i] = VK_NULL_HANDLE;
		}

		for (uint32_t i = 0; i < m_uSwapChainImageCount; i++)
		{
			vkDestroyImageView(pDevice->GetPlatform().GetVKDevice(), m_pSwapchainImageViews[i], nullptr);
			m_pSwapchainImageViews[i] = VK_NULL_HANDLE;
		}


		vdelete[] m_pSwapchainImages;
		vdelete[] m_pSwapchainImageViews;
		vdelete[] m_pFramebuffers;
		vdelete[] m_pFramebuffersNoCopy;

		vkDestroySwapchainKHR(pDevice->GetPlatform().GetVKDevice(), m_swapChain, nullptr);
		m_swapChain = VK_NULL_HANDLE;

		m_uSwapChainImageCount = 0;
	}
}

void Display_ps::Cleanup(usg::GFXDevice* pDevice)
{
	if (m_swapChain != VK_NULL_HANDLE)
	{
		DestroySwapChain(pDevice);
	}

	if (m_surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(pDevice->GetPlatform().GetVKInstance(), m_surface, nullptr);
		m_surface = VK_NULL_HANDLE;
	}

	vkDestroySemaphore(pDevice->GetPlatform().GetVKDevice(), m_imageAcquired, nullptr);
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
	m_uWidth = m_uHeight = 0;

	// Sometimes the window is getting minimized AS I'm creating it
	while (m_uWidth == 0 || m_uHeight == 0)
	{
		GetClientRect(m_hwnd, &dim);
		m_uWidth = dim.right - dim.left;
		m_uHeight = dim.bottom - dim.top;
		::Sleep(8);
	}

	VkResult res;

	// BEGIN PC SPECIFIC CODE
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = WINUTIL::GetInstanceHndl();
	createInfo.hwnd = m_hwnd;
	res = vkCreateWin32SurfaceKHR(devicePS.GetVKInstance(), &createInfo, nullptr, &m_surface);
	// END PC SPECIFIC CODE

	ASSERT(res == VK_SUCCESS);

	

	CreateSwapChain(pDevice);
	CreateSwapChainImageViews(pDevice);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;
	semaphoreCreateInfo.flags = 0;

	res = vkCreateSemaphore(devicePS.GetVKDevice(), &semaphoreCreateInfo, nullptr, &m_imageAcquired);
	ASSERT(res == VK_SUCCESS);

	usg::RenderPassDecl rpDecl;
	usg::RenderPassDecl::Attachment attach;
	usg::RenderPassDecl::SubPass subPass;
	usg::RenderPassDecl::AttachmentReference ref;
	// Loading as 9 times out of 10 we won't render to the backbuffer before doing a transfer from another target
	attach.eLoadOp = usg::RenderPassDecl::LOAD_OP_DONT_CARE;
	attach.eStoreOp = usg::RenderPassDecl::STORE_OP_STORE;
	attach.eInitialLayout = usg::RenderPassDecl::LAYOUT_UNDEFINED;
	attach.eFinalLayout = usg::RenderPassDecl::LAYOUT_PRESENT_SRC;
	ref.eLayout = usg::RenderPassDecl::LAYOUT_COLOR_ATTACHMENT;

	usg::RenderPassDecl::Dependency Dependencies[2];
	Dependencies[0].uSrcSubPass = usg::RenderPassDecl::SUBPASS_EXTERNAL;
	Dependencies[0].uDstSubPass = 0;
	Dependencies[0].uSrcStageFlags = usg::RenderPassDecl::SF_BOTTOM_OF_PIPE;
	Dependencies[0].uDstStageFlags = usg::RenderPassDecl::SF_COLOR_ATTACHMENT_OUTPUT;
	Dependencies[0].uSrcAccessFlags = usg::RenderPassDecl::AC_MEMORY_READ;
	Dependencies[0].uDstAccessFlags = usg::RenderPassDecl::AC_COLOR_ATTACHMENT_READ | usg::RenderPassDecl::AC_COLOR_ATTACHMENT_WRITE;

	Dependencies[1].uSrcSubPass = 0;
	Dependencies[1].uDstSubPass = usg::RenderPassDecl::SUBPASS_EXTERNAL;
	Dependencies[1].uSrcStageFlags = usg::RenderPassDecl::SF_COLOR_ATTACHMENT_OUTPUT;
	Dependencies[1].uDstStageFlags = usg::RenderPassDecl::SF_BOTTOM_OF_PIPE;
	Dependencies[1].uSrcAccessFlags = usg::RenderPassDecl::AC_COLOR_ATTACHMENT_READ | usg::RenderPassDecl::AC_COLOR_ATTACHMENT_WRITE;
	Dependencies[1].uDstAccessFlags = usg::RenderPassDecl::AC_MEMORY_READ;


	subPass.uColorCount = 1;
	subPass.pColorAttachments = &ref;
	ref.uIndex = 0;

	subPass.pColorAttachments = &ref;
	subPass.uColorCount = 1;
	rpDecl.pAttachments = &attach;
	rpDecl.uAttachments = 1;
	rpDecl.uSubPasses = 1;
	rpDecl.pSubPasses = &subPass;
	attach.format.eColor = m_eSwapChainFormat;	// FIXME: Match the true format
	m_directRenderPass = pDevice->GetRenderPass(rpDecl);

	attach.eLoadOp = usg::RenderPassDecl::LOAD_OP_LOAD_MEMORY;
	attach.eInitialLayout = usg::RenderPassDecl::LAYOUT_PRESENT_SRC;
	m_postCopyRenderPass = pDevice->GetRenderPass(rpDecl);

	InitFrameBuffers(pDevice);
}

bool Display_ps::GetFallbackPresentMode(VkPresentModeKHR& eModeInOut)
{
	switch (eModeInOut)
	{
	case VK_PRESENT_MODE_MAILBOX_KHR:
		eModeInOut = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		return true;
	case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
		eModeInOut = VK_PRESENT_MODE_FIFO_KHR;
		return true;
	}

	return false;
}


void Display_ps::CreateSwapChain(GFXDevice* pDevice)
{
	GFXDevice_ps& devicePS = pDevice->GetPlatform();

	// Iterate over each queue to learn whether it supports presenting:
	vector<VkBool32> supportsPresent;
	supportsPresent.resize(devicePS.GetQueueFamilyCount());
	for (uint32_t i = 0; i < devicePS.GetQueueFamilyCount(); i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(devicePS.GetPrimaryGPU(), i, m_surface, &supportsPresent[i]);
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
	VkResult res = vkGetPhysicalDeviceSurfaceFormatsKHR(devicePS.GetPrimaryGPU(), m_surface, &formatCount, NULL);
	ASSERT(res == VK_SUCCESS);
	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(devicePS.GetPrimaryGPU(), m_surface, &formatCount, surfFormats);
	ASSERT(res == VK_SUCCESS);
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	VkFormat eFormat;
	VkColorSpaceKHR colorSpace;
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		eFormat = VK_FORMAT_B8G8R8A8_UNORM;
		colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	}
	else
	{
		int iBestFormat = 0;
		#if 0
		m_bHDR = false;
		for (uint32 i = 0; i < formatCount; i++)
		{
			if (devicePS.GetUSGFormat(surfFormats[i].format) == CF_INVALID)
			{
				continue;
			}
			if (surfFormats[i].colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT)
			{
				iBestFormat = i;
				m_bHDR = true;
			}
			else if (surfFormats[i].colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT &&
				(iBestFormat == i) ||  surfFormats[iBestFormat].colorSpace != VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT)
			{
				iBestFormat = i;
				m_bHDR = true;
			}
		}
		#endif
		eFormat = surfFormats[iBestFormat].format;
		colorSpace = surfFormats[iBestFormat].colorSpace;
		m_eSwapChainFormat = devicePS.GetUSGFormat(eFormat);

	}

	VkSurfaceCapabilitiesKHR surfCapabilities;

	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devicePS.GetPrimaryGPU(), m_surface, &surfCapabilities);
	ASSERT(res == VK_SUCCESS);

	uint32_t presentModeCount;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(devicePS.GetPrimaryGPU(), m_surface, &presentModeCount, NULL);
	ASSERT(res == VK_SUCCESS);
	vector<VkPresentModeKHR> presentModes;
	presentModes.resize(presentModeCount);

	res = vkGetPhysicalDeviceSurfacePresentModesKHR(devicePS.GetPrimaryGPU(), m_surface, &presentModeCount, presentModes.data());
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

	uint32 uHMDCount = ModuleManager::Inst()->GetNumberOfInterfacesForType(IHeadMountedDisplay::GetModuleTypeNameStatic());
	
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	VkPresentModeKHR desiredSwapchainPresentMode = uHMDCount > 0 ? VK_PRESENT_MODE_IMMEDIATE_KHR : g_presentMapping[m_eVsync];

	bool bFound = false;

	do
	{
		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == desiredSwapchainPresentMode)
			{
				swapchainPresentMode = desiredSwapchainPresentMode;
				bFound = true;
				break;
			}
		}
	}
	while (!bFound && GetFallbackPresentMode(desiredSwapchainPresentMode));

	if (!bFound)
	{
		desiredSwapchainPresentMode = presentModes[0];
	}

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount + 1;
	if ((surfCapabilities.maxImageCount > 0) && (desiredNumberOfSwapChainImages > surfCapabilities.maxImageCount))
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
	swap_chain.imageColorSpace = colorSpace;
	swap_chain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swap_chain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swap_chain.queueFamilyIndexCount = 0;
	swap_chain.pQueueFamilyIndices = NULL;

	res = vkCreateSwapchainKHR(devicePS.GetVKDevice(), &swap_chain, nullptr, &m_swapChain);
	ASSERT(res == VK_SUCCESS);

	// Get the 
	res = vkGetSwapchainImagesKHR(devicePS.GetVKDevice(), m_swapChain, &m_uSwapChainImageCount, NULL);

	m_pSwapchainImages = vnew(ALLOC_GFX_RENDER_TARGET) VkImage[m_uSwapChainImageCount];
	m_pSwapchainImageViews = vnew(ALLOC_GFX_RENDER_TARGET) VkImageView[m_uSwapChainImageCount];
	ASSERT(m_pSwapchainImages != NULL);
	res = vkGetSwapchainImagesKHR(devicePS.GetVKDevice(), m_swapChain, &m_uSwapChainImageCount, m_pSwapchainImages);
	ASSERT(res == VK_SUCCESS);

	m_swapChainImageFormat = eFormat;

	#if 0
	VkHdrMetadataEXT metadata = {};
	metadata.sType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT;
	metadata.displayPrimaryRed.x = 0.708f;
	metadata.displayPrimaryRed.y = 0.292f;
	metadata.displayPrimaryGreen.x = 0.170f;
	metadata.displayPrimaryGreen.y = 0.797f;
	metadata.displayPrimaryBlue.x = 0.131f;
	metadata.displayPrimaryBlue.y = 0.046f;
	metadata.whitePoint.x = 0.003127f;
	metadata.whitePoint.y = 0.0003290f;
	metadata.maxLuminance = 1000.0f;
	metadata.minLuminance = 0.001f;
	metadata.maxContentLightLevel = 2000.0f;
	metadata.maxFrameAverageLightLevel = 500.0f;

	//vkSetHdrMetadataEXT(devicePS.GetVKDevice(), 1, &m_swapChain, &metadata);

	PFN_vkSetHdrMetadataEXT setHDRMetadata = VK_NULL_HANDLE;
	setHDRMetadata = (PFN_vkSetHdrMetadataEXT)vkGetDeviceProcAddr(devicePS.GetVKDevice(), "vkSetHdrMetadataEXT");

	if (setHDRMetadata)
	{
		setHDRMetadata(devicePS.GetVKDevice(), 1, &m_swapChain, &metadata);
	}
	#endif

}

void Display_ps::CreateSwapChainImageViews(GFXDevice* pDevice)
{
	for (uint32_t i = 0; i < m_uSwapChainImageCount; i++)
	{
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = NULL;
		color_image_view.format = m_swapChainImageFormat;
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

		VkResult res = vkCreateImageView(pDevice->GetPlatform().GetVKDevice(), &color_image_view, nullptr, &m_pSwapchainImageViews[i]);
		ASSERT(res == VK_SUCCESS);
	}
}

void Display_ps::SetAsTarget(VkCommandBuffer& cmd)
{
	VkRenderPassBeginInfo rp_begin;
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	if (m_bRTShouldLoad)
	{
		rp_begin.renderPass = m_postCopyRenderPass.GetContents()->GetPass();
		rp_begin.framebuffer = m_pFramebuffers[m_uActiveImage];
	}
	else
	{
		rp_begin.renderPass = m_directRenderPass.GetContents()->GetPass();
		rp_begin.framebuffer = m_pFramebuffersNoCopy[m_uActiveImage];
	}
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = m_uWidth;
	rp_begin.renderArea.extent.height = m_uHeight;
	rp_begin.clearValueCount = 0;
	rp_begin.pClearValues = nullptr;

	vkCmdBeginRenderPass(cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

	
	VkViewport viewport;
	viewport.height = (float)m_uHeight;
	viewport.width = (float)m_uWidth;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	viewport.x = 0;
	viewport.y = 0;
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.extent.width = m_uWidth;
	scissor.extent.height = m_uHeight;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(cmd, 0, 1, &scissor);

}

void Display_ps::InitFrameBuffers(GFXDevice* pDevice)
{
	VkResult res;
	VkImageView attachments[2];

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.attachmentCount = 1;
	fb_info.pAttachments = attachments;
	fb_info.width = m_uWidth;
	fb_info.height = m_uHeight;
	fb_info.layers = 1;

	uint32_t i;

	m_pFramebuffers = vnew(ALLOC_GFX_RENDER_TARGET) VkFramebuffer[m_uSwapChainImageCount];
	m_pFramebuffersNoCopy = vnew(ALLOC_GFX_RENDER_TARGET) VkFramebuffer[m_uSwapChainImageCount];
	for (i = 0; i < m_uSwapChainImageCount; i++)
	{
		attachments[0] = m_pSwapchainImageViews[i];
		fb_info.renderPass = m_postCopyRenderPass.GetContents()->GetPass();
		res = vkCreateFramebuffer(pDevice->GetPlatform().GetVKDevice(), &fb_info, NULL, &m_pFramebuffers[i]);
		fb_info.renderPass = m_directRenderPass.GetContents()->GetPass();
		res = vkCreateFramebuffer(pDevice->GetPlatform().GetVKDevice(), &fb_info, NULL, &m_pFramebuffersNoCopy[i]);

		ASSERT(res == VK_SUCCESS);
	}
}

void Display_ps::Transfer(GFXContext* pContext, RenderTarget* pTarget)
{
	GFXBounds bounds = { 0, 0, (sint32)m_uWidth, (sint32)m_uHeight };
	TransferRect(pContext, pTarget, bounds, bounds);
}

void Display_ps::TransferRect(GFXContext* pContext, RenderTarget* pTarget, const GFXBounds& srcBounds, const GFXBounds& dstBounds)
{
	// Currently don't support different bounds
	ASSERT(dstBounds.height == srcBounds.height);
	ASSERT(dstBounds.width == srcBounds.width);

	// FIXME: Remove the naff OpenGL inverted bounds
	uint32 uDstY = (m_uHeight - dstBounds.y - dstBounds.height);
	uint32 uSrcY = (pTarget->GetHeight() - srcBounds.y - srcBounds.height);

	VkImageBlit ic = {};
	ic.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ic.srcSubresource.mipLevel = 0;
	ic.srcSubresource.baseArrayLayer = 0;
	ic.srcSubresource.layerCount = 1;
	ic.srcOffsets[0].x = srcBounds.x;
	ic.srcOffsets[0].y = Math::Clamp(uSrcY, 0U, pTarget->GetHeight());
	ic.srcOffsets[1].x = srcBounds.x + srcBounds.width;
	ic.srcOffsets[1].y = Math::Clamp(uSrcY + srcBounds.height, 0U, pTarget->GetHeight());
	ic.srcOffsets[1].z = 1;
	ic.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ic.dstSubresource.mipLevel = 0;
	ic.dstSubresource.baseArrayLayer = 0;
	ic.dstSubresource.layerCount = 1;
	ic.dstOffsets[0].x = dstBounds.x;
	ic.dstOffsets[0].y = Math::Clamp(uDstY, 0U, m_uHeight);
	ic.dstOffsets[1].x = dstBounds.x + dstBounds.width;
	ic.dstOffsets[1].y = Math::Clamp(uDstY + dstBounds.height, 0U, m_uHeight);
	ic.dstOffsets[1].z = 1;

	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

	pContext->GetPlatform().SetImageLayout(m_pSwapchainImages[m_uActiveImage], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

	const Texture_ps& tex = pTarget->GetColorTexture(0)->GetPlatform();
	VkImage srcImage = tex.GetImage();
	pContext->GetPlatform().SetImageLayout(srcImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);
	VkFilter filter = srcBounds.width == dstBounds.width ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
	vkCmdBlitImage(pContext->GetPlatform().GetVkCmdBuffer(), srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_pSwapchainImages[m_uActiveImage], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ic, filter);

	pContext->GetPlatform().SetImageLayout(m_pSwapchainImages[m_uActiveImage], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, subresourceRange);
	//pContext->GetPlatform().ImageBarrier(m_pSwapchainImages[m_uActiveImage], VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	//pContext->GetPlatform().ImageBarrier(srcImage, VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	m_bRTShouldLoad = true;
}



void Display_ps::Present()
{
	m_bRTShouldLoad = false;
}

void Display_ps::SwapBuffers(GFXDevice* pDevice)
{
	static bool bFirst = true;
	if (!bFirst)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapChain;
		presentInfo.pImageIndices = &m_uActiveImage;
		// Check if a wait semaphore has been specified to wait for before presenting the image
	/*	if (m_presentComplete != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &m_presentComplete;
			presentInfo.waitSemaphoreCount = 1;
		}*/
		VkResult res = vkQueuePresentKHR(pDevice->GetPlatform().GetQueue(), &presentInfo);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || m_bWindowResized)
		{
			RecreateSwapChain(pDevice);
		}
	}
	else if (m_bWindowResized)
	{
		RecreateSwapChain(pDevice);
	}
	bFirst = false;

	vkAcquireNextImageKHR(pDevice->GetPlatform().GetVKDevice(), m_swapChain, UINT64_MAX, m_imageAcquired, (VkFence)nullptr, &m_uActiveImage);
}

void Display_ps::ScreenShot(const char* szFileName)
{

}


void Display_ps::RecreateSwapChain(GFXDevice* pDevice)
{
	VkDevice device = pDevice->GetPlatform().GetVKDevice();
	vkDeviceWaitIdle(device);

	DestroySwapChain(pDevice);

	CreateSwapChain(pDevice);
	CreateSwapChainImageViews(pDevice);
	InitFrameBuffers(pDevice);
	m_bWindowResized = false;

}

void Display_ps::Resize(usg::GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	if(m_uWidth != uWidth || m_uHeight != uHeight)
	{
		m_uWidth = uWidth;
		m_uHeight = uHeight;

		m_bWindowResized = true;
	}
}

void Display_ps::SetVSyncMode(VSyncMode eVsync)
{
	if(m_eVsync != eVsync)
	{
		m_eVsync = eVsync;
		m_bWindowResized = true;
	}
}

void Display_ps::Resize(usg::GFXDevice* pDevice)
{
	RECT dim;
	GetClientRect(m_hwnd, &dim);

	Resize(pDevice, dim.right - dim.left, m_uHeight = dim.bottom - dim.top);
}

void Display_ps::Minimized(usg::GFXDevice* pDevice)
{
	RECT dim;
	int width = 0; int height = 0;
	while (width == 0 || height == 0) {
		GetClientRect(m_hwnd, &dim);
		width = dim.right - dim.left;
		height = dim.bottom - dim.top;
		// Sleep for a frame at 120hz
		::Sleep(8);
		// Keep handling window message until we get the one that restores the window
		GFX::PostUpdate();
	}

	vkDeviceWaitIdle(pDevice->GetPlatform().GetVKDevice());

}

}

