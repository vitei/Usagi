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
#include "Engine/Graphics/Effects/Shader.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Resource/ParticleEffectResource.h"
#include "Engine/Resource/ParticleEmitterResource.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Resource/CollisionModelResource.h"
#include "Engine/Resource/CustomEffectResource.h"
#include "Engine/Resource/SkeletalAnimationResource.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Core/File/File.h"
#include "Engine/Memory/ScratchRaw.h"
#include "Engine/Core/Timer/ProfilingTimer.h"
#include "PakDecl.h"
#include "PakFile.h"

#define PAK_FILE_TIMINGS 0

namespace usg
{

	PakFile::PakFile() :
		ResourceBase(StaticResType)
	{
		m_pPersistantData = nullptr;
	}

	PakFile::~PakFile()
	{
		if (m_pPersistantData)
		{
			mem::Free(m_pPersistantData);
			m_pPersistantData = nullptr;
		}
	}


	void PakFile::Load(GFXDevice* pDevice, const char* szFileName)
	{
		ProfilingTimer loadTimer;

		SetupHash(szFileName);

#if PAK_FILE_TIMINGS
		loadTimer.ClearAndStart();
#endif

		File pakFile(szFileName);

#if PAK_FILE_TIMINGS
		loadTimer.Stop();
		LOG_MSG(DEBUG_MSG_RELEASE, "Opened %s in %f milliseconds\n", szFileName, loadTimer.GetTotalMilliSeconds());
#endif


#if PAK_FILE_TIMINGS
		loadTimer.ClearAndStart();
#endif
		PakFileDecl::ResourcePakHdr		header;

		pakFile.Read(sizeof(header), &header);
		pakFile.SeekPos(0);	// We will grab the header again to keep offsets consistent

		if (header.uFileCount == 0 || header.uVersionId != PakFileDecl::CURRENT_VERSION)
		{
			ASSERT(false);
			return;
		}
#if PAK_FILE_TIMINGS
		loadTimer.Stop();
		LOG_MSG(DEBUG_MSG_RELEASE, "Read header %s in %f milliseconds\n", szFileName, loadTimer.GetTotalMilliSeconds());
#endif

		size_t pakSize = pakFile.GetSize();
		size_t uPersistentDataSize = header.uResDataOffset == USG_INVALID_ID ? 0 : (uint32)pakFile.GetSize() - header.uResDataOffset;
		uint32 uTempDataSize = uPersistentDataSize > 0 ? header.uResDataOffset : (uint32)pakFile.GetSize();

#if PAK_FILE_TIMINGS
		loadTimer.ClearAndStart();
#endif
		ScratchRaw scratch(uTempDataSize, FILE_READ_ALIGN);
#if PAK_FILE_TIMINGS
		loadTimer.Stop();
		LOG_MSG(DEBUG_MSG_RELEASE, "Allocated data for %s in %f milliseconds\n", szFileName, loadTimer.GetTotalMilliSeconds());
#endif


#if PAK_FILE_TIMINGS
		loadTimer.ClearAndStart();
#endif
		pakFile.Read(uTempDataSize, scratch.GetRawData());
#if PAK_FILE_TIMINGS
		loadTimer.Stop();
		LOG_MSG(DEBUG_MSG_RELEASE, "Read file %s in %f milliseconds\n", szFileName, loadTimer.GetTotalMilliSeconds());
#endif
		if (uPersistentDataSize > 0)
		{
			m_pPersistantData = mem::Alloc(MEMTYPE_STANDARD, ALLOC_OBJECT, uPersistentDataSize, FILE_READ_ALIGN);
			pakFile.Read(uPersistentDataSize, m_pPersistantData);
		}

	

		const PakFileDecl::FileInfo* pFileInfo = scratch.GetDataAtOffset<PakFileDecl::FileInfo>(sizeof(PakFileDecl::ResourcePakHdr));
		for (uint32 i = 0; i < header.uFileCount; i++)
		{
#if PAK_FILE_TIMINGS
			loadTimer.ClearAndStart();
#endif
			LoadFile(pDevice, header.uResDataOffset, pFileInfo, scratch.GetRawData());
#if PAK_FILE_TIMINGS
			loadTimer.Stop();
			LOG_MSG(DEBUG_MSG_RELEASE, "Processed sub file %s in %f milliseconds\n", pFileInfo->szName, loadTimer.GetTotalMilliSeconds());
#endif
			pFileInfo = (PakFileDecl::FileInfo*)((uint8*)pFileInfo + pFileInfo->uTotalFileInfoSize);
		}
		
	}

	ResourceBase* PakFile::CreateResource(usg::ResourceType eType)
	{
		switch (eType)
		{
		case usg::ResourceType::EFFECT:
		{
			return vnew(ALLOC_OBJECT)Effect;
		}
		case usg::ResourceType::SHADER:
		{
			return vnew(ALLOC_OBJECT)Shader;
		}
		case usg::ResourceType::CUSTOM_EFFECT:
		{
			return vnew(ALLOC_OBJECT)CustomEffectResource;
		}
		case usg::ResourceType::TEXTURE:
		{
			return vnew(ALLOC_OBJECT)Texture;
		}
		case usg::ResourceType::MODEL:
		{
			return vnew(ALLOC_OBJECT)ModelResource;
		}
		case usg::ResourceType::SKEL_ANIM:
		{
			return vnew(ALLOC_OBJECT)SkeletalAnimationResource;
		}
		case usg::ResourceType::COLLISION:
		{
			return vnew(ALLOC_OBJECT)CollisionModelResource;
		}
		case usg::ResourceType::PARTICLE_EFFECT:
		{
			return vnew(ALLOC_OBJECT)ParticleEffectResource;
		}
		case usg::ResourceType::PARTICLE_EMITTER:
		{
			return vnew(ALLOC_OBJECT)ParticleEmitterResource;
		}
		default:
			ASSERT(false);
		}
		return nullptr;
	}

	void PakFile::LoadFile(GFXDevice* pDevice, uint32 uPersistentOffset, const PakFileDecl::FileInfo* pFileInfo, void* pFileScratch)
	{
		string name = pFileInfo->szName;
		name.make_lower();

		void* pData = nullptr;
		if (pFileInfo->uDataOffset == USG_INVALID_ID)
		{
			pData = nullptr;
		}
		else
		{
			if (pFileInfo->uFileFlags & PakFileDecl::FILE_FLAG_KEEP_DATA)
			{
				pData = ((uint8*)m_pPersistantData) - uPersistentOffset + pFileInfo->uDataOffset;
			}
			else
			{
				pData = ((uint8*)pFileScratch) + pFileInfo->uDataOffset;
			}
		}
		FileDependencies deps;
		if (pFileInfo->uDependenciesCount > 0)
		{
			const PakFileDecl::Dependency* pDependencies = PakFileDecl::GetDependencies(pFileInfo);
			deps.Init(this, pDependencies, pFileInfo->uDependenciesCount);
		}

		// FIXME: Make the init function virtual to save this mess
		ResourceBase* pBaseRes = nullptr;
		
		if ((usg::ResourceType)pFileInfo->uResourceType != usg::ResourceType::PROTOCOL_BUFFER)
		{
			pBaseRes = CreateResource((usg::ResourceType)pFileInfo->uResourceType);
			pBaseRes->Init(pDevice, pFileInfo, &deps, pData);
		}
		else
		{
			pBaseRes = vnew(ALLOC_OBJECT) ProtocolBufferFile(pData, pFileInfo->uDataSize);
			ProtocolBufferFile* pFile = (ProtocolBufferFile*)pBaseRes;
			pFile->SetupHash(pFileInfo->szName);
		}
	
		m_resources[pFileInfo->CRC] = BaseResHandle(pBaseRes);
	}

	BaseResHandle PakFile::GetResource(uint32 uCRC)
	{
		if (m_resources.find(uCRC) != m_resources.end())
		{
			return m_resources[uCRC];
		}

		return nullptr;
	}

	void PakFile::Cleanup(GFXDevice* pDevice)
	{

	}


	PakFileRaw::PakFileRaw()
	{

	}

	PakFileRaw::~PakFileRaw()
	{
		if (m_pData)
		{
			mem::Free(m_pData);
			m_pData = nullptr;
		}
	}

	void PakFileRaw::GetFilesOfType(ResourceType eType, usg::vector< string >& namesOut)
	{
		for (auto itr : m_files)
		{
			if (itr.second.pFileHeader->uResourceType == (uint32)eType)
			{
				namesOut.push_back(itr.second.pFileHeader->szName);
			}
		}
	}


	bool PakFileRaw::Load(const char* szFileName, bool bHeadersOnly)
	{
		File pakFile(szFileName);

		PakFileDecl::ResourcePakHdr		header;

		pakFile.Read(sizeof(header), &header);
		pakFile.SeekPos(0);	// We will grab the header again to keep offsets consistent

		if (header.uFileCount == 0 || header.uVersionId != PakFileDecl::CURRENT_VERSION)
		{
			ASSERT(false);
			return false;
		}

		size_t pakSize = pakFile.GetSize();
		size_t uPersistentDataSize = header.uResDataOffset == USG_INVALID_ID ? 0 : (uint32)pakFile.GetSize() - header.uResDataOffset;
		uint32 uTempDataSize = uPersistentDataSize > 0 ? header.uResDataOffset : (uint32)pakFile.GetSize();
		if (bHeadersOnly)
		{
			uTempDataSize = header.uTempDataOffset;
		}

		m_pData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_LOADING, uTempDataSize);

		pakFile.Read(uTempDataSize, m_pData);

		if (uPersistentDataSize > 0)
		{
			// Should be no persistent data in raw files
			ASSERT(false);
		}


		const PakFileDecl::FileInfo* pFileInfo = (const PakFileDecl::FileInfo*) (m_pData + sizeof(PakFileDecl::ResourcePakHdr));
		for (uint32 i = 0; i < header.uFileCount; i++)
		{
			FileRef fileRef;
			fileRef.pData = bHeadersOnly ? nullptr : ((uint8*)m_pData) + pFileInfo->uDataOffset;
			fileRef.pDependencies = bHeadersOnly ? nullptr : PakFileDecl::GetDependencies(pFileInfo);
			fileRef.pFileHeader = pFileInfo;

			ASSERT(m_files.find(fileRef.pFileHeader->CRC) == m_files.end());

			m_files[fileRef.pFileHeader->CRC] = fileRef;

			pFileInfo = (PakFileDecl::FileInfo*)((uint8*)pFileInfo + pFileInfo->uTotalFileInfoSize);

		}

		return true;
	}

	bool PakFileRaw::GetFile(const char* szName, FileRef& refOut) const
	{
		uint32 uCrc = utl::CRC32(szName);
		return GetFile(uCrc, refOut);
	}

	bool PakFileRaw::GetFile(uint32 uFileCRC, FileRef& refOut) const
	{
		auto itr = m_files.find(uFileCRC);
		if (itr != m_files.end())
		{
			refOut = (*itr).second;
			return true;
		}
		return false;
	}


}

