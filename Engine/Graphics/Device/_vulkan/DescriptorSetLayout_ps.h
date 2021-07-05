/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_LAYOUT_PS_H_
#define _USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_LAYOUT_PS_H_

#include "Engine/Graphics/Device/GFXDevice.h"
#include <vulkan/vulkan.h>
#include "Engine/Core/stl/vector.h"
#include "Engine/Core/stl/queue.h"

namespace usg {

struct DescriptorAlloc_ps
{
	uint32				uPoolIndex;
	VkDescriptorSet		descSet;
};

class DescriptorSetLayout_ps
{
public:
	DescriptorSetLayout_ps();
	~DescriptorSetLayout_ps();

	void Init(GFXDevice* pDevice, const class DescriptorSetLayout &parent);
	void Cleanup(GFXDevice* pDevice);

	VkDescriptorSetLayout GetVkLayout() const { return m_layout; }

	DescriptorAlloc_ps AllocDescriptorSet(GFXDevice* pDevice);
	void FreeDescriptorSet(GFXDevice* pDevice, DescriptorAlloc_ps& descAlloc);

private:
	PRIVATIZE_COPY(DescriptorSetLayout_ps)

	vector<VkDescriptorPoolSize>	m_poolSize;
	VkDescriptorSetLayout			m_layout;

	struct Allocator
	{
		VkDescriptorPool pool;
		queue<uint32>	 pendingDeleteFrames;
		uint32			 uAllocations;
		uint32			 uDeviceAllocId;
	};

	void UpdateFreeList(GFXDevice* pDevice, Allocator& allocator);

	vector<Allocator> m_allocators;
	
};

}


#endif

