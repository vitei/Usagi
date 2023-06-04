#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Resource/CustomEffectDecl.h"
#include "Engine/Graphics/RenderConsts.h"
#include "MaterialOverrides.h"


struct EnumTable
{
	const char* szVarName;
	uint32		uEnumValue;
};

static const EnumTable g_FuncTable[] =
{
	{ "Zero", usg::BLEND_FUNC_ZERO },
	{ "One", usg::BLEND_FUNC_ONE },
	{ "SrcColor", usg::BLEND_FUNC_SRC_COLOR },
	{ "OneMinusSrc", usg::BLEND_FUNC_ONE_MINUS_SRC_COLOR },
	{ "DstColor", usg::BLEND_FUNC_DST_COLOR  },
	{ "OneMinusDstColor", usg::BLEND_FUNC_ONE_MINUS_DST_COLOR  },
	{ "SrcAlpha", usg::BLEND_FUNC_SRC_ALPHA  },
	{ "OneMinusSrcAlpha", usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA  },
	{ "DstAlpha", usg::BLEND_FUNC_DST_ALPHA  },
	{ "OneMinusDstAlpha", usg::BLEND_FUNC_ONE_MINUS_DST_ALPHA  },
	{ "ConstantColor", usg::BLEND_FUNC_CONSTANT_COLOR  },
	{ "OneMinusConstantColor", usg::BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR  },
	{ "ConstantAlpha", usg::BLEND_FUNC_CONSTANT_ALPHA  },
	{ "OneMinusConstantAlpha", usg::BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA  },
	{ "AlphaSaturate", usg::BLEND_FUNC_SRC_ALPHA_SATURATE  },
	{ nullptr, 0 }
};


static const EnumTable g_stencilFuncTable[]
{
	{ "Keep", usg::STENCIL_OP_KEEP },
	{ "Zero", usg::STENCIL_OP_ZERO },
	{ "Replace", usg::STENCIL_OP_REPLACE },
	{ "Incr", usg::STENCIL_OP_INCR },
	{ "Decr", usg::STENCIL_OP_DECR },
	{ "Invert", usg::STENCIL_OP_INVERT },
	{ "IncrWrap", usg::STENCIL_OP_INCR_WRAP },
	{ "DecrWrap", usg::STENCIL_OP_DECR_WRAP },
	{ nullptr, 0 }
};

static const EnumTable g_stencilOpTable[]
{
	{ "Never", usg::STENCIL_TEST_NEVER },
	{ "Always", usg::STENCIL_TEST_ALWAYS },
	{ "Equal", usg::STENCIL_TEST_EQUAL },
	{ "NotEqual", usg::STENCIL_TEST_NOTEQUAL },
	{ "Less", usg::STENCIL_TEST_LESS },
	{ "LessOrEqual", usg::STENCIL_TEST_LEQUAL },
	{ "Greater", usg::STENCIL_TEST_GREATER },
	{ "GreateOrEqual", usg::STENCIL_TEST_GEQUAL },
	{ nullptr, 0 }
};

static const EnumTable g_opTable[]
{
	{ "Add", usg::BLEND_EQUATION_ADD },
	{ "Sub", usg::BLEND_EQUATION_SUBTRACT },
	{ "RevereSub", usg::BLEND_EQUATION_REVERSE_SUBTRACT },
	{ "Min", usg::BLEND_EQUATION_MIN },
	{ "Max", usg::BLEND_EQUATION_MAX },
	{ nullptr, 0 }
};

static const EnumTable g_layerTable[]
{
	{ "Opaque", usg::exchange::Translucency_Type_TRANSLUCENCY_OPAQUE },
	{ "Translucent", usg::exchange::Translucency_Type_TRANSLUCENCY_TRANSLUCENT },
	{ "Additive", usg::exchange::Translucency_Type_TRANSLUCENCY_ADDITIVE },
	{ "Subtractive", usg::exchange::Translucency_Type_TRANSLUCENCY_SUBTRACTIVE },
	{ nullptr, 0 }
};

static const EnumTable g_alphaTestTable[]
{
	{ "Never", usg::ALPHA_TEST_NEVER },
	{ "Always", usg::ALPHA_TEST_ALWAYS },
	{ "Equal", usg::ALPHA_TEST_EQUAL },
	{ "NotEqual", usg::ALPHA_TEST_NOTEQUAL },
	{ "Less", usg::ALPHA_TEST_LESS },
	{ "LessOrEqual", usg::ALPHA_TEST_LEQUAL },
	{ "Greater", usg::ALPHA_TEST_GREATER },
	{ "GreateOrEqual", usg::ALPHA_TEST_GEQUAL },
	{ nullptr, 0 }
};

static const EnumTable g_cullFaceTable[]
{
	{ "Front", usg::CULL_FACE_FRONT },
	{ "Back", usg::CULL_FACE_BACK },
	{ "None", usg::CULL_FACE_NONE },
	{ nullptr, 0 }
};


// Generally no good reason to override what is coming in from the model
#if 0
static const EnumTable g_wrapTable[]
{
	{ "Repeat", usg::SAMP_WRAP_REPEAT },
	{ "Mirror", usg::SAMP_WRAP_MIRROR },
	{ "Clamp", usg::SAMP_WRAP_CLAMP },
	{ nullptr, 0 }
};
#endif


	MaterialOverrides::MaterialOverrides() :
		m_pDependencies(nullptr)
	{
	}

	MaterialOverrides::~MaterialOverrides()
	{

	}

	bool MaterialOverrides::Init(const char* szFileName, const char* szDefaultPak, const char* szDefaultEffect, DependencyTracker* pDependencies)
	{
		m_pDependencies = pDependencies;
		strcpy_s(m_defaultPak, szDefaultPak);
		strcpy_s(m_defaultEffect, szDefaultEffect);

		FILE* pFile = nullptr;
		fopen_s(&pFile, szFileName, "r");
		if( pFile )
		{
			fclose(pFile);
			m_mainNode = YAML::LoadFile(szFileName);
			m_pDependencies->LogDependency(szFileName);
		}
		else
		{
			return false;
		}

		m_overrides = m_mainNode["MaterialOverrides"];

		return true;
	}

	bool MaterialOverrides::Init(const YAML::Node* pNode, const char* szDefaultPak, const char* szDefaultEffect, DependencyTracker* pDependencies)
	{
		m_pDependencies = pDependencies;
		strcpy_s(m_defaultPak, szDefaultPak);
		strcpy_s(m_defaultEffect, szDefaultEffect);

		m_mainNode = *pNode;

		m_overrides = m_mainNode["MaterialOverrides"];

		return m_overrides;
	}

	void MaterialOverrides::InitDefault(const char* szMaterialName, const std::vector<std::string>& definesIn, exchange::Material* pMatOut)
	{
		std::string effectSet = m_defaultPak;
		std::string effectName = m_defaultEffect;
		std::vector<std::string> defines = definesIn;
		if(m_overrides && m_overrides[szMaterialName])
		{
			YAML::Node material = m_overrides[szMaterialName];
			YAML::Node pak = material["ShaderPak"];
			YAML::Node effect = material["Effect"];
			if (pak && effect)
			{
				effectSet = pak.as<std::string>();
				effectName = effect.as<std::string>();
			}

			YAML::Node addDefines = material["AddDefines"];
			YAML::Node subDefines = material["SubDefines"];

			if(addDefines)
			{
				for (YAML::const_iterator it = addDefines.begin(); it != addDefines.end(); ++it)
				{
					std::string newDefine =  (*it).as<std::string>();
					if(newDefine.size() == 0)
						continue;

					bool bFound = false;
					for(auto& defineItr  : defines)
					{
						if( defineItr == newDefine )
						{
							bFound = true;
						}
					}
					if(!bFound)
					{
						defines.push_back(newDefine);
					}
				}
			}

			if (subDefines)
			{
				for (YAML::const_iterator it = subDefines.begin(); it != subDefines.end(); ++it)
				{
					std::string subDefine = (*it).as<std::string>();
					if (subDefine.size() == 0)
						continue;

					bool bFound = false;
					for (auto defineItr = defines.begin(); defineItr != defines.end(); ++ defineItr)
					{
						if ( (*defineItr) == subDefine)
						{
							defines.erase(defineItr);
							break;
						}
					}
				}
			}
		}
		
		const char* szUsagiPath = getenv("USAGI_DIR");
		std::string emuPath = szUsagiPath;
		// FIXME: Hunt for a matching material setting file
		emuPath += "\\Data\\GLSL\\effects\\";
		emuPath += effectSet;
		emuPath += ".yml";
		FILE* pFile = nullptr;
		if (fopen_s(&pFile, emuPath.c_str(), "r") != 0)
		{
			emuPath = szUsagiPath;
			emuPath = emuPath.substr(0, emuPath.find("Usagi"));
			emuPath += "\\Data\\GLSL\\effects\\";
			emuPath += effectSet;
			emuPath += ".yml";
			int ret = 0;
			if (ret = fopen_s(&pFile, emuPath.c_str(), "r") != 0)
			{
				FATAL_RELEASE(false, "Could not find effect %s, error %d\n", effectSet.c_str(), ret);
				return;
			}
			fclose(pFile);
		}

		m_pDependencies->LogDependency(emuPath.c_str());
		pMatOut->InitCustomMaterial(emuPath.c_str(), effectName.c_str(), defines);
	}

	void MaterialOverrides::StringToValue(const EnumTable* pTable, YAML::Node node, uint32& uValOut)
	{
		if (!node)
		{
			// This override wasn't specified
			return;
		}
		std::string value = node.as<std::string>();
		if (value.size() == 0)
		{
			RELEASE_WARNING("Invalid string for override\n");
			return;
		}
		while (pTable->szVarName != nullptr)
		{
			if (strcmp(pTable->szVarName, value.c_str()) == 0)
			{
				uValOut = pTable->uEnumValue;
				return;
			}
			pTable++;
		}
		RELEASE_WARNING("Invalid override semantic %s\n", value.c_str());

	}

	void MaterialOverrides::ApplyOverrides(const char* szMaterialName, exchange::Material* pMatOut)
	{
		if (!m_overrides || !m_overrides[szMaterialName])
		{
			// No overrides specified
			return;
		}

		YAML::Node material = m_overrides[szMaterialName];

		YAML::Node alphaNode = material["AlphaStateGroup"];
		if (alphaNode)
		{
			usg::AlphaStateGroup& alpha = pMatOut->pb().rasterizer.alphaState;
			StringToValue(g_FuncTable, alphaNode["rgbSrcFunc"], *(uint32*)&alpha.rgbSrcFunc);
			StringToValue(g_FuncTable, alphaNode["rgbDestFunc"], *(uint32*)&alpha.rgbDestFunc);
			StringToValue(g_opTable, alphaNode["rgbOp"], *(uint32*)&alpha.rgbOp);
			StringToValue(g_FuncTable, alphaNode["alphaSrcFunc"], *(uint32*)&alpha.alphaSrcFunc);
			StringToValue(g_FuncTable, alphaNode["alphaDestFunc"], *(uint32*)&alpha.alphaDestFunc);
			StringToValue(g_opTable, alphaNode["alphaOp"], *(uint32*)&alpha.alphaOp); 
			StringToValue(g_alphaTestTable, alphaNode["alphaTestFunc"], *(uint32*)&alpha.alphaTestFunc);

			if (alphaNode["alphaTestReference"])
			{
				alpha.alphaTestReference = alphaNode["alphaTestReference"].as<float>();
			}
			
		}

		YAML::Node stencilNode = material["StencilStateGroup"];
		if (stencilNode)
		{
			usg::exchange::StencilState& stencil = pMatOut->pb().rasterizer.stencilTest;

			if(stencilNode["enable"])
			{
				stencil.isEnable = stencilNode["enable"].as<bool>();
			}
			if (stencilNode["write"])
			{
				stencil.isEnable = stencilNode["write"].as<bool>();
			}
			StringToValue(g_stencilFuncTable, stencilNode["testFunc"], *(uint32*)&stencil.func);
			StringToValue(g_stencilOpTable, stencilNode["passOp"], *(uint32*)&stencil.passOperation);
			StringToValue(g_stencilOpTable, stencilNode["failOp"], *(uint32*)&stencil.failOperation);
			StringToValue(g_stencilOpTable, stencilNode["depthFailOp"], *(uint32*)&stencil.zFailOperation);
			if (stencilNode["readMask"])
			{
				stencil.readMask = stencilNode["readMask"].as<uint32>();
			}
			if (stencilNode["writeMask"])
			{
				stencil.writeMask = stencilNode["writeMask"].as<uint32>();
			}
			if (stencilNode["ref"])
			{
				stencil.ref = stencilNode["ref"].as<uint32>();
			}
		}

		if (material["Transparent"])
		{
			pMatOut->pb().rasterizer.blendEnabled = material["Transparent"].as<bool>();
		}

		StringToValue(g_cullFaceTable, material["CullFace"], *(uint32*)&pMatOut->pb().rasterizer.cullFace);

		YAML::Node layer = material["Layer"];
		if (layer)
		{
			StringToValue(g_layerTable, material["Layer"], *(uint32*)&pMatOut->pb().attribute.translucencyKind);

		}


		YAML::Node textureNode = material["Textures"];
		for (YAML::const_iterator it = textureNode.begin(); it != textureNode.end(); ++it)
		{
			YAML::Node hint = (*it)["hint"];
			YAML::Node texture = (*it)["texture"];

			if (hint)
			{
				usg::exchange::Texture* pTextures = pMatOut->pb().textures;
				uint32 uTexIndex = 0;
				uint32 uCoordinatorIndex = uTexIndex;

				// FIXME: Ordering of co-ordinators
				usg::exchange::TextureCoordinator& texCo = pMatOut->pb().textureCoordinators[uCoordinatorIndex];

				if (pMatOut->GetCustomFX(0).GetTextureIndex(hint.as<std::string>().c_str(), uTexIndex))
				{
					usg::exchange::Texture& tex = pMatOut->pb().textures[uTexIndex];
					if(texture)
					{
						strcpy_s(pTextures[uTexIndex].textureName, texture.as<std::string>().c_str());
					}

					GetTexCoordMapperOverrides((*it), texCo.sourceCoordinate, texCo.translate, texCo.scale, texCo.rotate);

					GetSamplerOverrides((*it), tex.minFilter, tex.magFilter, tex.mipFilter, tex.anisoLevel, tex.lodBias, tex.lodMinLevel );

				}
			}
			else
			{
				RELEASE_WARNING("Texture override missing required data\n");
			}
		}

		YAML::Node constantsNode = material["Constants"];
		for (YAML::const_iterator it = constantsNode.begin(); it != constantsNode.end(); ++it)
		{
			YAML::Node set = (*it)["set"];
			YAML::Node var = (*it)["name"];
			YAML::Node value = (*it)["value"];

			if (!set || !var)
			{
				RELEASE_WARNING("Constant override missing required data\n");
				continue;
			}

			for (uint32 i = 0; i < usg::exchange::_Material_RenderPass_count; i++)
			{
				for (uint32 j = 0; j < pMatOut->GetCustomFX(i).GetConstantSetCount(); j++)
				{
					if (strcmp(pMatOut->GetCustomFX(i).GetConstantSetName(j), set.as<std::string>().c_str()) != 0)
					{
						continue;
					}

					void* pDst = pMatOut->GetConstantSetData(i, j);
					pMatOut->GetCustomFX(i).OverrideData(j, var.as<std::string>().c_str(), value, pDst);
				}
				
			}
		}
		
	}

