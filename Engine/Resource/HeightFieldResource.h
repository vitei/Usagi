/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#include "Engine/Resource/ResourceBase.h"

namespace physx
{
struct PxHeightFieldSample;
}

namespace usg {

class Model;

class HeightFieldResource : public ResourceBase
{
public:
	HeightFieldResource() : ResourceBase(StaticResType) {}
	virtual ~HeightFieldResource();
	
	virtual bool Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData);

	virtual bool Init(const char* szName, uint32 uWidth, uint32 uHeight, sint16* pData);


	virtual void Cleanup(GFXDevice* pDevice);

	physx::PxHeightFieldSample* GetSamples() const { return m_pHeightData; }
	const static ResourceType StaticResType = ResourceType::HEIGHTFIELD;

	uint32 GetColumns() const { return m_uColumns; }
	uint32 GetRows() const { return m_uRows; }

private:
	
	physx::PxHeightFieldSample*	m_pHeightData = nullptr;
	uint32					m_uColumns = 0;
	uint32					m_uRows = 0;
	bool					m_bOwnsData = false;
};

}

