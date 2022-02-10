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
		m_pImages = nullptr;
		m_pWrites = nullptr;
	}

	DescriptorSet_ps::~DescriptorSet_ps()
	{
		if (m_pImages)
		{
			vdelete[] m_pImages;
			m_pImages = nullptr;
		}
		if (m_pWrites)
		{
			vdelete[] m_pWrites;
			m_pWrites = nullptr;
		}
	}

	void DescriptorSet_ps::Init(GFXDevice* pDevice, DescriptorSetLayout* pLayout)
	{
		m_uBuffers = GFX_NUM_DYN_BUFF;
		m_uActiveSet = m_uBuffers - 1;
		for (uint32 i = 0; i < m_uBuffers; i++)
		{
			m_descSet[i] = pLayout->GetPlatform().AllocDescriptorSet(pDevice);
		}

		ASSERT(!m_pImages && !m_pWrites);

		uint32 imageCount = pLayout->GetNumberOfType(DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) + pLayout->GetNumberOfType(DESCRIPTOR_TYPE_STORAGE_IMAGE);

		m_pImages = imageCount > 0 ? vnew(usg::ALLOC_GFX_SHADER) VkDescriptorImageInfo[imageCount] : nullptr;
		m_pWrites = vnew(usg::ALLOC_GFX_SHADER) VkWriteDescriptorSet[pLayout->GetDeclarationCount()];

		usg::MemClear(m_pWrites, sizeof(VkWriteDescriptorSet) * pLayout->GetDeclarationCount());

		m_bValid = true;
	}

	void DescriptorSet_ps::Cleanup(GFXDevice* pDevice, DescriptorSetLayout* pLayout)
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

		size_t uImageIndex = 0;

		m_dynamicBuffers.clear();
		
		for (uint32 i = 0; i < pLayout->GetDeclarationCount(); i++)
		{
			m_pWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			m_pWrites[i].pNext = NULL;
			m_pWrites[i].dstSet = m_descSet[m_uActiveSet].descSet;
			m_pWrites[i].dstBinding = pLayout->GetDeclaration(i)->uBinding;
			m_pWrites[i].descriptorCount = pLayout->GetDeclaration(i)->uCount;
			switch (pLayout->GetDeclaration(i)->eDescriptorType)
			{
			case DESCRIPTOR_TYPE_CONSTANT_BUFFER:
			{
				m_pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				m_pWrites[i].pBufferInfo = &pData->pConstBuffer->GetPlatform().GetDescriptorInfo();
				break;
			}
			case DESCRIPTOR_TYPE_CONSTANT_BUFFER_DYNAMIC:
			{
				m_pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;	// Dynamic so that we can update every frame
				m_pWrites[i].pBufferInfo = &pData->pConstBuffer->GetPlatform().GetBaseDescriptorInfo();
				for (uint32 j = 0; j < m_pWrites[i].descriptorCount; j++)
				{
					m_dynamicBuffers.push_back(&pData[j].pConstBuffer->GetPlatform());
				}
				break;
			}
			case DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			{
				m_pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				m_pWrites[i].dstBinding += SAMPLER_OFFSET;
				for (uint32 j = 0; j < m_pWrites[i].descriptorCount; j++)
				{
					VkDescriptorImageInfo& image = m_pImages[uImageIndex];
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
				}

				m_pWrites[i].pImageInfo = &m_pImages[uImageIndex++];
				break;
			}
			case DESCRIPTOR_TYPE_STORAGE_IMAGE:
			{
				m_pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				m_pWrites[i].dstBinding += SAMPLER_OFFSET;	// Share the offsets of standard samplers
				for (uint32 j = 0; j < m_pWrites[i].descriptorCount; j++)
				{
					VkDescriptorImageInfo& image = m_pImages[uImageIndex];
					if (pData[j].texData.imageView.IsDefault())
					{
						image.imageView = pData[j].texData.tex->GetPlatform().GetImageView();
					}
					else
					{
						const Texture_ps& texPlat = pData[j].texData.tex->GetPlatform();
						image.imageView = texPlat.GetImageView(pDevice, pData[j].texData.imageView);
					}
					image.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				}

				m_pWrites[i].pImageInfo = &m_pImages[uImageIndex++];
				break;
			}
			default:
				ASSERT(false);
			}

			pData += pLayout->GetDeclaration(i)->uCount;
		}

		vkUpdateDescriptorSets(pDevice->GetPlatform().GetVKDevice(), pLayout->GetDeclarationCount(), m_pWrites, 0, nullptr);
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
