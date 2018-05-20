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

		vkGetBufferMemoryRequirements(deviceVK, m_buffer[i], &mem_reqs);
		ASSERT(!err);

		mem_alloc.allocationSize = mem_reqs.size;
		mem_alloc.memoryTypeIndex = pDevice->GetPlatform().GetMemoryTypeIndex(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		err = vkAllocateMemory(deviceVK, &mem_alloc, NULL, &m_mem[i]);
		ASSERT(!err);
	}

	if (pVerts)
	{
		SetContents(pDevice, pVerts, uDataSize);
	}

	for (uint32 i = 0; i < m_uBufferCount; i++)
	{
		// FIXME: Don't have 3 buffers, just use the offset
		err = vkBindBufferMemory(deviceVK, m_buffer[i], m_mem[i], 0);
	}
}


void VertexBuffer_ps::CleanUp(GFXDevice* pDevice)
{
	ASSERT(false);
}


void* VertexBuffer_ps::LockData(GFXDevice* pDevice, uint32 uSize)
{
	m_uActiveVBO = (m_uActiveVBO + 1) % m_uBufferCount;
	VkResult err;
	void* pData;
	err = vkMapMemory(pDevice->GetPlatform().GetVKDevice(), m_mem[m_uActiveVBO], 0, uSize, 0, &pData);
	ASSERT(!err);

	return pData;
}

void VertexBuffer_ps::UnlockData(GFXDevice* pDevice, void* pData, uint32 uElements)
{
	vkUnmapMemory(pDevice->GetPlatform().GetVKDevice(), m_mem[m_uActiveVBO]);
}


void VertexBuffer_ps::SetContents(GFXDevice* pDevice, const void* const pData, uint32 uSize)
{
	m_uActiveVBO = (m_uActiveVBO + 1) % m_uBufferCount;
	void* pDest = LockData(pDevice, uSize);

	memcpy(pDest, pData, uSize);

	UnlockData(pDevice, pDest, uSize);
}

}
