#pragma once
#include "FileFactory.h"
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
		virtual void* GetData() override { return memory.data(); }
		virtual uint32 GetDataSize() override { return (uint32)memory.size(); };
		virtual void* GetCustomHeader() { return nullptr; }
		virtual uint32 GetCustomHeaderSize() { return 0; }


		std::vector<char> memory;
	};

	bool LoadTGA(const char* szFileName);
	bool LoadDDS(const char* szFileName);
};