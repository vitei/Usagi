#ifndef _USG_PARTICLE_EDITOR_FILE_LIST_H_
#define _USG_PARTICLE_EDITOR_FILE_LIST_H_

#include <fstream>
#include <iostream>
#include <filesystem>
#include "Engine/Core/stl/string.h"

template <int FileCount>
class FileList
{
public:
	FileList() { 	m_fileNames[0] = '\0';
					m_fileNames[1] = '\0';
					m_uFileNameCount = 0;
					m_bSubDirectories = false;
	}
	~FileList() {}

	void Init(const char* szDirectory, const char* requiredExt, bool bSubDirectories = false)
	{
		m_cmpExt.clear();
		if (requiredExt && requiredExt[0] != '\0')
		{
			m_cmpExt.push_back(requiredExt);
		}
		m_directory = szDirectory;
		m_bSubDirectories = bSubDirectories;

		Update();
	}

	void Init(const char* szDirectory, const usg::vector< usg::string >& requiredExt, bool bSubDirectories = false)
	{
		m_cmpExt = requiredExt;
		m_directory = szDirectory;
		m_bSubDirectories = bSubDirectories;

		Update();
	}

	template<class FileType>
	void ProcessFile(FileType& file)
	{
		// file object contains relative path in the case of 
		// directory iterators (i.e. just the file name)
		bool bValidFile = m_cmpExt.empty();
		for (auto itr : m_cmpExt)
		{
			if (file.extension() == itr.c_str())
			{
				bValidFile = true;
			}
		}

		if (bValidFile)
		{
			FileType parentPath = file.parent_path();
			usg::string rootPath = parentPath.generic_u8string().c_str();
			usg::string name = file.filename().generic_u8string().c_str();
			if (m_directory.length() < rootPath.length())
			{
				rootPath = rootPath.substr(m_directory.length() + 1, usg::string::npos);
				name = rootPath + usg::string("\\") + name;
			}

			str::Copy(&m_fileNames[m_uStringLength], name.c_str(), USG_MAX_PATH);
			m_pFileNames[m_uFileNameCount] = &m_fileNames[m_uStringLength];
			m_uStringLength += (str::StringLength(&m_fileNames[m_uStringLength]) + 1);
			m_uFileNameCount++;
		}
	}

	void Update()
	{
		m_uFileNameCount = 0;
		m_uStringLength = 0;
		usg::mem::setConventionalMemManagement(true);

		if (!m_bSubDirectories)
		{
			for (auto it = std::filesystem::directory_iterator(m_directory.c_str()); it != std::filesystem::directory_iterator(); ++it)
			{
				ProcessFile(it->path());
			}
		}
		else
		{
			for (auto it = std::filesystem::recursive_directory_iterator(m_directory.c_str()); it != std::filesystem::recursive_directory_iterator(); ++it)
			{
				ProcessFile(it->path());
			}
		}

		m_fileNames[m_uStringLength]='\0';
		usg::mem::setConventionalMemManagement(false);
	}

	uint32 GetFileCount() { return m_uFileNameCount; }
	const char** GetFileNames() { return m_pFileNames; }
	const char* GetFileName(uint32 uName) { return m_pFileNames[uName]; }
	const char* GetFileNamesRaw() { return m_fileNames; }

private:
	const char*				m_pFileNames[FileCount];
	char					m_fileNames[FileCount*USG_MAX_PATH];
	uint32					m_uStringLength;
	uint32					m_uFileNameCount;
	bool					m_bSubDirectories;

	usg::vector< usg::string >	m_cmpExt;
	usg::string					m_directory;
	
};


#endif
