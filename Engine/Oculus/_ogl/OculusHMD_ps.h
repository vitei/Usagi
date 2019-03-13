/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Oculus Rift Sensor
*****************************************************************************/
#pragma once

#include "Engine/Common/Common.h"
#include "Engine/Oculus/OculusHMD.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include "OVR_CAPI_GL.h"
#include "OVR_CAPI_Audio.h"

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

		GLuint GetSpectatorFBO() const { return m_mirrorFBO; }

	private:	
		// TODO: OpenGL specific stuff should be moved to the device
		GLuint				m_mirrorFBO;
		struct EyeTarget_ps
		{
			GLuint fbo;
		};

		EyeTarget_ps	m_targets_ps[2];
	};

}

