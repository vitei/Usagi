#include "Engine/Common/Common.h"
#include "FileFactoryWin.h"
#include <gli/generate_mipmaps.hpp>
#include <algorithm>
#include <fstream>


FileFactoryWin::FileFactoryWin() :
	FileFactory()
{
	// 565 compressed
	m_texFormats["BC1-srgb"] = { CMP_FORMAT::CMP_FORMAT_BC1, true };
	m_texFormats["BC1"] = { CMP_FORMAT::CMP_FORMAT_BC1, false };

	// 565 compressed + 4 bit alpha
	m_texFormats["BC2-srgb"] = { CMP_FORMAT::CMP_FORMAT_BC2, true };
	m_texFormats["BC2"] = { CMP_FORMAT::CMP_FORMAT_BC2, false };

	// 565 compressed + 8 bit alpha
	m_texFormats["BC3-srgb"] = { CMP_FORMAT::CMP_FORMAT_BC3, true };
	m_texFormats["BC3"] = { CMP_FORMAT::CMP_FORMAT_BC3, false };

	// 8 bit compressed grayscale
	m_texFormats["BC4"] = { CMP_FORMAT::CMP_FORMAT_BC4, false };

	// 88 two color channels
	m_texFormats["BC5"] = { CMP_FORMAT::CMP_FORMAT_BC5, false };

	// Three color channels, 4-7bits + 0-8 bits of alpha
	m_texFormats["BC7-srgb"] = { CMP_FORMAT::CMP_FORMAT_BC7, true };
	m_texFormats["BC7"] = { CMP_FORMAT::CMP_FORMAT_BC7, false };

	// rgb, half float hdr
	m_texFormats["BC6"] = { CMP_FORMAT::CMP_FORMAT_BC6H, false };

	// Uncompressed 888
	m_texFormats["rgb"] = { CMP_FORMAT::CMP_FORMAT_BGR_888, false };
	m_texFormats["srgb"] = { CMP_FORMAT::CMP_FORMAT_BGR_888, false };

	// Uncompressed 8888
	m_texFormats["rgba"] = { CMP_FORMAT::CMP_FORMAT_ABGR_8888, false };
	m_texFormats["srgba"] = { CMP_FORMAT::CMP_FORMAT_ABGR_8888, false };

	// Uncompressed 8
	m_texFormats["r"] = { CMP_FORMAT::CMP_FORMAT_R_8, false };

	CMP_InitFramework();
}

FileFactoryWin::~FileFactoryWin()
{

}

std::string FileFactoryWin::LoadFile(const char* szFileName, YAML::Node node)
{
	std::string name;
	if (HasExtension(szFileName, "png"))
	{
		name = LoadTexture(szFileName, node);
	}
	else if (HasExtension(szFileName, "tga"))
	{
		name = LoadTexture(szFileName, node);
	}
	else if (HasExtension(szFileName, "dds"))
	{
		name = LoadDDS(szFileName, node);
	}
	else if (HasExtension(szFileName, "wav"))
	{
		name = LoadRawFile(szFileName);
	}
	else
	{
		return FileFactory::LoadFile(szFileName, node);
	}

	if (name.size() > 0)
	{
		AddDependency(szFileName);
	}
	return name;
}



bool CompressionCallback(CMP_FLOAT fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2)
{
	return false;
}

// We don't send these through compressonator as we assume it's been formatted/ had mips set
std::string FileFactoryWin::LoadDDS(const char* szFileName, YAML::Node node)
{
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size());
	std::string relativeNameNoExt = RemoveExtension(relativePath);
	std::string outName = relativeNameNoExt + ".vtx";
	
	// Already references
	if (HasDestResource(outName))
	{
		return outName;
	}

	// We don't both doing any conversion, gli can handle it as is and compressonator crashes trying
	// to work on compressed files anyway
	TextureEntry* pTexture = new TextureEntry;
	pTexture->srcName = szFileName;
	pTexture->SetName(outName, usg::ResourceType::TEXTURE);

	gli::texture dds = gli::load(szFileName);
	if (dds.size() > 0)
	{

		pTexture->Init(dds, false);

		m_resources.push_back(pTexture);


		return outName;
	}
	else
	{
		LOG_MSG(false, "Unable to load texture %s", szFileName);
		return "";
	}
}

std::string FileFactoryWin::LoadTexture(const char* szFileName, YAML::Node node)
{
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size());
	std::string relativeNameNoExt = RemoveExtension(relativePath);
	std::string outName = relativeNameNoExt + ".vtx";
	std::string tmpFileName = m_tempDir + relativeNameNoExt + ".dds";

	// Already references
	if (HasDestResource(outName))
	{
		return outName;
	}

	CMP_MipSet MipSetIn;
	memset(&MipSetIn, 0, sizeof(CMP_MipSet));
	CMP_ERROR cmp_status = CMP_LoadTexture(szFileName, &MipSetIn);
	if (cmp_status != CMP_OK) {
		std::printf("Error %d: Loading source file!\n", cmp_status);
		return "";
	}

	TextureSettings textureSettings = GetTextureSettings(node);
	TexFormat format = GetTexFormat(textureSettings.format.c_str());
	// Fails to build mips on compressed formats
	textureSettings.bGenMips &= MipSetIn.m_format < CMP_FORMAT::CMP_FORMAT_ASTC;

	if (MipSetIn.m_nMipLevels <= 1 && textureSettings.bGenMips)
	{
		CMP_INT nMinSize = CMP_CalcMinMipSize(MipSetIn.m_nHeight, MipSetIn.m_nWidth, 10);
		CMP_GenerateMIPLevels(&MipSetIn, nMinSize);
	}

	KernelOptions   kernel_options;
	memset(&kernel_options, 0, sizeof(KernelOptions));


	kernel_options.format = format.format;   // Set the format to process
	kernel_options.fquality = 0.05f;		 // Set the quality of the result
	kernel_options.encodeWith = format.format == CMP_FORMAT::CMP_FORMAT_BC7 ? CMP_HPC : CMP_CPU;
	kernel_options.threads = 2;              // Multi-threading is handled by the build
	//kernel_options.width = MipSetIn.dwWidth;
	//kernel_options.height = MipSetIn.dwHeight;
	kernel_options.srcformat = MipSetIn.m_format;
	kernel_options.useSRGBFrames = format.bSRGB;

	if (format.format == CMP_FORMAT_BC1)
	{
		// Enable punch through alpha setting
		kernel_options.bc15.useAlphaThreshold = true;
		kernel_options.bc15.alphaThreshold = 128;

		// Enable setting channel weights
		kernel_options.bc15.useChannelWeights = true;
		kernel_options.bc15.channelWeights[0] = 0.3086f;
		kernel_options.bc15.channelWeights[1] = 0.6094f;
		kernel_options.bc15.channelWeights[2] = 0.0820f;
	}

	CMP_MipSet MipSetCmp;

	memsize pos = 0;
	std::string tmpPath = RemoveFileName(tmpFileName);
	do
	{
		pos = tmpPath.find_first_of("\\/", pos + 1);
		CreateDirectory(tmpPath.substr(0, pos).c_str(), NULL);
	} while (pos != std::string::npos);

	// Only compress if the original isn't (i.e. we're loading dds). This is mainly due to compressonator
	// being a buggy crashy pos.
	if (MipSetIn.m_format < CMP_FORMAT::CMP_FORMAT_ASTC)
	{
		memset(&MipSetCmp, 0, sizeof(CMP_MipSet));

		cmp_status = CMP_ProcessTexture(&MipSetIn, &MipSetCmp, kernel_options, CompressionCallback);
		FATAL_RELEASE(cmp_status == CMP_OK, "Failed to process file %s. Error %d", szFileName, cmp_status);
		if (cmp_status != CMP_OK)
		{
			// Failed 
			return "";
		}
		cmp_status = CMP_SaveTexture(tmpFileName.c_str(), &MipSetCmp);

	}
	else
	{
		cmp_status = CMP_SaveTexture(tmpFileName.c_str(), &MipSetIn);
	}
	FATAL_RELEASE(cmp_status == CMP_OK, "Failed to save file %s. Error %d", szFileName, cmp_status);


	gli::texture ktx = gli::load(tmpFileName.c_str());
	DeleteFile(tmpFileName.c_str());

	TextureEntry* pTexture = new TextureEntry;
	pTexture->srcName = szFileName;
	pTexture->SetName(outName, usg::ResourceType::TEXTURE);
	pTexture->Init(ktx, format.bSRGB);

	m_resources.push_back(pTexture);

	return outName;
}


FileFactoryWin::TexFormat FileFactoryWin::GetTexFormat(const char* szDstFormat)
{
	auto itr = m_texFormats.find(usg::string(szDstFormat));
	if (itr == m_texFormats.end())
	{
		FATAL_RELEASE(false, "Invalid format %s", szDstFormat);
		return { CMP_FORMAT::CMP_FORMAT_BC7, true };
	}
	return (*itr).second;
}
