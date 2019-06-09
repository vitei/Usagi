/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The structs that comprise the binary 
*****************************************************************************/
#ifndef _USG_RESOURCE_PAK_FILE_DECL_H_
#define _USG_RESOURCE_PAK_FILE_DECL_H_

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
			char			szName[128];
			uint32			CRC;
			uint32			uTotalFileInfoSize;
			uint32			uCustomHeaderSize;	// Straight after file info
			uint32			uDependenciesCount;	// After custom header
			uint32			uDataOffset;
			uint32			uDataSize;
			uint32			uResourceType;	// See ResourceBase
		};

		struct Dependency
		{
			uint32 FileCRC;
			uint32 PakIndex;	// USG_INVALID_ID if not present in this pak file
			uint32 UsageCRC;	// Hint to the resource how this is to be used
		};

		struct EffectEntry
		{
			uint32		CRC[(uint32)usg::ShaderType::COUNT] = {};
			uint32		uAttributeOffset;
			uint32		uAttributeCount;
			uint32		uSamplerOffset;
			uint32		uSamplerCount;
			uint32		uConstantSetDeclOffset;
			uint32		uConstantSetCount;
		};

		struct ShaderEntry
		{
			ShaderType	eShaderType;
		};

		template <class HeaderType>
		inline const HeaderType* GetCustomHeader(const FileInfo* pFileInfo)
		{
			ASSERT(sizeof(HeaderType) == pFileInfo->uCustomHeaderSize);
			return (HeaderType*)(((uint8*)pFileInfo) + sizeof(FileInfo));
		}

		inline const Dependency* GetDependencies(const FileInfo* pFileInfo)
		{
			return (Dependency*)(((uint8*)pFileInfo) + sizeof(FileInfo) + pFileInfo->uCustomHeaderSize);
		}


	}

}

#endif	// #ifndef _USG_RESOURCE_PAK_FILE_DECL_H_
