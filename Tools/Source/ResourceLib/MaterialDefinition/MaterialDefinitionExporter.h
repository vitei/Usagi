#ifndef MaterialDefinitionExporter_h__
#define MaterialDefinitionExporter_h__
#include "Engine/Resource/CustomEffectDecl.h"
#include "Engine/Graphics/RenderConsts.h"
#include <vector>
#include <yaml-cpp/yaml.h>


void GetSamplerOverrides(const YAML::Node& attributeNode, uint32& uMinFilter, uint32& uMagFilter, uint32& uMipFilter, uint32& uAniso, float& fLodBias, uint32& uBaseMip);
void GetTexCoordMapperOverrides(const YAML::Node& attributeNode, uint32& uSourceCoord, usg::Vector2f& vTrans, usg::Vector2f& vScale, float& fRot);

class MaterialDefinitionExporter
{
public:
	MaterialDefinitionExporter() {}
	virtual ~MaterialDefinitionExporter();

	int Load(const char* path, const std::string& defines);
	int Load(const char* path, const char* effect, const std::vector<std::string>& defineSets);
	int Load(YAML::Node& mainNode, const std::string& defines);
	bool LoadAttributes(YAML::Node& attributeNode);
	bool LoadSamplers(YAML::Node& attributeNode);
	bool LoadConstantSets(YAML::Node& attributeNode);

	void ExportFile( const char* path );
	void InitBinaryData();
	void InitAutomatedCode();

	struct ConstantSetData
	{
		usg::CustomEffectDecl::ConstantSet	set;
		std::vector<usg::CustomEffectDecl::Constant> constants;
		std::vector<uint8> rawData;
	};

	// Utility functions for setting up the models which make use of these definitions
	bool GetTextureIndex(const char* szHint, uint32& indexOut);
	bool GetTextureIndex(uint32 uTex, uint32& indexOut);
	bool GetAttributeIndex(const char* szHint, uint32& indexOut);
	uint32 GetVariableCount(const char* szName);
	uint32 GetConstantSetCount();
	// Returns the size of the constant set
	uint32 GetConstantSetSize(uint32 uSet);
	const char* GetConstantSetName(uint32 uSet);
	void CopyDefaultData(uint32 uSet, void* pDst);
	uint32 GetTextureCount() const { return (uint32)m_samplers.size(); }
	const char* GetDefaultTexName(uint32 uTex) { return m_samplers[uTex].texName; }
	void GetSamplerDefaults(uint32 uTex, uint32& uMinFilter, uint32& uMagFilter, uint32& uMipFilter, uint32& uAniso, float& fLodBias, uint32& uBaseMip);


	bool OverrideData(uint32 uPass, const char* szVariableName, const YAML::Node& node, void* pDst);

	template <class VariableType>
	void OverrideTyped(uint32 uSet, const char* szName, void* pDst, VariableType* pVar, uint32 uCount = 1, uint32 uStartId = 0)
	{
		return OverrideDefault(uSet, szName, pDst, (void*)pVar, sizeof(VariableType)*uCount, uStartId*sizeof(VariableType));
	}

	void OverrideDefault(uint32 uSet, const char* szName, void* pDst, void* pSrc, uint32 uSize, uint32 uOffset);

	template <class VariableType>
	bool GetVariableTyped(uint32 uSet, void* pSrc, const char* szName, VariableType* pData, uint32 uIndex = 0)
	{
		return GetVariable(uSet, pSrc, szName, (void*)pData, sizeof(VariableType), sizeof(VariableType)*uIndex);
	}

	bool GetVariable(uint32 uSet, void* pSrc, const char* szName, void* pData, uint32 uSize, uint32 uOffset);

	const usg::CustomEffectDecl::Header& GetHeader() const { return m_header; }
	uint32 GetHeaderSize() const { return sizeof(m_header); }
	const void* GetBinary() const { return (void*)m_binary.data(); }
	uint32 GetBinarySize() const { return (uint32)m_binary.size(); }

	uint64 GetCRC() const { ASSERT(m_binary.size() > 0); return m_uCRC; }
	const std::string& GetAutomatedCode(usg::ShaderType eType) const { return m_automatedCode[(uint32)eType]; }
private:
	bool IsValidWithDefineSet(const std::string& conditions);

	usg::CustomEffectDecl::Header m_header;

	std::string m_automatedCode[(uint32)usg::ShaderType::COUNT];
	std::vector<usg::CustomEffectDecl::Sampler> m_samplers;
	std::vector<usg::CustomEffectDecl::Attribute> m_attributes;
	std::vector<ConstantSetData> m_constantSets;
	std::vector<std::string> m_defines;

	std::vector<uint8> m_binary;
	
	union 
	{
		struct 
		{
			uint32 m_uHeaderCRC;
			uint32 m_uDataCRC;
		};
		uint64	m_uCRC = 0;
	};
};

#endif // MaterialAnimationConverter_h__
