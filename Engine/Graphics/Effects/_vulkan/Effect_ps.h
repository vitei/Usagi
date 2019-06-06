/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_EFFECT_PS_
#define _USG_GRAPHICS_PC_EFFECT_PS_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include "Engine/Resource/PakDecl.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class GFXDevice;
class PakFile;

class Effect_ps
{
public:
	Effect_ps();
	~Effect_ps();

	bool Init(GFXDevice* pDevice, PakFile* pakFile, const PakFileDecl::FileInfo* pFileHeader, const void* pData, uint32 uDataSize);
	void CleanUp(GFXDevice* pDevice);

	uint32 GetStageCount() const { return m_uStageCount; }
	const VkPipelineShaderStageCreateInfo* GetShaderStageCreateInfo() const { return m_stageCreateInfo; }

private:
	void GetShaderNames(const char* effectName, U8String &vsOut, U8String &psOut, U8String &gsOut);

	VkPipelineShaderStageCreateInfo m_stageCreateInfo[ShaderType::COUNT];
	uint32							m_uStageCount;

	PRIVATIZE_COPY(Effect_ps)
};

}

#endif