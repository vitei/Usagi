/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/RenderPassInitData.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include API_HEADER(Engine/Graphics/Device, RenderPass.h)

namespace usg {

	void RenderPass::Init(GFXDevice* pDevice, const class RenderPassInitData &decl, uint32 uId)
	{
		const RenderPassDecl::SubPass* pSubPass = decl.GetDecl().pSubPasses;
		ASSERT(decl.GetDecl().uSubPasses == 1);
		for (uint32 i = 0; i < pSubPass->uColorCount; i++)
		{
			uint32 uIndex = pSubPass->pColorAttachments[i].uIndex;
			if (decl.GetDecl().pAttachments[uIndex].eLoadOp == RenderPassDecl::LOAD_OP_CLEAR_MEMORY)
			{
				m_uClearFlags |= (RenderTarget::RT_FLAG_COLOR_0 << i);
			}
		}

		if (pSubPass->pDepthAttachment)
		{
			uint32 uIndex = pSubPass->pDepthAttachment->uIndex;
			if (decl.GetDecl().pAttachments[uIndex].eLoadOp == RenderPassDecl::LOAD_OP_CLEAR_MEMORY)
			{
				m_uClearFlags |= (RenderTarget::RT_FLAG_DS);
			}
		}
	}
}
