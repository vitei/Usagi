/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Vulkan  specific oculus rift code
*****************************************************************************/
#pragma once

#include "Engine/Common/Common.h"
#include "Engine/Oculus/OculusHMD.h"

namespace usg
{

	class OculusHMD_ps : public OculusHMD
	{
	public:
		OculusHMD_ps(ovrSession session);
		~OculusHMD_ps();

		virtual bool Init(GFXDevice* pDevice) final;
		virtual void Cleanup(GFXDevice* pDevice) final;

		virtual void Transfer(GFXContext* pContext, Eye eye, RenderTarget* pTarget) final;
		virtual void TransferSpectatorDisplay(GFXContext* pContext, Display* pDisplay) final;

	private:	
    	ovrMirrorTexture            m_mirrorTexture;
    	VkImage                     m_mirrorImage;
		
		struct EyeTarget_ps
		{
			VkExtent2D       size;
		};

		EyeTarget_ps		m_targets_ps[2];

	};

}

