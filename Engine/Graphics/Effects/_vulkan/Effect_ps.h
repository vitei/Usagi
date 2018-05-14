/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_EFFECT_PS_
#define _USG_GRAPHICS_PC_EFFECT_PS_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class GFXDevice;
typedef struct _EffectPak EffectPak;

class Effect_ps
{
public:
	Effect_ps();
	~Effect_ps();

	void Init(GFXDevice* pDevice, const char* szEffectName);
	bool Init(GFXDevice* pDevice, const EffectPak& pak, const void* pData, const char* szPackPath) { ASSERT(false); return false; }	// Not yet implemented on PC
	void CleanUp(GFXDevice* pDevice);

	uint32 GetStageCount() const { return m_uStageCount; }
	const VkPipelineShaderStageCreateInfo* GetShaderStageCreateInfo() const { return m_stageCreateInfo; }

private:
	void GetShaderNames(const char* effectName, U8String &vsOut, U8String &psOut, U8String &gsOut);
	VkPipelineShaderStageCreateInfo LoadShader(GFXDevice* pDevice, const U8String &fileName, VkShaderStageFlagBits stage);

	VkPipelineShaderStageCreateInfo m_stageCreateInfo[SHADER_TYPE_COUNT];
	uint32							m_uStageCount;

	PRIVATIZE_COPY(Effect_ps)
};

}

#endif