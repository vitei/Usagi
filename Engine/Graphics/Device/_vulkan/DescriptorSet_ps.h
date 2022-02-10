/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_VULKAN_DESCRIPTOR_SET_PS_H_
#define _USG_GRAPHICS_DEVICE_VULKAN_DESCRIPTOR_SET_PS_H_

#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/DescriptorSetLayout.h"
#include "Engine/Core/stl/vector.h"
#include <vulkan/vulkan.h>

namespace usg {

class DescriptorSetLayout;
class GFXDevice;
struct DescriptorData;
class ConstantSet_ps;

class DescriptorSet_ps
{
public:
	DescriptorSet_ps();
	~DescriptorSet_ps();

	// Set up the defaults
	void Init(GFXDevice* pDevice, DescriptorSetLayout* pLayout);
	void Cleanup(GFXDevice* pDevice, DescriptorSetLayout* pLayout);

	void UpdateDescriptors(GFXDevice* pDevice, const DescriptorSetLayout* pLayout, const DescriptorData* pData, bool bDoubleUpdate);

	// PS functions
	void Bind(VkCommandBuffer buffer, VkPipelineLayout layout, uint32 uSlot) const;
private:
	bool				m_bValid;
	DescriptorAlloc_ps	m_descSet[GFX_NUM_DYN_BUFF];
	uint32				m_uActiveSet;
	uint32				m_uBuffers;

	vector<const ConstantSet_ps*>	m_dynamicBuffers;
	struct VkDescriptorImageInfo*	m_pImages;
	struct VkWriteDescriptorSet*	m_pWrites;
};

}


#endif

