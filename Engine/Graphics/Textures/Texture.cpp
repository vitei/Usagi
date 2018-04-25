/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourcePak.pb.h"
#include "Texture.h" 

namespace usg {

bool Texture::m_sbTexIds[Texture::MAX_TEXTUTRE_IDS] = { false };
static uint16 g_uLastValidId = 0;

Texture::Texture(void)
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

void Texture::CleanUp(GFXDevice* pDevice)
{
	m_platform.CleanUp(pDevice);
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

bool Texture::Load(GFXDevice* pDevice, const char* szFilename, GPULocation eLocation)
{
	m_name = szFilename;
	SetupHash(m_name.CStr());
	bool bLoaded = m_platform.Load(pDevice, szFilename, eLocation);
	SetReady(true);
	return bLoaded;
}

bool Texture::Load(GFXDevice* pDevice, const TexturePak& pak, const void* pData, const char* szPackPath)
{
	// FIXME: When this is the only version store the width and height etc here
	m_name = szPackPath;
	m_name += pak.resHdr.strName;
	SetupHash(m_name.CStr());
	bool bLoaded = m_platform.Load(pDevice, pak, pData);
	// FIXME: This should be done internally
	SetReady(true);
	return bLoaded;
}

}


