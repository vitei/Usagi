/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: VkGPUHeap, for now riding off the old render target memory
//	re-use code to reduce the number of individual allocations
*****************************************************************************/
#pragma once

#include "Engine/Memory/MemUtil.h"
#include "Engine/Memory/MemAllocator.h"
#include "Engine/Memory/GPUHeap.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class VkMemAllocator;

class VkGPUHeap : protected GPUHeap
{
	typedef GPUHeap Inherited;
public:
	VkGPUHeap() : m_memory(nullptr), m_pMemoryMap(nullptr) {}

	void AllocData(VkDevice device, uint32 uMemTypeIdx, VkDeviceSize size, bool bMapMemory);
	void FreeData(VkDevice device);

	// Override
	void AddAllocator(VkMemAllocator* pAllocator);
	void RemoveAllocator(VkMemAllocator* pAllocator);
	bool CanAllocate(VkMemAllocator* pAllocator) { return Inherited::CanAllocate((MemAllocator*)pAllocator); }
	bool IsDynamic() { return m_pMemoryMap != nullptr; }
	VkDeviceMemory GetMemory() const { return m_memory; }
private:
	VkDeviceMemory		m_memory;
	void*				m_pMemoryMap;
};

}


