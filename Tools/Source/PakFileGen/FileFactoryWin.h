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
};