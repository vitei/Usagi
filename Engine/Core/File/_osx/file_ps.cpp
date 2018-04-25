/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Memory/Mem.h"
#include "File_ps.h"
#include <stdio.h>
#include <mach-o/dyld.h>
#include <CoreFoundation/CFBundle.h>

namespace usg
{

CFBundleRef g_mainBundle = NULL;

static const char* s_szAccessStrings[] =
{
	"rb",	// FILE_ACCESS_READ
	"wb",	// FILE_ACCESS_WRITE
	"rb+",	// FILE_ACCESS_READ_WRITE
};


bool File_ps::FileExists(const char* szName, const FILE_TYPE eFileType)
{
	/*
	FILE *file;

	CFURLRef resourceURL;
	resourceURL = CFBundleCopyResourceURL(g_mainBundle,
		CFStringCreateWithCString(NULL, szName, kCFStringEncodingASCII), NULL, NULL);

	return resourceURL ? true : false;
	 */
	
	char fileUrl[200];
	sprintf(fileUrl,"%s/%s","_romfiles/osx",szName);
	
	
	FILE *pFile = fopen(fileUrl, s_szAccessStrings[FILE_ACCESS_READ]);
	
	if (pFile)
	{
		fclose(pFile);
		return true;
	}
	
		
	return false;
}

void File_ps::InitFileSystem()
{
	g_mainBundle = CFBundleGetMainBundle();
	ASSERT(g_mainBundle);
}

File_ps::File_ps()
{

}

File_ps::File_ps(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eFileType)
{
	Open(szFileName, eMode, eFileType);
}

File_ps::~File_ps()
{
	Close();
}


bool File_ps::Open(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eFileType)
{
	/*
	char path[1024];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0)
		printf("executable path is %s\n", path);
	
	
	ASSERT(false);
	
    */
	

/*
    FILE *file;
	CFURLRef resourceURL;
	resourceURL = CFBundleCopyResourceURL(g_mainBundle,
		CFStringCreateWithCString(NULL, szFileName, kCFStringEncodingASCII), NULL, NULL);

	ASSERT(resourceURL);

	char fileUrl[200];
	bool res = CFURLGetFileSystemRepresentation(resourceURL, true, (UInt8*)fileUrl, 200);
	ASSERT(res);
  */

    
    char fileUrl[200];
    sprintf(fileUrl,"%s/%s","_romfiles/osx",szFileName);
    
    
    
	m_pFile = fopen(fileUrl, s_szAccessStrings[eMode]);

	if (!m_pFile)
	{
		printf("Error opening %s\n",fileUrl);
		
		char path[1024];
		uint32_t size = sizeof(path);
		if (_NSGetExecutablePath(path, &size) == 0)
			printf("executable path is %s\n", path);
		
	
	}
	
	ASSERT((m_pFile!=NULL) );

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

uint32 File_ps::Read(uint32 uSize, void* pDst)
{
    
	size_t uSizeRead = fread(pDst, 1, uSize, m_pFile);

	ASSERT(uSizeRead == uSize);
	
	return (uint32)uSizeRead;
}

	
uint32 File_ps::Write(uint32 uSize, void* pDst)
{
	
	size_t uSizeRead = fwrite(pDst, 1, uSize, m_pFile);
	
	ASSERT(uSizeRead == uSize);
	
	return (uint32)uSizeRead;
}

uint8 File_ps::ReadByte()
{
	return fgetc(m_pFile);
}

void File_ps::AdvanceBytes(uint32 uAdvance)
{
	fseek(m_pFile, uAdvance, SEEK_CUR);
}

void File_ps::SeekPos(uint32 uPos)
{
	fseek(m_pFile, uPos, SEEK_SET);
}

void File_ps::SeekEnd()
{
	ASSERT(m_pFile!=NULL);

	fseek(m_pFile, 0, SEEK_END);
}

uint32 File_ps::GetPos()
{
	ASSERT(m_pFile!=NULL);

	return (uint32)ftell(m_pFile);
}

uint32 File_ps::GetSize() 
{
	uint32 uSize;
	uint32 uPrevPos = GetPos();

	SeekEnd();
    uSize = GetPos();

	SeekPos(uPrevPos);

	return uSize;
}

}
