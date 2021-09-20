/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Core/stl/string.h"
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
	usg::string name(g_szFileDir[eFileType]);
	name += szName;
	fopen_s(&file, name.c_str(), "rb");
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

bool GetFileResult(const char* szPath, IShellItem* pShellItem, FilePathResult& path)
{
	LPWSTR str = nullptr;
	pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &str);
	usg::string fileOut = WStringToString(str);
	strcpy_s(path.szPath, fileOut.c_str());
	PathRelativePathTo(path.szRelativePath, path.szPath, 0, szPath, FILE_ATTRIBUTE_DIRECTORY);
	const char* szFileName = PathFindFileName(path.szPath);
	if (strcmp(path.szRelativePath, ".\\") == 0)
	{
		strcpy_s(path.szRelativePath, szFileName);
	}
	else
	{
		strcat_s(path.szRelativePath, szFileName);
	}

	return true;
}

// Absolutely HIDEOUS Vita onwards file path code
bool SelectPath(const FileOpenPath& pathIn, usg::vector<FilePathResult>& result, bool bSave)
{
	char szPath[USG_MAX_PATH];
	result.clear();

	if (!pathIn.szOpenDir)
	{
		GetCurrentDirectory(sizeof(szPath), szPath);
	}
	else if (PathIsRelative(pathIn.szOpenDir))
	{
		GetCurrentDirectory(sizeof(szPath), szPath);
		strcat_s(szPath, "\\");
		strcat_s(szPath, pathIn.szOpenDir);
	}
	else
	{
		strcpy_s(szPath, pathIn.szOpenDir);
	}

	GetFullPathName(szPath, sizeof(szPath), szPath, nullptr);

	usg::vector< COMDLG_FILTERSPEC> specs;
	usg::vector< usg::wstring > strings;
	strings.resize(pathIn.uFilterCount * 2);
	for (uint32 i=0; i< pathIn.uFilterCount; i++)
	{
		COMDLG_FILTERSPEC spec;

		usg::wstring szNameTmp = StringToWString(pathIn.pFilters[i].szDisplayName);

		strings.push_back(szNameTmp);
		spec.pszName = strings.back().data();

		usg::wstring szPatternTmp = StringToWString(pathIn.pFilters[i].szExtPattern);

		strings.push_back(szPatternTmp);
		spec.pszSpec = strings.back().data();

		specs.push_back(spec);
	}

	usg::wstring stemp = StringToWString(szPath);
	LPCWSTR sw = stemp.c_str();

	usg::string sext(pathIn.szDefaultExt ? pathIn.szDefaultExt : "");
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
		hr = pDialog->SetOptions(bSave ? FOS_PATHMUSTEXIST : pathIn.bAllowMulti ? FOS_ALLOWMULTISELECT|FOS_FILEMUSTEXIST : FOS_FILEMUSTEXIST);
		ASSERT(!(bSave && pathIn.bAllowMulti));
		hr = pDialog->SetDefaultExtension(swext);
		//hr = pFileOpenDialog->SetFilter(psiFilter);
		hr = pDialog->SetFileTypes((UINT)specs.size(), specs.data());
		hr = SHCreateItemFromParsingName(sw, NULL, IID_PPV_ARGS(&psiFolder));
		hr = pDialog->SetFolder(psiFolder);
		hr = pDialog->Show(nullptr);
		if (SUCCEEDED(hr))
		{
			if (bSave || !pathIn.bAllowMulti)
			{
				hr = pDialog->GetResult(&pShellItem);
				
				usg::FilePathResult tmp;
				bFound = GetFileResult(szPath, pShellItem, tmp);
				if (bFound)
				{
					result.push_back(tmp);
				}
			}
			else
			{
				IShellItemArray* pShellItemArray = NULL;
				DWORD numItems;

				hr = pFileOpenDialog->GetResults(&pShellItemArray);

				hr = pShellItemArray->GetCount(&numItems);
				if (SUCCEEDED(hr))
				{
					for (DWORD i = 0; i < numItems; i++)
					{
						hr = pShellItemArray->GetItemAt(i, &pShellItem);
						if (SUCCEEDED(hr))
						{
							result.push_back_uninitialized();
							bFound = GetFileResult(szPath, pShellItem, result.back());
						}
					}
				}


				bFound = result.size() > 0;

			}
		
		}
	}
	return bFound;
}

bool File_ps::UserFileOpenPath(const FileOpenPath& pathIn, usg::vector<FilePathResult>& result)
{
	return SelectPath(pathIn, result, false);
}

bool File_ps::UserFileSavePath(const FileOpenPath& pathIn, FilePathResult& result)
{
	usg::vector<FilePathResult> results;
	bool bRet = SelectPath(pathIn, results, true);
	if (results.size() > 0)
	{
		usg::MemCpy(&result, &results[0], sizeof(result));
	}

	return bRet;

}

bool File_ps::Delete(const char* szName, FILE_TYPE eFileType)
{
	usg::string name(g_szFileDir[eFileType]);
	name += szName;
	return ( remove(name.c_str()) == 0);
}


bool File_ps::CreateFileDirectory(const char* szName, FILE_TYPE eType)
{
	usg::string path= g_szFileDir[eType];
	path += szName;

	return (CreateDirectory(path.c_str(), NULL) == TRUE);
}

size_t File_ps::NumberOfFilesInDirectory(const char* szDirName, FILE_TYPE eFileType,
                                         size_t maximumFileCount)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	usg::string fullPath = g_szFileDir[eFileType];
	fullPath += szDirName;
	fullPath += "/*";
	hFind = FindFirstFile(fullPath.c_str(), &findFileData);
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
    
	usg::string fullPath = g_szFileDir[eFileType];
	fullPath += szFileName;
	fopen_s(&m_pFile, fullPath.c_str(), s_szAccessStrings[eMode]);
	
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
