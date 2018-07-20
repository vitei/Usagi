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
#include "Engine/Core/File/File.h"
#include "EffectPakDecl.h"
#include "EffectPak.h"

namespace usg
{

	EffectPak::EffectPak()
		: m_pEffects(nullptr)
		, m_uEffects;
		, m_pShaders;
		, m_uShaders;
	{
		
	}

	EffectPak::~EffectPak()
	{
		
	}


	void EffectPak::Init(GFXDevice* pDevice, const char* szFileName)
	{
		SetupHash(szFileName);

		File file(szFileName);
		ScratchRaw scratch(file.GetSize());
		file.Read(file.GetSize(), scratch.GetRawData());

		usg::EffectPakDecl::Header* pHdr = (usg::EffectPakDecl::Header*)scratch.GetRawData();
		usg::EffectPakDecl::ShaderEntry* pShaders = (usg::EffectPakDecl::ShaderEntry*)scratch.GetDataAtOffset(pHdr->uShaderBinaryOffset);
		usg::EffectPakDecl::EffectEntry* pEffects = (usg::EffectPakDecl::EffectEntry*)scratch.GetDataAtOffset(pHdr->uEffectDefinitionOffset);

		
	}

	void EffectPak::CleanUp(GFXDevice* pDevice)
	{
		for (uint32 i = 0; i < m_uEffects; i++)
		{
			m_pEffects->CleanUp(pDevice);
		}
		if (m_pEffects)
		{
			vdelete[] m_pEffects;
			m_pEffects = nullptr;
		}

		for (uint32 i = 0; i < m_uShaders; i++)
		{
			m_pShaders->CleanUp(pDevice);
		}
		if (m_pShaders)
		{
			vdelete[] m_pShaders;
			m_pShaders = nullptr;
		}
	}


}

