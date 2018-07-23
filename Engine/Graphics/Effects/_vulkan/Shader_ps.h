/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_SHADER_PS_
#define _USG_GRAPHICS_PC_SHADER_PS_
#include "Engine/Common/Common.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class GFXDevice;

class Shader_ps
{
public:
	Shader_ps();
	~Shader_ps();

	void Init(GFXDevice* pDevice, const char* szEffectName) { ASSERT(false); }
	bool Init(GFXDevice* pDevice, const void* pData, uint32 uDataSize);
	void CleanUp(GFXDevice* pDevice);

	// PS
	VkShaderModule GetShaderModule() { return m_shaderModule; }

private:
	VkShaderModule			m_shaderModule;

	PRIVATIZE_COPY(Shader_ps)
};

}

#endif