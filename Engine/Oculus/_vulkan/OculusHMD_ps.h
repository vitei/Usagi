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

		virtual const uint32 GetRequiredAPIExtensionCount(ExtensionType extType) const { return m_uExtensions[(size_t)extType]; }
		virtual const char* GetRequiredAPIExtension(ExtensionType extType, uint32 uIndex) const { return m_extensionNamePtrs[(size_t)extType][uIndex]; }

		// Try as we might it's impossible to avoid this call which doesn't fit within the interface
		virtual const char* GetModuleName() const { return "OculusHMD"; }
		bool GetPhysicalDevice(VkInstance instance, VkPhysicalDevice* deviceOut);
	private:	
		void ParseExtensionString(ExtensionType eType);

		char						m_extensionNames[(size_t)ExtensionType::Count][4096];
		const char*					m_extensionNamePtrs[(size_t)ExtensionType::Count][100];
		uint32						m_uExtensions[(size_t)ExtensionType::Count];
    	VkImage                     m_mirrorImage;
		
		struct EyeTarget_ps
		{
			VkExtent2D       size;
		};

		EyeTarget_ps		m_targets_ps[2];

	};

}

