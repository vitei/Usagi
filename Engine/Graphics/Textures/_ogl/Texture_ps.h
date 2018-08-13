/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_TEXTURE_H
#define _USG_GRAPHICS_PC_TEXTURE_H
#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class Texture;
class Effect;
class GFXDevice;
class Sampler;
typedef struct _TexturePak TexturePak;

class Texture_ps
{
public:
	Texture_ps();
	~Texture_ps();

	void Init(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uMipmaps, void* pPixels = NULL, TextureDimensions eTexDim = TD_TEXTURE2D);
	void Init(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight);
	void InitArray(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices);
	void InitArray(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices);
	void InitCubeMap(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight);
	void CleanUp(GFXDevice* pDevice);

	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	bool Load(GFXDevice* pDevice, const char* szFileName, GPULocation eLocation);
	bool Load(GFXDevice* pDevice, const TexturePak& pak, const void* pData) { ASSERT(false); return false; }	// Not yet implemented on PC
	static bool FileExists(const char* szFileName);

	void BindInt(uint32 uUnit) const;
	bool IsValid() const { return m_glTexHndl != 0; }
	GLuint GetTexHndl() const { return m_glTexHndl; }
	void SetSampler(uint32 uUnit, Sampler* pSampler) const;

	uint32 GetWidth () const;
	uint32 GetHeight() const;

#ifdef DEBUG_BUILD
	uint32 GetSizeInMemory() const { return 0; }
#endif

	void NotifyOfTextureID(uint32 uTexID) { }
	// No descriptors on OGL so nothing to get out of date
	uint32 GetUpdateIdx() const { return 0; }

private:
	void Reset();
	bool LoadWithGLI(const char* szFileName);
	bool LoadTGA(const char* szFileName);

	struct Header
	{
		uint32 uWidth;
		uint32 uHeight;
		uint32 uDepth;
		uint32 uFormat;
		uint32 uType;
	};


//	void BuildTextureData(TextureFormat eFormat, uint8* pData, bool bGenMips);

	mutable Sampler*	m_activeSampler;
	GLenum			m_texType;
	GLuint			m_glTexHndl;
	uint32			m_uWidth;
	uint32			m_uHeight;
	uint32			m_formatMapIndex;
	bool			m_bIsDepthFormat;
	bool			m_bHasMipMaps;
};

}

#endif
