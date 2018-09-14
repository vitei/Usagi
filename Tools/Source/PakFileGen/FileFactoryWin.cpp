#include "Engine/Common/Common.h"
#include "FileFactoryWin.h"
#include "gli/gli.hpp"
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


bool FileFactoryWin::LoadTGA(const char* szFileName)
{
	// TODO:
	return false;
}

bool FileFactoryWin::LoadDDS(const char* szFileName)
{
	gli::texture Texture = gli::load(szFileName);

	TextureEntry* pTexture = new TextureEntry;
	bool bResult = gli::save_ktx(Texture, pTexture->memory);
	m_resources.push_back(pTexture);
	return bResult;
}