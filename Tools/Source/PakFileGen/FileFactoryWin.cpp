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
	return FileFactory::LoadFile(szFileName);
}
