
#include "debug.h"

#ifdef WIN32
#include <stdio.h>
#include <windows.h>
#endif

void debugPrintf( const char* str, ... ) {
#ifdef WIN32
	char buf[256];

	va_list vlist;
	va_start( vlist, str );
	vsnprintf_s( buf, sizeof( buf ), sizeof( buf ) - 1, str, vlist );
	va_end( vlist );

	OutputDebugStringA( buf );
#else
#endif
}