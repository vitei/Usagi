/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: PakFile for multiple files merged into one
*****************************************************************************/
#ifndef _USG_RESOURCE_PAK_FILE_H_
#define _USG_RESOURCE_PAK_FILE_H_
#include "Engine/Common/Common.h"
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
		void CleanUp(GFXDevice* pDevice);

		// This should be valid before threading so we can return handles to resources
		usg::map<uint32, ResourceBase*>& GetResources() { return m_resources; }
		ResourceBase* GetResource(uint32 uCRC);

	private:
		void LoadFile(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFielInfo, void* pFileScratch);

		usg::map<uint32, ResourceBase*>	m_resources;

	};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_H_
