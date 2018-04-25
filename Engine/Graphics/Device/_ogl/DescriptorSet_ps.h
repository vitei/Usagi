/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_OGL_DESCRIPTOR_SET_PS_H_
#define _USG_GRAPHICS_DEVICE_OGL_DESCRIPTOR_SET_PS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"

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
	void Init(GFXDevice* pDevice, const DescriptorSetLayout* pLayout) {}
	void CleanUp(GFXDevice* pDevice) {}

	void UpdateDescriptors(GFXDevice* pDevice, const DescriptorSetLayout* pLayout, const DescriptorData* pData) {}
	void NotifyBufferChanged(uint32 uLayoutIndex, uint32 uSubIndex, const DescriptorData* pData) {}

private:
	// PC just uses the raw data
};

}


#endif

