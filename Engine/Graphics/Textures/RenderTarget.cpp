/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "RenderTarget.h"

namespace usg {

	RenderTarget::RenderTarget()
	{
		m_clearDepth = 1.0f;
		m_clearStencil = 0;
		m_pDepth = NULL;
		m_uTargetCount = 0;
		MemClear(m_pColorBuffer, sizeof(ColorBuffer*)*MAX_COLOR_TARGETS);
	}

	RenderTarget::~RenderTarget()
	{
	}


	void RenderTarget::Init(usg::GFXDevice* pDevice, ColorBuffer* pColorBuffer, DepthStencilBuffer* pDepth)
	{
		uint32 uColCount = pColorBuffer != NULL ? 1 : 0;
		InitMRT(pDevice, uColCount, &pColorBuffer, pDepth);
	}

	void RenderTarget::InitMRT(usg::GFXDevice* pDevice, uint32 uColCount, ColorBuffer** ppColorBuffer, DepthStencilBuffer* pDepth)
	{
		m_uTargetMask = 0;
		m_uTargetCount = uColCount;
		ASSERT(uColCount <= MAX_COLOR_TARGETS);
		for (uint32 i = 0; i < uColCount; i++)
		{
			m_pColorBuffer[i] = ppColorBuffer[i];
			m_uTargetMask |= (1 << ppColorBuffer[i]->GetRTLoc());
		}
		m_pDepth = pDepth;

		ASSERT(ConfirmCompataible(ppColorBuffer[0], pDepth));

		m_platform.InitMRT(pDevice, uColCount, ppColorBuffer, pDepth);
	}

	void RenderTarget::CleanUp(GFXDevice* pDevice)
	{
		m_platform.CleanUp(pDevice);
	}

	void RenderTarget::Resize(usg::GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
	{
		m_platform.Resize(pDevice, uWidth, uHeight);
	}


	bool RenderTarget::ConfirmCompataible(ColorBuffer* pColorBuffer, DepthStencilBuffer* pDepth)
	{
		if (!pDepth || !pColorBuffer)
			return true;

		if (pColorBuffer->GetSampleCount() != pDepth->GetSampleCount())
			return false;

		if (pColorBuffer->GetWidth() != pDepth->GetWidth())
			return false;

		if (pColorBuffer->GetHeight() != pDepth->GetHeight())
			return false;

		return true;
	}

	TextureHndl RenderTarget::GetDepthTexture() const
	{
		return m_pDepth ? m_pDepth->GetTexture() : TextureHndl(NULL);
	}


	RenderPassDecl::AttachmentLoadOp GetLoadOp(uint32 uFlag, uint32 uLoadFlags, uint32 uClearFlags)
	{
		if (uLoadFlags & uFlag)
		{
			return RenderPassDecl::LOAD_OP_LOAD_MEMORY;
		}
		if (uClearFlags & uFlag)
		{
			return RenderPassDecl::LOAD_OP_CLEAR_MEMORY;
		}
		return RenderPassDecl::LOAD_OP_DONT_CARE;
	}

	RenderPassHndl RenderTarget::InitRenderPass(GFXDevice* pDevice, uint32 uLoadFlags, uint32 uClearFlags, uint32 uStoreFlags, const RenderPassDecl::Dependency* pDependencies, uint32 uDependiences)
	{
		// Shouldn't be trying to load data that you are clearing
		ASSERT((uLoadFlags & uClearFlags) == 0);
		
		usg::RenderPassDecl decl;
		uint32 uAttachments = m_uTargetCount + (m_pDepth ? 1 : 0);
		vector<RenderPassDecl::Attachment> attachments;
		vector<RenderPassDecl::AttachmentReference> refs;
		attachments.resize(uAttachments);
		refs.resize(uAttachments);
		usg::RenderPassDecl::SubPass subPass;

		for (uint32 i = 0; i < m_uTargetCount; i++)
		{
			RenderPassDecl::Attachment& attach = attachments[i];
			RenderPassDecl::AttachmentReference& ref = refs[i];
			uint32 uFlag = (1 << i);
			attach.eLoadOp = GetLoadOp(uFlag, uLoadFlags, uClearFlags);
			attach.eStoreOp = uStoreFlags & uFlag ? RenderPassDecl::STORE_OP_STORE : RenderPassDecl::STORE_OP_DONT_CARE;
			attach.format.eColor = m_pColorBuffer[i]->GetFormat();
			ref.eLayout = usg::RenderPassDecl::LAYOUT_COLOR_ATTACHMENT;
			ref.uIndex = i;
		}
		
		if (m_pDepth)
		{
			RenderPassDecl::Attachment& attach = attachments[m_uTargetCount];
			RenderPassDecl::AttachmentReference& ref = refs[m_uTargetCount];
			uint32 uFlag = (1 << m_uTargetCount);
			attach.eLoadOp = GetLoadOp(uFlag, uLoadFlags, uClearFlags);
			attach.eStoreOp = uStoreFlags & uFlag ? RenderPassDecl::STORE_OP_STORE : RenderPassDecl::STORE_OP_DONT_CARE;
			attach.format.eDepth = m_pDepth->GetFormat();
			ref.eLayout = usg::RenderPassDecl::LAYOUT_DEPTH_STENCIL_ATTACHMENT;
			ref.uIndex = m_uTargetCount;

			subPass.pDepthAttachment = &ref;
		}


		subPass.pColorAttachments = refs.data();
		subPass.uColorCount = m_uTargetCount;
		decl.pAttachments = attachments.data();
		decl.uAttachments = (uint32)attachments.size();
		decl.uSubPasses = 1;
		decl.pSubPasses = &subPass;
		decl.pDependencies = pDependencies;
		decl.uDependencies = uDependiences;
		
		m_renderPass = pDevice->GetRenderPass(decl);

		m_platform.RenderPassUpdated(pDevice, m_renderPass);
		return m_renderPass;
	}


	void RenderTarget::SetClearColor(const Color &col, uint32 uTarget)
	{
		if (uTarget < m_uTargetCount)
		{
			m_clearColor[uTarget] = col;
			m_platform.SetClearColor(col, uTarget);
		}
	}

}