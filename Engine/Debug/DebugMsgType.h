/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Debug message types.  Separated out from Debug.h so that
//	it can safely be included by C files as well as C++ ones.
*****************************************************************************/
#ifndef __USG_DEBUG_MSG_TYPE_H__
#define __USG_DEBUG_MSG_TYPE_H__

typedef enum _DEBUG_MSG_FLAGS
{
	DEBUG_MSG_LOG		= (1<<0),
	DEBUG_MSG_WARNING	= (1<<1),
	DEBUG_MSG_ERROR		= (1<<2),
	DEBUG_MSG_RAW		= (1<<3),	// Don't show file or line number
	DEBUG_MSG_RELEASE	= (1<<4),	// Output even in release
	// TODO: Create user props files so that these can be enabled/disabled by user
	DEBUG_MSG_NETWORK	= (1<<5),
	DEBUG_MSG_AUDIO		= (1<<6),
	DEBUG_MSG_ENGINE	= (1<<6),
	DEBUG_MSG_GAME		= (1<<6),
} DEBUG_MSG_FLAGS;

#ifndef FINAL_BUILD
#ifdef __cplusplus
extern "C"
#endif
void cDebugprintf(const char *file, int line, const char* func, unsigned int uFlags,const char *format, ...);
#else
static inline void cDebugprintf(const char* file, int line, const char* func, unsigned int uFlags, const char* format, ...) {}
#endif

#endif

