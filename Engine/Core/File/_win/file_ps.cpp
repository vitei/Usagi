/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Memory/Mem.h"
#include "File_ps.h"
#include <stdio.h>
#include <shobjidl.h> 
#include <shlwapi.h>
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

usg::wstring StringToWString(const usg::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	if (len > 256)
	{
		ASSERT(false);
		return L"";
	}
	wchar_t szBuff[256];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, szBuff, len);
	usg::wstring r(szBuff);
	return r;
}

usg::string WStringToString(const usg::wstring& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
	if (len > 256)
	{
		ASSERT(false);
		return "";
	}
	char szBuff[256];
	WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, szBuff, len, 0, 0);
	usg::string r(szBuff);
	return r;
}

// Absolutely HIDEOUS Vita onwards file path code
bool SelectPath(FileOpenPath& pathInOut, bool bSave)
{
	char szPath[USG_MAX_PATH];

	if (!pathInOut.szOpenDir)
	{
		GetCurrentDirectory(sizeof(szPath), szPath);
	}
	else if (PathIsRelative(pathInOut.szOpenDir))
	{
		GetCurrentDirectory(sizeof(szPath), szPath);
		strcat_s(szPath, "\\");
		strcat_s(szPath, pathInOut.szOpenDir);
	}
	else
	{
		strcpy_s(szPath, pathInOut.szOpenDir);
	}

	GetFullPathName(szPath, sizeof(szPath), szPath, nullptr);

	usg::vector< COMDLG_FILTERSPEC> specs;
	usg::vector< usg::wstring > strings;
	for (uint32 i=0; i<pathInOut.uFilterCount; i++)
	{
		COMDLG_FILTERSPEC spec;

		usg::wstring szNameTmp = StringToWString(pathInOut.pFilters[i].szDisplayName);

		strings.push_back(szNameTmp);
		spec.pszName = strings.back().data();

		usg::wstring szPatternTmp = StringToWString(pathInOut.pFilters[i].szExtPattern);

		strings.push_back(szPatternTmp);
		spec.pszSpec = strings.back().data();

		specs.push_back(spec);
	}

	usg::wstring stemp = StringToWString(szPath);
	LPCWSTR sw = stemp.c_str();

	usg::string sext(pathInOut.szDefaultExt ? pathInOut.szDefaultExt : "");
	usg::wstring sexttemp = StringToWString(sext.c_str());
	LPCWSTR swext = sexttemp.c_str();

	bool bFound = false;
	IFileOpenDialog* pFileOpenDialog = NULL;
	IFileSaveDialog* pFileSaveDialog = NULL;
	IShellItem* pShellItem = NULL;

	HRESULT hr;
	if(!bSave)
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpenDialog));
	else
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileSaveDialog));
	
	IFileDialog* pDialog = bSave ? (IFileDialog*)pFileSaveDialog : (IFileDialog*)pFileOpenDialog;

	if (SUCCEEDED(hr))
	{
		IShellItem* psiFolder; //IShellItemFilter* psiFilter;
		hr = pDialog->SetOptions(bSave ? FOS_PATHMUSTEXIST : FOS_FILEMUSTEXIST);
		hr = pDialog->SetDefaultExtension(swext);
		//hr = pFileOpenDialog->SetFilter(psiFilter);
		hr = pDialog->SetFileTypes((UINT)specs.size(), specs.data());
		hr = SHCreateItemFromParsingName(sw, NULL, IID_PPV_ARGS(&psiFolder));
		hr = pDialog->SetFolder(psiFolder);
		hr = pDialog->Show(nullptr);
		if (SUCCEEDED(hr))
		{
			hr = pDialog->GetResult(&pShellItem);
			LPWSTR str = nullptr;
			pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &str);
			usg::string fileOut = WStringToString(str);
			strcpy_s(pathInOut.szPathOut, fileOut.c_str());
			PathRelativePathTo(pathInOut.szRelativePathOut, pathInOut.szPathOut, 0, szPath, FILE_ATTRIBUTE_DIRECTORY);
			const char* szFileName = PathFindFileName(pathInOut.szPathOut);
			if (strcmp(pathInOut.szRelativePathOut, ".\\") == 0)
			{
				strcpy_s(pathInOut.szRelativePathOut, szFileName);
			}
			else
			{
				strcat_s(pathInOut.szRelativePathOut, szFileName);
			}
			
			bFound = true;
		}
	}
	return bFound;
}

bool File_ps::UserFileOpenPath(FileOpenPath& pathInOut)
{
	return SelectPath(pathInOut, false);
}

bool File_ps::UserFileSavePath(FileOpenPath& pathInOut)
{
	return SelectPath(pathInOut, true);
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

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |COINIT_DISABLE_OLE1DDE);

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
