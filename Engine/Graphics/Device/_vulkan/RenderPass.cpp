/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Device, RenderPass.h)
#include API_HEADER(Engine/Graphics/Textures, TextureFormat_ps.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/RenderPassInitData.h"
#include "Engine/Core/stl/vector.h"

namespace usg
{

	static const VkAttachmentLoadOp g_loadOpMap[] =
	{
		VK_ATTACHMENT_LOAD_OP_LOAD, // LOAD_OP_LOAD_MEMORY,
		VK_ATTACHMENT_LOAD_OP_CLEAR, // LOAD_OP_CLEAR_MEMORY,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE// LOAD_OP_DONT_CARE
	};

	static const VkAttachmentStoreOp g_storeOpMap[] =
	{
		VK_ATTACHMENT_STORE_OP_STORE, // STORE_OP_STORE
		VK_ATTACHMENT_STORE_OP_DONT_CARE // STORE_OP_DONT_CARE
	};

	static const VkImageLayout g_layoutMap[] =
	{
		VK_IMAGE_LAYOUT_UNDEFINED,						// LAYOUT_UNDEFINED = 0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,		// LAYOUT_COLOR_ATTACHMENT,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL// LAYOUT_DEPTH_STENCIL_ATTACHMENT
	};

	VkAttachmentDescriptionFlags GetAttachFlags(uint32 uFlags)
	{
		VkAttachmentDescriptionFlags flagsOut = 0;
		if (uFlags & RenderPassDecl::AF_MEMORY_REUSE)
		{
			flagsOut |= VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
		}

		return flagsOut;
	}


	VkAccessFlags GetStageFlags(uint32 uFlags)
	{
		VkAttachmentDescriptionFlags flagsOut = 0;
		if (uFlags & RenderPassDecl::SF_BOTTOM_OF_PIPE)
		{
			flagsOut |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		if (uFlags & RenderPassDecl::SF_COLOR_ATTACHMENT_OUTPUT)
		{
			flagsOut |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}

		return flagsOut;
	}

	VkAccessFlags GetAccessFlags(uint32 uFlags)
	{
		VkAttachmentDescriptionFlags flagsOut = 0;
		if (uFlags & RenderPassDecl::AC_COLOR_ATTACHMENT_READ)
		{
			flagsOut |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}
		if (uFlags & RenderPassDecl::AC_COLOR_ATTACHMENT_WRITE)
		{
			flagsOut |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		return flagsOut;
	}

void RenderPass::Init(GFXDevice* pDevice, const RenderPassInitData &initData, uint32 uId)
{
	const RenderPassDecl& decl = initData.GetDecl();
	// FIXME: Don't use std vector
	vector<VkAttachmentDescription> attachmentDescriptions;
	attachmentDescriptions.resize(decl.uAttachments);

	vector<VkSubpassDescription>  subpassDescriptions;
	subpassDescriptions.resize(decl.uSubPasses);

	vector<uint32> preserve;
	preserve.resize(initData.GetPreserveCount());

	vector<VkAttachmentReference> references;
	references.resize(initData.GetReferenceCount());

	vector<VkSubpassDependency> dependencies;
	dependencies.resize(decl.uDependencies);

	for (uint32 i = 0; i < decl.uAttachments; i++)
	{
		const RenderPassDecl::Attachment& in = decl.pAttachments[i];
		attachmentDescriptions[i].flags = GetAttachFlags(in.uAttachFlags);
		switch (in.eAttachType)
		{
		case RenderPassDecl::ATTACH_COLOR:
			attachmentDescriptions[i].format = gColorFormatMap[in.format.eColor];
			break;
		case RenderPassDecl::ATTACH_DEPTH:
			attachmentDescriptions[i].format = gDepthFormatViewMap[in.format.eDepth];
			break;
		default:
			ASSERT(false);
		}
		attachmentDescriptions[i].samples			= gSampleCountMap[in.eSamples];
		attachmentDescriptions[i].loadOp			= g_loadOpMap[in.eLoadOp];
		attachmentDescriptions[i].storeOp			= g_storeOpMap[in.eStoreOp];
		attachmentDescriptions[i].stencilLoadOp		= g_loadOpMap[in.eLoadOp];
		attachmentDescriptions[i].stencilStoreOp	= g_storeOpMap[in.eStoreOp];
		attachmentDescriptions[i].initialLayout		= g_layoutMap[in.eInitialLayout];
		attachmentDescriptions[i].finalLayout		= g_layoutMap[in.eFinalLayout];
	}

	uint32 uPreserveOffset = 0;
	uint32 uReferenceOffset = 0;
	for (uint32 i = 0; i < decl.uSubPasses; i++)
	{
		const RenderPassDecl::SubPass& in = decl.pSubPasses[i];

		subpassDescriptions[i].pColorAttachments = in.uColorCount ? &references.data()[uReferenceOffset] : nullptr;
		subpassDescriptions[i].colorAttachmentCount = in.uColorCount;
		for (uint32 uAttach = 0; uAttach < in.uColorCount; uAttach++)
		{
			references[uReferenceOffset + uAttach].attachment = in.pColorAttachments[uAttach].uIndex;
			references[uReferenceOffset + uAttach].layout = g_layoutMap[in.pColorAttachments[uAttach].eLayout];
		}
		uReferenceOffset += in.uColorCount;

		subpassDescriptions[i].pInputAttachments = in.uInputCount ? &references.data()[uReferenceOffset] : nullptr;
		subpassDescriptions[i].inputAttachmentCount = in.uInputCount;
		for (uint32 uAttach = 0; uAttach < in.uInputCount; uAttach++)
		{
			references[uReferenceOffset + uAttach].attachment = in.pInputAttachments[uAttach].uIndex;
			references[uReferenceOffset + uAttach].layout = g_layoutMap[in.pInputAttachments[uAttach].eLayout];
		}
		uReferenceOffset += in.uInputCount;

		subpassDescriptions[i].pResolveAttachments = in.pResolveAttachments ? &references.data()[uReferenceOffset] : nullptr;
		if (in.pResolveAttachments)
		{
			for (uint32 uAttach = 0; uAttach < in.uColorCount; uAttach++)
			{
				references[uReferenceOffset + uAttach].attachment = in.pResolveAttachments[uAttach].uIndex;
				references[uReferenceOffset + uAttach].layout = g_layoutMap[in.pResolveAttachments[uAttach].eLayout];
			}
			uReferenceOffset += in.uColorCount;
		}

		subpassDescriptions[i].pPreserveAttachments = in.uPreserveCount ? &preserve.data()[uPreserveOffset] : nullptr;
		subpassDescriptions[i].preserveAttachmentCount = in.uPreserveCount;
		for (uint32 uAttach = 0; uAttach < in.uPreserveCount; uAttach++)
		{
			preserve[uPreserveOffset + uAttach] = in.puPreserveIndices[uAttach];
		}
		uPreserveOffset += in.uPreserveCount;
	}


	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassInfo.pAttachments = attachmentDescriptions.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
	renderPassInfo.pSubpasses = subpassDescriptions.data();


	// Don't forget the render passes! :)
	ASSERT(renderPassInfo.subpassCount > 0);
	
	for (uint32 i = 0; i < decl.uDependencies; i++)
	{
		const RenderPassDecl::Dependency& in = decl.pDependencies[i];

		dependencies[i].srcSubpass = in.uSrcSubPass == RenderPassDecl::SUBPASS_EXTERNAL ? VK_SUBPASS_EXTERNAL : in.uSrcSubPass;
		dependencies[i].dstSubpass = in.uDstSubPass == RenderPassDecl::SUBPASS_EXTERNAL ? VK_SUBPASS_EXTERNAL : in.uDstSubPass;
		dependencies[i].srcAccessMask = GetAccessFlags(in.uSrcAccessFlags);
		dependencies[i].dstAccessMask = GetAccessFlags(in.uDstAccessFlags);
		dependencies[i].srcStageMask = GetStageFlags(in.uSrcStageFlags);
		dependencies[i].dstStageMask = GetStageFlags(in.uDstStageFlags);
		dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	}
	renderPassInfo.dependencyCount = decl.uDependencies;
	renderPassInfo.pDependencies = dependencies.data();
	

	VkResult res = vkCreateRenderPass(pDevice->GetPlatform().GetVKDevice(), &renderPassInfo, nullptr, &m_renderPass);
	ASSERT(res == VK_SUCCESS);
}


}

