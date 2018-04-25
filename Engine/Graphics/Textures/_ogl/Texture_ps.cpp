/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/Effects/Effect.h"
#include API_HEADER(Engine/Graphics/Textures, Sampler.h)
#include API_HEADER(Engine/Graphics/Textures, Texture_ps.h)
#include "gli/gli.hpp"

namespace usg {


struct GLTextureFormatMap
{
	GLint	intFormat;
	GLenum	format;
	GLenum	type;
};

static const GLTextureFormatMap gTextureFormatMap[] =
{
	{ GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE },		// CF_RGBA_8888
	{ GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE },		// CF_RGBA_5551
	{ GL_RGB8,				GL_RGB,				GL_UNSIGNED_BYTE },		// CF_RGBA_565
	{ GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE },		// CF_RGBA_4444
	{ GL_RGB8,				GL_RGB,				GL_UNSIGNED_BYTE },		// CF_RGB_888
	{ GL_R32F,				GL_RED,				GL_FLOAT },				// CF_SHADOW
	{ GL_RGBA16F,			GL_RGBA,			GL_FLOAT },				// CF_RGBA_16F
	{ GL_RGB16F,			GL_RGB,				GL_FLOAT },				// CF_HDR
	{ GL_R32F,				GL_RED,				GL_FLOAT },				// CF_R_32F
	{ GL_R32F,				GL_RED,				GL_UNSIGNED_INT },		// CF_R_32
	{ GL_RG32F,				GL_RG,				GL_FLOAT },				// CF_RG_32F
	{ GL_RG16F,				GL_RG,				GL_FLOAT },				// CF_R_16F
	{ GL_RG16F,				GL_RG,				GL_FLOAT },				// CF_RG_16F
	{ GL_R8,				GL_RED,				GL_UNSIGNED_BYTE },		// CF_R_8
	{ GL_RGB16F,			GL_RGB,				GL_FLOAT },				// CF_NORMAL
	{ GL_SRGB8_ALPHA8,		GL_RGBA,			GL_UNSIGNED_BYTE }		// CF_SRGBA
};

static_assert(ARRAY_SIZE(gTextureFormatMap) == CF_COUNT, "Number of entries in gTextureFormatMap doesn't match the ColorFormat enum, has an extra value been added?");

static const GLTextureFormatMap gDepthFormatMap[] =
{
	{ GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT } ,		// DF_DEPTH_24,
	{ GL_DEPTH24_STENCIL8,	GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8 } ,// DF_DEPTH_24_S8,
	{ GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT } ,		// DF_DEPTH_16,
	{ GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT },				// DF_DEPTH_32F,
	{ GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT },		// DF_DEPTH_32,
};

static_assert(ARRAY_SIZE(gDepthFormatMap) == DF_COUNT, "Number of entries in gTextureFormatMap doesn't match the ColorFormat enum, has an extra value been added?");

static GLuint CreateTexture(const GLTextureFormatMap* pIntFormat, GLenum texType, uint32 uWidth, uint32 uHeight, void* pPixels)
{
	GLuint textureName;

	glGenTextures(1, &textureName);
	CHECK_OGL_ERROR();
	glBindTexture(texType, textureName);
	CHECK_OGL_ERROR();
	switch (texType)
	{
	case GL_TEXTURE_1D:
		glTexImage1D(texType, 0, pIntFormat->intFormat, uWidth, 0, pIntFormat->format, pIntFormat->type, pPixels);
		break;
	case GL_TEXTURE_2D:
		glTexImage2D(texType, 0, pIntFormat->intFormat, uWidth, uHeight, 0, pIntFormat->format, pIntFormat->type, pPixels);
		break;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_CUBE_MAP:
		ASSERT(false);
		break;
	}

	CHECK_OGL_ERROR();

	return textureName;
}

Texture_ps::Texture_ps()
	: m_activeSampler(NULL)
	, m_texType(0)
	, m_glTexHndl(GL_INVALID_INDEX)
	, m_uWidth(0)
	, m_uHeight(0)
	, m_formatMapIndex(0)
	, m_bIsDepthFormat(false)
	, m_bHasMipMaps(false)
{
}

Texture_ps::~Texture_ps()
{
	ASSERT(m_glTexHndl = GL_INVALID_INDEX);
}

void Texture_ps::Reset()
{
	if (m_glTexHndl != GL_INVALID_INDEX)
	{
		glDeleteTextures(1, &m_glTexHndl);
		m_glTexHndl = GL_INVALID_INDEX;
	}
	m_activeSampler = NULL;
	m_bHasMipMaps = false;
}

void Texture_ps::Init(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uMipmaps, void* pPixels, TextureDimensions eTexDim)
{
	// If being resized then delete the old handles
	Reset();

	const GLTextureFormatMap* pIntFormat = &gTextureFormatMap[eFormat];

	switch (eTexDim)
	{
	case TD_TEXTURE1D:
		m_texType = GL_TEXTURE_1D;
		break;
	case TD_TEXTURE2D:
		m_texType = GL_TEXTURE_2D;
		break;
	case TD_TEXTURE3D:
		m_texType = GL_TEXTURE_3D;
		break;
	case TD_TEXTURECUBE:
		m_texType = GL_TEXTURE_CUBE_MAP;
		break;
	default:
		ASSERT(false);
	}

	m_uWidth		 = uWidth;
	m_uHeight		 = uHeight;
	m_formatMapIndex = static_cast<uint32>(eFormat);
	m_glTexHndl		 = CreateTexture(pIntFormat, m_texType, uWidth, uHeight, pPixels);
}


void Texture_ps::Init(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight)
{
	// If being resized then delete the old handles
	Reset();

	const GLTextureFormatMap* pIntFormat = &gDepthFormatMap[eFormat];
	m_texType			= GL_TEXTURE_2D;
	m_uWidth			= uWidth;
	m_uHeight			= uHeight;
	m_bIsDepthFormat	= true;
	m_formatMapIndex	= static_cast<uint32>(eFormat);
	m_glTexHndl			= CreateTexture(pIntFormat, m_texType, uWidth, uHeight, NULL);
}


void Texture_ps::InitArray(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices)
{
	const GLTextureFormatMap* pIntFormat = &gTextureFormatMap[eFormat];
	m_texType	= GL_TEXTURE_2D_ARRAY;
	m_uWidth	= uWidth;
	m_uHeight	= uHeight;
	glGenTextures(1, &m_glTexHndl);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_glTexHndl);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, pIntFormat->intFormat, m_uWidth, m_uHeight, uSlices, 0, pIntFormat->format, pIntFormat->type, NULL);
}


void Texture_ps::InitArray(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices)
{
	const GLTextureFormatMap* pIntFormat = &gDepthFormatMap[eFormat];
	m_texType			= GL_TEXTURE_2D_ARRAY;
	m_uWidth			= uWidth;
	m_uHeight			= uHeight;
	m_bIsDepthFormat	= true;
	glGenTextures(1, &m_glTexHndl);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_glTexHndl);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, pIntFormat->intFormat, m_uWidth, m_uHeight, uSlices, 0, pIntFormat->format, pIntFormat->type, NULL);
}

void Texture_ps::InitCubeMap(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight)
{
	const GLTextureFormatMap* pIntFormat = &gDepthFormatMap[eFormat];

	m_texType = GL_TEXTURE_CUBE_MAP;
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	m_bIsDepthFormat = true;

	GLenum eTarget;
	glGenTextures(1, &m_glTexHndl);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_glTexHndl);

	for (int n = 0; n < 6; n++)
	{
		eTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + n;

		glTexImage2D(eTarget, 0, pIntFormat->intFormat, m_uWidth, m_uHeight, 0, pIntFormat->format, pIntFormat->type, NULL);

	}
}


void Texture_ps::CleanUp(GFXDevice* pDevice)
{
	Reset();
}

void Texture_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	if (m_texType != 0)
	{
		// If being resized then delete the old handles
		Reset();

		m_uWidth = uWidth;
		m_uHeight = uHeight;

		if (m_bIsDepthFormat)
		{
			m_glTexHndl = CreateTexture(&gDepthFormatMap[m_formatMapIndex], m_texType, uWidth, uHeight, NULL);
		}
		else
		{
			m_glTexHndl = CreateTexture(&gTextureFormatMap[m_formatMapIndex], m_texType, uWidth, uHeight, NULL);
		}
	}
}

bool Texture_ps::Load(GFXDevice* pDevice, const char* szFileName, GPULocation eLocation)
{
	U8String filename = szFileName;

	if(m_glTexHndl!=GL_INVALID_INDEX)
	{
		glDeleteTextures(1, &m_glTexHndl);	
	}

	U8String tmp = filename + ".dds";
			
	if (File::FileStatus(tmp.CStr()) == FILE_STATUS_VALID)
	{
		return LoadWithGLI(tmp.CStr());
	}
	else
	{
		tmp = filename + ".ktx";
		return LoadWithGLI(tmp.CStr());
	}

	return false;
}


bool Texture_ps::LoadWithGLI(const char* szFileName)
{
	bool bReturn = true;
	{
		mem::setConventionalMemManagement(true);
		usg::File texFile(szFileName);
		void* scratchMemory = NULL;
		ScratchRaw::Init(&scratchMemory, texFile.GetSize(), 4);
		texFile.Read(texFile.GetSize(), scratchMemory);
		gli::texture Texture = gli::load((char*)scratchMemory, texFile.GetSize());
		if (Texture.empty())
		{
			bReturn = false;
			ASSERT(false);
		}

		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
		GLenum Target = GL.translate(Texture.target());
		m_bHasMipMaps = Texture.levels() > 1;
		m_texType = Target;

		glGenTextures(1, &m_glTexHndl);
		glBindTexture(Target, m_glTexHndl);
		// Base and max level are not supported by OpenGL ES 2.0
		glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
		//Texture swizzle is not supported by OpenGL ES 2.0 and OpenGL 3.2
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

		glm::tvec3<GLsizei> const Extent(Texture.extent());
		GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());
		m_uWidth = Extent.x;
		m_uHeight = Extent.y;

		for (std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
		{
			for (std::size_t Level = 0; Level < Texture.levels(); ++Level)
			{
				for (std::size_t Face = 0; Face < Texture.faces(); ++Face)
				{
					GLsizei const LayerGL = static_cast<GLsizei>(Layer);
					glm::tvec3<GLsizei> Extent(Texture.extent(Level));
					Target = gli::is_target_cube(Texture.target())
						? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
						: Target;

					switch (Texture.target())
					{
					case gli::TARGET_1D:
						if (gli::is_compressed(Texture.format()))
							glCompressedTexImage1D(
								Target,
								Format.Internal,
								static_cast<GLint>(Level), 0, Extent.x,
								static_cast<GLsizei>(Texture.size(Level)),
								Texture.data(Layer, Face, Level));
						else
							glTexImage1D(
								Target, static_cast<GLint>(Level),
								Format.Internal,
								Extent.x,
								0,
								Format.External, Format.Type,
								Texture.data(Layer, Face, Level));
						break;
					case gli::TARGET_1D_ARRAY:
					case gli::TARGET_2D:
					case gli::TARGET_CUBE:
						if (gli::is_compressed(Texture.format()))
							glCompressedTexImage2D(
								Target, static_cast<GLint>(Level),
								Format.Internal,
								Extent.x,
								Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
								0,
								static_cast<GLsizei>(Texture.size(Level)),
								Texture.data(Layer, Face, Level));
						else
							glTexImage2D(
								Target, static_cast<GLint>(Level),
								Format.Internal,
								Extent.x,
								Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
								0,
								Format.External, Format.Type,
								Texture.data(Layer, Face, Level));
						break;
					case gli::TARGET_2D_ARRAY:
					case gli::TARGET_3D:
					case gli::TARGET_CUBE_ARRAY:
						if (gli::is_compressed(Texture.format()))
							glCompressedTexImage3D(
								Target, static_cast<GLint>(Level),
								Format.Internal,
								Extent.x, Extent.y,
								Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
								0,
								static_cast<GLsizei>(Texture.size(Level)),
								Texture.data(Layer, Face, Level));
						else
							glTexImage3D(
								Target, static_cast<GLint>(Level),
								Format.Internal,
								Extent.x, Extent.y,
								Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
								0,
								Format.External, Format.Type,
								Texture.data(Layer, Face, Level));
						break;
					default:
						assert(0);
						bReturn = false;
					}
				}
			}
		}


	}
	mem::setConventionalMemManagement(false);
	return bReturn;
}

void Texture_ps::BindInt(uint32 uTexUnit) const
{
	glActiveTexture( GL_TEXTURE0 + uTexUnit );
	//glEnable(m_texType);
	glBindTexture(m_texType, m_glTexHndl);
}

void Texture_ps::SetSampler(uint32 uUnit, Sampler *pSampler) const
{
	//if(pSampler!=m_activeSampler)
	{
		m_activeSampler = pSampler;
		pSampler->Apply(uUnit, m_texType, m_bHasMipMaps);
	}
}

uint32 Texture_ps::GetWidth() const
{
	return m_uWidth;
}

uint32 Texture_ps::GetHeight() const
{
	return m_uHeight;
}

bool Texture_ps::FileExists(const char* szFileName)
{
	U8String file(szFileName);
	file += ".dds";
	if(File::FileStatus(file.CStr()) == FILE_STATUS_VALID)
		return true;

	file = szFileName;
	file += ".ktx";
	return File::FileStatus(file.CStr()) == FILE_STATUS_VALID;
}

}
