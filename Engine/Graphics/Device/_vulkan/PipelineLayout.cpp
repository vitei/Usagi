/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/DescriptorSetLayout.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "PipelineLayout.h"

namespace usg {

PipelineLayout::PipelineLayout() :
	  m_uDescSetCount(0)
	, m_uDescSetFlags(0)
	, m_layout(VK_NULL_HANDLE)
{

}

PipelineLayout::~PipelineLayout()
{
	// FIXME: Needs a cleanup function
	//vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}
	

void PipelineLayout::Init(GFXDevice* pDevice, const PipelineLayoutDecl &decl, uint32 uId)
{
	VkDescriptorSetLayout* layoutBindings;
	ScratchObj<VkDescriptorSetLayout> desciptorScratch(layoutBindings, decl.uDescriptorSetCount);
	VkPipelineLayoutCreateInfo createInfo = {};

	for(uint32 i=0; i<decl.uDescriptorSetCount; i++)
	{
		layoutBindings[i] = decl.descriptorSets[i].GetContents()->GetPlatform().GetVkLayout();
	}

	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.pSetLayouts = layoutBindings;
	createInfo.setLayoutCount = decl.uDescriptorSetCount;

	VkResult eResult = vkCreatePipelineLayout(pDevice->GetPlatform().GetVKDevice(), &createInfo, nullptr, &m_layout);
	ASSERT(eResult == VK_SUCCESS);

	// So that we know which descriptor sets to apply
	m_uDescSetCount = decl.uDescriptorSetCount;
	m_uDescSetFlags = 0;
	for (uint32 i = 0; i < m_uDescSetCount; i++)
	{
		m_uDescSetFlags |= (1 << i);
	}
}


}

