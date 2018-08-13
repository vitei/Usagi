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

	// FIXME: Remove this old code, just a first pass so that we can take the OpenGL code as it has been defined
	void Effect_ps::GetShaderNames(const char* effectName, U8String &vsOut, U8String &psOut, U8String &gsOut)
	{
		U8String sourceFX = effectName;
		U8String sourcePath("shaders/");
		sourceFX += ".fx";

		File defFile(sourceFX.CStr());

		uint32 uPos = 0;
		char* szText;
		ScratchObj<char> scratchText(szText, (uint32)defFile.GetSize() + 1, FILE_READ_ALIGN);

		defFile.Read(defFile.GetSize(), szText);
		szText[defFile.GetSize()] = '\0';

		while (uPos < defFile.GetSize())
		{
			if (str::CompareLen(&szText[uPos], "PS", 2))
			{
				uPos += 3;
				psOut.CopySingleLine(&szText[uPos]);
				uPos += psOut.Length();
				psOut = sourcePath + psOut;
				continue;
			}

			if (str::CompareLen(&szText[uPos], "VS", 2))
			{
				uPos += 3;
				vsOut.CopySingleLine(&szText[uPos]);
				uPos += vsOut.Length();
				vsOut = sourcePath + vsOut;
				continue;
			}

			if (str::CompareLen(&szText[uPos], "GS", 2))
			{
				uPos += 3;
				gsOut.CopySingleLine(&szText[uPos]);
				uPos += gsOut.Length();
				gsOut = sourcePath + gsOut;
				continue;
			}

			uPos++;
		}
	}


	void Effect_ps::Init(GFXDevice* pDevice, const char* szEffectName)
	{
		ASSERT(false);
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

