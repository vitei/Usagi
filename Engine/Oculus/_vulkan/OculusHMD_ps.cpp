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

	OculusHMD_ps::OculusHMD_ps(ovrSession session) :
		OculusHMD(session)
	{

	}

	OculusHMD_ps::~OculusHMD_ps()
	{

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
		if (m_mirrorTexture)
		{
			ovr_DestroyMirrorTexture(m_session, m_mirrorTexture);
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