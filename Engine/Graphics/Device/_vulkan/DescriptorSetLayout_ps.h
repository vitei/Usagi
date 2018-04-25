/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_LAYOUT_PS_H_
#define _USG_GRAPHICS_DEVICE_DESCRIPTOR_SET_LAYOUT_PS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include <vulkan/vulkan.h>
#include "Engine/Core/Containers/vector.h"

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
		uint32			 uAllocations;
	};

	vector<Allocator> m_allocators;
	
};

}


#endif

