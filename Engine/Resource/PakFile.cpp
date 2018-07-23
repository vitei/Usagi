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


		}
		
	}

	void PakFile::CleanUp(GFXDevice* pDevice)
	{

	}


}

