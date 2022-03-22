#pragma once
#include "FileFactory.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "Engine/Core/stl/map.h"
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
		virtual const void* GetCustomHeader() { return nullptr; }
		virtual uint32 GetCustomHeaderSize() { return 0; }


		std::vector<char> memory;
	};

	std::string LoadTexture(const char* szFileName, YAML::Node node);

private:
	struct TexFormat
	{
		CMP_FORMAT format;
		bool bSRGB;
	};

	TexFormat GetTexFormat(const char* szDstFormat);

	usg::map< usg::string, TexFormat >	m_texFormats;
};