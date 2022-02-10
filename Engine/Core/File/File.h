/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A platform independent wrapper for file operations
*****************************************************************************/
#ifndef _USG_FILE_H_
#define _USG_FILE_H_
#include "Engine/Core/stl/vector.h"
#include "Engine/Core/stl/string.h"

namespace usg{

enum FILE_ACCESS_MODE
{
	FILE_ACCESS_READ = 0,
	FILE_ACCESS_WRITE,
	FILE_ACCESS_READ_WRITE_DEPRECATED
};

enum FILE_TYPE
{
	FILE_TYPE_RESOURCE = 0,
	FILE_TYPE_SAVE_DATA,
	FILE_TYPE_DEBUG_DATA,
	FILE_TYPE_EXTERNAL,
	FILE_TYPE_COUNT
};

enum FILE_INIT_RESULT
{
	FILE_INIT_RESULT_SUCCESS = 0,
	FILE_INIT_RESULT_SAVE_DATA_NOT_FORMATTED,
	FILE_INIT_RESULT_SAVE_DATA_CORRUPTION,
	FILE_INIT_RESULT_COUNT
};

enum FILE_STATUS
{
	FILE_STATUS_VALID = 0,
	FILE_STATUS_CORRUPT,
	FILE_STATUS_BAD_FORMAT,
	FILE_STATUS_NOT_FOUND,
	FILE_STATUS_NOT_FORMATTED,
	FILE_STATUS_NO_FREE_SPACE,
	FILE_STATUS_INVALID
};


struct FileOpenPath
{
	struct Filter
	{
		const char* szDisplayName;
		const char* szExtPattern;
	};
	const char* szWindowTitle = nullptr;
	const char* szDefaultExt = nullptr;
	const char* szOpenDir = nullptr;
	Filter* pFilters = nullptr;
	uint32  uFilterCount = 0;
	bool bAllowMulti = false;
};

struct FilePathResult
{
	char szRelativePath[260];
	char szPath[260];
};

}

#include OS_HEADER(Engine/Core/File, file_ps.h)

//#define DEBUG_FILE_READING

#ifdef DEBUG_FILE_READING
#include "Engine/Core/Timer/ProfilingTimer.h"
#endif

namespace usg {

#define FILE_READ_ALIGN 4


class File
{
public:
	File(void);
	File(const char* szFileName, FILE_ACCESS_MODE eMode = FILE_ACCESS_READ, FILE_TYPE eFileType = FILE_TYPE_RESOURCE);
	~File(void);

	static void InitFileSystem();
	static void FinalizeFileSystem();

	virtual bool Open(const char* szFileName, FILE_ACCESS_MODE eMode = FILE_ACCESS_READ, FILE_TYPE eFileType = FILE_TYPE_RESOURCE);
	virtual void Close();

	virtual bool IsOpen();
	virtual memsize Read(memsize uSize, void* pDst);
	virtual memsize Write(memsize uSize, const void* pSrc);
	virtual uint8 ReadByte();
	virtual void SeekPos(memsize uPos);
	virtual void AdvanceBytes(memsize uPos);
	virtual memsize GetPos();
	virtual memsize GetSize();


	static FILE_STATUS FileStatus(const char* szFileName, const FILE_TYPE eFileType = FILE_TYPE_RESOURCE) { return File_ps::FileStatus(szFileName, eFileType); }
	static bool UserFileOpenPath(const FileOpenPath& pathIn, usg::vector<FilePathResult>& result) { return File_ps::UserFileOpenPath(pathIn, result); }
	static bool UserFileSavePath(FileOpenPath& pathInOut, FilePathResult& result) { return File_ps::UserFileSavePath(pathInOut, result); }
	static bool CreateFileDirectory(const char* szDirName, FILE_TYPE eType = FILE_TYPE_SAVE_DATA)
	{
		ASSERT(eType!= FILE_TYPE_RESOURCE);
		return  usg::File_ps::CreateFileDirectory(szDirName, eType);
	}
	static bool Delete(const char* szFileName, const FILE_TYPE = FILE_TYPE_SAVE_DATA);

	// The parameter szDirName should be the relative path to a directory, but
	// should not include a trailing slash. The optional parameter maximumFileCount
	// specifies the maximum number of files to count for platforms that require us to
	// allocate resources to scan the directory.
	static size_t NumberOfFilesInDirectory(const char* szDirName,
	                                       FILE_TYPE eFileType = FILE_TYPE_RESOURCE,
	                                       size_t maximumFileCount = 50)
	{
		return usg::File_ps::NumberOfFilesInDirectory(szDirName, eFileType, maximumFileCount);
	}

#ifdef DEBUG_FILE_READING
	static void ResetReadTime();
#else
	static void ResetReadTime() { };
#endif

	File_ps& GetPlatform() { return m_platform; }
	FILE_ACCESS_MODE GetAccessMode() const { return m_eAccessMode; }

	static FILE_INIT_RESULT GetInitResult() { return m_sInitResult; }
	static void Mount(FILE_TYPE eMode) { File_ps::Mount(eMode); }
	static void Unmount(FILE_TYPE eMode, bool bSave) { File_ps::Unmount(eMode, bSave); }

private:
	PRIVATIZE_COPY(File)

	static FILE_INIT_RESULT m_sInitResult;

	FILE_ACCESS_MODE	m_eAccessMode;
	File_ps				m_platform;

#ifdef DEBUG_FILE_READING
	struct ProfileData*	  m_pProfile;
	usg::string			  m_name;
#endif
};


inline File::File(const char* szFileName, FILE_ACCESS_MODE eMode,FILE_TYPE eFileType)
{
	Open(szFileName, eMode, eFileType);
}




}

#endif
