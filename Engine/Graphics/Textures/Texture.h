/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_TEXTURE_H
#define _USG_GRAPHICS_TEXTURE_H
#include "Engine/Common/Common.h"
#include "Engine/Core/String/U8String.h"
#include API_HEADER(Engine/Graphics/Textures, texture_ps.h)

#include "Engine/Resource/ResourceBase.h"


namespace usg {


class Texture : public ResourceBase
{
public:
	Texture(void);
	virtual ~Texture(void);

	bool Load(GFXDevice* pDevice, const char* szFilename, GPULocation eLocation = GPU_LOCATION_FASTMEM);
	bool Load(GFXDevice* pDevice, const TexturePak& pak, const void* pData, const char* szPackPath);
	void UpdateTextureID();

	// TODO: Have create raw pass a texture format. For now we just assume RGBA8
	void CreateRaw(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, void* pPixels);
	static bool FileExists(const char* szFileName) { return Texture_ps::FileExists(szFileName); }

	void CleanUp(GFXDevice* pDevice);

	Texture_ps&       GetPlatform()       { return m_platform; }
	const Texture_ps& GetPlatform() const { return m_platform; }
	const U8String&   GetName    () const { return m_name; }
	uint16            GetId      () const { return m_uBindingId; }

	uint32 GetWidth () const;
	uint32 GetHeight() const;

#ifdef DEBUG_BUILD
	uint32 GetSizeInMemory() const { return m_platform.GetSizeInMemory(); }
#endif

	enum 
	{
		MAX_TEXTUTRE_IDS = 4096
	};
private:
	PRIVATIZE_COPY(Texture)
	static bool m_sbTexIds[MAX_TEXTUTRE_IDS];
	Texture_ps m_platform;
	U8String   m_name;
	uint16     m_uBindingId;
};

// FIXME: Remove this when we only have one method of loading
inline void Texture::CreateRaw(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, void* pPixels)
{
	m_platform.Init(pDevice, eFormat, uWidth, uHeight, 1, pPixels);
	SetReady(true);
}

}

#endif
