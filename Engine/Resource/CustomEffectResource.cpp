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
		{ "Custom", usg::SHADER_CONSTANT_CUSTOM_3 }
	};


	CustomEffectResource::CustomEffectResource() :
		ResourceBase(StaticResType)
	{
		m_pConstantSets = nullptr;
		m_pBinary = nullptr;
		m_pVertexDecl = nullptr;
		m_pAlloc = nullptr;
		m_pSamplers = nullptr;
		m_pAttributes = nullptr;
		m_pShaderConstDecl = nullptr;
		for (uint32 i = 0; i < MAX_CONSTANT_SETS; i++)
		{
			m_uConstDeclOffset[i] = 0;
		}
	}

	CustomEffectResource::~CustomEffectResource()
	{
		if(m_pAlloc)
		{
			mem::Free(m_pAlloc);
		}
		// FIXME: These should be exported as part of the file rather that seperate allocations
		if (m_pShaderConstDecl)
		{
			vdelete m_pShaderConstDecl;
		}
		if (m_pDescriptorDecl)
		{
			vdelete m_pDescriptorDecl;
		}
		if (m_pVertexDecl)
		{
			vdelete m_pVertexDecl;
		}
	}

	// FIXME: Deprecate this; binding point now specified in the declaration
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

	bool CustomEffectResource::Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const FileDependencies* pDependencies, const void* pData)
	{
		m_header = *(PakFileDecl::GetCustomHeader<CustomEffectDecl::Header>(pFileHeader));
		ASSERT(pFileHeader->uFileFlags & PakFileDecl::FILE_FLAG_KEEP_DATA);
		
		m_pBinary = pData;
		FixUpPointers(pDevice);

		return true;
	}


	void CustomEffectResource::Init(GFXDevice* pDevice, const char* szFileName)
	{
		SetupHash(szFileName);
		File file(szFileName, FILE_ACCESS_READ );

		memsize uBinarySize = file.GetSize() - sizeof(m_header);
		m_pAlloc = mem::Alloc(MEMTYPE_STANDARD, ALLOC_OBJECT, uBinarySize, FILE_READ_ALIGN);
		file.Read(sizeof(m_header), &m_header);
		file.Read(uBinarySize, m_pAlloc);
		m_pBinary = m_pAlloc;
		FixUpPointers(pDevice);
	}

	void CustomEffectResource::FixUpPointers(GFXDevice* pDevice)
	{
		m_pAttributes = (CustomEffectDecl::Attribute*)(((uint8*)m_pBinary) + m_header.uAttributeOffset);
		m_pSamplers = (CustomEffectDecl::Sampler*)(((uint8*)m_pBinary) + m_header.uSamplerOffset);

		ASSERT(m_header.uConstantSetCount < MAX_CONSTANT_SETS);

		m_pConstantSets = (CustomEffectDecl::ConstantSet*)(((uint8*)m_pBinary) + m_header.uConstantSetDeclOffset);

		uint32 uTotalShaderConsts = 0;

		for (uint32 uSet = 0; uSet < m_header.uConstantSetCount; uSet++)
		{
			ShaderConstantDecl* pDecl = nullptr;
			CustomEffectDecl::Constant* pConstant = (CustomEffectDecl::Constant*)((uint8*)m_pBinary) + m_pConstantSets[uSet].uDeclOffset;

			m_uConstDeclOffset[uSet] = uTotalShaderConsts;

			uTotalShaderConsts += m_pConstantSets[uSet].uConstants + 1;
		}

		// FIXME: Place in the binary data
		ShaderConstantDecl cap = SHADER_CONSTANT_END();
		m_pShaderConstDecl = vnew(ALLOC_OBJECT)ShaderConstantDecl[uTotalShaderConsts];
		for (uint32 uSet = 0; uSet < m_header.uConstantSetCount; uSet++)
		{
			ShaderConstantDecl* pDecl = &m_pShaderConstDecl[m_uConstDeclOffset[uSet]];
			CustomEffectDecl::Constant* pConstant = (CustomEffectDecl::Constant*)(((uint8*)m_pBinary) + m_pConstantSets[uSet].uDeclOffset);
			uint32 uVar = 0;
			for (; uVar < m_pConstantSets[uSet].uConstants; uVar++)
			{
				str::Copy(pDecl[uVar].szName, pConstant[uVar].szName, USG_IDENTIFIER_LEN);
				pDecl[uVar].eType = (ConstantType)pConstant[uVar].eConstantType;
				pDecl[uVar].uiCount = pConstant[uVar].uiCount;
				pDecl[uVar].uiOffset = pConstant[uVar].uiOffset;
				pDecl[uVar].uiSize = 0;
				pDecl[uVar].pSubDecl = nullptr;
			}
			// Cap off this shader declaration
			usg::MemCpy(&pDecl[uVar], &cap, sizeof(ShaderConstantDecl));

		}

		// FIXME: Place in the binary data
		uint32 uDescDeclCount = m_header.uConstantSetCount + m_header.uSamplerCount + 1;
		m_pDescriptorDecl = vnew(ALLOC_OBJECT)DescriptorDeclaration[uDescDeclCount];
		DescriptorDeclaration* pDescDecl = m_pDescriptorDecl;
		for (uint32 i = 0; i < m_header.uSamplerCount; i++)
		{
			pDescDecl->eDescriptorType = usg::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			pDescDecl->uCount = 1;
			pDescDecl->shaderType = usg::SHADER_FLAG_PIXEL;
			pDescDecl->uBinding = m_pSamplers[i].uIndex;
			pDescDecl++;
		}

		for (uint32 i = 0; i < m_header.uConstantSetCount; i++)
		{
			pDescDecl->eDescriptorType = usg::DESCRIPTOR_TYPE_CONSTANT_BUFFER;
			pDescDecl->uCount = 1;
			pDescDecl->shaderType = (ShaderTypeFlags)m_pConstantSets[i].uShaderSets;
			pDescDecl->uBinding = m_pConstantSets[i].uBinding;
			pDescDecl++;
		}

		*pDescDecl = DESCRIPTOR_END();

		// TODO: Once stable move this to the exporter side
		uint32 uVertexElemCnt = m_header.uAttributeCount + 1;
		m_pVertexDecl = vnew(ALLOC_OBJECT)VertexElement[uVertexElemCnt];
		VertexElement* pElement = m_pVertexDecl;
		memsize uOffset = 0;
		for (uint32 i = 0; i < m_header.uSamplerCount; i++)
		{
			// Keep the data aligned
			uOffset = AlignSizeUp(uOffset, g_uVertexElementSizes[m_pAttributes[i].eConstantType]);
			pElement->uAttribId = m_pAttributes[i].uIndex;
			pElement->uCount = m_pAttributes[i].uCount;
			pElement->eType = (usg::VertexElementType)m_pAttributes[i].eConstantType;
			pElement->bIntegerReg = false;
			pElement->bNormalised = false;
			pElement->uOffset = uOffset;
			uOffset += pElement->uCount * g_uVertexElementSizes[m_pAttributes[i].eConstantType];
			pElement++;
		}
		*pElement = VERTEX_DATA_END();

		m_descLayout = pDevice->GetDescriptorSetLayout(m_pDescriptorDecl);
	}

	uint32 CustomEffectResource::GetAttribBinding(const char* szAttrib) const
	{
		for(uint32 i=0; i< m_header.uAttributeCount; i++)
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
		return m_header.uAttributeCount;
	}

	const CustomEffectDecl::Attribute* CustomEffectResource::GetAttribute(uint32 uIndex) const
	{
		if (uIndex >= m_header.uAttributeCount)
		{
			return nullptr;
		}
		return &m_pAttributes[uIndex];
	}

	uint32 CustomEffectResource::GetSamplerBinding(const char* szSampler) const
	{
		for(uint32 i=0; i< m_header.uSamplerCount; i++)
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
		return m_header.uConstantSetCount;
	}

	uint32 CustomEffectResource::GetConstantSetBinding(uint32 uSet) const
	{
		ASSERT(uSet < m_header.uConstantSetCount);
		return m_pConstantSets[uSet].uBinding;
	}


	uint32 CustomEffectResource::GetConstantCount(uint32 uSet) const
	{
		ASSERT(uSet< m_header.uConstantSetCount);
		return m_pConstantSets[uSet].uConstants;	
	}

	const CustomEffectDecl::Constant* CustomEffectResource::GetConstant(uint32 uSet, uint32 uAttrib) const
	{
		ASSERT(uSet< m_header.uConstantSetCount);
		CustomEffectDecl::Constant* pConstant = (CustomEffectDecl::Constant*)(((uint8*)m_pBinary)+m_pConstantSets[uSet].uDeclOffset);
		ASSERT(uAttrib<m_pConstantSets[uSet].uConstants);
		return &pConstant[uAttrib];
	}


	void* CustomEffectResource::GetDefaultData(uint32 uSet) const
	{
		ASSERT(uSet< m_header.uConstantSetCount);
		return ((uint8*)(m_pBinary) + m_pConstantSets[uSet].uDataOffset);
	}

	const char* CustomEffectResource::GetDefaultTexture(uint32 uSamplerBinding)
	{
		for (uint32 i = 0; i < m_header.uSamplerCount; i++)
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
		return m_header.effectName;
	}

	const char* CustomEffectResource::GetDepthEffectName() const
	{
		return m_header.shadowEffectName;
	}

	const char* CustomEffectResource::GetDeferredEffectName() const
	{
		return m_header.deferredEffectName;
	}

	const char* CustomEffectResource::GetTransparentEffectName() const
	{
		return m_header.transparentEffectName;
	}

	const char* CustomEffectResource::GetOmniDepthEffectName() const
	{
		return m_header.omniShadowEffectName;
	}

}

