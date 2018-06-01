/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Graphics/Device/DescriptorData.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Textures, Sampler.h)

namespace usg {

	DescriptorSet_ps::DescriptorSet_ps()
	{
		m_bValid = false;
	}

	DescriptorSet_ps::~DescriptorSet_ps()
	{

	}

	void DescriptorSet_ps::Init(GFXDevice* pDevice, DescriptorSetLayout* pLayout)
	{
		m_descSet = pLayout->GetPlatform().AllocDescriptorSet(pDevice);

		m_bValid = true;
	}

	void DescriptorSet_ps::CleanUp(GFXDevice* pDevice, DescriptorSetLayout* pLayout)
	{
		if (m_bValid)
		{
			pLayout->GetPlatform().FreeDescriptorSet(pDevice, m_descSet);
		}
	}

	void DescriptorSet_ps::UpdateDescriptors(GFXDevice* pDevice, const DescriptorSetLayout* pLayout, const DescriptorData* pData)
	{
		vector<VkDescriptorImageInfo> images;
		images.reserve(pLayout->GetNumberOfType(DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));

		vector<VkWriteDescriptorSet> writes;
		writes.resize(pLayout->GetDeclarationCount());

		m_dynamicBuffers.clear();
		
		for (uint32 i = 0; i < pLayout->GetDeclarationCount(); i++)
		{
			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[i].pNext = NULL;
			writes[i].dstSet = m_descSet.descSet;
			writes[i].dstBinding = pLayout->GetDeclaration(i)->uBinding;
			writes[i].descriptorCount = pLayout->GetDeclaration(i)->uCount;
			switch (pLayout->GetDeclaration(i)->eDescriptorType)
			{
			case DESCRIPTOR_TYPE_CONSTANT_BUFFER:

				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	// Dynamic so that we can update every frame
				writes[i].pBufferInfo = &pData[i].pConstBuffer->GetPlatform().GetDescriptorInfo();
				m_dynamicBuffers.push_back(&pData[i].pConstBuffer->GetPlatform());
				break;
			case DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			{
				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				VkDescriptorImageInfo image = {};
				image.sampler = pData[i].texData.sampler.GetContents()->GetSampler();
				image.imageView = pData[i].texData.tex->GetPlatform().GetImageView();
				image.imageLayout = pData[i].texData.tex->GetPlatform().GetImageLayout();
				images.push_back(image);

				writes[i].pImageInfo = &images.back();
				break;
			}
			default:
				ASSERT(false);
			}
		}

		vkUpdateDescriptorSets(pDevice->GetPlatform().GetVKDevice(), pLayout->GetDeclarationCount(), writes.data(), 0, nullptr);
	}

	void DescriptorSet_ps::NotifyBufferChanged(uint32 uLayoutIndex, uint32 uSubIndex, const DescriptorData* pData)
	{
		// Do nothing - was only for the 3DS
	}


	void DescriptorSet_ps::Bind(VkCommandBuffer buffer, VkPipelineLayout layout, uint32 uSlot) const
	{
	/*	static uint32 uDynamicOffsets[32] = {};
		ASSERT(m_dynamicBuffers.size() < 32);

		for (uint32 i = 0; i < m_dynamicBuffers.size(); i++)
		{
			uDynamicOffsets[i] = m_dynamicBuffers[i]->GetActiveBufferOffset();
		}*/

		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, uSlot, 1, &m_descSet.descSet, 0, nullptr);// (uint32)m_dynamicBuffers.size(), uDynamicOffsets);
	}
}
