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
#include <SetupAPI.h>
#include "OVR_CAPI_Vk.h"
#include "OculusHMD_ps.h"



namespace usg
{

	OculusHMD_ps::OculusHMD_ps(ovrSession session, ovrGraphicsLuid luid) :
		OculusHMD(session, luid)
	{
		uint32_t extensionNamesSize = sizeof(m_extensionNames);
		ovr_GetInstanceExtensionsVk(luid, m_extensionNames, &extensionNamesSize);

		ParseExtensionString(m_extensionNames);
	}

	OculusHMD_ps::~OculusHMD_ps()
	{

	}

	void OculusHMD_ps::ParseExtensionString(char* names)
	{
		uint32_t extensionCount = 0;
		char* nextExtensionName = names;
		static const uint32_t arraySize = ARRAY_SIZE(m_extensionNamePtrs);
		while (*nextExtensionName && (extensionCount < arraySize))
		{
			m_extensionNamePtrs[extensionCount++] = nextExtensionName;
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

		m_uExtensions = extensionCount;
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