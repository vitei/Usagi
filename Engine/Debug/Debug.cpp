/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Debug/Debug.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Core/File/File.h"
#include "CircularBuffer.h"

#ifdef DEBUG_BUILD
const uint32 g_uPrintFlags = DEBUG_MSG_LOG | DEBUG_MSG_WARNING | DEBUG_MSG_ERROR | DEBUG_MSG_RAW | DEBUG_MSG_RELEASE;
const uint32 g_uExcludeFlags = 0;
#elif (defined FINAL_BUILD)
const uint32 g_uPrintFlags = 0;
const uint32 g_uExcludeFlags = 0xFFFFFFFF;
#else
const uint32 g_uPrintFlags = DEBUG_MSG_ERROR | DEBUG_MSG_RAW | DEBUG_MSG_RELEASE;
const uint32 g_uExcludeFlags = 0;
#endif

using namespace usg;

const int		g_kErrorMsgLength = 4096;
static char		g_errorMsg[g_kErrorMsgLength];

#ifndef FINAL_BUILD
typedef CircularBuffer<51200> LogBuffer;
static char		g_errorMsgB[g_kErrorMsgLength];

static LogBuffer sLogBuffer;

extern "C" void cDebugprintf(const char *szFile, int line, const char* func, uint32 uFlags, const char *format, ...)
{
	bool shouldLog = true;

	if (uFlags & g_uExcludeFlags)
	{
		return;
	}

	va_list vlist;
	va_start( vlist, format );
	if (uFlags & DEBUG_MSG_ERROR)
	{
		str::ParseVariableArgs(g_errorMsgB, sizeof(g_errorMsgB), format, vlist);
		str::ParseVariableArgsC(g_errorMsg, sizeof(g_errorMsg), "ERROR: %s", g_errorMsgB);
	}
	else if (uFlags & DEBUG_MSG_WARNING)
	{
		str::ParseVariableArgs(g_errorMsgB, sizeof(g_errorMsgB), format, vlist);
		str::ParseVariableArgsC(g_errorMsg, sizeof(g_errorMsg), "WARNING: %s", g_errorMsgB);
	}
	else
	{
		str::ParseVariableArgs(g_errorMsg, sizeof(g_errorMsg), format, vlist);
	}
	va_end(vlist);

	if (uFlags & DEBUG_MSG_RAW)
	{		
		if (uFlags & g_uPrintFlags)
		{
			DEBUG_PRINT_INT(g_errorMsg);
		}

		sLogBuffer.Write(g_errorMsg);
		return;
	}
	else
	{
		const char* tmp = szFile;
		while(*tmp != '\0')
		{
			if(*tmp == '\\' || *tmp == '/' )
			{
				szFile = tmp+1;
			}
			++tmp;
		}

		str::ParseVariableArgsC(g_errorMsgB, g_kErrorMsgLength, "%s, %d: %s", szFile, line, g_errorMsg);
		if (uFlags & g_uPrintFlags)
		{
			DEBUG_PRINT_INT(g_errorMsgB);
		}

		sLogBuffer.Write(g_errorMsgB);

		if(uFlags & DEBUG_MSG_ERROR)
		{
		//	DumpDebugLog();
			FATAL_RELEASE_INT(g_errorMsgB);
		}
	}

}
#else
void FatalInFinal(const char *szFile, int line, const char* func, const char *format, ...)
{
	va_list vlist;
	va_start( vlist, format );
	str::ParseVariableArgs( g_errorMsg, sizeof(g_errorMsg), format, vlist );
	FATAL_RELEASE_INT(g_errorMsg);
}

#endif

#ifndef FINAL_BUILD
void DumpDebugLog(const char* szBuildId)
{
	// FIXME: Name this
	const char* szVersionText = szBuildId ? szBuildId : "unversioned";
	usg::U8String dir = "_crash";
	usg::U8String log;
	log.ParseString("_crash/log.%s.txt", szVersionText);

		
	File::CreateFileDirectory(dir.CStr(), usg::FILE_TYPE_DEBUG_DATA);

	File outputFile(log.CStr(), usg::FILE_ACCESS_WRITE, usg::FILE_TYPE_DEBUG_DATA);

	if (outputFile.IsOpen())
	{
		sLogBuffer.RewindReadPos();
		while (sLogBuffer.GetRemainder()>0)
		{
			int len = sLogBuffer.GetRemainderBlock();
			const char *buf = sLogBuffer.Read(len);

			outputFile.Write(len, (void*)buf);
		}
	}

	//Graphics::saveFrameBuffers(dir);
}
#endif
