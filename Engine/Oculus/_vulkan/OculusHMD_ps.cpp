/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Vulkan  specific oculus rift code
*****************************************************************************/
#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "Engine/Graphics/Device/Display.h"
#include "Extras/OVR_Math.h"
#include "OculusVKExport.h"
#include <SetupAPI.h>
#include "OVR_CAPI_Vk.h"
#include "OculusHMD_ps.h"

static usg::OculusHMD_ps* g_pOculusHMD = nullptr;

bool GetHMDPhysicalDeviceVK(VkInstance instance, VkPhysicalDevice* deviceOut)
{
	if (g_pOculusHMD)
	{
		return g_pOculusHMD->GetPhysicalDevice(instance, deviceOut);
	}

	return false;

}

namespace usg
{

	OculusHMD_ps::OculusHMD_ps(ovrSession session, ovrGraphicsLuid luid) :
		OculusHMD(session, luid)
	{
		uint32_t extensionNamesSize = sizeof(m_extensionNames[(size_t)ExtensionType::Instance]);
		ovr_GetInstanceExtensionsVk(luid, m_extensionNames[(size_t)ExtensionType::Instance], &extensionNamesSize);

		ParseExtensionString(ExtensionType::Instance);

		ovr_GetDeviceExtensionsVk(luid, m_extensionNames[(size_t)ExtensionType::Device], &extensionNamesSize);

		ParseExtensionString(ExtensionType::Device);
		g_pOculusHMD = this;

		m_layerHeader.Type = ovrLayerType_EyeFov;
		m_layerHeader.Flags = 0;

		for (uint32 i = 0; i < 2; i++)
		{
			m_targets_ps[i].targets = nullptr;
		}
	}

	OculusHMD_ps::~OculusHMD_ps()
	{
		for (uint32 i = 0; i < 2; i++)
		{
			if (m_targets_ps[i].targets)
			{
				vdelete m_targets_ps[i].targets;
				m_targets_ps[i].targets = nullptr;
			}
		}
	}

	bool OculusHMD_ps::GetPhysicalDevice(VkInstance instance, VkPhysicalDevice* deviceOut)
	{
		ovrResult result = ovr_GetSessionPhysicalDeviceVk(m_session, m_luid, instance, deviceOut);
		if (!OVR_SUCCESS(result))
		{
			return false;
		}
		return true;
	}

	void OculusHMD_ps::ParseExtensionString(ExtensionType eType)
	{
		uint32_t extensionCount = 0;
		char* nextExtensionName = m_extensionNames[(size_t)eType];
		static const uint32_t arraySize = ARRAY_SIZE(m_extensionNamePtrs[(size_t)eType]);
		while (*nextExtensionName && (extensionCount < arraySize))
		{
			m_extensionNamePtrs[(size_t)eType][extensionCount++] = nextExtensionName;
			// Skip to a space or null
			while (*(++nextExtensionName))
			{
				if (*nextExtensionName == ' ')
				{
					// Null-terminate and break out of the loop
					*nextExtensionName++ = '\0';
					break;
				}
			}
		}

		m_uExtensions[(size_t)eType] = extensionCount;
	}

	bool OculusHMD_ps::Init(GFXDevice* pDevice)
	{
		// The mirror texture for rendering to the spectator display
		Display* pPrimaryDisplay = pDevice->GetDisplay(0);
		uint32 uWidth = 0; uint32 uHeight = 0;
		pPrimaryDisplay->GetPlatform().GetActualDimensions(uWidth, uHeight, false);
		ovrMirrorTextureDesc mirrorDesc = {};
		// This must match Platform.sc.format
		mirrorDesc.Format = OVR_FORMAT_B8G8R8A8_UNORM_SRGB;
		mirrorDesc.Width = uWidth;
		mirrorDesc.Height = uHeight;

		ovrResult result = ovr_CreateMirrorTextureWithOptionsVk(m_session, pDevice->GetPlatform().GetVKDevice(), &mirrorDesc, &m_mirrorTexture);
		if (!OVR_SUCCESS(result))
		{
			return false;
		}

		result = ovr_GetMirrorTextureBufferVk(m_session, m_mirrorTexture, &m_mirrorImage);
		if (!OVR_SUCCESS(result))
		{
			return false;
		}

		// Set up for each eye
		for (uint32 eye = 0; eye < 2; eye++)
		{
			ovrSizei size = ovr_GetFovTextureSize(m_session, ovrEyeType(eye), m_hmdDesc.DefaultEyeFov[eye], 1);

			m_targets[eye].uWidth = size.w;
			m_targets[eye].uHeight = size.h;

			ovrTextureSwapChainDesc desc = {};
			desc.Type = ovrTexture_2D;
			desc.ArraySize = 1;
			desc.Format = OVR_FORMAT_B8G8R8A8_UNORM_SRGB;
			desc.Width = size.w;
			desc.Height = size.h;
			desc.MipLevels = 1;
			desc.SampleCount = 1;
			desc.MiscFlags = ovrTextureMisc_DX_Typeless;
			desc.BindFlags = ovrTextureBind_DX_RenderTarget;
			desc.StaticImage = ovrFalse;

			ovrResult result = ovr_CreateTextureSwapChainVk(m_session, pDevice->GetPlatform().GetVKDevice(), &desc, &m_targets[eye].swapChain);
			if (!OVR_SUCCESS(result))
				return false;

			int textureCount = 0;
			ovr_GetTextureSwapChainLength(m_session, m_targets[eye].swapChain, &textureCount);
			m_targets_ps[eye].targets = vnew(ALLOC_GFX_INTERNAL)SwapChainTarget[textureCount];
			for (int i = 0; i < textureCount; ++i)
			{
				VkImage image;
				result = ovr_GetTextureSwapChainBufferVk(m_session, m_targets[eye].swapChain, i, &image);

				m_targets_ps[eye].targets->swapchainImage = image;
				// Create image view
				VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
				viewInfo.image = image;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
				viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
				viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
				viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
				viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.baseMipLevel = 0;
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = 1;
				VkResult vkRes = vkCreateImageView(pDevice->GetPlatform().GetVKDevice(), &viewInfo, nullptr, &m_targets_ps[eye].targets->swapchainImageView);
				ASSERT(vkRes == VK_SUCCESS);

				// Framebuffer isn't needed yet as we don't let allow direct rendering
				m_targets_ps[eye].targets->framebuffer = nullptr;
			}
		}

		// Let the compositor know which queue to synchronize with
		ovr_SetSynchronizationQueueVk(m_session, pDevice->GetPlatform().GetQueue());


		return OculusHMD::Init(pDevice);
	}


	void OculusHMD_ps::Cleanup(GFXDevice* pDevice)
	{
		for (uint32 i = 0; i < 2; i++)
		{
			if (m_targets_ps[i].targets)
			{
				vkDestroyImageView(pDevice->GetPlatform().GetVKDevice(), m_targets_ps[i].targets->swapchainImageView, nullptr);
				if (m_targets_ps[i].targets->framebuffer)
				{
					vkDestroyFramebuffer(pDevice->GetPlatform().GetVKDevice(), m_targets_ps[i].targets->framebuffer, nullptr);
				}

				// Note we don't own image, it will get destroyed when ovr_DestroyTextureSwapChain is called
				m_targets_ps[i].targets->swapchainImage = VK_NULL_HANDLE;
				m_targets_ps[i].targets->swapchainImageView = VK_NULL_HANDLE;
			}
		}

		OculusHMD::Cleanup(pDevice);
	}

	void OculusHMD_ps::Transfer(GFXContext* pContext, Eye eye, RenderTarget* pTarget)
	{
		VkImageBlit ic = {};
		ic.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ic.srcSubresource.mipLevel = 0;
		ic.srcSubresource.baseArrayLayer = 0;
		ic.srcSubresource.layerCount = 1;
		ic.srcOffsets[0].x = 0;
		ic.srcOffsets[0].y = 0;
		ic.srcOffsets[1].x = m_targets[(int)eye].uWidth;
		ic.srcOffsets[1].y = m_targets[(int)eye].uHeight;
		ic.srcOffsets[1].z = 1;
		ic.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ic.dstSubresource.mipLevel = 0;
		ic.dstSubresource.baseArrayLayer = 0;
		ic.dstSubresource.layerCount = 1;
		ic.dstOffsets[0].x = 0;
		ic.dstOffsets[0].y = 0;
		ic.dstOffsets[1].x = m_targets[(int)eye].uWidth;
		ic.dstOffsets[1].y = m_targets[(int)eye].uHeight;
		ic.dstOffsets[1].z = 1;

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

		int index;
		ovr_GetTextureSwapChainCurrentIndex(m_session, m_targets[(int)eye].swapChain, &index);

		pContext->GetPlatform().SetImageLayout(m_targets_ps[(uint32)eye].targets[index].swapchainImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

		const Texture_ps& tex = pTarget->GetColorTexture(0)->GetPlatform();
		VkImage srcImage = tex.GetImage();
		pContext->GetPlatform().SetImageLayout(srcImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);
		VkFilter filter = VK_FILTER_NEAREST;
		vkCmdBlitImage(pContext->GetPlatform().GetVkCmdBuffer(), srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_targets_ps[(uint32)eye].targets[index].swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ic, filter);

		pContext->GetPlatform().SetImageLayout(m_targets_ps[(uint32)eye].targets[index].swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, subresourceRange);
		
	}

	void OculusHMD_ps::TransferSpectatorDisplay(GFXContext* pContext, Display* pDisplay)
	{
		// PRESENT_SRC_KHR -> TRANSFER_DST_OPTIMAL
		VkImageMemoryBarrier presentBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		presentBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		presentBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		presentBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		presentBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		presentBarrier.image = pDisplay->GetPlatform().GetActiveImage();
		presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		presentBarrier.subresourceRange.baseMipLevel = 0;
		presentBarrier.subresourceRange.levelCount = 1;
		presentBarrier.subresourceRange.baseArrayLayer = 0;
		presentBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(pContext->GetPlatform().GetVkCmdBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &presentBarrier);

		VkImageCopy region = {};
		region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.mipLevel = 0;
		region.srcSubresource.baseArrayLayer = 0;
		region.srcSubresource.layerCount = 1;
		region.srcOffset = { 0, 0, 0 };
		region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.dstSubresource.mipLevel = 0;
		region.dstSubresource.baseArrayLayer = 0;
		region.dstSubresource.layerCount = 1;
		region.dstOffset = { 0, 0, 0 };
		uint32 uWidth, uHeight;
		pDisplay->GetActualDimensions(uWidth, uHeight, false);
		region.extent = { uWidth, uHeight, 1 };
		vkCmdCopyImage(pContext->GetPlatform().GetVkCmdBuffer(),
			m_mirrorImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			pDisplay->GetPlatform().GetActiveImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region);


		presentBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		presentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		presentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		presentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		presentBarrier.image = pDisplay->GetPlatform().GetActiveImage();
		presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		presentBarrier.subresourceRange.baseMipLevel = 0;
		presentBarrier.subresourceRange.levelCount = 1;
		presentBarrier.subresourceRange.baseArrayLayer = 0;
		presentBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(pContext->GetPlatform().GetVkCmdBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &presentBarrier);
	}

}