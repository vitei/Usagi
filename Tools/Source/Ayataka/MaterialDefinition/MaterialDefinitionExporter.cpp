#include "MaterialDefinitionExporter.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Core/Utility.h"
#include "common.h"
#include <yaml-cpp/yaml.h>

struct Mapping
{
	const char* szName;
	uint32 uValue;
};

static const Mapping g_constantMappings[]
{
	{ "mat4x4", usg::CT_MATRIX_44 },
	{ "mat3x4", usg::CT_MATRIX_43 },
	{ "vec4",	usg::CT_VECTOR_4 },
	{ "vec2",	usg::CT_VECTOR_2 },
	{ "float",	usg::CT_FLOAT },
	{ "int",	usg::CT_INT },
	{ "ivec4",	usg::CT_VECTOR4I },
	{ "bool",	usg::CT_BOOL },
	{ "invalid", usg::CT_STRUCT }
};


uint32 GetConstantType(const char* szTypeName)
{
	for (uint32 i = 0; i < ARRAY_SIZE(g_constantMappings); i++)
	{
		if (str::Compare(szTypeName, g_constantMappings[i].szName))
		{
			return g_constantMappings[i].uValue;
		}
	}

	ASSERT(false);
	return 0;
}


uint32 AlignOffset(uint32 uOffset, uint32 uElementType)
{
	uint32 uAlign = uElementType == usg::CT_BOOL ? 1 : 4;
	uOffset = (uOffset + uAlign - 1) - ((uOffset + uAlign - 1) % uAlign);
	return uOffset;
}

uint32 GetSize(uint32 uConstantType, uint32 uArraySize)
{
	return usg::g_uConstantSize[uConstantType] * uArraySize;
}

void SetDefaultData(usg::CustomEffectDecl::Constant& constant, const YAML::Node& node, void* pData)
{
	uint8* pD8 = ((uint8*)pData) + constant.uiOffset;;
	if (node)
	{
		switch (constant.eConstantType)
		{
		case usg::CT_MATRIX_44:
			ASSERT(node.size() == 16);
			for (uint32 i = 0; i < 16; i++)
			{
				float value = node[i].as<float>();
				memcpy(pD8, &value, sizeof(float));
				pD8 += sizeof(float);
			}
			break;
		case usg::CT_MATRIX_43:
			ASSERT(node.size() == 12);
			for (uint32 i = 0; i < 12; i++)
			{
				float value = node[i].as<float>();
				memcpy(pD8, &value, sizeof(float));
				pD8 += sizeof(float);
			}
			break;
		case usg::CT_VECTOR_4:
			ASSERT(node.size() == 4);
			for (uint32 i = 0; i < 4; i++)
			{
				float value = node[i].as<float>();
				memcpy(pD8, &value, sizeof(float));
				pD8 += sizeof(float);
			}
			break;
		case usg::CT_VECTOR_2:
			ASSERT(node.size() == 2);
			for (uint32 i = 0; i < 2; i++)
			{
				float value = node[i].as<float>();
				memcpy(pD8, &value, sizeof(float));
				pD8 += sizeof(float);
			}
			break;
		case usg::CT_FLOAT:
		{
			float value = node.as<float>();
			memcpy(pD8, &value, sizeof(float));
			break;
		}
		case usg::CT_INT:
		{
			int value = node.as<int>();
			memcpy(pD8, &value, sizeof(int));
			break;
		}
		case usg::CT_BOOL:
		{
			bool value = node.as<bool>();
			memcpy(pD8, &value, sizeof(bool));
			break;
		}
		default:
			ASSERT(false);
		}
	}
	else
	{
		memset(pD8, 0, usg::g_uConstantSize[constant.eConstantType] * constant.uiCount);
	}
}

MaterialDefinitionExporter::~MaterialDefinitionExporter()
{
	for (uint32 i = 0; i < m_constantSets.size(); i++)
	{
		if (m_constantSets[i].pRawData)
		{
			usg::mem::Free(m_constantSets[i].pRawData);
		}
	}
}

static memsize AlignSize(memsize uSize, memsize uAlign)
{
	memsize uMask = uAlign - 1;
	memsize uMisAlignment = (uSize & uMask);
	memsize uAdjustment = uAlign - uMisAlignment;
	if (uMisAlignment == 0)
		return uSize;

	return uSize + uAdjustment;
}

int MaterialDefinitionExporter::Load(const char* path)
{
	YAML::Node mainNode = YAML::LoadFile(path);

	m_effectName = mainNode["Shader"]["name"].as<std::string>();
	m_shadowEffectName = mainNode["Shader"]["shadow"].as<std::string>();
	m_deferredEffectName = mainNode["Shader"]["deferred"].as<std::string>();
	m_omniShadowEffectName = mainNode["Shader"]["omniShadow"].as<std::string>();

	YAML::Node attributes = mainNode["Attributes"];
	for (YAML::const_iterator it = attributes.begin(); it != attributes.end(); ++it)
	{
		usg::CustomEffectDecl::Attribute attrib;
		strcpy_s(attrib.hint,  sizeof(attrib.hint), (*it)["hint"].as<std::string>().c_str());
		attrib.uIndex = (*it)["index"].as<uint32>();
		m_attributes.push_back(attrib);
	}


	YAML::Node samplers = mainNode["Samplers"];
	for (YAML::const_iterator it = samplers.begin(); it != samplers.end(); ++it)
	{
		usg::CustomEffectDecl::Sampler sampler;
		strcpy_s(sampler.hint, sizeof(sampler.hint), (*it)["hint"].as<std::string>().c_str());
		sampler.uIndex = (*it)["index"].as<uint32>();
		m_samplers.push_back(sampler);
	}

	YAML::Node constantDefs = mainNode["ConstantDefs"];
	for (YAML::const_iterator setIt = constantDefs.begin(); setIt != constantDefs.end(); ++setIt)
	{
		ConstantSetData setData;
		YAML::Node variableDefs = (*setIt)["Variables"];
		setData.set.uConstants = (uint32_t)variableDefs.size();
		strcpy_s(setData.set.szName, sizeof(setData.set.szName), (*setIt)["name"].as<std::string>().c_str());
		uint32 uOffset = 0;
		// First collect the information about the variables
		for (YAML::const_iterator variableIt = variableDefs.begin(); variableIt != variableDefs.end(); ++variableIt)
		{
			usg::CustomEffectDecl::Constant constant;
			constant.eConstantType = GetConstantType((*variableIt)["type"].as<std::string>().c_str());
			strcpy_s(constant.szName, sizeof(constant.szName), (*variableIt)["name"].as<std::string>().c_str());
			constant.uNameHash = utl::CRC32(constant.szName);
			if ((*variableIt)["count"])
			{
				constant.uiCount = (*variableIt)["count"].as<uint32>();
			}
			else
			{
				constant.uiCount = 1;
			}
			uOffset = AlignOffset(uOffset, constant.eConstantType);
			constant.uiOffset = uOffset;
			uOffset += GetSize(constant.eConstantType, constant.uiCount);
			setData.constants.push_back(constant);
		}
		
		// Keep aligned to 4 bytes, helps us export and read
		setData.uRawDataSize = (uint32_t)AlignSize(uOffset, 4);
		setData.set.uDataSize = setData.uRawDataSize;
		setData.pRawData = usg::mem::Alloc(usg::MEMTYPE_STANDARD, usg::ALLOC_DEBUG, setData.uRawDataSize);

		uint32 uIndex = 0;
		for (YAML::const_iterator variableIt = variableDefs.begin(); variableIt != variableDefs.end(); ++variableIt)
		{
			SetDefaultData(setData.constants[uIndex], (*variableIt)["default"], setData.pRawData);
			uIndex++;
		}

		m_constantSets.push_back(setData);
	}



	return 0;
}

uint32 MaterialDefinitionExporter::GetConstantSetCount()
{
	return (uint32_t)m_constantSets.size();
}

uint32 MaterialDefinitionExporter::GetConstantSetSize(uint32 uSet)
{
	uint32 uSize = m_constantSets[uSet].uRawDataSize;
	return uSize;
}

const char* MaterialDefinitionExporter::GetConstantSetName(uint32 uSet)
{
	return m_constantSets[uSet].set.szName;
}

void MaterialDefinitionExporter::CopyDefaultData(uint32 uSet, void* pDst)
{
	memcpy(pDst, m_constantSets[uSet].pRawData, m_constantSets[uSet].uRawDataSize);
}

bool MaterialDefinitionExporter::GetTextureIndex(const char* szHint, uint32& indexOut)
{
	for (uint32 i = 0; i < m_samplers.size(); i++)
	{
		if (strcmp(szHint, m_samplers[i].hint) == 0)
		{
			indexOut = m_samplers[i].uIndex;
			return true;
		}
	}
	return false;
}

bool MaterialDefinitionExporter::GetAttributeIndex(const char* szHint, uint32& indexOut)
{
	for (uint32 i = 0; i < m_attributes.size(); i++)
	{
		if (strcmp(szHint, m_attributes[i].hint) == 0)
		{
			indexOut = m_attributes[i].uIndex;
			return true;
		}
	}
	return false;
}

void MaterialDefinitionExporter::OverrideDefault(uint32 uSet, const char* szName, void* pDst, void* pSrc, uint32 uSize, uint32 uOffset)
{
	ConstantSetData& constantSet = m_constantSets[uSet];
	for (uint32 i = 0; i < constantSet.constants.size(); i++)
	{
		usg::CustomEffectDecl::Constant& var = constantSet.constants[i];
		if (strcmp(var.szName, szName) == 0)
		{
			uOffset += constantSet.constants[i].uiOffset;
			if ((uOffset + uSize) <= (usg::g_uConstantSize[var.eConstantType] * var.uiCount) + var.uiOffset)
			{
				memcpy((void*)(((uint8*)pDst) + uOffset), pSrc, uSize);
			}
			else
			{
				ASSERT(false);
			}
		}
	}
}

bool MaterialDefinitionExporter::GetVariable(uint32 uSet, void* pSrc, const char* szName, void* pData, uint32 uSize, uint32 uOffset)
{
	ConstantSetData& constantSet = m_constantSets[uSet];
	for (uint32 i = 0; i < constantSet.constants.size(); i++)
	{
		usg::CustomEffectDecl::Constant& var = constantSet.constants[i];
		if (strcmp(var.szName, szName) == 0)
		{
			uOffset += constantSet.constants[i].uiOffset;
			if ((uOffset + uSize) <= (usg::g_uConstantSize[var.eConstantType] * var.uiCount) + var.uiOffset)
			{
				memcpy((void*)(pData), (void*)(((uint8*)pSrc)+uOffset), uSize);
				return true;
			}
			else
			{
				ASSERT(false);
			}
		}
	}
	return false;
}

void MaterialDefinitionExporter::ExportFile( const char* path )
{
	FILE* handle = fopen( path, "wb" );
	
	uint32 uHeaderSize = sizeof(usg::CustomEffectDecl::Header);
	uint32 uAttributeSize = sizeof(usg::CustomEffectDecl::Attribute) * (uint32_t)m_attributes.size();
	uint32 uSamplerSize = sizeof(usg::CustomEffectDecl::Sampler) * (uint32_t)m_samplers.size();
	uint32 uConstantSetSize = sizeof(usg::CustomEffectDecl::ConstantSet) * (uint32_t)m_constantSets.size();

	uint32 uDeclSize = 0;
	for (uint32 i = 0; i < m_constantSets.size(); i++)
	{
		uDeclSize += (uint32_t)m_constantSets[i].constants.size() * sizeof(usg::CustomEffectDecl::Constant);
	}
	 
	usg::CustomEffectDecl::Header header;
	header.uAttributeCount = (uint32_t)m_attributes.size();
	header.uAttributeOffset = uHeaderSize;
	header.uSamplerCount = (uint32_t)m_samplers.size();
	header.uSamplerOffset = header.uAttributeOffset + uAttributeSize;
	header.uConstantSetCount = (uint32_t)m_constantSets.size();
	header.uConstantSetDeclOffset = header.uSamplerOffset + uSamplerSize;

	strcpy_s(header.effectName, m_effectName.c_str());
	strcpy_s(header.shadowEffectName, m_shadowEffectName.c_str());
	strcpy_s(header.omniShadowEffectName, m_omniShadowEffectName.c_str());
	strcpy_s(header.deferredEffectName, m_deferredEffectName.c_str());

	// Write the header, attributes and samplers
	fwrite(&header, 1, sizeof(header), handle);
	fwrite(m_attributes.data(), 1, uAttributeSize, handle);
	fwrite(m_samplers.data(), 1, uSamplerSize, handle);

	// Write all of the headers
	uint32 uDeclOffset = header.uConstantSetDeclOffset + uConstantSetSize;
	uint32 uVariableOffset = uDeclOffset + uDeclSize;
	for (uint32 i = 0; i < (uint32_t)m_constantSets.size(); i++)
	{
		m_constantSets[i].set.uDeclOffset = uDeclOffset;
		m_constantSets[i].set.uDataOffset = uVariableOffset;
		uDeclOffset += (uint32_t)m_constantSets[i].constants.size() * sizeof(usg::CustomEffectDecl::Constant);
		uVariableOffset += m_constantSets[i].uRawDataSize;
		fwrite(&m_constantSets[i].set, sizeof(usg::CustomEffectDecl::ConstantSet), 1, handle);
	}
	uint32 uPos = ftell(handle);
	// Write the declarations
	for (uint32 i = 0; i < m_constantSets.size(); i++)
	{
		fwrite(m_constantSets[i].constants.data(), sizeof(usg::CustomEffectDecl::Constant), m_constantSets[i].constants.size(), handle);
	}

	uPos = ftell(handle);
	// Write the default data
	for (uint32 i = 0; i < m_constantSets.size(); i++)
	{
		fwrite(m_constantSets[i].pRawData, 1, m_constantSets[i].uRawDataSize, handle);
	}

	fclose( handle );
}

