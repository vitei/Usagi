/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourcePak.pb.h"
#include "Engine/Core/Containers/SharedPointer.h"
#include "Engine/Resource/ResourceDictionary.h"
#include "Engine/Resource/ResourceData.h"
#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Resource/ParticleEffectResource.h"
#include "Engine/Resource/ParticleEmitterResource.h"
#include "Engine/Memory/ScratchRaw.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Core/OS.h"
#include "ResourcePakLoader.h"

namespace usg
{
	ResourcePakLoader::ResourcePakLoader()
		:m_scratch()
		,m_file()
		,m_pData(nullptr)
		,m_pTextureHeaders(nullptr)
		,m_pEffectHeaders(nullptr)
		,m_pParticleEmitterHeaders(nullptr)
		,m_pParticleEffectHeaders(nullptr)
		,m_header()
		,m_pathName()
		,m_name()
		,m_bBinaryInMemory(false)
	{
	}

	ResourcePakLoader::~ResourcePakLoader()
	{

	}

	void ResourcePakLoader::StartLoad(const char* szPath, const char* szPakName)
	{
		m_pathName = szPath;
		m_name = m_pathName + szPakName;
		m_file.Open(m_name.CStr());
		memsize uFileSize = m_file.GetSize();

		m_bBinaryInMemory = m_file.GetSize() < SCRATCH_HEAP_SIZE;
		void* pData = nullptr;

		if(m_bBinaryInMemory)
		{
			m_scratch.Init(uFileSize, FILE_READ_ALIGN);
			pData = m_scratch.GetRawData();
			m_file.Read(uFileSize, pData);
			m_header = *(Resources*)pData;
			m_pData = (uint8*)pData;
		}
		else
		{
			m_file.Read(sizeof(Resources), (void*)&m_header);
			m_file.SeekPos(0);
			m_scratch.Init(m_header.uFullHeaderSize, FILE_READ_ALIGN);
			pData = m_scratch.GetRawData();
			m_file.Read(m_header.uFullHeaderSize, pData);
			m_pData = (uint8*)pData;
		}
		
		m_pTextureHeaders = reinterpret_cast<TexturePak*>((m_pData) + m_header.textures.uHdrOffset);
		m_pEffectHeaders = reinterpret_cast<EffectPak*>((m_pData)+m_header.effects.uHdrOffset);
		m_pParticleEmitterHeaders = reinterpret_cast<ParticleEmitterPak*>((m_pData)+m_header.particle_emitters.uHdrOffset);
		m_pParticleEffectHeaders = reinterpret_cast<ParticleEffectPak*>((m_pData)+m_header.particle_effects.uHdrOffset);

		m_file.Close();
	}

	void ResourcePakLoader::LoadEffects(usg::GFXDevice* pDevice, ResourceData<Effect>& effectData)
	{
		U8String effectName;
		for (uint32 i = 0; i < m_header.effects.uCount; i++)
		{
			const EffectPak& pak = m_pEffectHeaders[i];
			effectName = m_pathName + pak.resHdr.strName;
			EffectHndl	pEffect = effectData.GetResourceHndl(effectName);
			if (!pEffect)
			{
				effectData.StartLoad();
				Effect* pNC = vnew(ALLOC_RESOURCE_MGR) Effect;
				if (m_bBinaryInMemory)
				{
					pNC->Init(pDevice, pak, (void*)(m_pData + pak.resHdr.uDataOffset), m_pathName.CStr());
				}
				else
				{
					ScratchRaw scrath;
					scrath.Init(pak.resHdr.uDataSize, FILE_READ_ALIGN);
					bool bDidOpen = false;
					if (!m_file.IsOpen())
					{
						bDidOpen = true;
						m_file.Open(m_name.CStr());
					}
					m_file.SeekPos(pak.resHdr.uDataOffset);
					m_file.Read(pak.resHdr.uDataSize, scrath.GetRawData());
					pNC->Init(pDevice, pak, scrath.GetRawData(), m_pathName.CStr());
					if (!bDidOpen)
					{
						m_file.Close();
					}
				}
				effectData.AddResource(pNC);
			}
		}
	}

	template<typename TResourceType>
	struct ResourceLoadHelper
	{
		static const bool SkipLoadingIfAppShouldQuit = false;
	};

	template<>
	struct ResourceLoadHelper<Texture>
	{
		static const bool SkipLoadingIfAppShouldQuit = true;
	};

	template <typename TResourceType, typename TResourcePak>
	void ResourcePakLoader::Load(usg::GFXDevice* pDevice, ResourceData<TResourceType>& resourceData, const char * const szExtension)
	{
		if (ResourceLoadHelper<TResourceType>::SkipLoadingIfAppShouldQuit && OS::ShouldQuit())
		{
			return;
		}

		const TResourcePak * pPakHeaders;
		const ResourceGroup & resourceGroup = GetResourceGroup(pPakHeaders);

		U8String resourceName;
		for (uint32 i = 0; i < resourceGroup.uCount; i++)
		{
			const TResourcePak& pak = pPakHeaders[i];
			resourceName = m_pathName + pak.resHdr.strName + szExtension;
			SharedPointer<const TResourceType> pResource = resourceData.GetResourceHndl(resourceName);
			if (!pResource)
			{
				resourceData.StartLoad();
				TResourceType* pNC = vnew(ALLOC_RESOURCE_MGR) TResourceType;
				if (m_bBinaryInMemory)
				{
					pNC->Load(pDevice, pak, reinterpret_cast<const void*>(m_pData + pak.resHdr.uDataOffset), m_pathName.CStr());
				}
				else
				{
					ScratchRaw scrath;
					scrath.Init(pak.resHdr.uDataSize, FILE_READ_ALIGN);
					bool bDidOpen = false;
					if (!m_file.IsOpen())
					{
						bDidOpen = true;
						m_file.Open(m_name.CStr());
					}
					m_file.SeekPos(pak.resHdr.uDataOffset);
					m_file.Read(pak.resHdr.uDataSize, scrath.GetRawData());

					pNC->Load(pDevice, pak, scrath.GetRawData(), m_pathName.CStr());
					if (!bDidOpen)
					{
						m_file.Close();
					}
				}
				resourceData.AddResource(pNC);
			}
		}
	}

	template <typename TResourceType, typename TResourcePak>
	void ResourcePakLoader::Load(usg::GFXDevice* pDevice, const vector<RenderPassHndl>& renderPasses, ResourceData<TResourceType>& resourceData, const char * const szExtension)
	{
		if (ResourceLoadHelper<TResourceType>::SkipLoadingIfAppShouldQuit && OS::ShouldQuit())
		{
			return;
		}

		const TResourcePak * pPakHeaders;
		const ResourceGroup & resourceGroup = GetResourceGroup(pPakHeaders);

		U8String resourceName;
		for (uint32 i = 0; i < resourceGroup.uCount; i++)
		{
			const TResourcePak& pak = pPakHeaders[i];
			resourceName = m_pathName + pak.resHdr.strName + szExtension;
			SharedPointer<const TResourceType> pResource = resourceData.GetResourceHndl(resourceName);
			if (!pResource)
			{
				resourceData.StartLoad();
				TResourceType* pNC = vnew(ALLOC_RESOURCE_MGR) TResourceType;
				if (m_bBinaryInMemory)
				{
					pNC->Load(pDevice, renderPasses, pak, reinterpret_cast<const void*>(m_pData + pak.resHdr.uDataOffset), m_pathName.CStr());
				}
				else
				{
					ScratchRaw scrath;
					scrath.Init(pak.resHdr.uDataSize, FILE_READ_ALIGN);
					bool bDidOpen = false;
					if (!m_file.IsOpen())
					{
						bDidOpen = true;
						m_file.Open(m_name.CStr());
					}
					m_file.SeekPos(pak.resHdr.uDataOffset);
					m_file.Read(pak.resHdr.uDataSize, scrath.GetRawData());
					pNC->Load(pDevice, renderPasses, pak, scrath.GetRawData(), m_pathName.CStr());
					if (!bDidOpen)
					{
						m_file.Close();
					}
				}
				resourceData.AddResource(pNC);
			}
		}
	}

	void ResourcePakLoader::FinishLoad()
	{
		m_scratch.Free();
	}

	template void ResourcePakLoader::Load<Texture, TexturePak>(usg::GFXDevice*, ResourceData<Texture>&, const char * const);
	template void ResourcePakLoader::Load<ParticleEmitterResource, ParticleEmitterPak>(usg::GFXDevice*, const vector<RenderPassHndl>&, ResourceData<ParticleEmitterResource>&, const char * const);
	template void ResourcePakLoader::Load<ParticleEffectResource, ParticleEffectPak>(usg::GFXDevice*, ResourceData<ParticleEffectResource>&, const char * const);
}
