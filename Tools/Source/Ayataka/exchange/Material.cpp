#include "Engine/Common/Common.h"
#include "Material.h"

namespace exchange {


void Material::InitCustomMaterial(const char* szName)
{
	m_materialDef.Load(szName, "");
	for (uint32 i = 0; i < m_materialDef.GetConstantSetCount(); i++)
	{
		AddConstantSet(m_materialDef.GetConstantSetName(i), m_materialDef.GetConstantSetSize(i));
		m_materialDef.CopyDefaultData(i, GetConstantSetData(i));
		std::string name;
		name = szName;
		size_t uLast = name.find_last_of("/") != std::string::npos ? name.find_last_of("/") : name.find_last_of("\\");
		name = name.substr(uLast + 1, name.size() - 5 - uLast);
		strcpy_s(m_material.customEffectName, name.c_str());
	}
}

void Material::AddConstantSet(const char* szName, uint32 uSize)
{
	ASSERT(uSize % 4 == 0);
	uint32 uIndex = m_material.constants_count;
	strcpy_s(m_material.constants[uIndex].bufferName, szName);
	if (uIndex != 0)
	{
		m_material.constants[uIndex].uOffset = m_material.constants[uIndex - 1].uSize + m_material.constants[uIndex - 1].uOffset;
	}
	else
	{
		m_material.constants[uIndex].uOffset = 0;
	}
	m_material.constants[uIndex].uSize = uSize;
	ASSERT(m_material.constants[uIndex].uOffset + m_material.constants[uIndex].uSize < sizeof(m_material.constantData));
	m_material.constants_count++;
}

void* Material::GetConstantSetData(uint32 uSet)
{
	return ((uint8*)m_material.constantData) + m_material.constants[uSet].uOffset;
}


}
