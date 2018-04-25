/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "Engine/Graphics/Device/Display.h"
#include "Extras/OVR_Math.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Memory/Mem.h"
#include "OculusHMD.h"
#include <SetupAPI.h>
#include "Engine/Core/stl/string.h"

#pragma comment(lib, "SetupAPI.lib")

#if defined(_WIN32)
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

namespace usg
{
	static bool g_sbInitialised = false;

	static int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs)
	{
		return memcmp(&lhs, &rhs, sizeof(ovrGraphicsLuid));
	}


	static ovrGraphicsLuid GetDefaultAdapterLuid()
	{
		ovrGraphicsLuid luid = ovrGraphicsLuid();

#if defined(_WIN32)
		IDXGIFactory* factory = nullptr;

		if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
		{
			IDXGIAdapter* adapter = nullptr;

			if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
			{
				DXGI_ADAPTER_DESC desc;

				adapter->GetDesc(&desc);
				memcpy(&luid, &desc.AdapterLuid, sizeof(luid));
				adapter->Release();
			}

			factory->Release();
		}
#endif

		return luid;
	}

	Matrix4x4 Convert(const OVR::Matrix4f &in)
	{
		Matrix4x4 out;

		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				out.M[j][i] = in.M[i][j];

		return out;
	}

	Quaternionf Convert(const ovrQuatf &in)
	{
		return Quaternionf(in.x, in.y, in.z, in.w);
	}

	Vector3f Convert(const ovrVector3f& in)
	{
		return Vector3f(in.x, in.y, in.z);
	}


	Matrix4x4 Convert(const ovrPosef& in)
	{
		Quaternionf qRot = Convert(in.Orientation);
		Vector3f vPos = Convert(in.Position);

		Matrix4x4 mOut = qRot;
		mOut.Translate(vPos.x, vPos.y, vPos.z);

		return mOut;
	}

	static void FlipHandedness(const ovrPosef* inPose, ovrPosef* outPose) {
		outPose->Orientation.x = inPose->Orientation.x;
		outPose->Orientation.y = inPose->Orientation.y;
		outPose->Orientation.z = -inPose->Orientation.z;
		outPose->Orientation.w = -inPose->Orientation.w;

		outPose->Position.x = inPose->Position.x;
		outPose->Position.y = inPose->Position.y;
		outPose->Position.z = -inPose->Position.z;
	}

	OculusHMD* OculusHMD::TryCreate()
	{
		if (!g_sbInitialised)
		{
			ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
			ovrResult result = ovr_Initialize(&initParams);
			if (OVR_SUCCESS(result))
			{
				g_sbInitialised = true;
			}
			else
			{
				return nullptr;
			}
		}
		ovrSession session;
		ovrGraphicsLuid luid;
		ovrResult result = ovr_Create(&session, &luid);
		if (!OVR_SUCCESS(result))
			return nullptr;


		if (Compare(luid, GetDefaultAdapterLuid())) // If luid that the Rift is on is not the default adapter LUID...
		{
			ASSERT_MSG(false, "OpenGL supports only the default graphics adapter.");
			return nullptr;
		}

		// We have an oculus available, so return one!
		return vnew(ALLOC_OBJECT) OculusHMD(session);
	}

	OculusHMD::OculusHMD(ovrSession session)
	{
		m_session = session;
		m_mirrorTexture = nullptr;
		m_uFrameIndex = 0;
		m_mirrorFBO = 0;

		m_hmdTransformMatrix = Matrix4x4::Identity();
		m_eyeTransformMatrix[(uint32)Eye::Left] = Matrix4x4::Identity();
		m_eyeTransformMatrix[(uint32)Eye::Right] = Matrix4x4::Identity();

		for (uint32 i = 0; i < 2; i++)
		{
			m_targets[i].swapChain = nullptr;
		}
	}

	OculusHMD::~OculusHMD()
	{

	}

	Matrix4x4 OculusHMD::ConvertPose(const ovrPosef &pose) const
	{
		ovrPosef poseTmp;
		FlipHandedness(&pose, &poseTmp);
		return Convert(pose);
	}

#include <initguid.h>
#include <mmdeviceapi.h>


	bool OculusHMD::Init(GFXDevice* pDevice)
	{
		// The mirror target
		{
			Display* pPrimaryDisplay = pDevice->GetDisplay(0);
			uint32 uWidth = 0; uint32 uHeight = 0;
			pPrimaryDisplay->GetPlatform().GetActualDimensions(uWidth, uHeight, false);
			ovrMirrorTextureDesc desc;
			memset(&desc, 0, sizeof(desc));
			desc.Width = uWidth;
			desc.Height = uHeight;
			desc.Format = OVR_FORMAT_R8G8B8A8_UNORM;

			// Create mirror texture and an FBO used to copy mirror texture to back buffer
			ovrResult result = ovr_CreateMirrorTextureGL(m_session, &desc, &m_mirrorTexture);
			if (!OVR_SUCCESS(result))
			{
				ASSERT_MSG(false, "Failed to create mirror texture.");
				return false;
			}

			// Configure the mirror read buffer
			GLuint texId;
			ovr_GetMirrorTextureBufferGL(m_session, m_mirrorTexture, &texId);

			glGenFramebuffers(1, &m_mirrorFBO);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorFBO);
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
			//glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

			CHECK_OGL_ERROR();
			ASSERT(glIsFramebuffer(m_mirrorFBO) == GL_TRUE);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
		}

		typedef BOOL(WINAPI * PFNWGLSWAPINTERVALPROC)(int);
		PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");

		if (wglSwapIntervalEXT)
		{
			wglSwapIntervalEXT(0);
		}

		m_hmdDesc = ovr_GetHmdDesc(m_session);

		for(uint32 eye=0; eye<2; eye++)
		{

			ovrSizei size = ovr_GetFovTextureSize(m_session, ovrEyeType(eye), m_hmdDesc.DefaultEyeFov[eye], 1);

			m_targets[eye].uWidth = size.w;
			m_targets[eye].uHeight = size.h;

			ovrTextureSwapChainDesc desc = {};
			desc.Type = ovrTexture_2D;
			desc.ArraySize = 1;
			desc.Width = size.w;
			desc.Height = size.h;
			desc.MipLevels = 1;
			desc.Format = OVR_FORMAT_R8G8B8A8_UNORM;
			desc.SampleCount = 1;
			desc.StaticImage = ovrFalse;

			ovrResult result = ovr_CreateTextureSwapChainGL(m_session, &desc, &m_targets[eye].swapChain);

			int length = 0;
			ovr_GetTextureSwapChainLength(m_session, m_targets[eye].swapChain, &length);

			if (OVR_SUCCESS(result))
			{
				for (int i = 0; i < length; ++i)
				{
					GLuint chainTexId;
					ovr_GetTextureSwapChainBufferGL(m_session, m_targets[eye].swapChain, i, &chainTexId);
					glBindTexture(GL_TEXTURE_2D, chainTexId);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				}

				glGenFramebuffers(1, &m_targets[eye].fbo);
			}
			else
			{
				return false;
			}
		}

		GUID guid;
		ovrResult result = ovr_GetAudioDeviceOutGuidStr(m_deviceOutStrBuffer);
		result = ovr_GetAudioDeviceOutGuid(&guid);
		
		usg::wstring deviceIdStr;
		deviceIdStr.reserve(112);
		deviceIdStr.append(L"\\\\?\\SWD#MMDEVAPI#");
		deviceIdStr.append(m_deviceOutStrBuffer);
		deviceIdStr.push_back(L'#');
		size_t offset = deviceIdStr.size();
		deviceIdStr.resize(deviceIdStr.capacity());


		StringFromGUID2(DEVINTERFACE_AUDIO_RENDER, &deviceIdStr[offset], OVR_AUDIO_MAX_DEVICE_STR_SIZE);
		wcscpy_s(m_deviceOutStrBuffer, OVR_AUDIO_MAX_DEVICE_STR_SIZE, deviceIdStr.c_str());


		if (!OVR_SUCCESS(result))
		{
			ovrErrorInfo info;
			ovr_GetLastErrorInfo(&info);
			DEBUG_PRINT("%s", info.ErrorString);
		}

		// Call an update frame
		Update();


		return true;
	}

	void OculusHMD::GetRenderTargetDim(Eye eye, float pixelDensity, uint32 &uWidthOut, uint32 &uHeightOut) const
	{
		ovrEyeType eyeType = eye == Eye::Left ? ovrEye_Left : ovrEye_Right;
		ovrSizei idealTextureSize = ovr_GetFovTextureSize(m_session, ovrEyeType(eye), m_hmdDesc.DefaultEyeFov[eyeType], pixelDensity);

		uWidthOut = (uint32)idealTextureSize.w;
		uHeightOut = (uint32)idealTextureSize.h;
	}

	void OculusHMD::Cleanup(GFXDevice* pDevice)
	{
		if (m_mirrorFBO)
		{
			glDeleteFramebuffers(1, &m_mirrorFBO);
		}
		if (m_mirrorTexture)
		{
			ovr_DestroyMirrorTexture(m_session, m_mirrorTexture);
		}

		for (uint32 i = 0; i < 2; i++)
		{
			if (m_targets[i].swapChain)
			{
				ovr_DestroyTextureSwapChain(m_session, m_targets[i].swapChain);
				m_targets[i].swapChain = nullptr;
				glDeleteFramebuffers(1, &m_targets[i].fbo);
			}
		}

		ovr_Destroy(m_session);
	}



	void OculusHMD::Update()
	{
		m_hmdDesc = ovr_GetHmdDesc(m_session);

		ovrEyeRenderDesc eyeRenderDesc[2];
		eyeRenderDesc[0] = ovr_GetRenderDesc(m_session, ovrEye_Left, m_hmdDesc.DefaultEyeFov[0]);
		eyeRenderDesc[1] = ovr_GetRenderDesc(m_session, ovrEye_Right, m_hmdDesc.DefaultEyeFov[1]);

		// Get eye poses, feeding in correct IPD offset
		ovrPosef EyeRenderPose[2];
		ovrPosef HeadPose;
		ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose,
			eyeRenderDesc[1].HmdToEyePose };

		ovr_GetEyePoses(m_session, m_uFrameIndex, ovrTrue, HmdToEyePose, m_eyeRenderPoses, &m_sensorSampleTime);
		//EyeRenderPose[0] = m_eyeRenderPoses[0];
		//EyeRenderPose[1] = m_eyeRenderPoses[1];
		FlipHandedness(&m_eyeRenderPoses[0], &EyeRenderPose[0]);
		FlipHandedness(&m_eyeRenderPoses[1], &EyeRenderPose[1]);

		double frameTime = ovr_GetPredictedDisplayTime(m_session, m_uFrameIndex);
		m_trackingState = ovr_GetTrackingState(m_session, frameTime, ovrTrue);
		FlipHandedness(&m_trackingState.HeadPose.ThePose, &HeadPose);


		// Head
		m_qRotation = Convert(HeadPose.Orientation);

		m_hmdTransformMatrix = Convert(HeadPose);
		m_eyeTransformMatrix[(uint32)Eye::Left] = Convert(EyeRenderPose[0]);
		m_eyeTransformMatrix[(uint32)Eye::Right] = Convert(EyeRenderPose[1]);

	}

	Matrix4x4 OculusHMD::GetProjectionMatrix(Eye eye, float fNear, float fFar) const
	{
		uint32 uEye = eye == Eye::Left ? 0 : 1;
		unsigned int projectionFlags = ovrProjection_LeftHanded;
		#if !Z_RANGE_0_TO_1
			projectionFlags |= ovrProjection_ClipRangeOpenGL;
		#endif
		OVR::Matrix4f proj = ovrMatrix4f_Projection(m_hmdDesc.DefaultEyeFov[uEye], fNear, fFar, projectionFlags);
		Matrix4x4 mUsgProj = Convert(proj);

		return mUsgProj;
	}
	
	void OculusHMD::ResetTracking(bool bPos, bool bOri)
	{
		ovr_RecenterTrackingOrigin(m_session);
	}

	void OculusHMD::Transfer(GFXContext* pContext, Eye eye, RenderTarget* pTarget)
	{
		GLuint curTexId;
		int curIndex;
		ColorBuffer* pBuffer = pTarget->GetColorBuffer();
		ovr_GetTextureSwapChainCurrentIndex(m_session, m_targets[(uint32)eye].swapChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(m_session, m_targets[(uint32)eye].swapChain, curIndex, &curTexId);

		glBindFramebuffer(GL_FRAMEBUFFER, m_targets[(uint32)eye].fbo);	// Map the current swap chain texture to the FBO for the eye
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, pTarget->GetPlatform().GetOGLFBO());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_targets[(uint32)eye].fbo);
		glBlitFramebuffer(0, 0, pBuffer->GetWidth(), pBuffer->GetHeight(),
			0, 0, m_targets[(uint32)eye].uWidth, m_targets[(uint32)eye].uHeight,
			GL_COLOR_BUFFER_BIT,
			m_targets[(uint32)eye].uWidth == pBuffer->GetWidth() ? GL_NEAREST : GL_LINEAR);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		
		// Unbind the texture from the FBO
		glBindFramebuffer(GL_FRAMEBUFFER, m_targets[(uint32)eye].fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);


		// Now commit the swap chain for this eye
		ovr_CommitTextureSwapChain(m_session, m_targets[(uint32)eye].swapChain);

	}

	void OculusHMD::TransferSpectatorDisplay(GFXContext* pContext, Display* pDisplay)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, GetSpectatorFBO());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		uint32 uX, uY;
		pDisplay->GetActualDimensions(uX, uY, false);
		GLint w = (GLint)uX;
		GLint h = (GLint)uY;
		glBlitFramebuffer(0, h, w, 0, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		pDisplay->GetPlatform().SetDirty();
	}

	void OculusHMD::SubmitFrame()
	{
		// Do distortion rendering, Present and flush/sync

		ovrLayerEyeFov ld;
		ld.Header.Type = ovrLayerType_EyeFov;
		ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

		for (int eye = 0; eye < 2; ++eye)
		{
			ld.ColorTexture[eye] = m_targets[eye].swapChain;
			ld.Viewport[eye].Pos.x = 0; ld.Viewport[eye].Pos.y = 0;
			ld.Viewport[eye].Size.w = m_targets[eye].uWidth;
			ld.Viewport[eye].Size.h = m_targets[eye].uHeight;
			ld.Fov[eye] = m_hmdDesc.DefaultEyeFov[eye];
			ld.RenderPose[eye] = m_eyeRenderPoses[eye];
			ld.SensorSampleTime = m_sensorSampleTime;
		}

		ovrLayerHeader* layers = &ld.Header;
		ovrResult result = ovr_SubmitFrame(m_session, m_uFrameIndex, nullptr, &layers, 1);
		

		m_uFrameIndex++;
	}

	void OculusHMD::GetHMDTransform(usg::Matrix4x4& matOut) const
	{
		matOut = m_hmdTransformMatrix;
	}

	void OculusHMD::GetEyeTransform(Eye eye, usg::Matrix4x4& mMatOut) const
	{
		mMatOut = m_eyeTransformMatrix[(uint32)eye];
	}
}