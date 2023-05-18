/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Custom assert functions, main advantages are restoring of
//	the mouse cursor which will normally be hidden, and the ability to
//	output a message to the console.
*****************************************************************************/
#ifndef __USG_DEBUG_PS_H__
#define __USG_DEBUG_PS_H__

#ifdef assert
#undef assert
#endif

#include "tchar.h"
#include "DebugMsgType.h"

static HRESULT	g_hResult;

// TODO: Message box, Debug, Ignore, Ignore All
#ifndef FINAL_BUILD
#ifdef ASSERT
#undef ASSERT
#endif

extern void DumpDebugLog(const char* szBuildId);


#define ASSERT(cond) { if(!(cond)) cDebugprintf(__FILE__, __LINE__, __FUNCTION__, DEBUG_MSG_ERROR, "Assert");  }

#define ASSERT_RETURN( condition ) \
	{ASSERT( condition ); \
	if ( (condition) == false) \
	return;}

#define ASSERT_RETURN_VALUE( condition, value ) \
	{ASSERT( condition ); \
	if ( (condition) == false ) \
	return (value);}

#define ASSERT_MSG( cond, ... ) if(!(cond)) { cDebugprintf(__FILE__, __LINE__, __FUNCTION__, DEBUG_MSG_ERROR,__VA_ARGS__); } 

#else
#define ASSERT_RETURN( condition ) \
	if ( (condition) == false) return;
#define ASSERT_RETURN_VALUE( condition, value ) \
	if ( (condition) == false) return (value) ;

#define ASSERT(cond) (void)0
#define ASSERT_MSG(cond, ... ) (void)0
#endif



// For file loading only, will quit the game on failure, even in release mode
inline void FATAL_RELEASE_INT( const TCHAR* msg )
{
	if(IsDebuggerPresent())
	{
		ASSERT(false);
	}
	else
	{
		MessageBox(0, msg, TEXT("FATAL"), 0);	
		exit(-1);
	}
}

inline void DEBUG_PRINT_INT( const TCHAR* msg )
{
	OutputDebugString(msg);
}


// TODO: Message versions of these functions to indicate function which failed
inline void HRCHECK( HRESULT hr )
{
#ifdef _DEBUG												
	if(FAILED(hr))
	{
//		MessageBox(0, DXGetErrorString(hr), 0, 0);
		__debugbreak();
	}
#else
	hr;
#endif
}


#ifdef _DEBUG
#define HRCHECK_RETURN(hr)  { g_hResult = (hr);	if( !SUCCEEDED(g_hResult) ) { __debugbreak(); return false;	}	}				
#else
#define HRCHECK_RETURN(hr)  { g_hResult = (hr);	if( !SUCCEEDED(g_hResult) ) { return false; } }
#endif



inline bool FATAL_HRCHECK( HRESULT hr )
{
	if(FAILED(hr))
	{
//		MessageBox(0, DXGetErrorString(hr), 0, 0);
		__debugbreak();
		return false;
	}
	return true;
}


#endif