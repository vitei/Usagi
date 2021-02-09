/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Manages all of the loading/ unloading of game resources
//	TODO: Should be made multi-threaded, and to be honest the internals need
//	to be replaced entirely, it's just here to maintain an interface
*****************************************************************************/
#ifndef _USG_RESOURCE_RESOURCE_H_
#define _USG_RESOURCE_RESOURCE_H_

#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Graphics/Device/GFXHandles.h"
#include "Engine/Core/String/U8String.h"

namespace usg{


enum E_RESOURCE_GROUP
{
	E_RESOURCE_INIT = 0,
	E_RESOURCE_GLOBAL,
	E_RESOURCE_LEVEL,
	E_RESOURCE_COUNT
};

// TODO: Create a singleton parent class and register all singletons for cleanup
class ResourceMgr
{
public:
	static void Cleanup(usg::GFXDevice* pDevice);

	EffectHndl					GetEffect(GFXDevice* pDevice, const char* szEffectName);
	TextureHndl					GetTextureAbsolutePath(GFXDevice* pDevice, const char* szTextureName, bool bReplaceMissingTex = true, GPULocation eLocation = GPU_LOCATION_STANDARD);
	TextureHndl					GetTexture(GFXDevice* pDevice, const char* szTextureName, GPULocation eLocation = GPU_LOCATION_FASTMEM);
	ModelResHndl				GetModel(GFXDevice* pDevice, const char* szModelName, bool bFastMem=true);
	ModelResHndl				GetModelAsInstance(GFXDevice* pDevice, const char* szModelName);
	FontHndl					GetFont( GFXDevice* pDevice, const char* szFontName );
	SkeletalAnimationResHndl	GetSkeletalAnimation( const char* szFileName );
	MaterialAnimationResHndl	GetMaterialAnimation(const char* szFileName);
	ParticleEffectResHndl		GetParticleEffect(const char* szFileName);
	ParticleEmitterResHndl		GetParticleEmitter(GFXDevice* pDevice, const char* szFileName);
	CollisionModelResHndl		GetCollisionModel(const char* szFileName);

	// Contain customisable constant set data in addition to the effects
	CustomEffectResHndl			GetCustomEffectRes(GFXDevice* pDevice, const char* szFileName);

	void LoadPackage(usg::GFXDevice* pDevice, const char* szPath, const char* szName);

	// We can't return this as const as we need to iterate through it
	ProtocolBufferFile* GetBufferedFile(const char* szFileName);

	void SetModelDir(const char* szModelDir) { m_modelDir = szModelDir; }
	const U8String& GetModelDir() const { return m_modelDir; }
	void SetTextureDir(const char* szTextureDir) { m_textureDir = szTextureDir; }
	void SetEffectDir(const char* szEffectDir) { m_effectDir = szEffectDir; }
	void SetFontDir( const char* szFontDir ) { m_fontDir = szFontDir; }

	void FinishedStaticLoad();
	void ClearDynamicResources(GFXDevice* pDevice);
	void ClearAllResources(GFXDevice* pDevice);

	static ResourceMgr* Inst();

	void EnableReloadingOfDirtyAssets(bool bEnable) { m_bReloadIfDirty = bEnable; }
	void EnableLODS(bool bEnable) { m_bUseLODs = bEnable; }
	bool AreLodsEnabled() const { return m_bUseLODs;  }

#ifdef DEBUG_BUILD
	void ReportMemoryUsage();
#endif
	void DebugPrintTimings();

private:
	ModelResHndl _GetModel(GFXDevice* pDevice, const char* szModelName, bool bInstance, bool bFastMem = true);

	// TODO: Replace with dynamically resizing stack
	enum
	{
		MAX_SHARED_CONSTANTS = 150
	};

	ResourceMgr(void);
	~ResourceMgr(void);

	struct PIMPL;
	PIMPL*						m_pImpl;

	U8String					m_modelDir;
	U8String					m_textureDir;
	U8String					m_effectDir;
	U8String					m_fontDir;
	bool						m_bReloadIfDirty;
	bool						m_bUseLODs;

	static ResourceMgr*		m_pResource;
};

inline ResourceMgr* ResourceMgr::Inst()
{
	if(!m_pResource)
	{
		m_pResource = vnew(ALLOC_RESOURCE_MGR) ResourceMgr();
	}

	return m_pResource;
}

}

#endif
