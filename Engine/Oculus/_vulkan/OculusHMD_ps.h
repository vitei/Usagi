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
		OculusHMD_ps(ovrSession session, ovrGraphicsLuid luid);
		~OculusHMD_ps();

		virtual bool Init(GFXDevice* pDevice) final;
		virtual void Cleanup(GFXDevice* pDevice) final;

		virtual void Transfer(GFXContext* pContext, Eye eye, RenderTarget* pTarget) final;
		virtual void TransferSpectatorDisplay(GFXContext* pContext, Display* pDisplay) final;

		virtual const uint32 GetRequiredAPIExtensionCount() const { return m_uExtensions; }
		virtual const char* GetRequiredAPIExtension(uint32 uIndex) const { return m_extensionNamePtrs[uIndex]; }

	private:	
		void ParseExtensionString(char* names);

		char						m_extensionNames[4096];
		const char*					m_extensionNamePtrs[100];
		uint32						m_uExtensions;
    	VkImage                     m_mirrorImage;
		
		struct EyeTarget_ps
		{
			VkExtent2D       size;
		};

		EyeTarget_ps		m_targets_ps[2];

	};

}

