/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/Memutil.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/DescriptorSetLayout.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)


const int g_allocGroupSize = 64;

namespace usg {

	VkDescriptorType g_descriptorTypes[] =
	{
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 0,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 			// DESCRIPTOR_TYPE_CONSTANT_BUFFER,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	// DESCRIPTOR_TYPE_CONSTANT_BUFFER_DYNAMIC,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE			// DESCRIPTOR_TYPE_STORAGE_IMAGE
	};

	VkShaderStageFlags GetShaderFlags(ShaderTypeFlags eFlags)
	{
		VkShaderStageFlags flagsOut= 0;
		if(eFlags & SHADER_FLAG_VERTEX)
			flagsOut |= VK_SHADER_STAGE_VERTEX_BIT;
		if(eFlags & SHADER_FLAG_GEOMETRY)
			flagsOut |= VK_SHADER_STAGE_GEOMETRY_BIT;
		if(eFlags & SHADER_FLAG_PIXEL)
			flagsOut |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if(eFlags & SHADER_FLAG_TCONTROL)
			flagsOut |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		if(eFlags & SHADER_FLAG_TEVAL)
			flagsOut |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

		return flagsOut;

	}	

	DescriptorSetLayout_ps::DescriptorSetLayout_ps()
	{

	}

	DescriptorSetLayout_ps::~DescriptorSetLayout_ps()
	{

	}


	void DescriptorSetLayout_ps::UpdateFreeList(GFXDevice* pDevice, DescriptorSetLayout_ps::Allocator& allocator)
	{
		uint32 uFrame = (int)pDevice->GetFrameCount();
		uint32 uClearFrame = uFrame > GFX_NUM_DYN_BUFF ? uFrame - GFX_NUM_DYN_BUFF : UINT_MAX;

		while (!allocator.pendingDeleteFrames.empty())
		{
			if (allocator.pendingDeleteFrames.front() < uClearFrame || allocator.pendingDeleteFrames.front() > uFrame)
			{
				allocator.pendingDeleteFrames.pop();
				ASSERT(allocator.uAllocations > 0);
				allocator.uAllocations--;
			}
			else
			{
				return;
			}
		}
	}

	DescriptorAlloc_ps DescriptorSetLayout_ps::AllocDescriptorSet(GFXDevice* pDevice)
	{
		VkResult eResult;
		uint32 uAllocId = USG_INVALID_ID;
		for (uint32 i = 0; i < m_allocators.size(); i++)
		{
			if (m_allocators[i].pendingDeleteFrames.size() > 0)
			{
				UpdateFreeList(pDevice, m_allocators[i]);
			}
			if (m_allocators[i].uAllocations < g_allocGroupSize)
			{
				uAllocId = i;
				break;
			}
		}

		if (uAllocId == USG_INVALID_ID)
		{
			VkDescriptorPoolCreateInfo	poolCreateInfo = {};
			poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolCreateInfo.pNext = NULL;
			poolCreateInfo.poolSizeCount = (uint32)m_poolSize.size();
			poolCreateInfo.pPoolSizes = m_poolSize.data();
			poolCreateInfo.maxSets = g_allocGroupSize;
			poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

			Allocator alloc;
			alloc.uAllocations = 0;
			eResult = vkCreateDescriptorPool(pDevice->GetPlatform().GetVKDevice(), &poolCreateInfo, pDevice->GetPlatform().GetAllocCallbacks(), &alloc.pool);
			ASSERT(eResult == VK_SUCCESS);
			uAllocId = (uint32)m_allocators.size();
			m_allocators.push_back(alloc);
		}

		Allocator* pAlloc = &m_allocators[uAllocId];
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.descriptorPool = pAlloc->pool;
		allocInfo.pSetLayouts = &m_layout;
		allocInfo.descriptorSetCount = 1;

		DescriptorAlloc_ps allocRet;
		allocRet.uPoolIndex = uAllocId;
		eResult = vkAllocateDescriptorSets(pDevice->GetPlatform().GetVKDevice(), &allocInfo, &allocRet.descSet);
		ASSERT(eResult == VK_SUCCESS);
		pAlloc->uAllocations++;
		
		return allocRet;
	}

	void DescriptorSetLayout_ps::FreeDescriptorSet(GFXDevice* pDevice, DescriptorAlloc_ps& descAlloc)
	{
		ASSERT(descAlloc.uPoolIndex < m_allocators.size());
		if (descAlloc.uPoolIndex < m_allocators.size())
		{
			Allocator* pAlloc = &m_allocators[descAlloc.uPoolIndex];
			pDevice->GetPlatform().ReqDestroyDescriptorSet(pAlloc->pool, descAlloc.descSet);
			descAlloc.uPoolIndex = USG_INVALID_ID;
			descAlloc.descSet = nullptr;
			pAlloc->pendingDeleteFrames.push( pDevice->GetFrameCount() );
		}
	}

	void DescriptorSetLayout_ps::Init(GFXDevice* pDevice, const class DescriptorSetLayout &parent)
	{
		GFXDevice_ps& devicePS = pDevice->GetPlatform();

		VkDescriptorSetLayoutBinding* pBindings;
		ScratchObj<VkDescriptorSetLayoutBinding> layoutScratch(pBindings, parent.GetDeclarationCount());

		m_poolSize.resize(parent.GetDeclarationCount());


		for(uint32 i=0; i<parent.GetDeclarationCount(); i++)		
		{
			memset(&pBindings[i], 0, sizeof(VkDescriptorSetLayoutBinding));
			const DescriptorDeclaration* pDecl = parent.GetDeclaration(i);
			pBindings[i].descriptorType = g_descriptorTypes[pDecl->eDescriptorType];
			pBindings[i].stageFlags = GetShaderFlags(pDecl->shaderType);
			pBindings[i].binding = pDecl->uBinding;
			if (pDecl->eDescriptorType == DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || pDecl->eDescriptorType == DESCRIPTOR_TYPE_STORAGE_IMAGE)
			{
				pBindings[i].binding += SAMPLER_OFFSET;
			}
			pBindings[i].descriptorCount = pDecl->uCount;
			m_poolSize[i].type = pBindings[i].descriptorType;
			m_poolSize[i].descriptorCount = pDecl->uCount * g_allocGroupSize;
		}

		VkDescriptorSetLayoutCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.pNext = NULL;
		create_info.pBindings = pBindings;
		create_info.bindingCount = parent.GetDeclarationCount();

		VkResult eResult = vkCreateDescriptorSetLayout(devicePS.GetVKDevice(), &create_info, nullptr, &m_layout);
		ASSERT(eResult == VK_SUCCESS);

	}


	void DescriptorSetLayout_ps::Cleanup(GFXDevice* pDevice)
	{
		for (uint32 i = 0; i < m_allocators.size(); i++)
		{
			vkDestroyDescriptorPool(pDevice->GetPlatform().GetVKDevice(), m_allocators[i].pool, pDevice->GetPlatform().GetAllocCallbacks());
		}

		vkDestroyDescriptorSetLayout(pDevice->GetPlatform().GetVKDevice(), m_layout, pDevice->GetPlatform().GetAllocCallbacks());
		m_allocators.clear();
	}

}
