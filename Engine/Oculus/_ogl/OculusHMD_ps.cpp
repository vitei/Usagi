/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Vulkan  specific oculus rift code
*****************************************************************************/
#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/Display.h"
#include "OculusHMD_ps.h"

namespace usg
{

	OculusHMD_ps::OculusHMD_ps(ovrSession session, ovrGraphicsLuid luid) :
		OculusHMD(session, luid)
	{
		m_mirrorFBO = 0;
		m_layerHeader.Type = ovrLayerType_EyeFov;
		m_layerHeader.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.
	}

	OculusHMD_ps::~OculusHMD_ps()
	{

	}


	bool OculusHMD_ps::Init(GFXDevice* pDevice)
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

		for (uint32 eye = 0; eye < 2; eye++)
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

				glGenFramebuffers(1, &m_targets_ps[eye].fbo);
			}
			else
			{
				return false;
			}
		}


		return OculusHMD::Init(pDevice);
	}

	void OculusHMD_ps::Cleanup(GFXDevice* pDevice)
	{
		if (m_mirrorFBO)
		{
			glDeleteFramebuffers(1, &m_mirrorFBO);
		}

		for (uint32 i = 0; i < 2; i++)
		{
			if (m_targets[i].swapChain)
			{
				glDeleteFramebuffers(1, &m_targets_ps[i].fbo);
			}
		}

		OculusHMD::Cleanup(pDevice);
	}



	void OculusHMD_ps::Transfer(GFXContext* pContext, Eye eye, RenderTarget* pTarget)
	{
		GLuint curTexId;
		int curIndex;
		ColorBuffer* pBuffer = pTarget->GetColorBuffer();
		ovr_GetTextureSwapChainCurrentIndex(m_session, m_targets[(uint32)eye].swapChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(m_session, m_targets[(uint32)eye].swapChain, curIndex, &curTexId);

		glBindFramebuffer(GL_FRAMEBUFFER, m_targets_ps[(uint32)eye].fbo);	// Map the current swap chain texture to the FBO for the eye
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, pTarget->GetPlatform().GetOGLFBO());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_targets_ps[(uint32)eye].fbo);
		glBlitFramebuffer(0, 0, pBuffer->GetWidth(), pBuffer->GetHeight(),
			0, 0, m_targets[(uint32)eye].uWidth, m_targets[(uint32)eye].uHeight,
			GL_COLOR_BUFFER_BIT,
			m_targets[(uint32)eye].uWidth == pBuffer->GetWidth() ? GL_NEAREST : GL_LINEAR);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		// Unbind the texture from the FBO
		glBindFramebuffer(GL_FRAMEBUFFER, m_targets_ps[(uint32)eye].fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);


		// Now commit the swap chain for this eye
		ovr_CommitTextureSwapChain(m_session, m_targets[(uint32)eye].swapChain);

	}

	void OculusHMD_ps::TransferSpectatorDisplay(GFXContext* pContext, Display* pDisplay)
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

}