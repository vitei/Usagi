/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Device, VkMemAllocator.h)
#include "VkGPUHeap.h"

namespace usg {

void VkGPUHeap::AllocData(VkDevice device, uint32 uMemTypeIdx, VkDeviceSize size, bool bMapMemory)
{
	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.memoryTypeIndex = uMemTypeIdx;
	memAlloc.allocationSize = size;

	VkResult eResult = vkAllocateMemory(device, &memAlloc, nullptr, &m_memory);
	ASSERT(eResult == VK_SUCCESS);
	if (eResult == VK_SUCCESS)
	{
		// FIXME: Hardcoding max allocations to 16k for now
		Init(nullptr, size, 16384);

		if(bMapMemory)
		{
			vkMapMemory(device, m_memory, 0, VK_WHOLE_SIZE, 0, &m_pMemoryMap);
		}
		else
		{
			m_pMemoryMap = nullptr;
		}
	}
}

void VkGPUHeap::FreeData(VkDevice device)
{
	if (m_pMemoryMap)
	{
		vkUnmapMemory(device, m_memory);
		m_pMemoryMap = nullptr;
	}
	if(m_memory)
	{
		vkFreeMemory(device, m_memory, nullptr);
		m_memory = nullptr;
	}
}


void VkGPUHeap::AddAllocator(VkMemAllocator* pAllocator)
{
	Inherited::AddAllocator(pAllocator);
	uint8* pMemory = nullptr;
	if (m_pMemoryMap)
	{
		pMemory = ((uint8*)m_pMemoryMap) + pAllocator->GetMemOffset();
	}
	pAllocator->SetMemory(m_memory, (void*)pMemory);
}

void VkGPUHeap::RemoveAllocator(VkMemAllocator* pAllocator)
{
	Inherited::RemoveAllocator(pAllocator);
	pAllocator->SetMemory(nullptr, nullptr);
}

}
