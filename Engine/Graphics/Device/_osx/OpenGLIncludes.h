/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// We haven't a true abstraction in the sense that all our rendering is in a seperate library
// But we don't want to include the bloatware that is windows.h either....
#ifndef FE_GFX_PC_OPENGL_INCLUDES
#define FE_GFX_PC_OPENGL_INCLUDES

#ifdef ERROR
#undef ERROR
#endif

// For now we're including window.h to ensure no discrepancies
#if 0
// For gl.h
#define APIENTRY __stdcall
#define WINGDIAPI __declspec(dllimport)
// For glu.h
#define CALLBACK __stdcall
//typedef unsigned short wchar_t;
// For GLee.h

typedef void VOID, *LPVOID, *HANDLE;;
typedef unsigned int UINT;
typedef int BOOL;
typedef float FLOAT;
typedef unsigned __int16 USHORT;
typedef unsigned __int32 DWORD;
typedef signed __int32 INT32;
typedef signed __int64 INT64;
typedef char CHAR;
typedef long LONG;

typedef struct tagRECT
    {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
    } 	RECT;

#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HGLRC); 
#endif

// We want OpenGL 2 support, for now we're using GLee on windows
//#include "glew/include/GL/glew.h"
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl3.h>


//#define GL_INVALID_INDEXGL_INVALID_ENUM

// Conflict with windows, remove windows.h again when confirmed all working
#undef ERROR
#define ERROR(...) {}

// FIXME: Debug only
inline void CHECK_OGL_ERROR()
{
	int error = glGetError();
	ASSERT(error==0);
}


#endif