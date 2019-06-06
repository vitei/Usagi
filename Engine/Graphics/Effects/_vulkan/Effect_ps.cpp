/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/PakDecl.h"
#include "Engine/Resource/PakFile.h"
#include "Engine/Graphics/Effects/Shader.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Effects, Effect_ps.h)


namespace usg {

	static const VkShaderStageFlagBits g_shaderStages[]
	{
		VK_SHADER_STAGE_VERTEX_BIT,		// ShaderType::VS
		VK_SHADER_STAGE_FRAGMENT_BIT,	// ShaderType::PS
		VK_SHADER_STAGE_GEOMETRY_BIT	// ShaderType::GS
	};

	Effect_ps::Effect_ps()
	{
		m_uStageCount = 0;
	}
	
	Effect_ps::~Effect_ps()
	{

	}

	void Effect_ps::CleanUp(GFXDevice* pDevice)
	{

	}


	bool Effect_ps::Init(GFXDevice* pDevice, PakFile* pakFile, const PakFileDecl::FileInfo* pFileHeader, const void* pData, uint32 uDataSize)
	{
		const PakFileDecl::EffectEntry* pEffectHdr = PakFileDecl::GetCustomHeader<PakFileDecl::EffectEntry>(pFileHeader);
		
		m_uStageCount = 0;
		for (uint32 i = 0; i < (uint32)ShaderType::COUNT; i++)
		{
			if (pEffectHdr->CRC[i] != 0)
			{
				ResourceBase* pResourceBase = pakFile->GetResource(pEffectHdr->CRC[i]);
				ASSERT(pResourceBase && pResourceBase->GetResourceType() == ResourceType::SHADER);
				if(pResourceBase)
				{
					Shader* pShader = (Shader*)pResourceBase;
					VkPipelineShaderStageCreateInfo shaderStage = {};
					shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					shaderStage.pName = "main";
					shaderStage.module = pShader->GetPlatform().GetShaderModule();
					shaderStage.stage = g_shaderStages[i];
					m_stageCreateInfo[m_uStageCount++] = shaderStage;
				}
				else
				{
					return false;
				}
			}
		}
		return true;
	}
}

