#pragma once
#include "Engine/Resource/CustomEffectDecl.h"
#include "Engine/Graphics/RenderConsts.h"
#include "../Dependencies/DependencyTracker.h"
#include "Material.h"
#include <vector>
#include <yaml-cpp/yaml.h>

struct EnumTable;

class MaterialOverrides
{
public:
	MaterialOverrides();
	virtual ~MaterialOverrides();

	// We will re-load the material definition as needed as it may be overriden
	bool Init(const char* szFileName, const char* szDefaultPak, const char* szDefaultEffect, DependencyTracker* pDependencies);

	void InitDefault(const char* szMaterialName, const std::vector<std::string>& defines, ::exchange::Material* pMatOut);

	void ApplyOverrides(const char* szMaterialName, ::exchange::Material* pMatOut);

	void StringToValue(const EnumTable* pTable, YAML::Node node, uint32& uValOut);

private:
	char m_defaultPak[USG_MAX_PATH];
	char m_defaultEffect[USG_MAX_PATH];
	DependencyTracker* m_pDependencies;

	YAML::Node m_mainNode;
	YAML::Node m_overrides;

};

