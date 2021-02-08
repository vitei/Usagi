#include "Engine/Common/Common.h"
#include "Material.h"

namespace exchange {

// FIXME: Remove hardcoding, set through material editor
const char* szPassExtensions[usg::exchange::_Material_RenderPass_count]
{
	nullptr,
	"deferred",
	"translucent",
	"depth",
	"omni_depth"
};


void Material::InitCustomMaterial(const char* szPakName, const char* szEffectName, const std::vector<std::string>& defines)
{
	for (uint32 pass = 0; pass < usg::exchange::_Material_RenderPass_count; pass++)
	{
		std::vector<std::string> passDefines;
		if (szPassExtensions[pass])
		{
			passDefines.push_back(szPassExtensions[pass]);
		}
		// For now not adding any custom passes to depth effects
		for (auto itr : defines)
		{
			// TODO: Remove hardcoding when we can edit materials
			if (pass <= usg::exchange::Material_RenderPass_DEPTH || itr == "skel")
			{
				passDefines.push_back(itr);
			}
		}
		m_materialDef[pass].Load(szPakName, szEffectName, defines);

		for (uint32 i = 0; i < m_materialDef[pass].GetConstantSetCount(); i++)
		{
			AddConstantSet(m_materialDef[pass].GetConstantSetName(i), pass, m_materialDef[pass].GetConstantSetSize(i));
			m_materialDef[pass].CopyDefaultData(i, GetConstantSetData(pass, i));
		}

		std::string name;
		name = szEffectName;
		for (auto itr : passDefines)
		{
			if(itr.size() > 0)
			{
				name += std::string(".");
				name += itr;
			}
		}
	//	size_t uLast = name.find_last_of("/") != std::string::npos ? name.find_last_of("/") : name.find_last_of("\\");
	//	name = name.substr(uLast + 1, name.size() - 5 - uLast);
		// FIXME: Pack name from input
		sprintf_s(m_material.renderPasses[pass].effectName, "%s%s", "Model.", name.c_str());

	}

	for (uint32 i = 0; i < m_materialDef[0].GetTextureCount(); i++)
	{
		uint32 uDstIndex;

		m_materialDef[0].GetTextureIndex(i, uDstIndex);
		usg::exchange::Texture& tex = m_material.textures[uDstIndex];

		usg::exchange::TextureCoordinator& texCo = m_material.textureCoordinators[i];
		texCo.sourceCoordinate = 0;
		texCo.scale.Assign(1.0f, 1.0f);
		texCo.translate.Assign(0.0f, 0.0f);
		texCo.rotate = 0.0f;
		texCo.method = usg::exchange::TextureCoordinator_MappingMethod_UV_COORDINATE;

		m_materialDef[0].GetSamplerDefaults( i, tex.minFilter, tex.magFilter, tex.mipFilter, tex.anisoLevel, tex.lodBias, tex.lodMinLevel );
	}
	m_material.textureCoordinators_count = m_materialDef[0].GetTextureCount(); 
}

void Material::AddConstantSet(const char* szName, uint32 uPass, uint32 uSize)
{
	ASSERT(uSize % 4 == 0);
	usg::exchange::RenderPassData& pass = m_material.renderPasses[uPass];
	uint32 uIndex = pass.constants_count;
	pass.constants[uIndex].uOffset = 0;
	for (uint32 i = 0; i <= uPass; i++)
	{
		for(uint32 j=0; j<=m_material.renderPasses[i].constants_count; j++ )
		{
			pass.constants[uIndex].uOffset += m_material.renderPasses[i].constants[j].uSize;
		}
	}
	strcpy_s(pass.constants[uIndex].bufferName, szName);

	pass.constants[uIndex].uSize = uSize;
	ASSERT(pass.constants[uIndex].uOffset + pass.constants[uIndex].uSize < sizeof(m_material.constantData));
	pass.constants_count++;
}

void* Material::GetConstantSetData(uint32 uPass, uint32 uSet)
{
	return ((uint8*)m_material.constantData) + m_material.renderPasses[uPass].constants[uSet].uOffset;
}


}
