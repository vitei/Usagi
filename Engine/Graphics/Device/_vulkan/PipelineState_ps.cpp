/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "Engine/Graphics/Effects/InputBinding.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device/, PipelineLayout.h)
#include API_HEADER(Engine/Graphics/Device/, AlphaState.h)
#include API_HEADER(Engine/Graphics/Device/, RenderPass.h)
#include API_HEADER(Engine/Graphics/Device/, RasterizerState.h)
#include API_HEADER(Engine/Graphics/Device/, DepthStencilState.h)
#include API_HEADER(Engine/Graphics/Device/, GFXDevice_ps.h)

namespace usg
{

	static const VkPrimitiveTopology g_primTypeMapping[] =
	{
		VK_PRIMITIVE_TOPOLOGY_POINT_LIST,					//PT_POINTS = 0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,				//PT_TRIANGLES,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST,					//PT_LINES,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,	//PT_TRIANGLES_ADJ,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,		//PT_LINES_ADJ,
		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,					//PT_LINE_STRIP
		VK_PRIMITIVE_TOPOLOGY_PATCH_LIST					//PT_PATCH_LIST
	};
	
	static_assert(ARRAY_SIZE(g_primTypeMapping) == PT_COUNT, "Incorrect number of primitive defines");

	static const VkSampleCountFlagBits  g_sampleCounts[SAMPLE_COUNT_INVALID] =
	{
		VK_SAMPLE_COUNT_1_BIT,	// SAMPLE_COUNT_1_BIT
		VK_SAMPLE_COUNT_2_BIT,  // SAMPLE_COUNT_2_BIT
		VK_SAMPLE_COUNT_4_BIT,  // SAMPLE_COUNT_4_BIT
		VK_SAMPLE_COUNT_8_BIT,  // SAMPLE_COUNT_8_BIT
		VK_SAMPLE_COUNT_16_BIT, // SAMPLE_COUNT_16_BIT
		VK_SAMPLE_COUNT_32_BIT, // SAMPLE_COUNT_32_BIT
		VK_SAMPLE_COUNT_64_BIT  // SAMPLE_COUNT_64_BIT
	};

	PipelineState_ps::PipelineState_ps()
	{
		m_pipeline = VK_NULL_HANDLE;
		m_layout = VK_NULL_HANDLE;
	}
	
	PipelineState_ps::~PipelineState_ps()
	{
		// FIXME: Needs a destory call
		//vkDestroyPipeline(device, m_pipeline, nullptr);
		ASSERT(m_pipeline == VK_NULL_HANDLE);

	}

	void PipelineState_ps::Init(GFXDevice* pDevice, const PipelineInitData& decl)
	{
		m_layout = decl.layout.GetContents()->GetVKLayout();

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = decl.layout.GetContents()->GetVKLayout();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = g_primTypeMapping[decl.ePrimType];

		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = g_sampleCounts[decl.eSampleCount];

		VkPipelineTessellationStateCreateInfo tesselationState = {};
		tesselationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tesselationState.patchControlPoints = decl.uPatchControlPoints;


		const Effect* pEffect = decl.pEffect.get();

		// Dynamic states as we are aiming for the slightly more flexible directx 12 setup
		VkDynamicState eDynamicStates[] =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_BLEND_CONSTANTS
		};

		VkPipelineViewportStateCreateInfo vp = {};
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp.pNext = NULL;
		vp.viewportCount = 1;
		vp.scissorCount = 1;


		VkPipelineDynamicStateCreateInfo dynamic_info = {};

		dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_info.pDynamicStates = eDynamicStates;
		dynamic_info.dynamicStateCount = 3;

		// FIXME: We need the render target to be passed in
		pipelineCreateInfo.renderPass = decl.renderPass.GetContents()->GetPass();
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.stageCount = pEffect->GetPlatform().GetStageCount();
		pipelineCreateInfo.pStages = pEffect->GetPlatform().GetShaderStageCreateInfo();
		pipelineCreateInfo.pVertexInputState = &decl.pBinding.GetContents()->GetPlatform().GetVertexInputCreateInfo();
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &decl.ras.GetContents()->GetCreateInfo();
		pipelineCreateInfo.pColorBlendState = &decl.alpha.GetContents()->GetCreateInfo();
		pipelineCreateInfo.pViewportState = &vp;	// Making this a dynamicall updated bit of info
		pipelineCreateInfo.pDepthStencilState = &decl.depth.GetContents()->GetCreateInfo();
		pipelineCreateInfo.pDynamicState = &dynamic_info;

		if (decl.uPatchControlPoints)
		{
			pipelineCreateInfo.pTessellationState = &tesselationState;
		}
		
		VkResult result = vkCreateGraphicsPipelines(pDevice->GetPlatform().GetVKDevice(), pDevice->GetPlatform().GetPipelineCache(), 1, &pipelineCreateInfo, nullptr, &m_pipeline);
		ASSERT(result == VK_SUCCESS);
	}

	void PipelineState_ps::Cleanup(GFXDevice* pDevice)
	{
		vkDestroyPipeline(pDevice->GetPlatform().GetVKDevice(), m_pipeline, nullptr);
		m_pipeline = VK_NULL_HANDLE;
	}


}

