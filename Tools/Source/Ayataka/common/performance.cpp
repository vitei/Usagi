#include "performance.h"

#if defined(_PERF_MEASURE) && !defined(PLATFORM_OSX)

#include <Windows.h>
#include <stack>

typedef std::stack<uint32_t> StackPerfCount;

namespace {
	StackPerfCount* pStack = NULL;
}

void perfInit( void )
{
	pStack = vnew(ALLOC_OBJECT) StackPerfCount;
}

void perfTerm( void )
{
	SAFE_DELETE( pStack );
}

void perfMeasureBegin( void )
{
	pStack->push( GetTickCount() );
}

void perfMeasureBeginPrint(const char *filename, int line)
{
	perfMeasureBegin();
	printf( "[%s:%d:%c] measure begin\n", filename, line, 'A' + pStack->size() );
}

uint32_t perfMeasureEnd( void )
{
	uint32_t time = 0;
	if( pStack->size() > 0 ) {
		time = GetTickCount() - pStack->top();
		pStack->pop();
	}
	return time;
}

void perfMeasureEndPrint(const char *filename, int line)
{
	char depth = 'A' + pStack->size();
	printf( "[%s:%d:%c] %dms\n", filename, line, depth, perfMeasureEnd() );
}

#endif // #ifdef _PERF_MEASURE
