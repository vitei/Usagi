/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_TEXTURE_H
#define _USG_GRAPHICS_TEXTURE_H
#include API_HEADER(Engine/Graphics/Textures, texture_ps.h)

#include "Engine/Resource/ResourceBase.h"


namespace usg {


class Texture : public ResourceBase
{
public:
	Texture(void);
	virtual ~Texture(void);

	virtual bool Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData) override;
	bool Load(GFXDevice* pDevice, const char* szFilename, GPULocation eLocation = GPU_LOCATION_FASTMEM);
	void UpdateTextureID();

	// TODO: Have create raw pass a texture format. For now we just assume RGBA8
	void CreateRaw(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, void* pPixels, bool bDynamic = false);
	void SetRawData(GFXDevice* pDevice, GFXContext* pContext, void* pData) { m_platform.SetRawData(pDevice, pContext, pData); }
	static bool FileExists(const char* szFileName) { return Texture_ps::FileExists(szFileName); }

	void Cleanup(GFXDevice* pDevice);

	Texture_ps&       GetPlatform()       { return m_platform; }
	const Texture_ps& GetPlatform() const { return m_platform; }
	const usg::string&   GetName    () const { return m_name; }
	uint16            GetId      () const { return m_uBindingId; }

	uint32 GetWidth () const;
	uint32 GetHeight() const;

#ifdef DEBUG_BUILD
	uint32 GetSizeInMemory() const override { return m_platform.GetSizeInMemory(); }
#endif

	// For keeping track of resizing/ recreation
	uint32 GetUpdateIdx() const { return m_platform.GetUpdateIdx(); }

	ImageViewHndl GetImageViewHandle(GFXDevice* pDevice, const ImageViewDef& def) { return m_platform.GetImageView(pDevice, def); }
	ImageViewHndl GetImageViewHandle() { return m_platform.GetImageView(); }

	const static ResourceType StaticResType = ResourceType::TEXTURE;

	enum 
	{
		MAX_TEXTUTRE_IDS = 4096
	};

private:
	PRIVATIZE_RES_COPY(Texture)

	static bool		m_sbTexIds[MAX_TEXTUTRE_IDS];
	Texture_ps		m_platform;
	usg::string		m_name;
	uint16			m_uBindingId;
};

// FIXME: Remove this when we only have one method of loading
inline void Texture::CreateRaw(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, void* pPixels, bool bDynamic)
{
	m_platform.Init(pDevice, eFormat, uWidth, uHeight, 1, pPixels, uHeight == 1 ? TD_TEXTURE1D : TD_TEXTURE2D, bDynamic ? TU_FLAG_SHADER_READ | TU_FLAG_TRANSFER_DST : TU_FLAG_SHADER_READ);
	SetReady(true);
}

}

#endif
