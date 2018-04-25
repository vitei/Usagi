#ifndef _USG_PARTICLE_EDITOR_FILE_LIST_H_
#define _USG_PARTICLE_EDITOR_FILE_LIST_H_
#include "Engine/Common/Common.h"
#include <filesystem>
#include <fstream>

template <int FileCount>
class FileList
{
public:
	FileList() { 	m_fileNames[0] = '\0';
					m_fileNames[1] = '\0';
					m_uFileNameCount = 0;
	}
	~FileList() {}

	void Init(const char* szDirectory, const char* requiredExt)
	{
		m_cmpExt = requiredExt;
		m_directory = szDirectory;
		Update();
	}

	void Update()
	{
		m_uFileNameCount = 0;
		m_uStringLength = 0;
		usg::mem::setConventionalMemManagement(true);
		for (auto it = std::experimental::filesystem::directory_iterator(m_directory.CStr()); it != std::experimental::filesystem::directory_iterator(); ++it)
		{
			// file object contains relative path in the case of 
			// directory iterators (i.e. just the file name)
			const auto& file = it->path();  
			if(m_cmpExt.Length() == 0 || file.extension() == m_cmpExt.CStr())
			{
				usg::U8String name(file.filename().c_str());
				str::Copy(&m_fileNames[m_uStringLength], name.CStr(), USG_MAX_PATH);
				m_pFileNames[m_uFileNameCount] = &m_fileNames[m_uStringLength];
				m_uStringLength+= (str::StringLength(&m_fileNames[m_uStringLength])+1);
				m_uFileNameCount++;
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

	usg::U8String			m_cmpExt;
	usg::U8String			m_directory;
	
};


#endif
