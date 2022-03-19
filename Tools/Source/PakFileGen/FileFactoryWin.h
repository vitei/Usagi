#pragma once
#include "FileFactory.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "Engine/Core/stl/map.h"
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

	std::string LoadTGA(const char* szFileName, YAML::Node node);
	bool LoadUncompressedTGA(usg::TGAFile& tga, gli::texture2d& texture);
	std::string LoadDDS(const char* szFileName, YAML::Node node);

private:
	gli::format GetTexFormat(const char* szDstFormat);

	usg::map< usg::string, gli::format >	m_texFormats;
};