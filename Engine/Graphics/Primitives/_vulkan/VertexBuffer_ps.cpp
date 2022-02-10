/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Primitives, VertexBuffer_ps.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)

namespace usg {

VertexBuffer_ps::VertexBuffer_ps()
{
	m_uBufferCount = 0;
	m_uActiveVBO = 0;
}

VertexBuffer_ps::~VertexBuffer_ps()
{
	
}

void VertexBuffer_ps::Init(GFXDevice* pDevice, const void* const pVerts, uint32 uDataSize, GPUUsage eUpdateType, GPULocation eLocation)
{
	m_uBufferCount = eUpdateType == GPU_USAGE_STATIC ? 1 : GFX_NUM_DYN_BUFF;
	VkDevice& deviceVK = pDevice->GetPlatform().GetVKDevice();

	VkBufferCreateInfo buf_info = { };
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = uDataSize;
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
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
	}

	vkGetBufferMemoryRequirements(deviceVK, m_buffer[0], &mem_reqs);
	ASSERT(!err);

	mem_alloc.allocationSize = mem_reqs.size;
	mem_alloc.memoryTypeIndex = pDevice->GetPlatform().GetMemoryTypeIndex(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	VkDeviceSize size = mem_alloc.allocationSize;
	m_uBufferSize = AlignSizeUp(size, mem_reqs.alignment);
	if (m_uBufferCount > 1)
	{
		size = m_uBufferSize * m_uBufferCount;
	}

	m_memoryAlloc.Init(mem_alloc.memoryTypeIndex, (uint32)size, (uint32)mem_reqs.alignment, m_uBufferCount > 1);
	pDevice->GetPlatform().AllocateMemory(&m_memoryAlloc);

	if (pVerts)
	{
		SetContents(pDevice, pVerts, uDataSize);
	}

	VkDeviceSize uOffset = 0;
	for (uint32 i = 0; i < m_uBufferCount; i++)
	{
		err = vkBindBufferMemory(deviceVK, m_buffer[i], m_memoryAlloc.GetMemory(), m_memoryAlloc.GetMemOffset() + uOffset);
		uOffset += m_uBufferSize;
	}
}


void VertexBuffer_ps::Cleanup(GFXDevice* pDevice)
{
	VkDevice& deviceVK = pDevice->GetPlatform().GetVKDevice();

	for (uint32 i = 0; i < m_uBufferCount; i++)
	{
		pDevice->GetPlatform().ReqDestroyBuffer(m_buffer[i]);
	}

	pDevice->GetPlatform().FreeMemory(&m_memoryAlloc);

	m_uBufferCount = 0;
}


void* VertexBuffer_ps::LockData(GFXDevice* pDevice, uint32 uSize)
{
	m_uActiveVBO = (m_uActiveVBO + 1) % m_uBufferCount;
	void* pData;
	if(m_uBufferCount > 1)
	{
		pData = ((uint8*)m_memoryAlloc.GetMappedMemory() + (m_uBufferSize * m_uActiveVBO));
	}
	else
	{
		VkResult err = vkMapMemory(pDevice->GetPlatform().GetVKDevice(), m_memoryAlloc.GetMemory(), m_memoryAlloc.GetMemOffset(), uSize, 0, &pData);
		ASSERT(!err);
	}

	return pData;
}

void VertexBuffer_ps::UnlockData(GFXDevice* pDevice, void* pData, uint32 uElements)
{
	if(m_uBufferCount == 1)
	{
		vkUnmapMemory(pDevice->GetPlatform().GetVKDevice(), m_memoryAlloc.GetMemory());
	}
}


void VertexBuffer_ps::SetContents(GFXDevice* pDevice, const void* const pData, uint32 uSize)
{
	void* pDest = LockData(pDevice, uSize);

	memcpy(pDest, pData, uSize);

	UnlockData(pDevice, pDest, uSize);
}

}
