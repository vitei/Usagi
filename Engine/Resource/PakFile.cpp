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
#include "Engine/Resource/CustomEffectResource.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Core/File/File.h"
#include "Engine/Memory/ScratchRaw.h"
#include "PakDecl.h"
#include "PakFile.h"

namespace usg
{

	PakFile::PakFile() :
		ResourceBase(StaticResType)
	{
		m_pPersistantData = nullptr;
	}

	PakFile::~PakFile()
	{
		
	}


	void PakFile::Load(GFXDevice* pDevice, const char* szFileName)
	{
		SetupHash(szFileName);
		File pakFile(szFileName);

		PakFileDecl::ResourcePakHdr		header;

		pakFile.Read(sizeof(header), &header);
		pakFile.SeekPos(0);	// We will grab the header again to keep offsets consistent

		if (header.uFileCount == 0 || header.uVersionId != PakFileDecl::CURRENT_VERSION)
		{
			ASSERT(false);
			return;
		}

		size_t pakSize = pakFile.GetSize();
		size_t uPersistentDataSize = header.uResDataOffset == USG_INVALID_ID ? 0 : (uint32)pakFile.GetSize() - header.uResDataOffset;
		uint32 uTempDataSize = uPersistentDataSize > 0 ? header.uResDataOffset : (uint32)pakFile.GetSize();


		ScratchRaw scratch(uTempDataSize, FILE_READ_ALIGN);

		pakFile.Read(uTempDataSize, scratch.GetRawData());

		if (uPersistentDataSize > 0)
		{
			m_pPersistantData = mem::Alloc(MEMTYPE_STANDARD, ALLOC_OBJECT, uPersistentDataSize, FILE_READ_ALIGN);
			pakFile.Read(uPersistentDataSize, m_pPersistantData);
		}
	

		const PakFileDecl::FileInfo* pFileInfo = scratch.GetDataAtOffset<PakFileDecl::FileInfo>(sizeof(PakFileDecl::ResourcePakHdr));
		for (uint32 i = 0; i < header.uFileCount; i++)
		{
			LoadFile(pDevice, header.uResDataOffset, pFileInfo, scratch.GetRawData());
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
		default:
			ASSERT(false);
		}
		return nullptr;
	}

	void PakFile::LoadFile(GFXDevice* pDevice, uint32 uPersistentOffset, const PakFileDecl::FileInfo* pFileInfo, void* pFileScratch)
	{
		U8String name = pFileInfo->szName;
		name.ToLower();

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
		ResourceBase* pBaseRes = CreateResource((usg::ResourceType)pFileInfo->uResourceType);
	
		pBaseRes->Init(pDevice, pFileInfo, &deps, pData);
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


}

