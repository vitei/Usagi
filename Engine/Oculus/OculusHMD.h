/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Oculus Rift Sensor
*****************************************************************************/
#pragma once

#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/IHeadMountedDisplay.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include "OVR_CAPI_GL.h"
#include "OVR_CAPI_Audio.h"

namespace usg
{

	class OculusHMD : public IHeadMountedDisplay
	{
	public:
		OculusHMD(ovrSession session);
		~OculusHMD();

		virtual bool Init(GFXDevice* pDevice) final;
		virtual void Cleanup(GFXDevice* pDevice) final;

		virtual void Update() final;
		
		virtual void ResetTracking(bool bPos, bool bOri) final;
		
		virtual void GetRenderTargetDim(Eye eye, float pixelDensity, uint32 &uWidthOut, uint32 &uHeightOut ) const final;

		virtual Matrix4x4 GetProjectionMatrix(Eye eye, float fNear, float fFar) const final;

		virtual void Transfer(GFXContext* pContext, Eye eye, RenderTarget* pTarget) final;
		virtual void TransferSpectatorDisplay(GFXContext* pContext, Display* pDisplay) final;

		virtual void SubmitFrame() final;
		virtual char16* GetAudioDeviceName() final { return m_deviceOutStrBuffer; }

		virtual void GetHMDTransform(usg::Matrix4x4& matOut) const;
		virtual void GetEyeTransform(Eye eye, usg::Matrix4x4& mMatOut) const;

		GLuint GetSpectatorFBO() const { return m_mirrorFBO; }

		ovrSession GetSession() { return m_session; }

		static OculusHMD* TryCreate();

		const ovrTrackingState& GetTrackingState() const { return m_trackingState; }
		const ovrHmdDesc& GetHMDDesc() const { return m_hmdDesc; }

		Matrix4x4 ConvertPose(const ovrPosef &pose) const;

	private:	
		// TODO: OpenGL specific stuff should be moved to the device
		GLuint				m_mirrorFBO;
		struct EyeTarget
		{
			ovrTextureSwapChain	swapChain;
			GLuint fbo;
			uint32 uWidth;
			uint32 uHeight;
		};

		WCHAR				m_deviceOutStrBuffer[OVR_AUDIO_MAX_DEVICE_STR_SIZE];
		double				m_sensorSampleTime;
		EyeTarget			m_targets[2];
		ovrPosef			m_eyeRenderPoses[2];
    	ovrSession			m_session;
		ovrHmdDesc			m_hmdDesc;
		ovrTrackingState	m_trackingState;
		ovrMirrorTexture	m_mirrorTexture;
		uint32				m_uFrameIndex;	// Update as we render each scene

		// Usagi native format
		Quaternionf			m_qRotation;
		Matrix4x4			m_hmdTransformMatrix;
		Matrix4x4 			m_eyeTransformMatrix[(uint32)Eye::Count];
	};

}

