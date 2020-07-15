#include "Engine/Common/Common.h"
#include "MaterialDefinitionExporter.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Memory/MemUtil.h"
#include "Engine/Core/Utility.h"
#include <yaml-cpp/yaml.h>

struct Mapping
{
	const char* szName;
	uint32 uValue;
};

static const Mapping g_textureTypeMappings[]
{
	{ "sampler2D", usg::TD_TEXTURE2D },
	{ "sampler1D", usg::TD_TEXTURE1D },
	{ "samplerCube", usg::TD_TEXTURECUBE },
	{ "sampler3D", usg::TD_TEXTURE3D }
};

static const Mapping g_constantMappings[]
{
	{ "mat4x4", usg::CT_MATRIX_44 },
	{ "mat3x4", usg::CT_MATRIX_43 },
	{ "vec4",	usg::CT_VECTOR_4 },
	{ "vec3",	usg::CT_VECTOR_3 },
	{ "vec2",	usg::CT_VECTOR_2 },
	{ "float",	usg::CT_FLOAT },
	{ "int",	usg::CT_INT },
	{ "ivec2",	usg::CT_VECTOR2I },
	{ "ivec4",	usg::CT_VECTOR4I },
	{ "uvec4",	usg::CT_VECTOR4U },
	{ "bool",	usg::CT_BOOL },
	{ "invalid", usg::CT_STRUCT },
	{ nullptr, USG_INVALID_ID }
};

static const Mapping g_shaderMappings[]
{
	{ "VS", (uint32)usg::ShaderTypeFlags::SHADER_FLAG_VERTEX },
	{ "GS", (uint32)usg::ShaderTypeFlags::SHADER_FLAG_GEOMETRY },
	{ "PS", (uint32)usg::ShaderTypeFlags::SHADER_FLAG_PIXEL },
	{ nullptr, USG_INVALID_ID }
};

static const Mapping g_bufferBindingMappings[]
{
	{ "Material", usg::SHADER_CONSTANT_MATERIAL },
	{ "Material1", usg::SHADER_CONSTANT_MATERIAL_1 },
	{ "Custom0", usg::SHADER_CONSTANT_CUSTOM_0 },
	{ "Custom1", usg::SHADER_CONSTANT_CUSTOM_1 },
	{ "Custom2", usg::SHADER_CONSTANT_CUSTOM_2 },
	{ "Custom3", usg::SHADER_CONSTANT_CUSTOM_3 },
	{ nullptr, USG_INVALID_ID }
};

static const uint32 g_shaderFlagMap[]
{
	usg::SHADER_FLAG_VERTEX,
	usg::SHADER_FLAG_PIXEL,
	usg::SHADER_FLAG_GEOMETRY
};

uint32 GetType(const char* szTypeName, const Mapping* pMapping)
{
	while(pMapping->szName)
	{
		if (str::Compare(szTypeName, pMapping->szName))
		{
			return pMapping->uValue;
		}
		pMapping++;
	}

	ASSERT(false);
	return 0;
}

const char* GetType(uint32 uType, const Mapping* pMapping)
{
	while (pMapping->szName)
	{
		if (uType == pMapping->uValue)
		{
			return pMapping->szName;
		}
		pMapping++;
	}

	ASSERT(false);
	return nullptr;
}

uint32 GetConstantType(const char* szTypeName)
{
	return GetType(szTypeName, g_constantMappings);
}

const char* GetConstantType(uint32 uConstantType)
{
	return GetType(uConstantType, g_constantMappings);
}

uint32 GetShaderType(const char* szTypeName)
{
	std::stringstream stream;
	std::string entry;
	stream << szTypeName;
	uint32 uOut = 0;
	while (stream >> entry)
	{
		uOut |= GetType(entry.c_str(), g_shaderMappings);
	}
	return uOut;
}

uint32 GetTextureMapping(const char* szTypeName)
{
	return GetType(szTypeName, g_textureTypeMappings);
}

const char* GetTextureMapping(uint32 uConstantType)
{
	return GetType(uConstantType, g_textureTypeMappings);
}

uint32 GetBufferMapping(const char* szTypeName)
{
	return GetType(szTypeName, g_bufferBindingMappings);
}

const char* GetBufferMapping(uint32 uConstantType)
{
	return GetType(uConstantType, g_bufferBindingMappings);
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

void SetDefaultData(uint32 uConstantType, uint32 uConstantCount, const YAML::Node& node, uint8* pD8)
{
	if (node)
	{
		uint8* pSrc = pD8;
		switch (uConstantType)
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
		case usg::CT_VECTOR_3:
			ASSERT(node.size() == 3);
			for (uint32 i = 0; i < 3; i++)
			{
				float value = node[i].as<float>();
				memcpy(pD8, &value, sizeof(float));
				pD8 += sizeof(float);
			}
			break;
		case usg::CT_VECTOR4I:
			ASSERT(node.size() == 4);
			for (uint32 i = 0; i < 4; i++)
			{
				int value = node[i].as<int>();
				memcpy(pD8, &value, sizeof(int));
				pD8 += sizeof(int);
			}
			break;
		case usg::CT_VECTOR4U:
			ASSERT(node.size() == 4);
			for (uint32 i = 0; i < 4; i++)
			{
				uint32 value = node[i].as<uint32>();
				memcpy(pD8, &value, sizeof(uint32));
				pD8 += sizeof(uint32);
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
			pD8 += sizeof(float);
			break;
		}
		case usg::CT_INT:
		{
			int value = node.as<int>();
			memcpy(pD8, &value, sizeof(int));
			pD8 += sizeof(int);
			break;
		}
		case usg::CT_BOOL:
		{
			bool value = node.as<bool>();
			memcpy(pD8, &value, sizeof(bool));
			pD8 += sizeof(bool);
			break;
		}
		default:
			ASSERT(false);
		}

		for (uint32 i = 1; i < uConstantCount; i++)
		{
			memcpy(pD8, pSrc, usg::g_uConstantSize[uConstantType]);
			pD8 += usg::g_uConstantSize[uConstantType];
		}
	}
	else
	{
		memset(pD8, 0, usg::g_uConstantSize[uConstantType] * uConstantCount);
	}
}

void SetDefaultData(usg::CustomEffectDecl::Constant& constant, const YAML::Node& node, void* pData)
{
	uint8* pD8 = ((uint8*)pData) + constant.uiOffset;
	SetDefaultData(constant.eConstantType, constant.uiCount, node, pD8);
}

void SetDefaultData(usg::CustomEffectDecl::Attribute& attrib, const YAML::Node& node)
{
	uint8* pData8 = (uint8*)attrib.defaultData;
	SetDefaultData(attrib.eConstantType, 1, node, pData8);
}

MaterialDefinitionExporter::~MaterialDefinitionExporter()
{

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

bool MaterialDefinitionExporter::LoadAttributes(YAML::Node& attributes)
{
	for (YAML::const_iterator it = attributes.begin(); it != attributes.end(); ++it)
	{
		usg::CustomEffectDecl::Attribute attrib;
		strcpy_s(attrib.hint, sizeof(attrib.hint), (*it)["hint"].as<std::string>().c_str());
		strcpy_s(attrib.name, sizeof(attrib.name), (*it)["name"].as<std::string>().c_str());
		attrib.uIndex = (*it)["index"].as<uint32>();
		attrib.eConstantType = GetConstantType((*it)["type"].as<std::string>().c_str());
		attrib.uCount = (*it)["count"] ? (*it)["count"].as<uint32>() : 1;
		SetDefaultData(attrib, (*it)["default"]);
		if ((*it)["defines"])
		{
			if (!IsValidWithDefineSet((*it)["defines"].as<std::string>()))
			{
				continue;
			}
		}
		m_attributes.push_back(attrib);
	}

	return true;
}

bool MaterialDefinitionExporter::LoadSamplers(YAML::Node& samplers)
{
	for (YAML::const_iterator it = samplers.begin(); it != samplers.end(); ++it)
	{
		usg::CustomEffectDecl::Sampler sampler;
		strcpy_s(sampler.hint, sizeof(sampler.hint), (*it)["hint"].as<std::string>().c_str());
		sampler.uIndex = (*it)["index"].as<uint32>();
		if ((*it)["default"])
		{
			strcpy_s(sampler.texName, sizeof(sampler.texName), (*it)["default"].as<std::string>().c_str());
		}
		else
		{
			usg::MemClear(sampler.texName, sizeof(sampler.texName));
		}
		if ((*it)["defines"])
		{
			if (!IsValidWithDefineSet((*it)["defines"].as<std::string>()))
			{
				continue;
			}
		}
		if ((*it)["type"])
		{
			sampler.eTexType = GetTextureMapping((*it)["type"].as<std::string>().c_str());
		}
		else
		{
			sampler.eTexType = usg::TD_TEXTURE2D;
		}
		m_samplers.push_back(sampler);
	}

	return true;
}

bool MaterialDefinitionExporter::LoadConstantSets(YAML::Node& constantDefs)
{
	for (YAML::const_iterator setIt = constantDefs.begin(); setIt != constantDefs.end(); ++setIt)
	{
		ConstantSetData setData;
		if ((*setIt)["defines"])
		{
			if (!IsValidWithDefineSet((*setIt)["defines"].as<std::string>()))
			{
				continue;
			}
		}
		if ((*setIt)["shaderType"])
		{
			std::string type = (*setIt)["shaderType"].as<std::string>();
			setData.set.uShaderSets = GetShaderType(type.c_str());
		}
		setData.set.uBinding = GetBufferMapping((*setIt)["binding"].as<std::string>().c_str());
		YAML::Node variableDefs = (*setIt)["Variables"];
		setData.set.uConstants = (uint32_t)variableDefs.size();
		if ((*setIt)["name"])
		{
			strcpy_s(setData.set.szName, sizeof(setData.set.szName), (*setIt)["name"].as<std::string>().c_str());
		}
		else
		{
			memset(setData.set.szName, 0, sizeof(setData.set.szName));
		}
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
			if ((*variableIt)["defines"])
			{
				if (!IsValidWithDefineSet((*variableIt)["defines"].as<std::string>()))
				{
					continue;
				}
			}
			setData.constants.push_back(constant);
		}

		// Keep aligned to 4 bytes, helps us export and read
		uint32 uRawDataSize = (uint32_t)AlignSize(uOffset, 4);

		setData.set.uDataSize = uRawDataSize;
		setData.rawData.resize(uRawDataSize);

		uint32 uIndex = 0;
		for (YAML::const_iterator variableIt = variableDefs.begin(); variableIt != variableDefs.end(); ++variableIt)
		{
			if ((*variableIt)["defines"])
			{
				if (!IsValidWithDefineSet((*variableIt)["defines"].as<std::string>()))
				{
					continue;
				}
			}
			SetDefaultData(setData.constants[uIndex], (*variableIt)["default"], setData.rawData.data());
			uIndex++;
		}

		m_constantSets.push_back(setData);
	}
	return true;
}

int MaterialDefinitionExporter::Load(YAML::Node& mainNode, const std::string& defines)
{
	m_defines.clear();
	std::string defineList = defines;
	size_t nextDefine = std::string::npos;

	while (!defineList.empty())
	{
		nextDefine = defineList.find_first_of(' ');
		if (nextDefine != std::string::npos)
		{
			m_defines.push_back(defineList.substr(0, nextDefine));
			defineList = defineList.substr(nextDefine + 1);
		}
		else
		{
			// The last define
			m_defines.push_back(defineList);
			defineList.clear();
		}

	} while (nextDefine != std::string::npos);

	YAML::Node attributes = mainNode["Attributes"];
	LoadAttributes(attributes);

	YAML::Node samplers = mainNode["Samplers"];
	LoadSamplers(samplers);

	YAML::Node constantDefs = mainNode["ConstantDefs"];
	LoadConstantSets(constantDefs);



	return 0;
}

int MaterialDefinitionExporter::Load(const char* path, const std::string& defines)
{
	YAML::Node mainNode = YAML::LoadFile(path);
	return Load(mainNode, defines);

}


int MaterialDefinitionExporter::Load(const char* path, const char* effect, const std::vector<std::string>& defineSets)
{
	YAML::Node mainNode = YAML::LoadFile(path);
	YAML::Node customFX = mainNode["CustomEffects"];
	YAML::Node effects = mainNode["Effects"];
	std::string defines;
	YAML::Node effectNode;
	YAML::Node customFXNode;

	uint32 uMaxDefinesFound = 0;
	for (YAML::iterator it = effects.begin(); it != effects.end(); ++it)
	{
		uint32 uFoundThisNode = 0;

		if ((*it)["define_sets"])
		{
			YAML::Node defineNode = (*it)["define_sets"];
			for (YAML::const_iterator it = defineNode.begin(); it != defineNode.end(); ++it)
			{
				for (memsize i = 0; i < defineSets.size(); i++)
				{
					if (defineSets[i] == (*it)["name"].as<std::string>())
					{
						uFoundThisNode++;
					}
				}
			}
		}
		if ((*it)["global_sets"])
		{
			YAML::Node defineNode = (*it)["global_sets"];
			for (YAML::const_iterator it = defineNode.begin(); it != defineNode.end(); ++it)
			{
				for (memsize i = 0; i < defineSets.size(); i++)
				{
					if (defineSets[i] == (*it)["name"].as<std::string>())
					{
						uFoundThisNode++;
					}
				}
			}
		}


		if ((*it)["name"] && (*it)["name"].as<std::string>() == effect && ((uFoundThisNode > uMaxDefinesFound) || defineSets.size() == 0) )
		{
			uMaxDefinesFound = uFoundThisNode;
			effectNode = (*it).as<YAML::Node>();
		}
	}
	
	if (effectNode.IsNull())
	{
		return -1;
	}

	if(effectNode["custom_effect"])
	{
		std::string customFXString = effectNode["custom_effect"].as<std::string>();
		customFXNode = customFX[customFXString];
	}
	else
	{
		return -1;
	}

	if (effectNode["define_sets"])
	{
		YAML::Node defineNode = effectNode["defineSets"];
		for (YAML::const_iterator it = defineNode.begin(); it != defineNode.end(); ++it)
		{
			for (memsize i = 0; i < defineSets.size(); i++)
			{
				if (defineSets[i] == (*it)["name"].as<std::string>())
				{
					if (defines.size() > 0)
					{
						defines += "";
					}
					defines += (*it)["defines"].as<std::string>();
				}
			}
		}
	}
	if (effectNode["global_sets"])
	{
		YAML::Node defineNode = effectNode["global_sets"];
		for (YAML::const_iterator it = defineNode.begin(); it != defineNode.end(); ++it)
		{
			for (memsize i = 0; i < defineSets.size(); i++)
			{
				if (defineSets[i] == (*it)["name"].as<std::string>())
				{
					if (defines.size() > 0)
					{
						defines += "";
					}
					defines += (*it)["defines"].as<std::string>();
				}
			}
		}
	}

	Load(customFXNode, defines);

	return 1;

}

uint32 MaterialDefinitionExporter::GetConstantSetCount()
{
	return (uint32_t)m_constantSets.size();
}

uint32 MaterialDefinitionExporter::GetConstantSetSize(uint32 uSet)
{
	uint32 uSize = (uint32)m_constantSets[uSet].rawData.size();
	return uSize;
}

const char* MaterialDefinitionExporter::GetConstantSetName(uint32 uSet)
{
	return m_constantSets[uSet].set.szName;
}

void MaterialDefinitionExporter::CopyDefaultData(uint32 uSet, void* pDst)
{
	memcpy(pDst, m_constantSets[uSet].rawData.data(), m_constantSets[uSet].rawData.size());
}

bool MaterialDefinitionExporter::GetTextureIndex(uint32 uTex, uint32& indexOut)
{
	if (uTex < m_samplers.size())
	{
		indexOut = m_samplers[uTex].uIndex;
		return true;
	}
	return false;
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

bool MaterialDefinitionExporter::IsValidWithDefineSet(const std::string& conditions)
{
	std::string defineList = conditions;
	size_t nextDefine = std::string::npos;
	std::string thisCondition;

	while (!defineList.empty())
	{
		nextDefine = defineList.find_first_of(' ');
		if (nextDefine != std::string::npos)
		{
			thisCondition = defineList.substr(0, nextDefine);
			defineList = defineList.substr(nextDefine + 1);
		}
		else
		{
			// The last define
			thisCondition = defineList;
			defineList.clear();
		}

		bool bShouldNotHave = thisCondition[0] == '!';
		bool bFound = false;
		if (bShouldNotHave)
		{
			thisCondition = thisCondition.substr(1);
		}
		for (int i = 0; i < m_defines.size(); i++)
		{
			if (thisCondition == m_defines[i])
			{
				bFound = true;
			}
		}

		if (bShouldNotHave == bFound)
		{
			return false;
		}

	} while (nextDefine != std::string::npos);

	return true;
}

void MaterialDefinitionExporter::InitBinaryData()
{
	uint32 uHeaderSize = sizeof(usg::CustomEffectDecl::Header);
	uint32 uAttributeSize = sizeof(usg::CustomEffectDecl::Attribute) * (uint32_t)m_attributes.size();
	uint32 uSamplerSize = sizeof(usg::CustomEffectDecl::Sampler) * (uint32_t)m_samplers.size();
	uint32 uConstantSetSize = sizeof(usg::CustomEffectDecl::ConstantSet) * (uint32_t)m_constantSets.size();

	uint32 uDeclSize = 0;
	for (uint32 i = 0; i < m_constantSets.size(); i++)
	{
		uDeclSize += (uint32_t)m_constantSets[i].constants.size() * sizeof(usg::CustomEffectDecl::Constant);
	}

	uint32 uRawDataSize = 0;
	for (uint32 i = 0; i < m_constantSets.size(); i++)
	{
		uRawDataSize += (uint32)m_constantSets[i].rawData.size();
	}

	uint32 uBinarySize = uAttributeSize + uSamplerSize + uConstantSetSize + uDeclSize + uRawDataSize;
	m_binary.resize(uBinarySize);

	m_header.uAttributeCount = (uint32_t)m_attributes.size();
	m_header.uAttributeOffset = 0;
	m_header.uSamplerCount = (uint32_t)m_samplers.size();
	m_header.uSamplerOffset = m_header.uAttributeOffset + uAttributeSize;
	m_header.uConstantSetCount = (uint32_t)m_constantSets.size();
	m_header.uConstantSetDeclOffset = m_header.uSamplerOffset + uSamplerSize;

	memcpy(m_binary.data() + m_header.uAttributeOffset, m_attributes.data(),uAttributeSize);
	memcpy(m_binary.data() + m_header.uSamplerOffset, m_samplers.data(), uSamplerSize);

	uint32 uDeclOffset = m_header.uConstantSetDeclOffset + uConstantSetSize;
	uint32 uVariableOffset = uDeclOffset + uDeclSize;

	uint8* pDst = m_binary.data() + m_header.uConstantSetDeclOffset;

	for (uint32 i = 0; i < (uint32_t)m_constantSets.size(); i++)
	{
		m_constantSets[i].set.uDeclOffset = uDeclOffset;
		m_constantSets[i].set.uDataOffset = uVariableOffset;
		uDeclOffset += (uint32_t)m_constantSets[i].constants.size() * sizeof(usg::CustomEffectDecl::Constant);
		uVariableOffset += (uint32)m_constantSets[i].rawData.size();
		memcpy(pDst, &m_constantSets[i].set, sizeof(usg::CustomEffectDecl::ConstantSet));
		pDst += sizeof(usg::CustomEffectDecl::ConstantSet);
	}

	for (uint32 i = 0; i < m_constantSets.size(); i++)
	{
		memcpy(pDst, m_constantSets[i].constants.data(), sizeof(usg::CustomEffectDecl::Constant)*m_constantSets[i].constants.size());
		pDst += sizeof(usg::CustomEffectDecl::Constant)*m_constantSets[i].constants.size();
	}

	for (uint32 i = 0; i < m_constantSets.size(); i++)
	{
		memcpy(pDst, m_constantSets[i].rawData.data(), m_constantSets[i].rawData.size());
		pDst += m_constantSets[i].rawData.size();
	}

	m_uHeaderCRC = utl::CRC32(&m_header, sizeof(m_header));
	m_uDataCRC = utl::CRC32(m_binary.data(), (uint32)m_binary.size());

}

void MaterialDefinitionExporter::InitAutomatedCode()
{
	for (uint32 i = 0; i < (uint32)usg::ShaderType::COUNT; i++)
	{
		m_automatedCode[i] = "";
	}

	char buffer[512];

	for (const auto& sampler : m_samplers)
	{
		// TODO: When the custom effects are fully integrated we can start naming these
		sprintf_s(buffer, sizeof(buffer), "SAMPLER_LOC(1, %d) uniform %s sampler%d;\n", sampler.uIndex, GetTextureMapping(sampler.eTexType), sampler.uIndex);

		m_automatedCode[(uint32)usg::ShaderType::PS] += buffer;
	}
	m_automatedCode[(uint32)usg::ShaderType::VS] += "\n";

	for (const auto& attrib : m_attributes)
	{
		if (attrib.uCount > 1)
		{
			sprintf_s(buffer, sizeof(buffer), "ATTRIB_LOC(%d) in %s %s[%d];\n", attrib.uIndex, GetConstantType(attrib.eConstantType), attrib.name, attrib.uCount);
		}
		else
		{
			sprintf_s(buffer, sizeof(buffer), "ATTRIB_LOC(%d) in %s %s;\n", attrib.uIndex, GetConstantType(attrib.eConstantType), attrib.name);
		}
		m_automatedCode[(uint32)usg::ShaderType::VS] += buffer;
	}
	m_automatedCode[(uint32)usg::ShaderType::VS] += "\n";

	for (uint32 uShaderType = 0; uShaderType < (uint32)usg::ShaderType::COUNT; uShaderType++)
	{
		for (uint32 i = 0; i < (uint32_t)m_constantSets.size(); i++)
		{
			if ( (m_constantSets[i].set.uShaderSets & g_shaderFlagMap[uShaderType]) )
			{
				sprintf_s(buffer, sizeof(buffer), "BUFFER_LAYOUT(1,  %d) uniform %s\n{\n", m_constantSets[i].set.uBinding, GetBufferMapping(m_constantSets[i].set.uBinding));
				m_automatedCode[uShaderType] += buffer;
				for (uint32 j = 0; j < (uint32)m_constantSets[i].constants.size(); j++)
				{
					usg::CustomEffectDecl::Constant& constant = m_constantSets[i].constants[j];
					if (constant.uiCount == 1)
					{
						sprintf_s(buffer, sizeof(buffer), "\t %s %s;\n", GetConstantType(constant.eConstantType), constant.szName);
					}
					else
					{
						sprintf_s(buffer, sizeof(buffer), "\t %s %s[%d];\n", GetConstantType(constant.eConstantType), constant.szName, constant.uiCount);
					}
					m_automatedCode[uShaderType] += buffer;
				}
				sprintf_s(buffer, sizeof(buffer), "}%s;\n\n", m_constantSets[i].set.szName);
				m_automatedCode[uShaderType] += buffer;
			}
		}
	}
}

void MaterialDefinitionExporter::ExportFile( const char* path )
{
	FILE* handle = nullptr;
	fopen_s(&handle, path, "wb");

	InitBinaryData();

	fwrite(&m_header, sizeof(m_header), 1, handle);
	fwrite(m_binary.data(), 1, m_binary.size(), handle);

	fclose( handle );
}

