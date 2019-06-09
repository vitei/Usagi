/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Effect.h"


namespace usg {


	Effect::Effect() :
		ResourceBase(StaticResType),
		m_pSamplers(nullptr),
		m_pAttributes(nullptr),
		m_pConstantSets(nullptr),
		m_pHeader(nullptr),
		m_pBinary(nullptr)
	{
	}

	Effect::~Effect()
	{
		if (m_pBinary)
		{
			mem::Free(m_pBinary);
			m_pBinary = nullptr;
		}
	}

	bool Effect::Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const FileDependencies* pDependencies, const void* pData)
	{
		const PakFileDecl::EffectEntry* pEffectHdr = PakFileDecl::GetCustomHeader<PakFileDecl::EffectEntry>(pFileHeader);
		m_pBinary = mem::Alloc(MEMTYPE_STANDARD, ALLOC_GFX_SHADER, pFileHeader->uCustomHeaderSize, 8);
		MemCpy(m_pBinary, pEffectHdr, pFileHeader->uCustomHeaderSize);
		m_pHeader = (PakFileDecl::EffectEntry*)m_pBinary;

		SetupHash(pFileHeader->szName);
		bool bLoaded = m_platform.Init(pDevice, pFileHeader, pDependencies, pData, pFileHeader->uDataSize);
		// FIXME: This should be done internally
		SetReady(true);
		return bLoaded;
	}


}
