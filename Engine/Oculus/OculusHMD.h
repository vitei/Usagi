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
	protected:
		// Can't instantiate, need the PS version
		OculusHMD(ovrSession session, ovrGraphicsLuid luid);
	public:
		~OculusHMD();

		virtual bool Init(GFXDevice* pDevice) override;
		virtual void Cleanup(GFXDevice* pDevice) override;

		virtual void Update() final;
		
		virtual void ResetTracking(bool bPos, bool bOri) final;
		
		virtual void GetRenderTargetDim(Eye eye, float pixelDensity, uint32 &uWidthOut, uint32 &uHeightOut ) const final;

		virtual Matrix4x4 GetProjectionMatrix(Eye eye, float fNear, float fFar) const final;

		virtual void SubmitFrame() final;
		virtual char16* GetAudioDeviceName() final { return m_deviceOutStrBuffer; }

		virtual void GetHMDTransform(usg::Matrix4x4& matOut) const;
		virtual void GetEyeTransform(Eye eye, usg::Matrix4x4& mMatOut) const;

		ovrSession GetSession() { return m_session; }

		static OculusHMD* TryCreate();

		const ovrTrackingState& GetTrackingState() const { return m_trackingState; }
		const ovrHmdDesc& GetHMDDesc() const { return m_hmdDesc; }

		Matrix4x4 ConvertPose(const ovrPosef &pose) const;

	protected:	
		struct EyeTarget
		{
			ovrTextureSwapChain	swapChain;
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
		ovrGraphicsLuid		m_luid;
		uint32				m_uFrameIndex;	// Update as we render each scene

		// Usagi native format
		Quaternionf			m_qRotation;
		Matrix4x4			m_hmdTransformMatrix;
		Matrix4x4 			m_eyeTransformMatrix[(uint32)Eye::Count];
	};

}

