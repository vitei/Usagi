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
		// TODO: We should support startic buffers
		m_uBuffers = 0;	
	}

	DescriptorSet_ps::~DescriptorSet_ps()
	{

	}

	void DescriptorSet_ps::Init(GFXDevice* pDevice, DescriptorSetLayout* pLayout)
	{
		m_uBuffers = GFX_NUM_DYN_BUFF;
		m_uActiveSet = m_uBuffers - 1;
		for (uint32 i = 0; i < m_uBuffers; i++)
		{
			m_descSet[i] = pLayout->GetPlatform().AllocDescriptorSet(pDevice);
		}

		m_bValid = true;
	}

	void DescriptorSet_ps::CleanUp(GFXDevice* pDevice, DescriptorSetLayout* pLayout)
	{
		if (m_bValid)
		{
			for (uint32 i = 0; i < m_uBuffers; i++)
			{
				pLayout->GetPlatform().FreeDescriptorSet(pDevice, m_descSet[i]);
			}
			m_bValid = false;
		}
	}

	void DescriptorSet_ps::UpdateDescriptors(GFXDevice* pDevice, const DescriptorSetLayout* pLayout, const DescriptorData* pData, bool bDoubleUpdate)
	{
		if (!bDoubleUpdate)
		{
			m_uActiveSet = (m_uActiveSet + 1) % m_uBuffers;
		}
		vector<VkDescriptorImageInfo> images;
		images.reserve(pLayout->GetNumberOfType(DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));

		vector<VkWriteDescriptorSet> writes;
		writes.resize(pLayout->GetDeclarationCount());

		m_dynamicBuffers.clear();
		
		for (uint32 i = 0; i < pLayout->GetDeclarationCount(); i++)
		{
			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[i].pNext = NULL;
			writes[i].dstSet = m_descSet[m_uActiveSet].descSet;
			writes[i].dstBinding = pLayout->GetDeclaration(i)->uBinding;
			writes[i].descriptorCount = pLayout->GetDeclaration(i)->uCount;
			switch (pLayout->GetDeclaration(i)->eDescriptorType)
			{
			case DESCRIPTOR_TYPE_CONSTANT_BUFFER:
			{
				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writes[i].pBufferInfo = &pData->pConstBuffer->GetPlatform().GetDescriptorInfo();
				break;
			}
			case DESCRIPTOR_TYPE_CONSTANT_BUFFER_DYNAMIC:
			{
				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;	// Dynamic so that we can update every frame
				writes[i].pBufferInfo = &pData->pConstBuffer->GetPlatform().GetBaseDescriptorInfo();
				size_t uIndex = m_dynamicBuffers.size();
				for (uint32 j = 0; j < writes[i].descriptorCount; j++)
				{
					m_dynamicBuffers.push_back(&pData[j].pConstBuffer->GetPlatform());
				}
				break;
			}
			case DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			{
				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writes[i].dstBinding += SAMPLER_OFFSET;
				size_t uIndex = images.size();
				for (uint32 j = 0; j < writes[i].descriptorCount; j++)
				{
					VkDescriptorImageInfo image = {};
					image.sampler = pData[j].texData.sampler.GetContents()->GetSampler();
					if (pData[j].texData.imageView.IsDefault())
					{
						image.imageView = pData[j].texData.tex->GetPlatform().GetImageView();
					}
					else
					{
						const Texture_ps& texPlat = pData[j].texData.tex->GetPlatform();
						image.imageView = texPlat.GetImageView(pDevice, pData[j].texData.imageView);
					}
					image.imageLayout = pData[j].texData.tex->GetPlatform().GetImageLayout();
					images.push_back(image);
				}

				writes[i].pImageInfo = &images[uIndex];
				break;
			}
			case DESCRIPTOR_TYPE_STORAGE_IMAGE:
			{
				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				writes[i].dstBinding += SAMPLER_OFFSET;	// Share the offsets of standard samplers
				size_t uIndex = images.size();
				for (uint32 j = 0; j < writes[i].descriptorCount; j++)
				{
					VkDescriptorImageInfo image = {};
					image.imageView = pData[j].texData.tex->GetPlatform().GetImageView();
					image.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					images.push_back(image);
				}

				writes[i].pImageInfo = &images[uIndex];
				break;
			}
			default:
				ASSERT(false);
			}

			pData += pLayout->GetDeclaration(i)->uCount;
		}

		vkUpdateDescriptorSets(pDevice->GetPlatform().GetVKDevice(), pLayout->GetDeclarationCount(), writes.data(), 0, nullptr);
	}


	void DescriptorSet_ps::Bind(VkCommandBuffer buffer, VkPipelineLayout layout, uint32 uSlot) const
	{
		if (m_dynamicBuffers.size())
		{
			static uint32 uDynamicOffsets[32] = {};
			ASSERT(m_dynamicBuffers.size() < 32);

			for (uint32 i = 0; i < m_dynamicBuffers.size(); i++)
			{
				uDynamicOffsets[i] = m_dynamicBuffers[i]->GetActiveBufferOffset();
			}

			vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, uSlot, 1, &m_descSet[m_uActiveSet].descSet, (uint32)m_dynamicBuffers.size(), uDynamicOffsets);
		}
		else
		{
			vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, uSlot, 1, &m_descSet[m_uActiveSet].descSet, 0, nullptr);
		}
	}
}
