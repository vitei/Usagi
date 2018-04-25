/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific file data
*****************************************************************************/
#ifndef _USG_FILE_PS_H_
#define _USG_FILE_PS_H_
#include <stdio.h>

namespace usg
{

class File_ps
{
public:
	File_ps(void);
	File_ps(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eFileType);
	~File_ps(void);

	static void InitFileSystem();

	bool Open(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eFileType);
	void Close();

	bool IsOpen();
	uint32 Read(uint32 uSize, void* pDst);
	uint32 Write(uint32 uSize, void* pSrc);
	uint8 ReadByte();
	void SeekPos(uint32 uPos);
	void AdvanceBytes(uint32 uPos);
	void SeekEnd();
	uint32 GetPos();
	uint32 GetSize();

	static bool FileExists(const char* szName, const FILE_TYPE eFileType = FILE_TYPE_RESOURCE);

	static bool CreateFileDirectory(const char* szDirName, FILE_TYPE eType)
	{
		return false;
	}
	
private:
	File_ps(const File_ps& File) {};
	const File_ps& operator=(const File_ps& src);
	FILE*	m_pFile;

};

}

#endif
