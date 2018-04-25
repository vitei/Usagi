/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The pre-compiled header file, includes all those libraries
//	which are not likely to commonly change.
*****************************************************************************/
#pragma once

#ifndef USG_COMMON
#define USG_COMMON

#ifndef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0
#endif

#ifdef PLATFORM_SWITCH
#include "_switch/common_ps.h"
#elif (defined PLATFORM_PC)
#include "_win/common_ps.h"
#elif (defined PLATFORM_XB1)
#include "_xb1/Common_ps.h"
#elif (defined PLATFORM_PS4)
#include "_ps4/Common_ps.h"
#elif (defined PLATFORM_WIIU)
#include "_cafe/common_ps.h"
#elif (defined PLATFORM_OSX)
#include "_osx/Common_ps.h"
#elif (defined PLATFORM_IOS)
#include "_ios/Common_ps.h"
#else
#error "Unsupported Platform"
#endif


const sint32	USG_IDENTIFIER_LEN = 32;
const sint32	USG_MAX_NAME = 32;
const sint32	USG_MAX_PATH = 256;
const uint32	USG_INVALID_ID = 0xFFFFFFFF;
const uint64	USG_INVALID_ID64 = 0xFFFFFFFFFFFFFFFF;
const uint8		USG_INVALID_ID8 = 0xFF;
const uint16	USG_INVALID_ID16 = 0xFFFF;

// Hard-coding this to reduce header includes
typedef uint32 pb_size_t;
typedef sint32 pb_ssize_t;

#define PRIVATIZE_COPY(NameOfClass) 	NameOfClass(NameOfClass &rhs) { ASSERT(false); } \
										NameOfClass& operator=(NameOfClass &rhs) { ASSERT(false); return *this; }

#define UNUSED_VAR(var)      ((void)&var);
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#ifndef CCW_WINDING
#define CCW_WINDING 1
#endif

#if !defined(DEPENDENT_TEMPLATE)
#define DEPENDENT_TEMPLATE template
#endif

// Helper macro to check a pointer for NULL before using it
#define SAFE_DEREF(p) if(p != NULL) p

// Standard stuff accessible from anywhere
#include "Engine/Debug/Debug.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Core/stl/memory.h"
#include "GameHandle.h"

#endif // USG_COMMON
