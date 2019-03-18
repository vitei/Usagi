/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Vulkan  specific oculus rift code
*****************************************************************************/
#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
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
				vkCreateImageView(pDevice->GetPlatform().GetVKDevice(), &viewInfo, nullptr, &m_targets_ps[eye].targets->swapchainImageView);
			}
		}


		return OculusHMD::Init(pDevice);
	}


	void OculusHMD_ps::Cleanup(GFXDevice* pDevice)
	{
		for (uint32 i = 0; i < 2; i++)
		{
			if (m_targets_ps[i].targets)
			{
				vkDestroyImageView(pDevice->GetPlatform().GetVKDevice(), m_targets_ps[i].targets->swapchainImageView, nullptr);
				vkDestroyFramebuffer(pDevice->GetPlatform().GetVKDevice(), m_targets_ps[i].targets->framebuffer, nullptr);

				// Note we don't own image, it will get destroyed when ovr_DestroyTextureSwapChain is called
				m_targets_ps[i].targets->swapchainImage = VK_NULL_HANDLE;
				m_targets_ps[i].targets->swapchainImageView = VK_NULL_HANDLE;
			}
		}

		OculusHMD::Cleanup(pDevice);
	}

	void OculusHMD_ps::Transfer(GFXContext* pContext, Eye eye, RenderTarget* pTarget)
	{
		
	}

	void OculusHMD_ps::TransferSpectatorDisplay(GFXContext* pContext, Display* pDisplay)
	{

	}

}