/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Effect.h"


namespace usg {


	Effect::Effect() :
		m_pSamplers(nullptr),
		m_pAttributes(nullptr),
		m_pConstantSets(nullptr),
		m_pHeader(nullptr),
		m_pBinary(nullptr)
	{
		m_resourceType = ResourceType::EFFECT;
	}

	Effect::~Effect()
	{
		if (m_pBinary)
		{
			mem::Free(m_pBinary);
			m_pBinary = nullptr;
		}
	}

	bool Effect::Init(GFXDevice* pDevice, PakFile* pFile, const PakFileDecl::FileInfo* pFileHeader, const void* pData)
	{
		const PakFileDecl::EffectEntry* pEffectHdr = PakFileDecl::GetCustomHeader<PakFileDecl::EffectEntry>(pFileHeader);
		m_pBinary = mem::Alloc(MEMTYPE_STANDARD, ALLOC_GFX_SHADER, pFileHeader->uCustomHeaderSize, 8);
		MemCpy(m_pBinary, pEffectHdr, pFileHeader->uCustomHeaderSize);
		m_pHeader = (PakFileDecl::EffectEntry*)m_pBinary;

		m_name = pFileHeader->szName;
		SetupHash(m_name.CStr());
		bool bLoaded = m_platform.Init(pDevice, pFile, pFileHeader, pData, pFileHeader->uDataSize);
		// FIXME: This should be done internally
		SetReady(true);
		return bLoaded;
	}


}
