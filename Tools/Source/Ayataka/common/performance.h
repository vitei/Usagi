#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <stdint.h>

#ifdef _DEBUG
//#define _PERF_MEASURE
#endif

#if defined(_PERF_MEASURE) && !defined(PLATFORM_OSX)

void perfInit( void );
void perfTerm( void );

void perfMeasureBegin( void );
void perfMeasureBeginPrint( const char* filename, int line );
uint32_t perfMeasureEnd( void );
void perfMeasureEndPrint( const char* filename, int line );

#define PERF_MEASURE_BEGIN() perfMeasureBeginPrint( __FILE__, __LINE__ )
#define PERF_MEASURE_END() perfMeasureEndPrint( __FILE__, __LINE__ )

#else
#define perfInit() (void)0
#define perfTerm() (void)0

#define PERF_MEASURE_BEGIN()	(void)0
#define PERF_MEASURE_END()	  (void)0
#endif

#endif // PERFORMANCE_H
