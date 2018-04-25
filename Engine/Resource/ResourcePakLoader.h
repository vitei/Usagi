/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Class for loading a pak of related assets
*****************************************************************************/
#pragma once

#ifndef USG_RESOURCE_RESOURCE_PAK_LOADER_H
#define USG_RESOURCE_RESOURCE_PAK_LOADER_H
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceData.h"
#include "Engine/Core/File/File.h"

namespace usg{

class ResourcePakLoader
{
public:
	ResourcePakLoader();
	~ResourcePakLoader();

	void StartLoad(const char* szPathName, const char* szPakName);
	void LoadEffects(usg::GFXDevice* pDevice, ResourceData<Effect>& effectData);
	template <typename TResourceType, typename TResourcePak>
	void Load(usg::GFXDevice* pDevice, ResourceData<TResourceType>& resourceData, const char* const szExtension = "");
	template <typename TResourceType, typename TResourcePak>
	void Load(usg::GFXDevice* pDevice, const vector<RenderPassHndl>& renderPasses, ResourceData<TResourceType>& resourceData, const char* const szExtension = "");
	void FinishLoad();

	const U8String& GetName() const { return m_name; }

private:

	const ResourceGroup & GetResourceGroup(const TexturePak * & pHeaders) const
	{
		pHeaders = m_pTextureHeaders;
		return m_header.textures;
	}

	const ResourceGroup & GetResourceGroup(const ParticleEmitterPak * & pHeaders) const
	{ 
		pHeaders = m_pParticleEmitterHeaders;
		return m_header.particle_emitters;
	}

	const ResourceGroup & GetResourceGroup(const ParticleEffectPak * & pHeaders) const
	{
		pHeaders = m_pParticleEffectHeaders;
		return m_header.particle_effects;
	}

	ScratchRaw	m_scratch;
	File		m_file;

	uint8*				m_pData;
	TexturePak*			m_pTextureHeaders;
	EffectPak*			m_pEffectHeaders;
	ParticleEmitterPak*	m_pParticleEmitterHeaders;
	ParticleEffectPak*	m_pParticleEffectHeaders;

	Resources	m_header;
	U8String	m_pathName;
	U8String	m_name;
	// If the texture package is larger than the available scratch memory
	// we fall back to just loading the header
	bool		m_bBinaryInMemory;
};

} // namespace usg

#endif // USG_RESOURCE_RESOURCE_PAK_LOADER_H
