/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A platform independent for asynchronous file operation
//	internally uses a singleton, but not something we want to expose
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/AsyncFile.h"
#include "Engine/Core/Thread/Thread.h"
#include "Engine/Core/Thread/CriticalSection.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Core/Thread/MessageQueue.h"

uint32 MAX_READ_REQUESTS = 5;

namespace usg
{
	class AsyncFileRequest
	{
	public:
		AsyncFileRequest() : m_selfHndl(this)
		{
			m_bComplete = false;
		}

		~AsyncFileRequest()
		{
			m_selfHndl.reset();
		}

		void Init(uint32 uBytes, void* pBuffer, AsyncRequestCallback fnCallback)
		{
			m_uBytesRequested = uBytes;
			m_bComplete = false;
			m_pBuffer = pBuffer;
			m_fnCallback = fnCallback;
		}
		uint32 GetBytesReadOrWritten() { return m_bComplete ? m_uBytesReadOrWritten : 0; }

		void* GetBuffer() { return m_pBuffer;  }
		uint32 GetRequestedBytes() { return m_uBytesRequested; }

		void Join()
		{
			while (!m_bComplete)
			{
				Thread::Sleep(2);
			}
		}

		AsyncRequestHndl& GetHndl() { return m_selfHndl; }

		void SetReadComplete(uint32 uBytes)
		{
			m_bComplete = true;
			m_uBytesReadOrWritten = uBytes;
		}

		void TriggerCallback()
		{
			if (m_fnCallback)
			{
				m_fnCallback(m_selfHndl);
			}
		}

	private:
		volatile bool		m_bComplete;
		void*				m_pBuffer;
		AsyncRequestHndl	m_selfHndl;
		AsyncRequestCallback m_fnCallback;
		uint32				m_uBytesReadOrWritten;
		uint32				m_uBytesRequested;
	};

	class AsyncFile : public usg::Thread
	{
	public:
		AsyncFile() : m_selfHndl(this), m_requests(MAX_READ_REQUESTS)
		{
			m_criticalSection.Initialize();
			m_requestQueue.Init(5);
			m_bIsProcessingReq = false;
		}
		virtual ~AsyncFile()
		{
			m_selfHndl.reset();
		}

		void Open(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eType)
		{
			CriticalSection::ScopedLock lock(m_criticalSection);
			m_eOpenMode = eMode;
			m_file.Open(szFileName, eMode, eType);
		}

		AsyncRequestHndl AddRequest(FILE_ACCESS_MODE eMode, uint32 uBytes, void* pData, AsyncRequestCallback fnCallback);
		AsyncFileHndl& GetHndl() { return m_selfHndl; }
		FILE_ACCESS_MODE GetMode() { return m_eOpenMode; }
		uint32 GetSize()
		{
			CriticalSection::ScopedLock lock(m_criticalSection);
			return static_cast<uint32>(m_file.GetSize()); 
		}

		bool IsActive()
		{
			return (m_requestQueue.IsEmpty() && !m_bIsProcessingReq);
		}

	private:

		virtual void Exec();

		MessageQueue<AsyncFileRequest*>	m_requestQueue;
		AsyncFileHndl					m_selfHndl;
		CriticalSection					m_criticalSection;
		FILE_ACCESS_MODE				m_eOpenMode;
		FastPool<AsyncFileRequest>		m_requests;
		usg::File 						m_file;
		volatile bool					m_bIsProcessingReq;
	};

	class AsyncFileMgr
	{
	public:	
		AsyncFileMgr(uint32 uMaxFiles) : m_fileThreads(uMaxFiles, false, false)
		{
			m_criticalSection.Initialize();
		}

		~AsyncFileMgr()
		{
			JoinAllThreads();
		}


		AsyncFileHndl OpenFile(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eType);
		void JoinAllThreads();
		void CheckForFreeThreads();
		bool IsSaveInProgress();
		CriticalSection& GetCriticalSection() { return m_criticalSection;  }
	private:
		void FileFinished(AsyncFile* pFile);

		AsyncFile		file;

		CriticalSection		m_criticalSection;
		FastPool<AsyncFile>	m_fileThreads;
		List<AsyncFile>		m_clearList;
	};

	static AsyncFileMgr* g_asyncFileMgr = NULL;



	void AsyncFile::Exec()
	{
		AsyncFileRequest* pRequest;
		while (!m_selfHndl.unique() || !m_requestQueue.IsEmpty())
		{
			m_bIsProcessingReq = true;
			if(m_requestQueue.TryDequeue(pRequest))
			{
				uint32 uBytes = 0;
				switch(m_eOpenMode)
				{
					case FILE_ACCESS_READ:
						uBytes = static_cast<uint32>(m_file.Read(pRequest->GetRequestedBytes(), pRequest->GetBuffer()));
						break;
					case FILE_ACCESS_WRITE:
						uBytes = static_cast<uint32>(m_file.Write(pRequest->GetRequestedBytes(), pRequest->GetBuffer()));
						break;
					default:
						ASSERT(false);
					}
				pRequest->SetReadComplete(uBytes);
				pRequest->TriggerCallback();
				// FIXME: Remove from the queue
			}
			m_bIsProcessingReq = false;
			Thread_ps::Sleep( 5 );
		}

		m_file.Close();

		// Nothing left to stream and no references to this file
		EndThread();
	}

	AsyncRequestHndl AsyncFile::AddRequest(FILE_ACCESS_MODE eMode, uint32 uBytes, void* pData, AsyncRequestCallback fnCallback)
	{
		if(m_eOpenMode != eMode)
		{
			ASSERT(false);	// Wrong mode
			return AsyncRequestHndl(NULL);
		}
		else
		{
			// Stay in the loop until we have one - could be a risk
			while(true)
			{
				{
					CriticalSection::ScopedLock lock(m_criticalSection);
					AsyncFileRequest* info = NULL;
					if(m_requests.CanAlloc())
					{
						info = m_requests.Alloc();
					}
					else
					{
						// Look for an unreferenced request that we can reuse
						// FIXME: Should have a delete function on the shared pointer that handles this
						for (FastPool<AsyncFileRequest>::Iterator it = m_requests.Begin(); !it.IsEnd(); ++it)
						{
							if ((*it)->GetHndl().unique())
							{
								info = (*it);
							}
						}
					}

					if (info)
					{
						info->Init(uBytes, pData, fnCallback);
						m_requestQueue.Enqueue(info);
						return info->GetHndl();
					}
				}
				Thread::Sleep(1);
			}
		}
	}

	AsyncFileHndl AsyncFileMgr::OpenFile(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eType)
	{
		// Stay in the loop until we have one
		while (true)
		{
			{
				CriticalSection::ScopedLock lock(m_criticalSection);
				g_asyncFileMgr->CheckForFreeThreads();
				if (m_fileThreads.CanAlloc())
				{
					AsyncFile* fileThread = m_fileThreads.Alloc();
					fileThread->Open(szFileName, eMode, eType);
					AsyncFileHndl hndl = fileThread->GetHndl();
					fileThread->StartThread();
					return hndl;
				}
			}
			Thread::Sleep(1);
		}
	}

	void AsyncFileMgr::CheckForFreeThreads()
	{
		for (FastPool<AsyncFile>::Iterator it = m_fileThreads.Begin(); !it.IsEnd(); ++it)
		{
			if ( (*it)->GetHndl().unique() && (*it)->IsThreadEnd())
			{
				m_clearList.AddToEnd((*it));
			}
		}

		for (List<AsyncFile>::Iterator it = m_clearList.Begin(); !it.IsEnd(); ++it)
		{
			(*it)->JoinThread();	// It's over but we need to finalize it
			m_fileThreads.Free((*it));
		}
		m_clearList.Clear();
	}

	bool AsyncFileMgr::IsSaveInProgress()
	{
		CriticalSection::ScopedLock lock(m_criticalSection);
		for (FastPool<AsyncFile>::Iterator it = m_fileThreads.Begin(); !it.IsEnd(); ++it)
		{
			if ((*it)->GetMode() == FILE_ACCESS_WRITE && !(*it)->IsActive())
			{
				return true;
			}
		}
		return false;
	}

	void AsyncFileMgr::FileFinished(AsyncFile* pFile)
	{
		CriticalSection::ScopedLock lock(m_criticalSection);
		ASSERT(pFile->GetHndl().unique());
		m_fileThreads.Free(pFile);
	}

	void AsyncFileMgr::JoinAllThreads()
	{
		for(FastPool<AsyncFile>::Iterator it = m_fileThreads.Begin(); !it.IsEnd(); ++it)
		{
			(*it)->JoinThread();
		}
	}

	bool AsyncIsSaveInProgress()
	{
		return g_asyncFileMgr->IsSaveInProgress();
	}

	void InitAsyncLoading(uint32 uMaxFiles)
	{
		ASSERT(g_asyncFileMgr==NULL);
		g_asyncFileMgr = vnew(ALLOC_OBJECT) AsyncFileMgr(uMaxFiles);
	}

	void ShutDownAsyncLoading()
	{
		if (g_asyncFileMgr != NULL)
		{
			vdelete g_asyncFileMgr;
			g_asyncFileMgr = NULL;
		}
	}

	AsyncFileHndl AsyncOpen(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eType)
	{
		ASSERT(g_asyncFileMgr!=NULL);
		if (File::FileStatus(szFileName, eType) == FILE_STATUS_VALID)
		{
			return g_asyncFileMgr->OpenFile(szFileName, eMode, eType);
		}
		return AsyncFileHndl(NULL);
	}

	uint32 AsyncFileSize(AsyncFileHndl handle)
	{
		if (handle)
		{
			return handle->GetSize();
		}
		return 0;
	}

	AsyncRequestHndl AsyncReadFile(AsyncFileHndl handle, void* pBuffer, uint32 uReadSize, AsyncRequestCallback fnCallback)
	{
		if(handle.get() == NULL)
		{
			ASSERT(false);
			return AsyncRequestHndl(NULL);
		}
		return handle->AddRequest(FILE_ACCESS_READ, uReadSize, pBuffer, fnCallback);
	}

	AsyncRequestHndl AsyncWriteFile(AsyncFileHndl handle, void* pSrc, uint32 uWriteSize, AsyncRequestCallback fnCallback)
	{
		if(handle.get() == NULL)
		{
			ASSERT(false);
			return AsyncRequestHndl(NULL);
		}
		return handle->AddRequest(FILE_ACCESS_WRITE, uWriteSize, pSrc, fnCallback);
	}

	uint32 AsyncGetBytesReadOrWritten(AsyncRequestHndl hndl)
	{
		if(hndl.get() == NULL)
		{
			ASSERT(false);
			return 0;
		}
		return hndl->GetBytesReadOrWritten();
	}

	// Wait for a request to be completed
	uint32 AsyncWait(AsyncRequestHndl hndl)
	{
		if (hndl.get() == NULL)
		{
			// Nothing to wait on
			ASSERT(false);
			return 0;
		}
		hndl->Join();
		return hndl->GetBytesReadOrWritten();
	}

	CriticalSection& AsyncGetCriticalSection()
	{
		return g_asyncFileMgr->GetCriticalSection();
	}
}
