#pragma once
#include "FileFactory.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include <gli/gli.hpp>
#include <sstream>


class FileFactoryWin : public FileFactory
{
public:
	FileFactoryWin();
	virtual ~FileFactoryWin();

	virtual bool LoadFile(const char* szFileName) override;

protected:
	struct TextureEntry : public ResourceEntry
	{
		virtual const void* GetData() override { return memory.data(); }
		virtual uint32 GetDataSize() override { return (uint32)memory.size(); };
		virtual const void* GetCustomHeader() { return nullptr; }
		virtual uint32 GetCustomHeaderSize() { return 0; }


		std::vector<char> memory;
	};

	bool LoadTGA(const char* szFileName);
	bool LoadUncompressedTGA(usg::TGAFile& tga, gli::texture2d& texture);
	bool LoadCompressedTGA(usg::TGAFile& tga, gli::texture2d& texture);
	bool LoadDDS(const char* szFileName);
};