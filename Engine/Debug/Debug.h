/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Custom assert functions, main advantages are restoring of
//	the mouse cursor which will normally be hidden, and the ability to
//	output a message to the console.
*****************************************************************************/
#ifndef __USG_DEBUG_H__
#define __USG_DEBUG_H__

#include "Engine/Common/Common.h"
#include OS_HEADER(Engine/Debug, Debug_ps.h)
#include "DebugMsgType.h"

#ifndef FINAL_BUILD
// Cache the debug log on all versions of the game except for the final rom
void DumpDebugLog(const char* szBuildId = NULL);
#else
void FatalInFinal(const char *file, int line, const char* func, const char *format, ...);
#endif

#ifndef FINAL_BUILD
// But only actually print out the log on a debug build
#define DEBUG_PRINT( ... )	cDebugprintf(__FILE__, __LINE__, __FUNCTION__, DEBUG_MSG_LOG,__VA_ARGS__)
#define LOG_MSG( uFlags, ... )	cDebugprintf(__FILE__, __LINE__, __FUNCTION__, uFlags, __VA_ARGS__)
#else
#define DEBUG_PRINT( ... ) ((void) 0)
#define LOG_MSG( ... ) ((void) 0)
#endif

#ifndef FINAL_BUILD
#define FATAL_RELEASE(in, ...) { if(in == false) { cDebugprintf(__FILE__, __LINE__, __FUNCTION__, DEBUG_MSG_ERROR, __VA_ARGS__); } }
#else
#define FATAL_RELEASE(in, ...) { if(in == false) { FatalInFinal(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } }
#endif

#endif
