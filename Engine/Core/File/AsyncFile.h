/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A platform independent for asynchronous file operation
*****************************************************************************/
#ifndef _USG_ASYNC_FILE_H_
#define _USG_ASYNC_FILE_H_
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/SharedPointer.h"
#include "Engine/Core/File/File.h"

namespace usg
{
	class AsyncFile;
	class AsyncFileRequest;	// TODO: Make this more generic, not just for files

	// FIXME: Rather than shared pointers these should be a new reference tracker type (but fortunately our sharedpointer implementation lets us treat it as such)
	typedef SharedPointer<AsyncFile> 		AsyncFileHndl;
	typedef SharedPointer<AsyncFileRequest>	AsyncRequestHndl;

	typedef void (*AsyncRequestCallback) (AsyncRequestHndl handle);

	AsyncFileHndl AsyncOpen(const char* szFileName, FILE_ACCESS_MODE eMode, FILE_TYPE eType);
	AsyncRequestHndl AsyncReadFile(AsyncFileHndl handle, void* pBuffer, uint32 uReadSize, AsyncRequestCallback fnCallback = NULL);
	uint32 AsyncFileSize(AsyncFileHndl handle);
	AsyncRequestHndl AsyncWriteFile(AsyncFileHndl handle, void* pSrc, uint32 uWriteSize, AsyncRequestCallback fnCallback = NULL);
	uint32 AsyncGetBytesReadOrWritten(AsyncRequestHndl hndl);
	// Wait for a request to be completed
	uint32 AsyncWait(AsyncRequestHndl hndl);

	bool AsyncIsSaveInProgress();
	class CriticalSection& AsyncGetCriticalSection();

}

#endif
