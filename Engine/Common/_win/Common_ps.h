/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The pre-compiled header file, includes all those libraries
//	which are not likely to commonly change.
*****************************************************************************/
#ifndef __USG_COMMON_PS_H__
#define __USG_COMMON_PS_H__

#define WIN32_LEAN_AND_MEAN

#if defined(DEBUG) || defined(_DEBUG)
#define DEBUG_BUILD TRUE
#endif

// Suppress the definition of min/max of minwindef.h
#define NOMINMAX
#define USE_VULKAN

//#include <winext/cafe.h>
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <limits.h>
#include <new>
#include <stdlib.h>
#undef min
#undef max

#define STRINGIFY(STR) #STR 
#define XSTR(s) STRINGIFY(s)
#ifdef USE_VULKAN
#define API_HEADER(PATH, FILE) STRINGIFY(PATH/_vulkan/FILE)
#else
#define API_HEADER(PATH, FILE) STRINGIFY(PATH/_ogl/FILE)
#endif
#define FRAGMENT_HEADER(PATH, FILE) STRINGIFY(PATH/_fragment/FILE)
#define OS_HEADER(PATH, FILE) STRINGIFY(PATH/_win/FILE)
#define AUDIO_HEADER(PATH, FILE) STRINGIFY(PATH/_xaudio2/FILE)
#ifndef ALIGNED_VAR
#define ALIGNED_VAR(varType, varAlign, varDef) __declspec(align(varAlign)) varType varDef
#endif

#define DLL_EXPORT extern "C" _declspec(dllexport)

#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )


#define SUPPRESS_WARNING(WARNING, STATEMENT) STATEMENT
#define NO_OPERATION __noop

#define NO_INLINE_TEMPL
#ifndef USE_VULKAN
#define OGL_UVS 1
#define Z_RANGE_0_TO_1 0
#else
#define Z_RANGE_0_TO_1 1
#endif

typedef unsigned char      uint8;
typedef signed char        sint8;
typedef signed short       sint16;
typedef unsigned short     uint16;
typedef signed int         sint32;
typedef unsigned int       uint32;
typedef long long          sint64;
typedef unsigned long long uint64;
typedef wchar_t            char16; 
typedef float              float32;
typedef double             float64;
typedef float              real;	// set to double for double precision
typedef size_t             memsize;
typedef HWND               WindHndl;


// Visual studio allows you to use dependent template names without
// using the "template" keyword -- and fail to compile if you *do*
// use the keyword.  To work around this, override the template
// keyword with a blank string here.
#define DEPENDENT_TEMPLATE

// Annoyingly I can't find a way to turn off warnings-as-errors for a specific warning
// in Visual Studio, so disabling DEPRECATED messages for now.  If you want to turn these
// on, uncomment the line below and comment out the line after it.
//#define DEPRECATED(msg, func) __declspec(deprecated(msg)) func
#define DEPRECATED(msg, func) func

#endif
