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
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Core/File/File.h"
#include "Engine/Memory/ScratchRaw.h"
#include "PakDecl.h"
#include "PakFile.h"

namespace usg
{

	PakFile::PakFile()
	{
		
	}

	PakFile::~PakFile()
	{
		
	}


	void PakFile::Load(GFXDevice* pDevice, const char* szFileName)
	{
		File pakFile(szFileName);
		ScratchRaw scratch(pakFile.GetSize(), FILE_READ_ALIGN);
		pakFile.Read(pakFile.GetSize(), scratch.GetRawData());

		const PakFileDecl::ResourcePakHdr* pHeader = scratch.GetDataAtOffset<PakFileDecl::ResourcePakHdr>(0);
		
		if (pHeader->uFileCount == 0 || pHeader->uVersionId != PakFileDecl::CURRENT_VERSION)
		{
			ASSERT(false);
			return;
		}

		const PakFileDecl::FileInfo* pFileInfo = scratch.GetDataAtOffset<PakFileDecl::FileInfo>(sizeof(PakFileDecl::ResourcePakHdr));
		for (uint32 i = 0; i < pHeader->uFileCount; i++)
		{
			pFileInfo = (PakFileDecl::FileInfo*)((uint8*)pFileInfo+pFileInfo->uTotalFileInfoSize);
			LoadFile(pDevice, pFileInfo, scratch.GetRawData());


		}
		
	}


	void PakFile::LoadFile(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileInfo, void* pFileScratch)
	{
		U8String name = pFileInfo->szName;
		name.ToLower();


		if (name.HasExtension("spv"))
		{
			Shader* pShader = vnew(ALLOC_OBJECT)Shader;
			pShader->Init(pDevice, pFileInfo->szName, ((uint8*)pFileScratch) + pFileInfo->uDataOffset, pFileInfo->uDataSize);
			m_resources[pFileInfo->CRC] = pShader;
		}
		else if (name.HasExtension("fx"))
		{
			Effect* pEffect = vnew(ALLOC_OBJECT)Effect;
			pEffect->Init(pDevice, this, pFileInfo, ((uint8*)pFileScratch) + pFileInfo->uDataOffset);
		}
	}

	ResourceBase* PakFile::GetResource(uint32 uCRC)
	{
		if (m_resources.find(uCRC) != m_resources.end())
		{
			return m_resources[uCRC];
		}

		return nullptr;
	}

	void PakFile::CleanUp(GFXDevice* pDevice)
	{

	}


}

