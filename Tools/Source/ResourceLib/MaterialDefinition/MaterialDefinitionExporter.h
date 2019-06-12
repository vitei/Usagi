#ifndef MaterialDefinitionExporter_h__
#define MaterialDefinitionExporter_h__
#include "Engine/Resource/CustomEffectDecl.h"
#include <vector>
#include <yaml-cpp/yaml.h>

class MaterialDefinitionExporter
{
public:
	MaterialDefinitionExporter() {}
	virtual ~MaterialDefinitionExporter();

	int Load(const char* path);
	bool LoadAttributes(YAML::Node& attributeNode);
	bool LoadSamplers(YAML::Node& attributeNode);
	bool LoadConstantSets(YAML::Node& attributeNode);
	void ExportFile( const char* path );
	void InitBinaryData();

	struct ConstantSetData
	{
		usg::CustomEffectDecl::ConstantSet	set;
		std::vector<usg::CustomEffectDecl::Constant> constants;
		void*	pRawData;
		uint32	uRawDataSize;
	};

	// Utility functions for setting up the models which make use of these definitions
	bool GetTextureIndex(const char* szHint, uint32& indexOut);
	bool GetAttributeIndex(const char* szHint, uint32& indexOut);
	uint32 GetConstantSetCount();
	// Returns the size of the constant set
	uint32 GetConstantSetSize(uint32 uSet);
	const char* GetConstantSetName(uint32 uSet);
	void CopyDefaultData(uint32 uSet, void* pDst);
	uint32 GetTextureCount() const { return (uint32)m_samplers.size(); }
	const char* GetDefaultTexName(uint32 uTex) { return m_samplers[uTex].texName; }

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
	const void* GetBinary() const { return (void*)m_pBinary; }
	uint32 GetBinarySize() const { return m_uBinarySize; }
private:

	usg::CustomEffectDecl::Header m_header;

	std::string m_effectName;
	std::string m_deferredEffectName;
	std::string m_transparentEffectName;
	std::string m_shadowEffectName;
	std::string m_omniShadowEffectName;
	std::vector<usg::CustomEffectDecl::Sampler> m_samplers;
	std::vector<usg::CustomEffectDecl::Attribute> m_attributes;
	std::vector<ConstantSetData> m_constantSets;

	uint8*	m_pBinary = nullptr;
	uint32	m_uBinarySize = 0;
};

#endif // MaterialAnimationConverter_h__
