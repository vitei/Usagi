#include "Engine/Common/Common.h"
#include "FileFactoryWin.h"
#include <gli/generate_mipmaps.hpp>
#include <algorithm>
#include <fstream>


FileFactoryWin::FileFactoryWin() :
	FileFactory()
{
	// 565 compressed
	m_texFormats["BC1-srgb"] = gli::format::FORMAT_RGBA_DXT1_SRGB_BLOCK8;
	m_texFormats["BC1"] = gli::format::FORMAT_RGBA_DXT1_UNORM_BLOCK8;

	// 565 compressed + 4 bit alpha
	m_texFormats["BC2-srgb"] = gli::format::FORMAT_RGBA_DXT3_SRGB_BLOCK16;
	m_texFormats["BC2"] = gli::format::FORMAT_RGBA_DXT3_UNORM_BLOCK16;

	// 565 compressed + 8 bit alpha
	m_texFormats["BC3-srgb"] = gli::format::FORMAT_RGBA_DXT5_SRGB_BLOCK16;
	m_texFormats["BC3"] = gli::format::FORMAT_RGBA_DXT5_UNORM_BLOCK16;

	// 8 bit compressed grayscale
	m_texFormats["BC4"] = gli::format::FORMAT_R_ATI1N_UNORM_BLOCK8;

	// 88 two color channels
	m_texFormats["BC5"] = gli::format::FORMAT_RG_ATI2N_UNORM_BLOCK16;

	// Three color channels, 4-7bits + 0-8 bits of alpha
	m_texFormats["BC7-srgb"] = gli::format::FORMAT_RGBA_BP_SRGB_BLOCK16;
	m_texFormats["BC7"] = gli::format::FORMAT_RGBA_BP_UNORM_BLOCK16;

	// Uncompressed 888
	m_texFormats["rgb"] = gli::format::FORMAT_BGR8_UNORM_PACK8;
	m_texFormats["srgb"] = gli::format::FORMAT_BGR8_SRGB_PACK8;

	// Uncompressed 8888
	m_texFormats["rgba"] = gli::format::FORMAT_BGRA8_UNORM_PACK8;
	m_texFormats["srgba"] = gli::format::FORMAT_BGRA8_SRGB_PACK8;

	// Uncompressed 8
	m_texFormats["r"] = gli::format::FORMAT_L8_UNORM_PACK8;
}

FileFactoryWin::~FileFactoryWin()
{

}

bool FileFactoryWin::LoadFile(const char* szFileName, YAML::Node node)
{
	if (HasExtension(szFileName, "tga"))
	{
		LoadTGA(szFileName, node);
	}
	else if (HasExtension(szFileName, "dds"))
	{
		LoadDDS(szFileName, node);
	}
	else if (HasExtension(szFileName, "wav"))
	{
		LoadRawFile(szFileName);
	}
	else
	{
		return FileFactory::LoadFile(szFileName, node);
	}

	AddDependency(szFileName);
	return true;
}


// TODO: Move these functions to the tga file class
bool FileFactoryWin::LoadUncompressedTGA(usg::TGAFile& tga, gli::texture2d& texture)
{
	const usg::TGAFile::Header& header = tga.GetHeader();
	uint32 uSize = (header.uBitsPerPixel * header.uWidth * header.uHeight);
	glm::u8 * LinearAddress = texture[0].data<glm::u8>();
	memcpy(LinearAddress, tga.GetData(), uSize);


	return true;
}


bool FileFactoryWin::LoadTGA(const char* szFileName, YAML::Node node)
{
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size() + 1);
	std::string relativeNameNoExt = RemoveExtension(relativePath);

	usg::TGAFile file;

	gli::format eFormat = gli::FORMAT_BGRA8_UNORM_PACK8;
	if (file.GetHeader().uBitsPerPixel == 24)
	{
		eFormat = gli::FORMAT_BGR8_UNORM_PACK8;
	}
	else if (file.GetHeader().uBitsPerPixel == 8)
	{
		eFormat = gli::FORMAT_R8_UNORM_PACK8;
	}

	const usg::TGAFile::Header& header = file.GetHeader();
	gli::texture2d texture = gli::texture2d(eFormat, gli::texture2d::extent_type(file.GetHeader().uWidth, file.GetHeader().uHeight));

	LoadUncompressedTGA(file, texture);

	TextureSettings textureSettings = GetTextureSettings(node);
	// We want to convert TGA if no conversion settings specified otherwise we'd end up with nothing but uncompressed giant textures
	textureSettings.bConvert = true;

	gli::format eTargetFormat = GetTexFormat(textureSettings.format.c_str());
	if (textureSettings.bConvert)
	{
		texture = gli::convert(texture, eTargetFormat);
	}
	if (textureSettings.bGenMips)
	{
		texture = gli::generate_mipmaps(texture, gli::FILTER_LINEAR);
	}

	TextureEntry* pTexture = new TextureEntry;
	pTexture->srcName = szFileName;
	pTexture->SetName(relativeNameNoExt + ".ktx", usg::ResourceType::TEXTURE);
	bool bResult = gli::save_ktx(texture, pTexture->memory);
	if (!bResult)
	{
		delete pTexture;
	}
	else
	{
		m_resources.push_back(pTexture);
	}

	return bResult;
}

bool FileFactoryWin::LoadDDS(const char* szFileName, YAML::Node node)
{
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size() + 1);
	std::string relativeNameNoExt = RemoveExtension(relativePath);

	gli::texture texture = gli::load(szFileName);

	TextureEntry* pTexture = new TextureEntry;
	pTexture->srcName = szFileName;
	pTexture->SetName(relativeNameNoExt + ".ktx", usg::ResourceType::TEXTURE);

	TextureSettings textureSettings = GetTextureSettings(node);
	gli::format eTargetFormat = GetTexFormat(textureSettings.format.c_str());
	if (textureSettings.bConvert)
	{
		if (texture.max_face() == 1)
		{
			gli::texture2d texSrc(texture);

			texSrc = gli::convert(texSrc, eTargetFormat);

			if (textureSettings.bGenMips && (texSrc.max_layer() == 1))
			{
				texSrc = gli::generate_mipmaps(texSrc, gli::FILTER_LINEAR);
			}

			texture = gli::texture(texSrc);
		}
		else if (texture.max_face() == 6)
		{
			gli::texture_cube texCube(texture);
			texCube =  gli::convert(texCube, eTargetFormat);

			if (textureSettings.bGenMips && (texCube.max_layer() == 1))
			{
				texCube = gli::generate_mipmaps(texCube, gli::FILTER_LINEAR);
			}

			texture = gli::texture(texCube);
		}

	}

	bool bResult = gli::save_ktx(texture, pTexture->memory);
	if (!bResult)
	{
		delete pTexture;
	}
	else
	{
		m_resources.push_back(pTexture);
	}
	return bResult;
}

gli::format FileFactoryWin::GetTexFormat(const char* szDstFormat)
{
	auto itr = m_texFormats.find(usg::string(szDstFormat));
	if (itr == m_texFormats.end())
	{
		FATAL_RELEASE(false, "Invalid format %s", szDstFormat);
		return gli::FORMAT_BGRA8_UNORM_PACK8;
	}
	return (*itr).second;
}
