/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_SAMPLER_H
#define _USG_GRAPHICS_PC_SAMPLER_H

#include "Engine/Graphics/Device/RenderState.h"
#include <vulkan/vulkan.h>

namespace usg {

class Sampler
{
public:
	Sampler();
	~Sampler();

	void Init(GFXDevice* pDevice, const SamplerDecl &decl, uint32 uId);
	void CleanUp(GFXDevice* pDevice);

	// PS specific
	VkSampler GetSampler() const { return m_sampler; }
private:
	VkSampler m_sampler;
};

}

#endif