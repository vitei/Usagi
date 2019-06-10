/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Effect.h"


namespace usg {


	Effect::Effect() :
		ResourceBase(StaticResType)
	{
	}

	Effect::~Effect()
	{
	}

	bool Effect::Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const FileDependencies* pDependencies, const void* pData)
	{
		SetupHash(pFileHeader->szName);
		bool bLoaded = m_platform.Init(pDevice, pFileHeader, pDependencies, pData, pFileHeader->uDataSize);
		// FIXME: This should be done internally
		SetReady(true);
		return bLoaded;
	}


}
