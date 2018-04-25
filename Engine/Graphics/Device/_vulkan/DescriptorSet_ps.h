/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_VULKAN_DESCRIPTOR_SET_PS_H_
#define _USG_GRAPHICS_DEVICE_VULKAN_DESCRIPTOR_SET_PS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/DescriptorSetLayout.h"
#include "Engine/Core/stl/vector.h"
#include <vulkan/vulkan.h>

namespace usg {

class DescriptorSetLayout;
class GFXDevice;
struct DescriptorData;

class DescriptorSet_ps
{
public:
	DescriptorSet_ps();
	~DescriptorSet_ps();

	// Set up the defaults
	void Init(GFXDevice* pDevice, DescriptorSetLayout* pLayout);
	void CleanUp(GFXDevice* pDevice, DescriptorSetLayout* pLayout);

	void UpdateDescriptors(GFXDevice* pDevice, const DescriptorSetLayout* pLayout, const DescriptorData* pData);
	void NotifyBufferChanged(uint32 uLayoutIndex, uint32 uSubIndex, const DescriptorData* pData);

	// PS functions
	void Bind(VkCommandBuffer buffer, VkPipelineLayout layout, uint32 uSlot) const;
private:
	bool				m_bValid;
	DescriptorAlloc_ps	m_descSet;

	vector<const ConstantSet_ps*> m_dynamicBuffers;

};

}


#endif

