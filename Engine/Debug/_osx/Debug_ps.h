/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Custom assert functions, main advantages are restoring of
//	the mouse cursor which will normally be hidden, and the ability to
//	output a message to the console.
*****************************************************************************/
#ifndef __USG_DEBUG_PS_H__
#define __USG_DEBUG_PS_H__

#include <stdarg.h>

#ifdef assert
#undef assert
#endif

/*
const int		g_kErrorMsgLength = 5096;
static char		g_errorMsg[g_kErrorMsgLength];
static char		g_errorMsgB[g_kErrorMsgLength];
static HRESULT	g_hResult;
*/

// TODO: Message box, Debug, Ignore, Ignore All
#ifndef FINAL_BUILD
inline void ASSERT(bool condition)
{
	if(! condition )
	{
//		ShowCursor( TRUE );
		//__debugbreak();
		__builtin_trap();
	}
}
#define ASSERT_RETURN( condition ) \
	ASSERT( condition ); \
	if ( (condition) == false) \
	return;

#define ASSERT_RETURN_VALUE( condition, value ) \
	ASSERT( condition  ); \
	if ( (condition) == false ) \
	return (value);

#else
#define ASSERT_RETURN( condition ) \
	if ( (condition) == false) return;
#define ASSERT_RETURN_VALUE( condition, value ) \
	if ( (condition) == false) return (value) ;

	#define ASSERT(cond) (void)0
#endif



inline void ASSERT_MSG(bool condition, const char* msg, ...)
{
#ifndef FINAL_BUILD
	if(! condition )
	{
		va_list va;
		va_start(va, msg);
//		ShowCursor( TRUE );
		fprintf( stderr, msg, va);
		__builtin_trap();
	}
#else
	(void)0;
#endif
}

// For file loading only, will quit the game on failure, even in release mode
inline void FATAL_RELEASE_INT( const char* msg )
{
	printf(msg);
	
	ASSERT(false);	// So we can debug first
}

// TODO: This should be on it's own thread, periodically doing printouts
// TODO: Optionally output to final using a logger
inline void DEBUG_PRINT_INT( const char* msg )
{
    printf(msg);
    
}


// TODO: Message versions of these functions to indicate function which failed
inline void HRCheck( HRESULT hr )
{
#ifdef _DEBUG												
	if(FAILED(hr))
	{
//		MessageBox(0, DXGetErrorString(hr), 0, 0);
		__builtin_trap();
	}
#else
//	hr;
#endif
}


#ifdef _DEBUG
#define HRCHECK_RETURN(hr)  { g_hResult = (hr);	if( !SUCCEEDED(g_hResult) ) { __debugbreak(); return false;	}	}				
#else
#define HRCHECK_RETURN(hr)  { g_hResult = (hr);	if( !SUCCEEDED(g_hResult) ) { return false; } }
#endif



inline bool FatalHRCheck( HRESULT hr )
{
	if(FAILED(hr))
	{
//		MessageBox(0, DXGetErrorString(hr), 0, 0);
		__builtin_trap();
		return false;
	}
	return true;
}


#endif
