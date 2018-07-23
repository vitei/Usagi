/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The structs that comprise the binary 
*****************************************************************************/
#ifndef _USG_RESOURCE_PAK_FILE_DECL_H_
#define _USG_RESOURCE_PAK_FILE_DECL_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"

namespace usg
{
	namespace PakFileDecl
	{

		const uint32 CURRENT_VERSION = 0x1;

		struct ResourcePakHdr
		{
			uint32 uVersionId;
			uint32 uFileCount;
		};

		struct FileInfo
		{
			char			szName[64];
			uint32			CRC;
			uint32			uTotalFileInfoSize;
			uint32			uCustomHeaderSize;	// Straight after file info
			uint32			uDependenciesCount;	// After custom header
			uint32			uDataOffset;
			uint32			uDataSize;
		};

		struct Dependency
		{
			char   szName[64];
			uint32 FileCRC;
		};

		struct EffectEntry
		{
			uint32		CRC[(uint32)usg::ShaderType::COUNT] = {};
		};

		struct ShaderEntry
		{
			ShaderType	eShaderType;
		};

	}

}

#endif	// #ifndef _USG_RESOURCE_PAK_FILE_DECL_H_
