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

}

#endif	// #ifndef _USG_RESOURCE_PAK_FILE_H_
