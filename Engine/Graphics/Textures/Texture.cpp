/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/String_Util.h"
#include "Texture.h" 

namespace usg {

bool Texture::m_sbTexIds[Texture::MAX_TEXTUTRE_IDS] = { false };
static uint16 g_uLastValidId = 0;

Texture::Texture(void)
	: ResourceBase(StaticResType)
{
	// FIXME: Make this thread safe with a critical section
	m_uBindingId = USG_INVALID_ID16;
	UpdateTextureID();
}

void Texture::UpdateTextureID()
{
	uint16 oldValue = m_uBindingId;
	for (uint16 i = 0; i < Texture::MAX_TEXTUTRE_IDS; i++)
	{
		uint16 uIndex = (g_uLastValidId + i) % Texture::MAX_TEXTUTRE_IDS;
		if (!m_sbTexIds[uIndex])
		{
			m_sbTexIds[uIndex] = true;
			m_uBindingId = uIndex;
			g_uLastValidId = uIndex;
			break;
		}
	}

	if (oldValue != USG_INVALID_ID16)
	{
		m_sbTexIds[oldValue] = false;
	}

	ASSERT(m_uBindingId != USG_INVALID_ID16);
	m_platform.NotifyOfTextureID(m_uBindingId);
}

Texture::~Texture(void)
{
	ASSERT(m_uBindingId >= 0 && m_uBindingId < Texture::MAX_TEXTUTRE_IDS);
	m_sbTexIds[m_uBindingId] = false;
	g_uLastValidId = m_uBindingId;
}

void Texture::Cleanup(GFXDevice* pDevice)
{
	m_platform.Cleanup(pDevice);
}

uint32 Texture::GetWidth() const
{
	ASSERT(IsReady());
	return m_platform.GetWidth();
}

uint32 Texture::GetHeight() const
{
	ASSERT(IsReady());
	return m_platform.GetHeight();
}

float Texture::GetAspect() const
{
	float fWidth = (float)GetWidth();
	float fHeight = float(GetHeight());

	return fWidth / fHeight;
}

bool Texture::Load(GFXDevice* pDevice, const char* szFilename, GPULocation eLocation)
{
	m_name = szFilename;
	SetupHash(m_name.c_str());
	bool bLoaded = m_platform.Load(pDevice, szFilename, eLocation);
	SetReady(true);
	return bLoaded;
}

bool Texture::Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData)
{
	// Remove extension
	// FIXME: More consistency over asset names
	m_name = pFileHeader->szName;
	str::TruncateExtension(m_name);
	SetupHash(m_name.c_str());
	const PakFileDecl::TextureHeader* pHdr = PakFileDecl::GetCustomHeader<PakFileDecl::TextureHeader>(pFileHeader);
	bool bLoaded = m_platform.Load(pDevice, pData, pFileHeader->uDataSize, pHdr);
	m_platform.SetName(pDevice, m_name.c_str());
	SetReady(true);
	return bLoaded;
}



}


