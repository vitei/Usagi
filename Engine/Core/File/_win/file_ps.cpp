/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Memory/Mem.h"
#include "File_ps.h"
#include <stdio.h>
#include <commdlg.h>
#include <errno.h>

namespace usg {

	const char* g_szFileDir[FILE_TYPE_COUNT] = 
	{
		"",
		"..\\_savedata\\",
		"..\\_dump\\",
		"_savedata\\"
	};


static const char* s_szAccessStrings[] =
{
	"rb",	// FILE_ACCESS_READ
	"wb",	// FILE_ACCESS_WRITE
	"rb+",	// FILE_ACCESS_READ_WRITE_DEPRECATED
};


FILE_STATUS File_ps::FileStatus(const char* szName, const FILE_TYPE eFileType)
{
    
	FILE *file;
	U8String name(g_szFileDir[eFileType]);
	name += szName;
	fopen_s(&file, name.CStr(), "rb");
    if( (file!=NULL) )
    {
        fclose(file);
        return FILE_STATUS_VALID;
    }
    
    return FILE_STATUS_NOT_FOUND;
}


bool File_ps::UserFileOpenPath(FileOpenPath& pathInOut)
{
	OPENFILENAME open;
	ZeroMemory(&open, sizeof(open));

	open.lStructSize = sizeof(LPOPENFILENAMEA);
	open.lpstrFilter = pathInOut.szFilters;
	open.nFileOffset = 1;
	open.lpstrFile[0] = '\0';
	open.nMaxFile = sizeof(pathInOut.szPathOut);
	open.lpstrTitle = pathInOut.szWindowTitle;
	open.Flags = OFN_FILEMUSTEXIST;

	BOOL Selected = GetOpenFileName(&open);

	return Selected;
}

bool File_ps::Delete(const char* szName, FILE_TYPE eFileType)
{
	U8String name(g_szFileDir[eFileType]);
	name += szName;
	return ( remove(name.CStr()) == 0);
}


bool File_ps::CreateFileDirectory(const char* szName, FILE_TYPE eType)
{
	U8String path= g_szFileDir[eType];
	path += szName;

	return (CreateDirectory(path.CStr(), NULL) == TRUE);
}

size_t File_ps::NumberOfFilesInDirectory(const char* szDirName, FILE_TYPE eFileType,
                                         size_t maximumFileCount)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	U8String fullPath = g_szFileDir[eFileType];
	fullPath += szDirName;
	fullPath += "/*";
	hFind = FindFirstFile(fullPath.CStr(), &findFileData);
	ASSERT(hFind != INVALID_HANDLE_VALUE);
	size_t entryCount = 0;

	do
	{
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}

		entryCount++;
	}
	while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);

	return entryCount;
}

FILE_INIT_RESULT File_ps::InitFileSystem()
{
	CreateDirectory("..\\_savedata", 0);
	CreateDirectory("..\\_dump", 0);

	return FILE_INIT_RESULT_SUCCESS;
}

File_ps::File_ps()
	: m_pFile(nullptr)
{
}

File_ps::File_ps(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eFileType)
{
	m_pFile = NULL;
	Open(szFileName, eMode, eFileType);
}

File_ps::~File_ps()
{
	Close();
}


bool File_ps::Open(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eFileType)
{
    
	U8String fullPath = g_szFileDir[eFileType];
	fullPath += szFileName;
	fopen_s(&m_pFile, fullPath.CStr(), s_szAccessStrings[eMode]);
	
	return (m_pFile!=NULL );
}

void File_ps::Close()
{
	if(m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

bool File_ps::IsOpen()
{
	return (m_pFile!=NULL);
}

memsize File_ps::Read(memsize uSize, void* pDst)
{
	size_t uSizeRead = fread(pDst, 1, uSize, m_pFile);

//	ASSERT(uSizeRead == uSize);
	
	return (uint32)uSizeRead;
}

memsize File_ps::Write(memsize uSize, const void* pSrc)
{
	size_t uSizeWritten = fwrite(pSrc, 1, uSize, m_pFile);

	ASSERT(uSizeWritten == uSize);
	
	return (uint32)uSizeWritten;
}

uint8 File_ps::ReadByte()
{
	return fgetc(m_pFile);
}

void File_ps::AdvanceBytes(memsize uAdvance)
{
	fseek(m_pFile, (long)uAdvance, SEEK_CUR);
}

void File_ps::SeekPos(memsize uPos)
{
	fseek(m_pFile, (long)uPos, SEEK_SET);
}

void File_ps::SeekEnd()
{
	ASSERT(m_pFile!=NULL);

	fseek(m_pFile, 0, SEEK_END);
}

memsize File_ps::GetPos()
{
	ASSERT(m_pFile!=NULL);

	return (uint32)ftell(m_pFile);
}

memsize File_ps::GetSize()
{
	memsize uSize;
	memsize uPrevPos = GetPos();

	SeekEnd();
    uSize = GetPos();

	SeekPos(uPrevPos);

	return uSize;
}

}
