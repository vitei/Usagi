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
	}

	OculusHMD_ps::~OculusHMD_ps()
	{

	}

	bool OculusHMD_ps::GetPhysicalDevice(VkInstance instance, VkPhysicalDevice* deviceOut)
	{
		return false;
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
		static const uint32_t arraySize = ARRAY_SIZE(m_extensionNamePtrs);
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
		Display* pPrimaryDisplay = pDevice->GetDisplay(0);
		uint32 uWidth = 0; uint32 uHeight = 0;
		pPrimaryDisplay->GetPlatform().GetActualDimensions(uWidth, uHeight, false);
		ovrMirrorTextureDesc mirrorDesc = {};
		// This must match Platform.sc.format
		mirrorDesc.Format = OVR_FORMAT_B8G8R8A8_UNORM_SRGB;
		mirrorDesc.Width = uWidth;
		mirrorDesc.Height = uHeight;
		
#if 0
		ovrResult result = ovr_GetSessionPhysicalDeviceVk(m_session, m_luid , instance, &physicalDevice));

		if (!OVR_SUCCESS(result))
		{
			return false;
		}
#endif

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


		return OculusHMD::Init(pDevice);
	}


	void OculusHMD_ps::Cleanup(GFXDevice* pDevice)
	{
		OculusHMD::Cleanup(pDevice);
	}

	void OculusHMD_ps::Transfer(GFXContext* pContext, Eye eye, RenderTarget* pTarget)
	{


	}

	void OculusHMD_ps::TransferSpectatorDisplay(GFXContext* pContext, Display* pDisplay)
	{

	}

}