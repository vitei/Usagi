#include "Engine/Common/Common.h"
#include "FileFactoryWin.h"
#include <gli/generate_mipmaps.hpp>
#include <gli/texture2d.hpp>
#include <gli/convert.hpp>
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

	// Uncompressed 16
	m_texFormats["r16"] = { CMP_FORMAT::CMP_FORMAT_R_16, false };

	// Uncompressed 32 bit float
	m_texFormats["r32f"] = { CMP_FORMAT::CMP_FORMAT_R_32F, false };


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

		bool bSRGB = false;
		if (node)
		{
			const YAML::Node force = node["sRGB"];
			if(force)
			{
				bSRGB = force.as<bool>();
			}
		}

		pTexture->Init(dds, bSRGB);

		m_resources.push_back(pTexture);


		return outName;
	}
	else
	{
		LOG_MSG(false, "Unable to load texture %s", szFileName);
		return "";
	}
}


std::string FileFactoryWin::LoadHeightmap(const char* szFileName, const YAML::Node& node)
{
	// Terrain file is just yaml renamed
	YAML::Node mainNode = YAML::LoadFile(szFileName);

	if (mainNode && mainNode["Heightmap"])
	{
		std::string heightmap = mainNode["Heightmap"].as<std::string>();


		// Load the texture
		YAML::Node out;
		out.force_insert("mips", false);
		out.force_insert("format", "r16");
		out.force_insert("sRGB", false);



		std::string relativePath = std::string(szFileName).substr(m_rootDir.size());
		std::string terrainDir = std::string(szFileName).substr(0, std::string(szFileName).find_last_of("\\/")+1);
		std::string relativeNameNoExt = RemoveExtension(relativePath);
		std::string outName = relativeNameNoExt + ".vtx";
		std::string collisionOut = relativeNameNoExt + ".hfld";
		std::string tmpFileName = m_tempDir + relativeNameNoExt + ".dds";

		heightmap = terrainDir + heightmap;

		// Already references
		if (HasDestResource(outName))
		{ 
			return outName;
		}

		CreateTempKTXTexture(heightmap.c_str(), out, tmpFileName.c_str());


		gli::texture2d ktx(gli::load(tmpFileName.c_str()));
		DeleteFile(tmpFileName.c_str());

		// Necessary to get rid of all of the swizzle etc
		gli::texture2d TextureConverted = gli::convert(ktx, gli::FORMAT_R16_UNORM_PACK16);

		HeightfieldEntry* pHeightfield = new HeightfieldEntry;
		pHeightfield->srcName = szFileName;
		pHeightfield->SetName(collisionOut.c_str(), usg::ResourceType::HEIGHTFIELD);
		pHeightfield->Init(TextureConverted);

		// We convert the texture to 32bit to interpolate in the highp shader
		gli::texture2d TextureHp = gli::convert(ktx, gli::FORMAT_R32_SFLOAT_PACK32);

		std::string texName = RemoveExtension(szFileName) + ".vtx";
		TextureEntry* pTexture = new TextureEntry;
		pTexture->srcName = szFileName;
		pTexture->SetName(outName, usg::ResourceType::TEXTURE);
		pTexture->Init(TextureHp,false);


		std::string expectedTexName = RemoveExtension(heightmap.substr(m_rootDir.size()).c_str());

		m_resources.push_back(pHeightfield);
		m_resources.push_back(pTexture);



		return outName;
	}
	return "";
}


std::string FileFactoryWin::CreateTempKTXTexture(const char* szFileName, YAML::Node node, const char* szTmpFileName)
{
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
	textureSettings.bGenMips &= !CMP_IsFloatFormat(MipSetIn.m_format);

	if (MipSetIn.m_nMipLevels <= 1 && textureSettings.bGenMips)
	{
		CMP_INT nMinSize = CMP_CalcMinMipSize(MipSetIn.m_nHeight, MipSetIn.m_nWidth, 10);
		CMP_GenerateMIPLevels(&MipSetIn, nMinSize);
	}

	KernelOptions   kernel_options;
	memset(&kernel_options, 0, sizeof(KernelOptions));

	KernelDeviceInfo info;
	CMP_GetDeviceInfo(&info);

	kernel_options.format = format.format;   // Set the format to process
	kernel_options.fquality = 0.15f;		 // Set the quality of the result
	//format.format == CMP_FORMAT::CMP_FORMAT_BC7 ? CMP_HPC : CMP_CPU;
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
	std::string tmpPath = RemoveFileName(szTmpFileName);
	do
	{
		pos = tmpPath.find_first_of("\\/", pos + 1);
		CreateDirectory(tmpPath.substr(0, pos).c_str(), NULL);
	} while (pos != std::string::npos);

	// Only compress if the original isn't (i.e. we're loading dds). This is mainly due to compressonator
	// being a buggy crashy pos.
	if (!CMP_IsCompressedFormat(MipSetIn.m_format) && CMP_IsCompressedFormat(format.format) )
	{
		memset(&MipSetCmp, 0, sizeof(CMP_MipSet));

		cmp_status = CMP_ProcessTexture(&MipSetIn, &MipSetCmp, kernel_options, CompressionCallback);
		FATAL_RELEASE(cmp_status == CMP_OK, "Failed to process file %s. Error %d", szFileName, cmp_status);
		if (cmp_status != CMP_OK)
		{
			// Failed 
			return "";
		}
		cmp_status = CMP_SaveTexture(szTmpFileName, &MipSetCmp);

	}
	else
	{
		cmp_status = CMP_SaveTexture(szTmpFileName, &MipSetIn);
	}
	FATAL_RELEASE(cmp_status == CMP_OK, "Failed to save file %s. Error %d", szFileName, cmp_status);

	return szTmpFileName;
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

	CreateTempKTXTexture(szFileName, node, tmpFileName.c_str());

	gli::texture ktx = gli::load(tmpFileName.c_str());
	DeleteFile(tmpFileName.c_str());

	TextureSettings textureSettings = GetTextureSettings(node);
	TexFormat format = GetTexFormat(textureSettings.format.c_str());


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
