#include "Engine/Common/Common.h"
#include "printer.h"
#include "common.h"

namespace {
	FILE* pFileHandle;
}

void printerInit()
{
	pFileHandle = NULL;
}


void printerTerm()
{
	ASSERT( pFileHandle == NULL );
}


void printerOpen(const char *path)
{
	pFileHandle = fopen( path, "w" );
	ASSERT( pFileHandle != NULL );
}

void printerClose()
{
	fclose( pFileHandle );
	pFileHandle = NULL;
}


void printerWrite(const char *string, ...)
{
	va_list vaList;

	va_start( vaList, string );

	char hoge[256];
	sprintf( hoge, string, vaList );
	fprintf( pFileHandle, string, vaList );

	va_end( vaList );
}

FILE *printerHandle()
{
	return pFileHandle;
}
