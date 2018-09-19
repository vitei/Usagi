#include "Engine/Common/Common.h"
#include "FileFactoryWin.h"
#include <algorithm>
#include <fstream>


FileFactoryWin::FileFactoryWin() :
	FileFactory()
{

}

FileFactoryWin::~FileFactoryWin()
{

}

bool FileFactoryWin::LoadFile(const char* szFileName)
{
	if (HasExtension(szFileName, "tga"))
	{
		// Process the fbx file
		LoadTGA(szFileName);
	}
	else if (HasExtension(szFileName, "dds"))
	{
		// Process the fbx file
		LoadDDS(szFileName);
	}
	else
	{
		return FileFactory::LoadFile(szFileName);
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

bool FileFactoryWin::LoadCompressedTGA(usg::TGAFile& tga, gli::texture2d& texture)
{
	const usg::TGAFile::Header& header = tga.GetHeader();
	uint32 uSize = (header.uBitsPerPixel * header.uWidth * header.uHeight);
	glm::u8 * LinearAddress = texture[0].data<glm::u8>();

	uint8 uChunkHeader = 0;
	uint32 uCurrentPixel = 0;
	uint32 uCurrentByte = 0;
	uint8* pData = tga.GetData();
	do
	{
		uChunkHeader = *pData++;

		if (uChunkHeader < 128)
		{
			++uChunkHeader;
			for (int i = 0; i < uChunkHeader; ++i, ++uCurrentPixel)
			{
				LinearAddress[uCurrentByte++] = *pData++;
				LinearAddress[uCurrentByte++] = *pData++;
				LinearAddress[uCurrentByte++] = *pData++;
				if (header.uBitsPerPixel > 24) LinearAddress[uCurrentByte++] = *pData++;
			}
		}
		else
		{
			uChunkHeader -= 127;

			for (int i = 0; i < uChunkHeader; ++i, ++uCurrentPixel)
			{
				LinearAddress[uCurrentByte++] = *pData++;
				LinearAddress[uCurrentByte++] = *pData++;
				LinearAddress[uCurrentByte++] = *pData++;
				if (header.uBitsPerPixel > 24) LinearAddress[uCurrentByte++] = *pData++;
			}
		}
	} while (uCurrentPixel < ((uint32)header.uWidth * (uint32)header.uHeight));

	return true;
}


bool FileFactoryWin::LoadTGA(const char* szFileName)
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

	uint8 uUnCompressed[12] = { 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	uint8 uIsCompressed[12] = { 0x0, 0x0, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

	if ( !std::memcmp(uUnCompressed, &header, sizeof(uUnCompressed)) )
	{
		LoadUncompressedTGA(file, texture);
	}
	else
	{
		LoadCompressedTGA(file, texture);
	}


	TextureEntry* pTexture = new TextureEntry;
	pTexture->srcName = szFileName;
	pTexture->name = relativeNameNoExt + ".ktx";
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

bool FileFactoryWin::LoadDDS(const char* szFileName)
{
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size() + 1);
	std::string relativeNameNoExt = RemoveExtension(relativePath);

	gli::texture Texture = gli::load(szFileName);

	TextureEntry* pTexture = new TextureEntry;
	pTexture->srcName = szFileName;
	pTexture->name = relativeNameNoExt + ".ktx";

	bool bResult = gli::save_ktx(Texture, pTexture->memory);
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