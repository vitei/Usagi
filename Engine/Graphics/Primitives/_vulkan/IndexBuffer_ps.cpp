/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Primitives, IndexBuffer_ps.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)

namespace usg {


IndexBuffer_ps::IndexBuffer_ps() :
m_uActiveIBO(0), m_uBufferCount(0)
{
}

IndexBuffer_ps::~IndexBuffer_ps()
{
	
}


void IndexBuffer_ps::Init(GFXDevice* pDevice, const void* pIndices, uint32 uCount, uint32 uIndexSize, bool bStatic, GPULocation eLocation)
{
	m_uIndexSize = uIndexSize;
	switch (uIndexSize)
	{
	case 2:
		m_eIndexType = VK_INDEX_TYPE_UINT16;
		break;
	case 4:
		m_eIndexType = VK_INDEX_TYPE_UINT32;
		break;
	default:
		ASSERT(false);
	}

	m_uBufferCount = eLocation == GPU_USAGE_STATIC ? 1 : GFX_NUM_DYN_BUFF;
	VkDevice& deviceVK = pDevice->GetPlatform().GetVKDevice();

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = uCount * uIndexSize;
	buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	buf_info.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;

	VkMemoryRequirements mem_reqs;

	VkResult err;
	for (uint32 i = 0; i < m_uBufferCount; i++)
	{
		err = vkCreateBuffer(deviceVK, &buf_info, NULL, &m_buffer[i]);
		ASSERT(!err);

		vkGetBufferMemoryRequirements(deviceVK, m_buffer[i], &mem_reqs);
		ASSERT(!err);

		mem_alloc.allocationSize = mem_reqs.size;
		mem_alloc.memoryTypeIndex = pDevice->GetPlatform().GetMemoryTypeIndex(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		err = vkAllocateMemory(deviceVK, &mem_alloc, NULL, &m_mem[i]);
		ASSERT(!err);
	}

	SetContents(pDevice, pIndices, uCount);

	for (uint32 i = 0; i < m_uBufferCount; i++)
	{
		// FIXME: Don't have 3 buffers, just use the offset
		err = vkBindBufferMemory(deviceVK, m_buffer[i], m_mem[i], 0);
	}
}

void IndexBuffer_ps::CleanUp(GFXDevice* pDevice)
{
	VkDevice& deviceVK = pDevice->GetPlatform().GetVKDevice();

	for (uint32 i=0; i<m_uBufferCount; i++)
	{
		vkDestroyBuffer(deviceVK, m_buffer[i], nullptr);
		vkFreeMemory(deviceVK, m_mem[i], nullptr);
	}
}

void IndexBuffer_ps::SetContents(GFXDevice* pDevice, const void* pData, uint32 uIndexCount)
{
	m_uActiveIBO = (m_uActiveIBO + 1) % m_uBufferCount;
	void *pDest;
	VkResult err = vkMapMemory(pDevice->GetPlatform().GetVKDevice(), m_mem[m_uActiveIBO], 0, uIndexCount*m_uIndexSize, 0, &pDest);
      
	ASSERT(!err);

	memcpy(pDest, pData, uIndexCount*m_uIndexSize);

	vkUnmapMemory(pDevice->GetPlatform().GetVKDevice(), m_mem[m_uActiveIBO]);
}

}
