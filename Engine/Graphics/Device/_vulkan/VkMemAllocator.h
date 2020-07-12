/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once
#include "Engine/Memory/MemAllocator.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class VkMemAllocator : public MemAllocator
{
	typedef MemAllocator Inherited;
public:
	VkMemAllocator() : m_memory(nullptr), m_uPoolIndex(0), m_pMemoryMap(nullptr), m_bDynamicCPUMap(false) {}
	virtual ~VkMemAllocator() {}

	void Init(uint32 uPoolIndex, uint32 uSize, uint32 uAlign, bool bDynamicCPUMap)
	{
		m_uPoolIndex = uPoolIndex;
		m_bDynamicCPUMap = bDynamicCPUMap;
		Inherited::Init(uSize, uAlign, true); 
	}

	void SetMemory(VkDeviceMemory memory, void* pMemMap) { m_memory = memory;  m_pMemoryMap = pMemMap; }
	// The offset is already applied
	void* GetMappedMemory() const { return m_pMemoryMap; }

	VkDeviceMemory GetMemory() { return m_memory; }
	VkDeviceSize GetMemOffset() { return (VkDeviceSize)GetData(); }

	uint32 GetPoolId() const { return m_uPoolIndex; }
	bool NeedsDynamicCPUMap() const { return m_bDynamicCPUMap; }

	// Not using callbacks atm
	virtual void AllocatedCallback(void* pMem) override {}
	virtual void ReleasedCallback() override {}

private:
	VkDeviceMemory	m_memory;
	uint32			m_uPoolIndex;
	void*			m_pMemoryMap;
	bool			m_bDynamicCPUMap;
};

}

