/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector4f.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Effects/Shader.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Resource/ResourceMgr.h"
#include "engine/core/String/StringCRC.h"
#include "Engine/core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Resource/SkeletalAnimationResource.h"
#include "Engine/Resource/MaterialAnimationResource.h"
#include "Engine/Resource/ParticleEffectResource.h"
#include "Engine/Resource/ParticleEmitterResource.h"
#include "Engine/Resource/CollisionModelResource.h"
#include "Engine/Resource/CustomEffectResource.h"
#include "Engine/Layout/Fonts/Font.h"
#include "Engine/Core/stl/string.h"
#include "Engine/Resource/ResourceData.h"
#include "Engine/Resource/ResourceDictionary.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Resource/PakFile.h"
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
		ResourceData					resources;
	};

ResourceMgr* ResourceMgr::m_pResource = nullptr;

ResourceMgr::ResourceMgr(void)
{
	m_pImpl = vnew(ALLOC_OBJECT) PIMPL;

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
	usg::string name = szPath;
	name += szName;
	name += ".pak";
	ResourcePakHndl hndl = m_pImpl->resources.GetResourceHndl(name.c_str(), ResourceType::PAK_FILE);
	// Only load if we don't already have one
	if (!hndl)
	{
		if (File::FileStatus(name.c_str()) == FILE_STATUS_VALID)
		{
#ifdef DEBUG_SHOW_PAK_LOAD_TIME
			ProfilingTimer loadTimer;
			loadTimer.Start();
#endif
			PakFile* pakFile = vnew(ALLOC_OBJECT)PakFile;
			pakFile->Load(pDevice, name.c_str());
			usg::map<uint32, BaseResHandle>& resources = pakFile->GetResources();
			for (auto& itr : resources)
			{
				m_pImpl->resources.AddResource(itr.second);
			}
			pakFile->ClearHandles();
			m_pImpl->resources.AddResource(pakFile);
#ifdef DEBUG_SHOW_PAK_LOAD_TIME
			loadTimer.Stop();
			DEBUG_PRINT("Loaded %s in %f milliseconds\n", name.c_str(), loadTimer.GetTotalMilliSeconds());
#endif
		}
	}
	// Nothing on PC yet so no assert
}


EffectHndl ResourceMgr::GetEffect(GFXDevice* pDevice, const char* szEffectName)
{
	usg::string stringName = szEffectName;

	string packageName = stringName.substr(0, stringName.find_first_of("."));
	LoadPackage(pDevice, m_effectDir.c_str(), packageName.c_str());
	stringName += ".fx";

	EffectHndl pEffect = m_pImpl->resources.GetResourceHndl(stringName, ResourceType::EFFECT);

	return pEffect;


}

CollisionModelResHndl ResourceMgr::GetCollisionModel(const char* szFileName)
{
	usg::string u8Name = szFileName;
	CollisionModelResHndl pModel = m_pImpl->resources.GetResourceHndl(u8Name, ResourceType::COLLISION);

	// TODO: Remove from the final build, should load in blocks
	if (!pModel)
	{
		CollisionModelResource* pModelPtr = vnew(ALLOC_RESOURCE_MGR) CollisionModelResource;
		m_pImpl->resources.StartLoad();
		pModelPtr->Init(szFileName);
		pModel = m_pImpl->resources.AddResource(pModelPtr);
	}

	return pModel;
}


CustomEffectResHndl ResourceMgr::GetCustomEffectRes(GFXDevice* pDevice, const char* szFileName)
{
	usg::string u8Name = szFileName;
	CustomEffectResHndl pEffect = m_pImpl->resources.GetResourceHndl(u8Name, ResourceType::CUSTOM_EFFECT);

	// TODO: Remove from the final build, should load in blocks
	if (!pEffect)
	{
		CustomEffectResource* pEffectPtr = vnew(ALLOC_RESOURCE_MGR) CustomEffectResource;
		m_pImpl->resources.StartLoad();
		pEffectPtr->Init(pDevice, szFileName);
		pEffect = m_pImpl->resources.AddResource(pEffectPtr);
	}

	return pEffect;
}


ProtocolBufferFile* ResourceMgr::GetBufferedFile(const char* szFileName)
{
	usg::string u8Name = szFileName;
	ProtocolBufferFile* pFile = const_cast<ProtocolBufferFile*>(m_pImpl->resources.GetResource<ProtocolBufferFile>(u8Name, ResourceType::PROTOCOL_BUFFER));

	if(!pFile)
	{
		m_pImpl->resources.StartLoad();
		pFile = vnew(ALLOC_RESOURCE_MGR) ProtocolBufferFile(szFileName, FILE_ACCESS_READ, FILE_TYPE_RESOURCE);
		pFile->SetupHash(szFileName);
		m_pImpl->resources.AddResource(pFile);
	}

	if(pFile)
	{
		pFile->Reset();
	}

	return pFile;
}

TextureHndl	 ResourceMgr::GetTextureAbsolutePath(GFXDevice* pDevice, const char* szTextureName, bool bReplaceMissingTex, GPULocation eGPULocation)
{
	usg::string u8Name = szTextureName;
	TextureHndl	pTexture = m_pImpl->resources.GetResourceHndl(u8Name, ResourceType::TEXTURE);

	// TODO: Remove from the final build, should load in blocks
	if(!pTexture)
	{
		// FIXME: Make this platform independent and use a pre-loaded dummy texture
		if(Texture::FileExists(szTextureName))
		{
			m_pImpl->resources.StartLoad();
			Texture* pNC = vnew(ALLOC_RESOURCE_MGR) Texture;
			pNC->Load(pDevice, szTextureName, eGPULocation);
			pTexture = m_pImpl->resources.AddResource(pNC);
		}
		else
		{
			if(!str::Find(szTextureName, "missing_texture") && bReplaceMissingTex)
			{
				DEBUG_PRINT("Unable to load texture %s\n", szTextureName);
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
	usg::string path = m_textureDir + szTextureName;
	return GetTextureAbsolutePath(pDevice, path.c_str(), true, eLocation);
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
	usg::string u8Name = m_fontDir + szFontName;
	FontHndl pFont = m_pImpl->resources.GetResourceHndl(u8Name, ResourceType::FONT);
	if( !pFont )
	{
		if ( pDevice == nullptr )
		{
			DEBUG_PRINT("Font not loaded and no device specified to load font using.");
		}
		else
		{
			m_pImpl->resources.StartLoad();
			Font* pNC = vnew(ALLOC_RESOURCE_MGR) Font;
			pNC->Load(pDevice, this, u8Name.c_str());
			pFont = m_pImpl->resources.AddResource(pNC);
		}
	}
	return pFont;
}



ParticleEmitterResHndl ResourceMgr::GetParticleEmitter(GFXDevice* pDevice, const char* szFileName)
{
	usg::string path = "Particle/";
	path += szFileName;
	path += ".pem";
	ParticleEmitterResHndl pEffect = m_pImpl->resources.GetResourceHndl(path.c_str(), ResourceType::PARTICLE_EMITTER);
	if (!pEffect)
	{
		if (File::FileStatus(path.c_str()) == FILE_STATUS_VALID)
		{
			m_pImpl->resources.StartLoad();
			ParticleEmitterResource* pResPtr = vnew(ALLOC_RESOURCE_MGR) ParticleEmitterResource;

			bool b = pResPtr->Load(pDevice, path.c_str());
			ASSERT(b);
			pEffect = m_pImpl->resources.AddResource(pResPtr);
		}
		else
		{
			DEBUG_PRINT("Particle emitter not found!!! %s\n", path.c_str());
		}
	}

	return pEffect;
}


ParticleEffectResHndl ResourceMgr::GetParticleEffect(const char* szFileName)
{
	usg::string path = "Particle/";
	path += szFileName;
	path += ".pfx";
	ParticleEffectResHndl pEffect = m_pImpl->resources.GetResourceHndl(path.c_str(), ResourceType::PARTICLE_EFFECT);
	if (!pEffect)
	{
		if (File::FileStatus(path.c_str()) == FILE_STATUS_VALID)
		{
			m_pImpl->resources.StartLoad();
			ParticleEffectResource* pNC = vnew(ALLOC_RESOURCE_MGR) ParticleEffectResource;
			bool b = pNC->Load(path.c_str());
			ASSERT(b);
			pEffect = m_pImpl->resources.AddResource(pNC);
		}
		else
		{
			DEBUG_PRINT("Particle effect not found!!! %s\n", path.c_str());
		}
	}

	return pEffect;
}

MaterialAnimationResHndl ResourceMgr::GetMaterialAnimation(const char* szFileName)
{
	usg::string path = m_modelDir + szFileName;
	SkeletalAnimationResHndl p = m_pImpl->resources.GetResourceHndl(path.c_str(), ResourceType::MAT_ANIM);
	if (!p)
	{
		if (File::FileStatus(path.c_str()) == FILE_STATUS_VALID)
		{
			m_pImpl->resources.StartLoad();
			MaterialAnimationResource* pNC = vnew(ALLOC_RESOURCE_MGR) MaterialAnimationResource;

			bool b = pNC->Load(path.c_str());
			ASSERT(b);
			p = m_pImpl->resources.AddResource(pNC);
		}
		else {
			DEBUG_PRINT("!!!Material animation not found!!! %s\n", path.c_str());
		}
	}
	return p;
}

SkeletalAnimationResHndl ResourceMgr::GetSkeletalAnimation( const char* szFileName )
{
	usg::string path = m_modelDir + szFileName;
	SkeletalAnimationResHndl p = m_pImpl->resources.GetResourceHndl(path.c_str(), ResourceType::SKEL_ANIM);
	if( !p )
	{
		if( File::FileStatus( path.c_str() ) == FILE_STATUS_VALID)
		{
			m_pImpl->resources.StartLoad();
			SkeletalAnimationResource* pNC = vnew(ALLOC_RESOURCE_MGR) SkeletalAnimationResource;

			bool b = pNC->Load( path.c_str() );
			ASSERT( b );
			p = m_pImpl->resources.AddResource( pNC );
		}
		else {
			DEBUG_PRINT( "!!!Skeletal animation not found!!! %s\n", path.c_str() );
		}
	}
	return p;
}

void ResourceMgr::FinishedStaticLoad()
{
	m_pImpl->resources.SetTag(1);
}

void ResourceMgr::ClearDynamicResources(GFXDevice* pDevice)
{
	m_pImpl->resources.FreeResourcesWithTag(pDevice, 1);
}

void ResourceMgr::ClearAllResources(GFXDevice* pDevice)
{
	m_pImpl->resources.FreeAllResources(pDevice);
}

ModelResHndl ResourceMgr::_GetModel(GFXDevice* pDevice, const char* szModelName, bool bInstance, bool bFastMem)
{
	usg::string u8Name = m_modelDir + szModelName;
	ModelResHndl pModel = m_pImpl->resources.GetResourceHndl(u8Name.c_str(), ResourceType::MODEL);
	if(!pModel)
	{
		m_pImpl->resources.StartLoad();
		ModelResource* pNC = vnew(ALLOC_RESOURCE_MGR) ModelResource;
		pNC->Load(pDevice, u8Name.c_str(), bInstance, bFastMem);
		pModel = m_pImpl->resources.AddResource(pNC);
	}
	return pModel;
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
	uint32 uCount = m_pImpl->resources.GetResourceCount();
	Report* pReports = nullptr;
	ScratchObj<Report> scratch(pReports, uCount);

	for(uint32 i=0; i<uCount; i++)
	{
		// TODO: Make name common to all resources, atleast in debug so we can sort all resources
		const Texture* pRes = m_pImpl->resources.GetResource<Texture>(i);
		if (pRes && pRes->GetSizeInMemory() > 0)
		{
			str::Copy(pReports[i].name, pRes->GetName().c_str(), 64);
			pReports[i].uSize = pRes->GetSizeInMemory();
		}
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
