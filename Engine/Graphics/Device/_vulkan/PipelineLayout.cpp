/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/DescriptorSetLayout.h"
#include "PipelineLayout.h"

namespace usg {

PipelineLayout::PipelineLayout()
{

}

PipelineLayout::~PipelineLayout()
{
	// FIXME: Needs a cleanup function
	//vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}
	

void PipelineLayout::Init(GFXDevice* pDevice, const PipelineLayoutDecl &decl)
{
	VkDescriptorSetLayout* layoutBindings;
	ScratchObj<VkDescriptorSetLayout> desciptorScratch(layoutBindings, decl.uDescriptorSetCount);
	VkPipelineLayoutCreateInfo createInfo = {};

	for(uint32 i=0; i<decl.uDescriptorSetCount; i++)
	{
		layoutBindings[i] = decl.descriptorSets[i].GetContents()->GetPlatform().GetVkLayout();
	}

	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.pSetLayouts = layoutBindings;
	createInfo.setLayoutCount = decl.uDescriptorSetCount;

	VkResult eResult = vkCreatePipelineLayout(pDevice->GetPlatform().GetVKDevice(), &createInfo, nullptr, &m_layout);
	ASSERT(eResult == VK_SUCCESS);
}


}

