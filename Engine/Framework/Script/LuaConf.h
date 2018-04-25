/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _LUA_CONF_H
#define _LUA_CONF_H

#include "Engine/Debug/DebugMsgType.h"

// Default to 32 bits
#define LUA_32BITS

// Turn off automatic adding of '.0' to floating point numbers.
// This makes use of some localisation libraries which we don't have.
#define LUA_COMPAT_FLOATSTRING

// As if the compilers we work with would be up-to-date enough to support C99!
// They've only had 17 years to catch up!
#define LUA_USE_C89

// It'd be nice if we could use the 'bit32' library for bitmasks.
#define LUA_COMPAT_BITLIB

// There's a comment next to l_randomizePivot() saying that 0 is a reasonable
// value if you don't need the randomness in sort()
#define l_randomizePivot() 0

// luai_makeseed() uses time() as its seed by default... we don't have time(),
// so just use rand(), assuming it will have been seeded by the time we initialise
// lua.
#define luai_makeseed() rand()

// Use DEBUG_PRINT for our string writing
#define lua_writestring(s,l)      cDebugprintf(__FILE__, __LINE__, __FUNCTION__, DEBUG_MSG_RAW, s)
#define lua_writeline()           cDebugprintf(__FILE__, __LINE__, __FUNCTION__, DEBUG_MSG_RAW, "\n")
#define lua_writestringerror(s,p) cDebugprintf(__FILE__, __LINE__, __FUNCTION__, DEBUG_MSG_RAW, (s), (p))

#endif //_LUA_CONF_H
