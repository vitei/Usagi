/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: PakFile for multiple files merged into one
*****************************************************************************/
#ifndef _USG_RESOURCE_PAK_FILE_H_
#define _USG_RESOURCE_PAK_FILE_H_

#include "Engine/Graphics/Device/GFXHandles.h"
#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Resource/PakDecl.h"
#include "Engine/Core/stl/map.h"
#include "Engine/Core/stl/string.h"
#include "CustomEffectDecl.h"

namespace usg
{

	class PakFile : public ResourceBase
	{
	public:
		PakFile();
		~PakFile();

		void Load(GFXDevice* pDevice, const char* szFileName);
		void Cleanup(GFXDevice* pDevice);

		// This should be valid before threading so we can return handles to resources
		usg::map<uint32, BaseResHandle>& GetResources() { return m_resources; }
		BaseResHandle GetResource(uint32 uCRC);
		const static ResourceType StaticResType = ResourceType::PAK_FILE;
		void ClearHandles() { m_resources.clear(); }

	private:
		void LoadFile(GFXDevice* pDevice, uint32 uPersistentOffset, const PakFileDecl::FileInfo* pFileInfo, void* pFileScratch);
		static ResourceBase* CreateResource(usg::ResourceType eType);
		
		void*							m_pPersistantData;

		usg::map<uint32, BaseResHandle>	m_resources;
	};

	// For loading outside of the resource manager
	class PakFileRaw
	{
	public:
		PakFileRaw();
		~PakFileRaw();

		bool Load(const char* szFileName, bool bHeadersOnly = false);

		struct FileRef
		{
			const PakFileDecl::FileInfo* pFileHeader = nullptr;
			const PakFileDecl::Dependency* pDependencies = nullptr;
			const void* pData = nullptr;
		};

		bool GetFile(const char* szName, FileRef& refOut) const;
		bool GetFile(uint32 uFileCRC, FileRef& refOut) const;
		void GetFilesOfType(ResourceType eType, usg::vector< string >& namesOut);

	private:
		map<uint32, FileRef > m_files;
		uint8*	m_pData = nullptr;
		uint32	m_uFileSize = 0;
	};

}

#endif	// #ifndef _USG_RESOURCE_PAK_FILE_H_
