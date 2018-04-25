/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The pre-compiled header file, includes all those libraries
//	which are not likely to commonly change.
*****************************************************************************/
#ifndef __USG_COMMON_PS_H__
#define __USG_COMMON_PS_H__


#if defined(DEBUG) || defined(_DEBUG)
#define DEBUG_BUILD TRUE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <limits.h>
#include <typeinfo>

#define PACK( __Declaration__ ) __Declaration__ __attribute__((packed))

#define STRINGIFY(STR) #STR 
#define XSTR(s) STRINGIFY(s)
#define API_HEADER(PATH, FILE) STRINGIFY(PATH/_ogl/FILE)
#define FRAGMENT_HEADER(PATH, FILE) STRINGIFY(PATH/_fragment/FILE)
#define VR_HEADER(PATH, FILE) STRINGIFY(PATH/_ovr/FILE)
#define OS_HEADER(PATH, FILE) STRINGIFY(PATH/_osx/FILE)
#define AUDIO_HEADER(PATH, FILE) STRINGIFY(PATH/_osx/FILE)
#define THREAD_HEADER(PATH, FILE) STRINGIFY(PATH/_pthread/FILE)
#define ALIGNED_VAR(varType, varAlign, varDef) __declspec(align(varAlign)) varType varDef

#define PRAGMA_ARGUMENT(WARNING_FLAG) STRINGIFY(clang diagnostic ignored WARNING_FLAG)
#define SUPPRESS_WARNING(WARNING, STATEMENT) _Pragma("clang diagnostic push") \
_Pragma(PRAGMA_ARGUMENT(WARNING)) \
STATEMENT; \
_Pragma("clang diagnostic pop")

#define NO_OPERATION 

#define NO_INLINE_TEMPL

#define Z_RANGE_0_TO_1 		0

typedef unsigned char       uint8;
typedef signed char         sint8;
typedef signed short        sint16;
typedef unsigned short      uint16;
typedef signed int          sint32;
typedef unsigned int        uint32;
typedef long long           sint64;
typedef unsigned long long  uint64;
typedef wchar_t             char16;
typedef float               float32;
typedef double              float64;
typedef float               real;	// set to double for double precision
typedef size_t              memsize;
typedef int                 HRESULT;
typedef unsigned char       BYTE; // More ugly windows nonsense scattered all over our code :-(

#define FAILED(hr) (((HRESULT)(hr)) < 0)

template<typename t>
t max(const t& a, const t& b) {
	return a > b ? a : b;
}

/*

#define TRUE                true
#define FALSE               false
*/


// Swap these comments if you want to see deprecation warnings
//#define DEPRECATED(msg, func) func __attribute__ ((deprecated))
#define DEPRECATED(msg, func) func




#endif
