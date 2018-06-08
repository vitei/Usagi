/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector4f.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Resource/ResourceMgr.h"
#include "engine/core/String/StringCRC.h"
#include "Engine/core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Resource/SkeletalAnimationResource.h"
#include "Engine/Resource/ParticleEffectResource.h"
#include "Engine/Resource/ParticleEmitterResource.h"
#include "Engine/Resource/CollisionModelResource.h"
#include "Engine/Resource/CustomEffectResource.h"
#include "Engine/Layout/Fonts/Font.h"
#include "Engine/Resource/ResourceData.h"
#include "Engine/Resource/ResourceDictionary.h"
#include "Engine/Resource/ResourcePak.pb.h"
#include "Engine/Resource/ResourcePakLoader.h"
#include "Engine/Resource/ResourcePakHdr.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Core/String/String_Util.h"
#include <cstring>

#ifdef DEBUG_BUILD
#define DEBUG_SHOW_PAK_LOAD_TIME
#ifdef DEBUG_SHOW_PAK_LOAD_TIME
#include "Engine/Core/Timer/ProfilingTimer.h"
#endif
#endif

namespace usg{

	struct ResourceMgr::PIMPL
	{
		ResourceData<Texture>					textures;
		ResourceData<Effect>					effects;
		ResourceData<ModelResource>				models;
		ResourceData<Font>						fonts;
		ResourceData<ProtocolBufferFile>		protocolBuffers;
		ResourceData<SkeletalAnimationResource> skeletalAnims;
		ResourceData<ParticleEffectResource>	particleEffects;
		ResourceData<ParticleEmitterResource>	particleEmitters;
		ResourceData<CollisionModelResource>	collisionModel;
		ResourceData<CustomEffectResource>		customEffects;
		ResourceData<ResourcePakHdr>			resourcePaks;

		List<ResourceDataBase>				resourceSets;
		vector<RenderPassHndl>				renderPasses;

		ResourcePakLoader					pakLoader;
	};

ResourceMgr* ResourceMgr::m_pResource = nullptr;

ResourceMgr::ResourceMgr(void)
{
	m_pImpl = vnew(ALLOC_OBJECT) PIMPL;

	m_pImpl->resourceSets.AddToEnd(&m_pImpl->models);
	m_pImpl->resourceSets.AddToEnd(&m_pImpl->particleEffects);
	m_pImpl->resourceSets.AddToEnd(&m_pImpl->particleEmitters);
	m_pImpl->resourceSets.AddToEnd(&m_pImpl->skeletalAnims);
	m_pImpl->resourceSets.AddToEnd(&m_pImpl->textures);
	m_pImpl->resourceSets.AddToEnd(&m_pImpl->effects);
	m_pImpl->resourceSets.AddToEnd(&m_pImpl->fonts);
	m_pImpl->resourceSets.AddToEnd(&m_pImpl->protocolBuffers);
	m_pImpl->resourceSets.AddToEnd(&m_pImpl->collisionModel);
	m_pImpl->resourceSets.AddToEnd(&m_pImpl->customEffects);
	m_pImpl->resourceSets.AddToEnd(&m_pImpl->resourcePaks);

	m_bReloadIfDirty = false;
	m_bUseLODs = true;

	m_modelDir = "Models/";
	m_textureDir = "Textures/";
	m_effectDir = "Effects/";
	m_fontDir = "Fonts/";

	ResourceDictionary::init();
}

ResourceMgr::~ResourceMgr(void)
{
	ResourceDictionary::cleanup();
	vdelete m_pImpl;
}


void ResourceMgr::Cleanup(usg::GFXDevice* pDevice)
{
	if(m_pResource)
	{
		m_pResource->ClearAllResources(pDevice);
		vdelete m_pResource;
		m_pResource = nullptr;
	}
}

void ResourceMgr::LoadPackage(usg::GFXDevice* pDevice, const char* szPath, const char* szName)
{
	U8String name = szPath;
	name += szName;
	SharedPointer<const ResourcePakHdr> hndl = m_pImpl->resourcePaks.GetResourceHndl(name.CStr());
	// Only load if we don't already have one
	if (!hndl)
	{
		if (File::FileStatus(name.CStr()) == FILE_STATUS_VALID)
		{
#ifdef DEBUG_SHOW_PAK_LOAD_TIME
			ProfilingTimer loadTimer;
			loadTimer.Start();
#endif

			m_pImpl->pakLoader.StartLoad(szPath, szName);
			ResourcePakHdr* pPakHdr = vnew(ALLOC_RESOURCE_MGR) ResourcePakHdr;
			m_pImpl->resourcePaks.StartLoad();
			pPakHdr->Init(m_pImpl->pakLoader);
			m_pImpl->resourcePaks.AddResource(pPakHdr);
			m_pImpl->pakLoader.Load<Texture, TexturePak>(pDevice, m_pImpl->textures);
			m_pImpl->pakLoader.LoadEffects(pDevice, m_pImpl->effects);
			m_pImpl->pakLoader.Load<ParticleEmitterResource, ParticleEmitterPak>(pDevice, m_pImpl->renderPasses, m_pImpl->particleEmitters, ".pem");
			m_pImpl->pakLoader.Load<ParticleEffectResource, ParticleEffectPak>(pDevice, m_pImpl->particleEffects, ".pfx");
			m_pImpl->pakLoader.FinishLoad();

#ifdef DEBUG_SHOW_PAK_LOAD_TIME
			loadTimer.Stop();
			DEBUG_PRINT("Loaded %s in %f seconds\n", name.CStr(), loadTimer.GetTotalSeconds());
#endif
		}
	}
	// Nothing on PC yet so no assert
}

EffectHndl ResourceMgr::GetEffectAbsolutePath(GFXDevice* pDevice, const char* szEffectName)
{
	U8String u8Name = szEffectName;
	EffectHndl pEffect = m_pImpl->effects.GetResourceHndl(u8Name);

	// TODO: Remove from the final build, should load in blocks
	if (!pEffect)
	{
		Effect* pNC = vnew(ALLOC_RESOURCE_MGR) Effect;
		m_pImpl->effects.StartLoad();
		pNC->Init(pDevice, u8Name.CStr());
		pEffect = m_pImpl->effects.AddResource(pNC);
	}

	return pEffect;
}

EffectHndl ResourceMgr::GetEffect(GFXDevice* pDevice, const char* szEffectName)
{
	U8String u8Name = m_effectDir + szEffectName;
	return GetEffectAbsolutePath(pDevice, u8Name.CStr());

}

CollisionModelResHndl ResourceMgr::GetCollisionModel(const char* szFileName)
{
	U8String u8Name = szFileName;
	CollisionModelResHndl pModel = m_pImpl->collisionModel.GetResourceHndl(u8Name);

	// TODO: Remove from the final build, should load in blocks
	if (!pModel)
	{
		CollisionModelResource* pModelPtr = vnew(ALLOC_RESOURCE_MGR) CollisionModelResource;
		m_pImpl->collisionModel.StartLoad();
		pModelPtr->Init(szFileName);
		pModel = m_pImpl->collisionModel.AddResource(pModelPtr);
	}

	return pModel;
}


CustomEffectResHndl ResourceMgr::GetCustomEffectRes(GFXDevice* pDevice, const char* szFileName)
{
	U8String u8Name = szFileName;
	CustomEffectResHndl pEffect = m_pImpl->customEffects.GetResourceHndl(u8Name);

	// TODO: Remove from the final build, should load in blocks
	if (!pEffect)
	{
		CustomEffectResource* pEffectPtr = vnew(ALLOC_RESOURCE_MGR) CustomEffectResource;
		m_pImpl->customEffects.StartLoad();
		pEffectPtr->Init(pDevice, szFileName);
		pEffect = m_pImpl->customEffects.AddResource(pEffectPtr);
	}

	return pEffect;
}


ProtocolBufferFile* ResourceMgr::GetBufferedFile(const char* szFileName)
{
	U8String u8Name = szFileName;
	ProtocolBufferFile* pFile = const_cast<ProtocolBufferFile*>(m_pImpl->protocolBuffers.GetResource(u8Name));

	if(!pFile)
	{
		m_pImpl->protocolBuffers.StartLoad();
		pFile = vnew(ALLOC_RESOURCE_MGR) ProtocolBufferFile(szFileName, FILE_ACCESS_READ, FILE_TYPE_RESOURCE);
		pFile->SetupHash(szFileName);
		m_pImpl->protocolBuffers.AddResource(pFile);
	}

	if(pFile)
	{
		pFile->Reset();
	}

	return pFile;
}

TextureHndl	 ResourceMgr::GetTextureAbsolutePath(GFXDevice* pDevice, const char* szTextureName, GPULocation eGPULocation)
{
	U8String u8Name = szTextureName;
	TextureHndl	pTexture = m_pImpl->textures.GetResourceHndl(u8Name);

	// TODO: Remove from the final build, should load in blocks
	if(!pTexture)
	{
		// FIXME: Make this platform independent and use a pre-loaded dummy texture
		if(Texture::FileExists(szTextureName))
		{
			m_pImpl->textures.StartLoad();
			Texture* pNC = vnew(ALLOC_RESOURCE_MGR) Texture;
			pNC->Load(pDevice, szTextureName, eGPULocation);
			pTexture = m_pImpl->textures.AddResource(pNC);
		}
		else
		{
			DEBUG_PRINT("Unable to load texture %s\n", szTextureName);
			if(!str::Find(szTextureName, "missing_texture"))
			{
				return GetTextureAbsolutePath(pDevice, "Textures/missing_texture", eGPULocation);
			}
			else
			{
				return nullptr;
			}
			
		}

	}
	else
	{
		// For the editor
#ifdef PLATFORM_PC
		if(m_bReloadIfDirty)
		{
			const_cast<Texture*>(pTexture.get())->Load(pDevice, szTextureName, eGPULocation);
		}
#endif
	}


	return pTexture;
}

TextureHndl	ResourceMgr::GetTexture(GFXDevice* pDevice, const char* szTextureName, GPULocation eLocation)
{
	U8String path = m_textureDir + szTextureName;
	return GetTextureAbsolutePath(pDevice, path.CStr(), eLocation);
}


ModelResHndl ResourceMgr::GetModel(GFXDevice* pDevice, const char* szModelName, bool bFastMem)
{
	const bool bInstance = false;
	return _GetModel( pDevice, szModelName, bInstance, bFastMem);
}

ModelResHndl ResourceMgr::GetModelAsInstance(GFXDevice* pDevice, const char* szModelName)
{
	const bool bInstance = true;
	return _GetModel( pDevice, szModelName, bInstance );
}

FontHndl ResourceMgr::GetFont( GFXDevice* pDevice, const char* szFontName )
{
	U8String u8Name = m_fontDir + szFontName;
	FontHndl pFont = m_pImpl->fonts.GetResourceHndl(u8Name);
	if( !pFont )
	{
		if ( pDevice == nullptr )
		{
			DEBUG_PRINT("Font not loaded and no device specified to load font using.");
		}
		else
		{
			m_pImpl->fonts.StartLoad();
			Font* pNC = vnew(ALLOC_RESOURCE_MGR) Font;
			pNC->Load(pDevice, u8Name.CStr());
			pFont = m_pImpl->fonts.AddResource(pNC);
		}
	}
	return pFont;
}



ParticleEmitterResHndl ResourceMgr::GetParticleEmitter(GFXDevice* pDevice, const char* szFileName)
{
	U8String path = "Particle/";
	path += szFileName;
	path += ".pem";
	ParticleEmitterResHndl pEffect = m_pImpl->particleEmitters.GetResourceHndl(path.CStr());
	if (!pEffect)
	{
		if (File::FileStatus(path.CStr()) == FILE_STATUS_VALID)
		{
			m_pImpl->particleEmitters.StartLoad();
			ParticleEmitterResource* pResPtr = vnew(ALLOC_RESOURCE_MGR) ParticleEmitterResource;

			bool b = pResPtr->Load(pDevice, m_pImpl->renderPasses, path.CStr());
			ASSERT(b);
			pEffect = m_pImpl->particleEmitters.AddResource(pResPtr);
		}
		else
		{
			DEBUG_PRINT("Particle emitter not found!!! %s\n", path.CStr());
		}
	}

	return pEffect;
}


ParticleEffectResHndl ResourceMgr::GetParticleEffect(const char* szFileName)
{
	U8String path = "Particle/";
	path += szFileName;
	path += ".pfx";
	ParticleEffectResHndl pEffect = m_pImpl->particleEffects.GetResourceHndl(path.CStr());
	if (!pEffect)
	{
		if (File::FileStatus(path.CStr()) == FILE_STATUS_VALID)
		{
			m_pImpl->particleEffects.StartLoad();
			ParticleEffectResource* pNC = vnew(ALLOC_RESOURCE_MGR) ParticleEffectResource;
			bool b = pNC->Load(path.CStr());
			ASSERT(b);
			pEffect = m_pImpl->particleEffects.AddResource(pNC);
		}
		else
		{
			DEBUG_PRINT("Particle effect not found!!! %s\n", path.CStr());
		}
	}

	return pEffect;
}

SkeletalAnimationResHndl ResourceMgr::GetSkeletalAnimation( const char* szFileName )
{
	U8String path = m_modelDir + szFileName;
	SkeletalAnimationResHndl p = m_pImpl->skeletalAnims.GetResourceHndl(path.CStr());
	if( !p )
	{
		if( File::FileStatus( path.CStr() ) == FILE_STATUS_VALID)
		{
			m_pImpl->skeletalAnims.StartLoad();
			SkeletalAnimationResource* pNC = vnew(ALLOC_RESOURCE_MGR) SkeletalAnimationResource;

			bool b = pNC->Load( path.CStr() );
			ASSERT( b );
			p = m_pImpl->skeletalAnims.AddResource( pNC );
		}
		else {
			DEBUG_PRINT( "!!!Skeletal animation not found!!! %s\n", path.CStr() );
		}
	}
	return p;
}

void ResourceMgr::FinishedStaticLoad()
{
	for (List<ResourceDataBase>::Iterator it = m_pImpl->resourceSets.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->SetTag(1);
		(*it)->SetStaticLoading(false);	// Theoretically allow garbage collection
	}
}

void ResourceMgr::ClearDynamicResources(GFXDevice* pDevice)
{
	for (List<ResourceDataBase>::Iterator it = m_pImpl->resourceSets.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->FreeResourcesWithTag(pDevice, 1);
	}
}

void ResourceMgr::ClearAllResources(GFXDevice* pDevice)
{
	for (List<ResourceDataBase>::Iterator it = m_pImpl->resourceSets.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->FreeAllResources(pDevice);
	}
}

ModelResHndl ResourceMgr::_GetModel(GFXDevice* pDevice, const char* szModelName, bool bInstance, bool bFastMem)
{
	U8String u8Name = m_modelDir + szModelName;
	ModelResHndl pModel = m_pImpl->models.GetResourceHndl(u8Name);
	if(!pModel)
	{
		m_pImpl->models.StartLoad();
		ModelResource* pNC = vnew(ALLOC_RESOURCE_MGR) ModelResource;
		pNC->Load(pDevice, u8Name.CStr(), m_pImpl->renderPasses, bInstance, bFastMem);
		pModel = m_pImpl->models.AddResource(pNC);
	}
	return pModel;
}


uint32 ResourceMgr::GetParticleEffectCount()
{
	return m_pImpl->particleEffects.GetResourceCount();
}

const ParticleEffectResource* ResourceMgr::GetParticleEffectByIndex(uint32 uIndex)
{
	return m_pImpl->particleEffects.GetResource(uIndex);
}

#ifdef DEBUG_BUILD

struct Report
{
	char name[64];
	uint32 uSize;
};

inline int CompareResources(const void* a, const void* b)   // comparison function
{
	const Report* pReport1 = (const Report*)(a);
	const Report* pReport2 = (const Report*)(b);
	if(pReport2->uSize < pReport1->uSize) return -1;
	if(pReport2->uSize > pReport1->uSize) return 1;
	return 0;
}

void ResourceMgr::ReportMemoryUsage()
{
	uint32 uCount = m_pImpl->textures.GetResourceCount();
	Report* pReports = nullptr;
	ScratchObj<Report> scratch(pReports, uCount);

	for(uint32 i=0; i<uCount; i++)
	{
		const Texture* pTexture = m_pImpl->textures.GetResource(i);
		str::Copy(pReports[i].name, pTexture->GetName().CStr(), 64);
		pReports[i].uSize = pTexture->GetSizeInMemory();
	}

	qsort(pReports, uCount, sizeof(Report), CompareResources);

	File fileOut("FileUsage.txt", FILE_ACCESS_WRITE, FILE_TYPE_DEBUG_DATA);
	U8String textOut;
	for(uint32 i=0; i<uCount; i++)
	{
		U8String text;
		text.ParseString("%s: %u\n", pReports[i].name, pReports[i].uSize);
		textOut += text;
	}
	fileOut.Write(textOut.Length() + 1, (void*)textOut.CStr());
}

void ResourceMgr::DebugPrintTimings()
{
#ifdef DEBUG_RESOURCE_MGR
	DEBUG_PRINT("Effects Find: %f Load %f\n", m_pImpl->effects.GetFindTime(), m_pImpl->effects.GetLoadTime());
	DEBUG_PRINT("Textures Find: %f Load %f\n", m_pImpl->textures.GetFindTime(), m_pImpl->textures.GetLoadTime());
	DEBUG_PRINT("Models Find: %f Load %f\n", m_pImpl->models.GetFindTime(), m_pImpl->models.GetLoadTime());
	DEBUG_PRINT("Part Emitters Find: %f Load %f\n", m_pImpl->particleEmitters.GetFindTime(), m_pImpl->particleEmitters.GetLoadTime());
	DEBUG_PRINT("Part Effects Find: %f Load %f\n", m_pImpl->particleEffects.GetFindTime(), m_pImpl->particleEffects.GetLoadTime());
	DEBUG_PRINT("Collision model Find: %f Load %f\n", m_pImpl->collisionModel.GetFindTime(), m_pImpl->collisionModel.GetLoadTime());
	DEBUG_PRINT("Protocol buffers Find: %f Load %f\n", m_pImpl->protocolBuffers.GetFindTime(), m_pImpl->protocolBuffers.GetLoadTime());
	DEBUG_PRINT("Skeletal anims Find: %f Load %f\n", m_pImpl->skeletalAnims.GetFindTime(), m_pImpl->skeletalAnims.GetLoadTime());

	for (List<ResourceDataBase>::Iterator it = m_pImpl->resourceSets.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->ClearTimers();
	}
#endif
}

#endif

}
