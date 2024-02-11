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
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Core/stl/map.h"
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
			pDescDecl->shaderType = (ShaderTypeFlags)m_pSamplers[i].uShaderSets;
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


		usg::map<uint32, memsize> buffers;	// Buffer idx, offset
		for (uint32 i = 0; i < m_header.uAttributeCount; i++)
		{
			if (buffers.find(m_pAttributes[i].uVBIndex) == buffers.end())
			{
				buffers[i] = memsize(0);
			}
		}

		// TODO: Once stable move this to the exporter side
		uint32 uVertexElemCnt = m_header.uAttributeCount + (uint32)buffers.size();
		m_pVertexDecl = vnew(ALLOC_OBJECT)VertexElement[uVertexElemCnt];

		for (auto itr : buffers)
		{
			VertexElement* pElement = m_pVertexDecl;
			for (uint32 i = 0; i < m_header.uAttributeCount; i++)
			{
				// Keep the data aligned
				uint32 uVertexBuffer = m_pAttributes[i].uVBIndex;

				if(uVertexBuffer != itr.first)
					continue;

				buffers[uVertexBuffer] = AlignSizeUp(buffers[uVertexBuffer], g_uConstantCPUAllignment[m_pAttributes[i].eConstantType]);
				pElement->uAttribId = m_pAttributes[i].uIndex;
				pElement->uCount = g_veCountMapping[m_pAttributes[i].eConstantType] * m_pAttributes[i].uCount;
				// FIXME: This is actually the constant type; need to remove the VE types (they should be interchangeable)
				pElement->eType = g_veMapping[m_pAttributes[i].eConstantType];
				pElement->bIntegerReg = false;
				pElement->bNormalised = false;
				pElement->uOffset = buffers[uVertexBuffer];
				buffers[uVertexBuffer] += pElement->uCount * g_uConstantCPUAllignment[m_pAttributes[i].eConstantType];
				pElement++;
			}
			*pElement = VERTEX_DATA_END();
		}

		for (auto itr : buffers)
		{
			m_vertexSizes[itr.first] = AlignSizeUp(itr.second, 4);
		}
		

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


	const VertexElement* CustomEffectResource::GetVertexElement(const char* szAttribHint) const
	{
		uint32 uBinding = GetAttribBinding(szAttribHint);
		if(uBinding == USG_INVALID_ID)
			return nullptr;

		for (uint32 i = 0; i < m_header.uAttributeCount; i++)
		{
			if (m_pVertexDecl[i].uAttribId == uBinding)
			{
				return &m_pVertexDecl[i];
			}
		}
		return nullptr;
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

	bool CustomEffectResource::SetVertexAttribute(void* pVertData, const char* szName, const void* pSrc, uint32 uSrcSize, uint32 uVertexId, uint32 uIndex, uint32 uVertCount) const
	{
		memsize uOffsetBase = uVertexId * GetVertexSize();
		memsize uSubOffset = (uIndex * uSrcSize);
		bool bFound = false;
		for (uint32 i = 0; i < m_header.uAttributeCount; i++)
		{
			if (str::Compare(szName, m_pAttributes[i].name))
			{
				uOffsetBase += m_pVertexDecl[i].uOffset;
				if ((m_pVertexDecl[i].uCount * g_uConstantSize[m_pVertexDecl[i].eType]) < (uSrcSize + uSubOffset))
				{
					ASSERT(false);
					return false;
				}
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			ASSERT(false);
			return false;
		}

		for (uint32 i = 0; i < uVertCount; i++)
		{
			usg::MemCpy((uint8*)(pVertData)+uOffsetBase+uSubOffset, pSrc, uSrcSize);
			uOffsetBase += GetVertexSize();
		}
		return true;
	}


	memsize CustomEffectResource::GetVertexSize(uint32 uBuffer) const
	{
		auto itr = m_vertexSizes.find(uBuffer);
		if (itr != m_vertexSizes.end())
		{
			return (*itr).second;
		}
		return 0;
	}


	const VertexElement* CustomEffectResource::GetVertexElements(uint32 uBuffer) const
	{	
		auto itr = m_vertexSizes.find(uBuffer);
		if (itr == m_vertexSizes.end())
		{
			ASSERT(false);
			return nullptr;
		}
		int iBufferIdx = 0;
		for (auto itr : m_vertexSizes)
		{
			if (itr.first == uBuffer)
			{
				break;
			}
			iBufferIdx++;
		}

		// They are in order
		VertexElement* pDecl = m_pVertexDecl;
		while (iBufferIdx)
		{
			// Look for the vertex caps
			while (pDecl->eType != usg::VE_INVALID)
			{
				pDecl++;
			}
			iBufferIdx--;
		}

		return pDecl;
	}


	void CustomEffectResource::InitWithDefaultData(class ScratchRaw& scratch, uint32 uBuffer, uint32 uVertexCount) const
	{
		const VertexElement* pElements = GetVertexElements(uBuffer);

		uint32 uSize = (uint32)GetVertexSize(uBuffer);

		if(uSize == 0)
			return;

		scratch.Init(uVertexCount * uSize, 4);

		const VertexElement* pElement = pElements;

		while (pElement->eType != VE_INVALID)
		{
			for (uint32 i = 0; i < m_header.uAttributeCount; i++)
			{
				if (m_pAttributes[i].uVBIndex == uBuffer && m_pAttributes[i].uIndex == pElement->uAttribId)
				{
					usg::MemCpy(scratch.GetDataAtOffset((uint32)pElement->uOffset), m_pAttributes[i].defaultData, pElement->uCount * g_uConstantCPUAllignment[m_pAttributes[i].eConstantType]);
					break;
				}
			}
			pElement++;
		}

		for (uint32 i = 1; i < uVertexCount; i++)
		{
			usg::MemCpy(scratch.GetDataAtOffset(uSize * i), scratch.GetRawData(), uSize);
		}
	}

	void CustomEffectResource::InitVertexBuffer(usg::GFXDevice* pDevice, class VertexBuffer& buffer, uint32 uBuffer, uint32 uVertexCount, bool bSetDefaults) const
	{
		uint32 uSize = (uint32)GetVertexSize(uBuffer);

		void* pData = nullptr;
		if(bSetDefaults)
		{
			ScratchRaw scratch;

			InitWithDefaultData(scratch, uBuffer, uVertexCount);
			pData = scratch.GetRawData();
		}

		buffer.Init(pDevice, pData, uSize, uVertexCount, "CustomVB");
	}


}


