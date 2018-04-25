/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Core/String/String_Util.h"

namespace usg {

#ifdef DEBUG_FILE_READING	
	const uint32 MAX_EXTENSIONS = 20;
	struct ProfileData
	{
		char ext[10];
		ProfilingTimer timer;
		uint32	uReadCount;
		uint32  uReadSize;
	};
	uint32 g_extensionTypes = 0;
	ProfileData profileData[MAX_EXTENSIONS];
	
	ProfileData* GetProfile(const char* szExt)
	{
		if (!szExt)
			return NULL;
		uint32 i=0;
		for(; i<g_extensionTypes; i++)
		{
			ProfileData* pData = &profileData[i];
			if(str::Compare(pData->ext, szExt))
			{
				return pData;
			}
		}

		if(g_extensionTypes<MAX_EXTENSIONS)
		{
			ProfileData* pData = &profileData[i];
			pData->uReadCount = 0;
			pData->uReadSize = 0;
			str::Copy(pData->ext, szExt, sizeof(pData->ext));
			g_extensionTypes++;
			return pData;
		}

		return NULL;

	}

#endif

	FILE_INIT_RESULT File::m_sInitResult = FILE_INIT_RESULT_SUCCESS;

	void InitAsyncLoading(uint32 uMaxFiles);
	void ShutDownAsyncLoading();

	void File::InitFileSystem()
	{
		m_sInitResult = File_ps::InitFileSystem();
		InitAsyncLoading(6);
	}

	File::File(void)
	{
#ifdef DEBUG_FILE_READING
		m_pProfile = NULL;
#endif
	}

	void File::FinalizeFileSystem()
	{
		ShutDownAsyncLoading();
		File_ps::FinalizeFileSystem();
	}

	File::~File(void)
	{
#ifdef DEBUG_FILE_READING
		if (m_pProfile)
		{
			m_pProfile->timer.Stop();
			m_pProfile = NULL;
		}
#endif
	}

	bool File::Open(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eType)
	{
		m_eAccessMode = eMode;
#ifdef DEBUG_FILE_READING	
		m_name = szFileName;
		m_pProfile = GetProfile(m_name.GetExtension());
		if (m_pProfile)
		{
			m_pProfile->timer.Start();
			m_pProfile->uReadCount++;
		}
#endif
		bool bFound = m_platform.Open(szFileName, eMode, eType);
		if(!bFound)
		{
			DEBUG_PRINT("Failed to open file %s\n", szFileName);
		}
		return bFound;
	}

	void File::Close()
	{
#ifdef DEBUG_FILE_READING
		if (m_pProfile)
		{
			m_pProfile->timer.Stop();
			m_pProfile = NULL;
		}
#endif
		m_platform.Close();
	}

	bool File::IsOpen()
	{
		return m_platform.IsOpen();
	}

	memsize File::Read(memsize uSize, void* pDst)
	{
#ifdef DEBUG_FILE_READING
		if (m_pProfile)
		{
			m_pProfile->uReadSize += uSize;
		}
#endif
		memsize uBytes = m_platform.Read(uSize, pDst);
		return uBytes;	
	}

	memsize File::Write(memsize uSize, const void* pSrc)
	{
		return m_platform.Write(uSize, pSrc);
	}

	uint8 File::ReadByte()
	{
		return m_platform.ReadByte();
	}

	void File::SeekPos(memsize uPos)
	{
		m_platform.SeekPos(uPos);
	}

	void File::AdvanceBytes(memsize uPos)
	{
		m_platform.AdvanceBytes(uPos);
	}

	memsize File::GetPos()
	{
		return m_platform.GetPos();
	}

	memsize File::GetSize()
	{
		return m_platform.GetSize();
	}

	bool File::Delete(const char* szFileName, const FILE_TYPE eFileType)
	{
		ASSERT(eFileType != FILE_TYPE_RESOURCE);
		if (eFileType != FILE_TYPE_RESOURCE)
		{
			return File_ps::Delete(szFileName, eFileType);
		}
		return false;
	}

#ifdef DEBUG_FILE_READING
	void File::ResetReadTime()
	{
		float32 fTotalSeconds = 0;
		for (uint32 i = 0; i < g_extensionTypes; i++)
		{
			ProfileData* pProfile = &profileData[i];
			if (pProfile->uReadCount > 0)
			{
				LOG_MSG(DEBUG_MSG_RAW| DEBUG_MSG_RELEASE, "Ext %s time %fs count %u size %u\n", pProfile->ext, pProfile->timer.GetTotalSeconds(), pProfile->uReadCount, pProfile->uReadSize);
				fTotalSeconds += pProfile->timer.GetTotalSeconds();
			}
			pProfile->timer.Clear();
			pProfile->uReadCount = 0;
			pProfile->uReadSize = 0;
		}
		LOG_MSG(DEBUG_MSG_RAW | DEBUG_MSG_RELEASE, "Total file read time %f\n", fTotalSeconds);
	}
#endif

}
