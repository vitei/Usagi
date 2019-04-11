/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The binary and associated states for a data defined effect
//	declaration (e.g. for models and particle effects)
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Resource/ResourceMgr.h"
#include "CustomEffectResource.h"
#include "CustomEffectDecl.h"

namespace usg
{

	struct Mapping
	{
		const char* szName;
		uint32 uValue;
	};

	static const Mapping g_constantMappings[]
	{
		{ "Material", usg::SHADER_CONSTANT_MATERIAL },
		{ "Material1", usg::SHADER_CONSTANT_MATERIAL_1 },
		{ "Colors",	usg::SHADER_CONSTANT_COLORS },
		{ "Custom", usg::SHADER_CONSTANT_CUSTOM }
	};


	CustomEffectResource::CustomEffectResource()
	{
		m_pHeader = NULL;
		m_pConstantSets = NULL;
		m_pBinary = NULL;
		m_pSamplers = NULL;
		m_pAttributes = NULL;
		for (uint32 i = 0; i < MAX_CONSTANT_SETS; i++)
		{
			m_uConstDeclOffset[i] = 0;
		}
	}

	CustomEffectResource::~CustomEffectResource()
	{
		if(m_pBinary)
		{
			mem::Free(m_pBinary);
		}
	}

	uint32 CustomEffectResource::GetBindingPoint(uint32 uSet)
	{
		const char* szName = m_pConstantSets[uSet].szName;
		for (uint32 i = 0; i < ARRAY_SIZE(g_constantMappings); i++)
		{
			if (str::Compare(szName, g_constantMappings[i].szName))
			{
				return g_constantMappings[i].uValue;
			}
		}
		ASSERT(false);
		return SHADER_CONSTANT_MATERIAL;

	}

	void CustomEffectResource::Init(GFXDevice* pDevice, const char* szFileName)
	{
		SetupHash(szFileName);
		File file(szFileName, FILE_ACCESS_READ );

		m_pBinary = mem::Alloc(MEMTYPE_STANDARD, ALLOC_OBJECT, (uint32)file.GetSize(), FILE_READ_ALIGN);
		file.Read(file.GetSize(), m_pBinary);
		m_pHeader = (CustomEffectDecl::Header*)m_pBinary;

		m_pAttributes = (CustomEffectDecl::Attribute*)(((uint8*)m_pBinary) + m_pHeader->uAttributeOffset);
		m_pSamplers = (CustomEffectDecl::Sampler*)(((uint8*)m_pBinary) + m_pHeader->uSamplerOffset);

		ASSERT(m_pHeader->uConstantSetCount < MAX_CONSTANT_SETS);

		m_pConstantSets = (CustomEffectDecl::ConstantSet*)(((uint8*)m_pBinary) + m_pHeader->uConstantSetDeclOffset);

		uint32 uTotalShaderConsts = 0;

		for (uint32 uSet = 0; uSet < m_pHeader->uConstantSetCount; uSet++)
		{
			ShaderConstantDecl* pDecl = NULL;
			CustomEffectDecl::Constant* pConstant = (CustomEffectDecl::Constant*)((uint8*)m_pBinary) + m_pConstantSets[uSet].uDeclOffset;
			
			m_uConstDeclOffset[uSet] = uTotalShaderConsts;

			uTotalShaderConsts += m_pConstantSets[uSet].uConstants + 1;
		}

		ShaderConstantDecl cap = SHADER_CONSTANT_END();
		m_pShaderConstDecl = vnew(ALLOC_OBJECT)ShaderConstantDecl[uTotalShaderConsts];
		for(uint32 uSet=0; uSet<m_pHeader->uConstantSetCount; uSet++)
		{
			ShaderConstantDecl* pDecl = &m_pShaderConstDecl[m_uConstDeclOffset[uSet]];
			CustomEffectDecl::Constant* pConstant = (CustomEffectDecl::Constant*)(((uint8*)m_pBinary)+m_pConstantSets[uSet].uDeclOffset);
			uint32 uVar = 0;
			for( ; uVar < m_pConstantSets[uSet].uConstants; uVar++ )
			{
				str::Copy(pDecl[uVar].szName, pConstant[uVar].szName, USG_IDENTIFIER_LEN);
				pDecl[uVar].eType = (ConstantType)pConstant[uVar].eConstantType;
				pDecl[uVar].uiCount = pConstant[uVar].uiCount;
				pDecl[uVar].uiOffset = pConstant[uVar].uiOffset;
				pDecl[uVar].uiSize = 0;
				pDecl[uVar].pSubDecl = NULL;
			}
			// Cap off this shader declaration
			usg::MemCpy(&pDecl[uVar], &cap, sizeof(ShaderConstantDecl));

		}

	}

	uint32 CustomEffectResource::GetAttribBinding(const char* szAttrib) const
	{
		for(uint32 i=0; i<m_pHeader->uAttributeCount; i++)
		{
			if( str::Compare(szAttrib, m_pAttributes[i].hint) )
			{
				return m_pAttributes[i].uIndex;
			}
		}
		return USG_INVALID_ID;
	}

	uint32 CustomEffectResource::GetAttribCount() const
	{
		return m_pHeader->uAttributeCount;
	}

	const CustomEffectDecl::Attribute* CustomEffectResource::GetAttribute(uint32 uIndex) const
	{
		if (uIndex >= m_pHeader->uAttributeCount)
		{
			return nullptr;
		}
		return &m_pAttributes[uIndex];
	}

	uint32 CustomEffectResource::GetSamplerBinding(const char* szSampler) const
	{
		for(uint32 i=0; i<m_pHeader->uSamplerCount; i++)
		{
			if( str::Compare(szSampler, m_pSamplers[i].hint) )
			{
				return i;
			}
		}
		return USG_INVALID_ID;
	}


	uint32 CustomEffectResource::GetConstantSetCount() const
	{
		return m_pHeader->uConstantSetCount;
	}

	uint32 CustomEffectResource::GetConstantCount(uint32 uSet) const
	{
		ASSERT(uSet<m_pHeader->uConstantSetCount);
		return m_pConstantSets[uSet].uConstants;	
	}

	const CustomEffectDecl::Constant* CustomEffectResource::GetConstant(uint32 uSet, uint32 uAttrib) const
	{
		ASSERT(uSet<m_pHeader->uConstantSetCount);
		CustomEffectDecl::Constant* pConstant = (CustomEffectDecl::Constant*)(((uint8*)m_pBinary)+m_pConstantSets[uSet].uDeclOffset);
		ASSERT(uAttrib<m_pConstantSets[uSet].uConstants);
		return &pConstant[uAttrib];
	}


	void* CustomEffectResource::GetDefaultData(uint32 uSet) const
	{
		ASSERT(uSet<m_pHeader->uConstantSetCount);
		return ((uint8*)(m_pBinary) + m_pConstantSets[uSet].uDataOffset);
	}

	const char* CustomEffectResource::GetDefaultTexture(uint32 uSamplerBinding)
	{
		for (uint32 i = 0; i < m_pHeader->uSamplerCount; i++)
		{
			if (m_pSamplers[i].uIndex == uSamplerBinding)
			{
				return m_pSamplers[i].texName;
			}
		}
		return nullptr;
	}

	const char* CustomEffectResource::GetEffectName() const
	{
		return m_pHeader->effectName;
	}

	const char* CustomEffectResource::GetDepthEffectName() const
	{
		return m_pHeader->shadowEffectName;
	}

	const char* CustomEffectResource::GetDeferredEffectName() const
	{
		return m_pHeader->deferredEffectName;
	}

	const char* CustomEffectResource::GetOmniDepthEffectName() const
	{
		return m_pHeader->omniShadowEffectName;
	}

}

