#pragma once
#include "FileFactory.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "Engine/Core/stl/map.h"
#include "Engine/Resource/PakDecl.h"
#include "compressonator.h"
#include <gli/gli.hpp>
#include <sstream>


class FileFactoryWin : public FileFactory
{
public:
	FileFactoryWin();
	virtual ~FileFactoryWin();

	virtual std::string LoadFile(const char* szFileName, YAML::Node node) override;

protected:
	struct TextureEntry : public ResourceEntry
	{
		virtual const void* GetData() override { return memory.data(); }
		virtual uint32 GetDataSize() override { return (uint32)memory.size(); };
		virtual const void* GetCustomHeader() { return customHeaderMem.data(); }
		virtual uint32 GetCustomHeaderSize() { return (uint32)customHeaderMem.size(); }

		void Init(const gli::texture& tex, bool bForceSRGB)
		{
			glm::tvec3<uint32> const extent(tex.extent());
			usg::PakFileDecl::TextureHeader hdr;
			hdr.uWidth = extent[0];
			hdr.uHeight = extent[1];
			hdr.uDepth = extent[2];
			hdr.uFaces = static_cast<uint32>(tex.layers() * tex.faces());
			hdr.uMips = (uint32)tex.levels();
			hdr.uIntFormat = tex.format();
			if (bForceSRGB)
			{
				hdr.uIntFormat = ForceSRGB(hdr.uIntFormat);
			}

			
			std::vector< usg::PakFileDecl::TexLayerInfo > layers;
			for (uint32 uLevel = 0; uLevel < tex.levels(); uLevel++)
			{
				glm::tvec3<uint32> LevelExtent(tex.extent(uLevel));

				usg::PakFileDecl::TexLayerInfo layerInfo;
				layerInfo.uWidth = LevelExtent[0];
				layerInfo.uHeight = LevelExtent[1];
				layerInfo.uDepth = LevelExtent[2];
				layerInfo.uSize = (uint32)tex.size(uLevel);

				layers.push_back(layerInfo);
			}

			customHeaderMem.resize(sizeof(hdr) + (layers.size() * sizeof(layers[0])));
			memcpy(customHeaderMem.data(), &hdr, sizeof(hdr));

			memcpy(&customHeaderMem[sizeof(hdr)], layers.data(), (layers.size() * sizeof(layers[0])));

			memory.resize(tex.size());
			memcpy(memory.data(), tex.data(), tex.size());
		}

		uint32 ForceSRGB(uint32 uFormat)
		{
			switch (uFormat)
			{
			case gli::format::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
				return gli::format::FORMAT_RGBA_DXT1_SRGB_BLOCK8;
			case gli::format::FORMAT_RGBA_DXT5_UNORM_BLOCK16:		
				return gli::format::FORMAT_RGBA_DXT5_SRGB_BLOCK16;
			case gli::format::FORMAT_BGR8_UNORM_PACK8:		
				return gli::format::FORMAT_BGR8_SRGB_PACK8;
			case gli::format::FORMAT_RG8_UNORM_PACK8:	
				return gli::format::FORMAT_RG8_SRGB_PACK8;
			case gli::format::FORMAT_RGBA_BP_UNORM_BLOCK16:			
				return gli::format::FORMAT_RGBA_BP_SRGB_BLOCK16;
			case gli::format::FORMAT_RGBA8_UNORM_PACK8:
				return gli::format::FORMAT_RGBA8_SRGB_PACK8;
			case gli::format::FORMAT_BGRA8_UNORM_PACK8:
				return gli::format::FORMAT_BGRA8_SRGB_PACK8;
			default:
				break;
			}

			return uFormat;
		}

	private:

		std::vector<char> memory;
		std::vector<char> customHeaderMem;
	};

	std::string LoadTexture(const char* szFileName, YAML::Node node);
	std::string LoadDDS(const char* szFileName, YAML::Node node);

private:
	struct TexFormat
	{
		CMP_FORMAT format;
		bool bSRGB;
	};

	TexFormat GetTexFormat(const char* szDstFormat);

	usg::map< usg::string, TexFormat >	m_texFormats;
};