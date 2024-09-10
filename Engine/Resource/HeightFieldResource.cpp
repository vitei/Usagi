/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "HeightFieldResource.h"
#include "Engine/Physics/PhysX.h"
#include "Engine/Physics/PhysXMeshCache.h"


using namespace usg;

HeightFieldResource::~HeightFieldResource()
{
	
}


bool HeightFieldResource::Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData)
{
	const PakFileDecl::HeightfieldHeader* pHeightData = PakFileDecl::GetCustomHeader< PakFileDecl::HeightfieldHeader>(pFileHeader);
	m_uRows = pHeightData->uRows;
	m_uColumns = pHeightData->uColumns;

	m_pHeightData = (physx::PxHeightFieldSample*)pData;
	m_bOwnsData = false;
	SetupHash(pFileHeader->szName);
	SetReady(true);

	return true;
}


bool HeightFieldResource::Init(const char* szName, uint32 uWidth, uint32 uHeight, sint16* pData)
{
	m_uColumns = uWidth;
	m_uRows = uHeight;

	uint32 uCount = uWidth * uHeight;
	uint32 uSize = uCount * sizeof(physx::PxHeightFieldSample);
	m_pHeightData = (physx::PxHeightFieldSample*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, uSize);
	for (uint32 i = 0; i < uCount; i++)
	{
		m_pHeightData[i].clearTessFlag();
		m_pHeightData[i].height = pData[i];
		m_pHeightData[i].materialIndex0 = 0;
		m_pHeightData[i].materialIndex1 = 0;
	}

	SetupHash(szName);
	SetReady(true);

	return true;
}

void HeightFieldResource::Cleanup(GFXDevice* pDevice)
{
	if (m_bOwnsData && m_pHeightData)
	{
		mem::Free(m_pHeightData);
		m_pHeightData = nullptr;
	}
}
