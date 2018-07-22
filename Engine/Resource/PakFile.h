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


	private:

	};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_H_
