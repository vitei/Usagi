/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/PakDecl.h"
#include "Engine/Resource/PakFile.h"
#include "Engine/Graphics/Effects/Shader.h"
#include "Engine/Resource/FileDependencies.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Effects, Effect_ps.h)


namespace usg {

	static uint32 g_usageCRC[] =
	{
		utl::CRC32("vertex_shader"),
		utl::CRC32("fragment_shader"),
		utl::CRC32("geometry_shader"),
		utl::CRC32("tessellation_control_shader"),
		utl::CRC32("tessellation_evaluation_shader"),
	};

	static_assert(ARRAY_SIZE(g_usageCRC) == (memsize)ShaderType::COUNT, "Incorrect number of shader stages defined");

	static const VkShaderStageFlagBits g_shaderStages[]
	{
		VK_SHADER_STAGE_VERTEX_BIT,		// ShaderType::VS
		VK_SHADER_STAGE_FRAGMENT_BIT,	// ShaderType::PS
		VK_SHADER_STAGE_GEOMETRY_BIT,	// ShaderType::GS
		VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, // ShaderType::TC
		VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT // ShaderType::TE
	};
	
	static_assert(ARRAY_SIZE(g_shaderStages) == (memsize)ShaderType::COUNT, "Incorrect number of shader stages defined");


	Effect_ps::Effect_ps()
	{
		m_uStageCount = 0;
	}
	
	Effect_ps::~Effect_ps()
	{

	}

	void Effect_ps::Cleanup(GFXDevice* pDevice)
	{

	}


	bool Effect_ps::Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const FileDependencies* pDependencies, const void* pData, uint32 uDataSize)
	{
		m_uStageCount = 0;
		for (uint32 i = 0; i < (uint32)ShaderType::COUNT; i++)
		{
			BaseResHandle resource = pDependencies->GetDependencyByUsageCRC(g_usageCRC[i]);
			ASSERT(!resource || resource->GetResourceType() == ResourceType::SHADER);
			if(resource)
			{
				Shader* pShader = (Shader*)resource.get();
				VkPipelineShaderStageCreateInfo shaderStage = {};
				shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStage.pName = "main";
				shaderStage.module = pShader->GetPlatform().GetShaderModule();
				shaderStage.stage = g_shaderStages[i];
				m_stageCreateInfo[m_uStageCount++] = shaderStage;
			}
		}
		return true;
	}
}

