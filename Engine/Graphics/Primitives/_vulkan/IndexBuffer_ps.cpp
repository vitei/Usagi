/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Primitives, IndexBuffer_ps.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)

namespace usg {


IndexBuffer_ps::IndexBuffer_ps() :
m_uActiveIBO(0), m_uBufferCount(0), m_uBufferSize(0), m_bUseStaging(false)
{
}

IndexBuffer_ps::~IndexBuffer_ps()
{
	
}


void IndexBuffer_ps::CreateStagingBuffer(GFXDevice* pDevice, uint32 uDataSize)
{
	VkDevice& deviceVK = pDevice->GetPlatform().GetVKDevice();

	VkBufferCreateInfo buf_info = { };
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = uDataSize;
	buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buf_info.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;

	VkMemoryRequirements mem_reqs;

	VkResult err;
	for (uint32 i = 0; i < m_uBufferCount; i++)
	{
		err = vkCreateBuffer(deviceVK, &buf_info, NULL, &m_stagingBuffer[i]);
		ASSERT(!err);
	}

	vkGetBufferMemoryRequirements(deviceVK, m_stagingBuffer[0], &mem_reqs);
	ASSERT(!err);

	mem_alloc.allocationSize = mem_reqs.size;
	mem_alloc.memoryTypeIndex = pDevice->GetPlatform().GetMemoryTypeIndex(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkDeviceSize size = mem_alloc.allocationSize;
	m_uBufferSize = AlignSizeUp(size, mem_reqs.alignment);
	if (m_uBufferCount > 1)
	{
		size = m_uBufferSize * m_uBufferCount;
	}

	m_stagingMemoryAlloc.Init(mem_alloc.memoryTypeIndex, (uint32)size, (uint32)mem_reqs.alignment, m_uBufferCount > 1);
	pDevice->GetPlatform().AllocateMemory(&m_stagingMemoryAlloc);
}

void IndexBuffer_ps::CreateFinalBuffer(GFXDevice* pDevice, uint32 uDataSize, bool bHasStaging)
{
	VkDevice& deviceVK = pDevice->GetPlatform().GetVKDevice();

	VkBufferCreateInfo buf_info = { };
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = uDataSize;
	buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if (bHasStaging)
	{
		buf_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}
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
	if (bHasStaging)
	{
		mem_alloc.memoryTypeIndex = pDevice->GetPlatform().GetMemoryTypeIndex(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}
	else
	{
		mem_alloc.memoryTypeIndex = pDevice->GetPlatform().GetMemoryTypeIndex(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	VkDeviceSize size = mem_alloc.allocationSize;
	m_uBufferSize = AlignSizeUp(size, mem_reqs.alignment);
	if (m_uBufferCount > 1)
	{
		size = m_uBufferSize * m_uBufferCount;
	}

	m_memoryAlloc.Init(mem_alloc.memoryTypeIndex, (uint32)size, (uint32)mem_reqs.alignment, m_uBufferCount > 1);
	pDevice->GetPlatform().AllocateMemory(&m_memoryAlloc);
}

void IndexBuffer_ps::CleanupStaging(GFXDevice* pDevice)
{
	if (m_bUseStaging)
	{
		VkDevice& deviceVK = pDevice->GetPlatform().GetVKDevice();

		for (uint32 i = 0; i < m_uBufferCount; i++)
		{
			pDevice->GetPlatform().ReqDestroyBuffer(m_stagingBuffer[i]);
		}

		pDevice->GetPlatform().FreeMemory(&m_stagingMemoryAlloc);
	}
	m_bUseStaging = false;
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



	m_uBufferCount = bStatic ? 1 : GFX_NUM_DYN_BUFF;

	if (eLocation == GPU_LOCATION_FASTMEM && bStatic)
	{
		// If we wanted to allow staging areas on dynamic we'd need to pass in the context on updating the vertex data
		m_bUseStaging = true;
	}
	else
	{
		m_bUseStaging = false;
	}

	if (!m_bUseStaging)
	{
		CreateFinalBuffer(pDevice, uCount * uIndexSize, false);
	}
	else
	{
		CreateStagingBuffer(pDevice, uCount * uIndexSize);
		CreateFinalBuffer(pDevice, uCount * uIndexSize, true);
	}

	SetContents(pDevice, pIndices, uCount);

	VkDevice& deviceVK = pDevice->GetPlatform().GetVKDevice();
	VkDeviceSize uOffset = 0;
	VkResult err;
	for (uint32 i = 0; i < m_uBufferCount; i++)
	{
		err = vkBindBufferMemory(deviceVK, m_buffer[i], m_memoryAlloc.GetMemory(), m_memoryAlloc.GetMemOffset() + uOffset);
		uOffset += m_uBufferSize;
	}


	if (m_bUseStaging)
	{
		uOffset = 0;
		for (uint32 i = 0; i < m_uBufferCount; i++)
		{
			err = vkBindBufferMemory(deviceVK, m_stagingBuffer[i], m_stagingMemoryAlloc.GetMemory(), m_stagingMemoryAlloc.GetMemOffset() + uOffset);
			uOffset += m_uBufferSize;
		}

		VkBufferCopy copyRegion = {};
		copyRegion.size = uCount * uIndexSize;
		VkCommandBuffer copyCmd = pDevice->GetPlatform().CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		vkCmdCopyBuffer(copyCmd, m_stagingBuffer[0], m_buffer[0], 1, &copyRegion);
		pDevice->GetPlatform().FlushCommandBuffer(copyCmd, true);
	}

	if (bStatic)
	{
		// There should be no additional requests to update this data so get the memory back
		CleanupStaging(pDevice);
	}
}

void IndexBuffer_ps::Cleanup(GFXDevice* pDevice)
{
	VkDevice& deviceVK = pDevice->GetPlatform().GetVKDevice();

	for (uint32 i = 0; i < m_uBufferCount; i++)
	{
		pDevice->GetPlatform().ReqDestroyBuffer(m_buffer[i]);
	}

	pDevice->GetPlatform().FreeMemory(&m_memoryAlloc);

	CleanupStaging(pDevice);
}

void IndexBuffer_ps::SetContents(GFXDevice* pDevice, const void* pData, uint32 uIndexCount)
{
	VkMemAllocator& allocator = m_bUseStaging ? m_stagingMemoryAlloc : m_memoryAlloc;

	m_uActiveIBO = (m_uActiveIBO + 1) % m_uBufferCount;
	void* pDest;
	if (m_uBufferCount > 1)
	{
		pDest = ((uint8*)allocator.GetMappedMemory() + (m_uBufferSize * m_uActiveIBO));
	}
	else
	{
		VkResult err = vkMapMemory(pDevice->GetPlatform().GetVKDevice(), allocator.GetMemory(), allocator.GetMemOffset(), uIndexCount * m_uIndexSize, 0, &pDest);
		ASSERT(!err);
	}

	memcpy(pDest, pData, uIndexCount*m_uIndexSize);

	if(m_uBufferCount <= 1)
	{
		vkUnmapMemory(pDevice->GetPlatform().GetVKDevice(), allocator.GetMemory());
	}
}

}
