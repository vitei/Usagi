/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_ENGINE_MEMORY_MEM_H
#define USG_ENGINE_MEMORY_MEM_H
#include "Engine/Memory/ScratchObj.h"
#include "Engine/Memory/Allocator.h"

#if defined PLATFORM_OSX || defined TOOL_BUILD || defined PLATFORM_SWITCH_DEVICE
#include <new>
#endif

#ifndef  FINAL_BUILD
#define DEBUG_MEMORY
#endif

#include OS_HEADER(Engine/Memory, Mem_ps.h)

// FIXME: This is a bit ugly, just getting the functionality in place until
// someone volunteers to write a proper memory manager ;)
// Oh bollocks, turns out that's me. It's likely to remain ugly.

namespace usg{

class MemHeap;
class Allocator;

extern MemType g_uDefaultMemType;


namespace mem
{
	struct AllocatorData
	{
		Allocator* pAllocators[MEMTYPE_COUNT];
	};



	void InitialiseDefault();
	void InitialiseAllocators(AllocatorData* pInitData);
	void Cleanup();
	Allocator* GetAllocator(MemType eType);

	void* Alloc(MemType eHeap, MemAllocType eType, size_t uSize, uint32 uAlign = 8, bool bNotifyGPU = false);

	void Free(MemType eHeap, void*, bool bNotifyGPU = false);

	// For now only impacts the GPU stacks, but will alter the grouping on all memory
	void AddMemoryTag();
	void FreeToLastTag();

	void Free(void*);

	const char* GetAllocString(MemAllocType eType);
	MemHeap* GetMainHeap();
	MemHeap* GetNetHeap();

#if defined (PLATFORM_PC) || defined(PLATFORM_SWITCH)
	extern void setConventionalMemManagement(bool b);
#endif
}

}

// Later on we'll be wrapping debug info
#ifndef FINAL_BUILD
	#ifdef EXCEPTIONS_ENABLED
		#define NEW_THROW throw()
	#else
		#define NEW_THROW
	#endif

#define vnew(a)					new(a, usg::g_uDefaultMemType, __FILE__)
#define vnew_aligned(a, b, c)	new(a, b, c, __FILE__)
#define vnewHeap(a, b)			new(a, b, __FILE__)
#define vdelete					delete
#else
#define NEW_THROW

#define vnew(a)					new(a, usg::g_uDefaultMemType)
#define vnew_aligned(a, b, c)	new(a, b, c)
#define vnewRaw					new
#define vnewHeap(a, b)			new(a, b)
#define vdelete					delete
#endif

#if (!defined PLATFORM_OSX) && (!defined PLATFORM_SWITCH_DEVICE)
void* operator new(size_t size);
void  operator delete(void* ptr);
#endif



#if (!defined PLATFORM_WIIU && !defined PLATFORM_PS4 && !defined PLATFORM_OSX && !defined PLATFORM_SWITCH_DEVICE)

#ifndef __PLACEMENT_NEW_INLINE
	#define __PLACEMENT_NEW_INLINE
	inline void* operator new(size_t size, void* pData) NEW_THROW { return pData; }
	inline void operator delete(void*, void*) NEW_THROW {}
#endif

#ifndef __PLACEMENT_VEC_NEW_INLINE
	#define __PLACEMENT_VEC_NEW_INLINE
	inline void* operator new[](size_t, void* pData) NEW_THROW{ return pData; }
	inline void  operator delete[](void*, void*) NEW_THROW{}
#endif
#endif


#ifndef FINAL_BUILD

inline void* operator new(size_t size, usg::MemAllocType eType, usg::MemType eHeap, const char* szFileName) NEW_THROW
{
	void* pData = usg::mem::Alloc(eHeap, eType, size, 8);
	return pData;
}

inline void* operator new(size_t size, size_t align, usg::MemAllocType eType, usg::MemType eHeap, const char* szFileName) NEW_THROW
{
	void* pData = usg::mem::Alloc(eHeap, eType, size, (uint32)align);
	return pData;
}

#ifdef EXCEPTIONS_ENABLED
inline void operator delete(void* pData, usg::MemAllocType eType, usg::MemType eHeap,const char* szFileName) NEW_THROW
{
	delete pData;
}

inline void operator delete(void* pData, size_t align, usg::MemAllocType eType, usg::MemType eHeap,const char* szFileName) NEW_THROW
{
	delete pData;
}
#endif

inline void* operator new(size_t size, uint32 uAlign, usg::MemAllocType eType, usg::Allocator* pAllocator, const char* szFileName) NEW_THROW
{
	void* pData = pAllocator->Alloc(eType, static_cast<uint32>(size), uAlign);
	return pData;
}

inline void* operator new[](size_t size, usg::MemAllocType eType, usg::MemType eHeap, const char* szFileName) NEW_THROW
{
	return usg::mem::Alloc(eHeap, eType, size, 8);
}

inline void* operator new[](size_t size, size_t align, usg::MemAllocType eType, usg::MemType eHeap, const char* szFileName) NEW_THROW
{
	return usg::mem::Alloc(eHeap, eType, size, static_cast<uint32>(align));
}

#ifdef EXCEPTIONS_ENABLED
inline void operator delete[](void* pData, usg::MemAllocType eType, usg::MemType eHeap, const char* szFileName) NEW_THROW
{
	delete pData;
}

inline void operator delete[](void* pData, size_t align, usg::MemAllocType eType, usg::MemType eHeap, const char* szFileName) NEW_THROW
{
	delete pData;
}
#endif

inline void* operator new[](size_t size, uint32 uAlign, usg::MemAllocType eType, usg::Allocator* pAllocator, const char* szFileName) NEW_THROW
{
	void* pData = pAllocator->Alloc(eType, static_cast<uint32>(size), uAlign);
	return pData;
}


#else

inline void* operator new(size_t size, usg::MemAllocType eType, usg::MemType eHeap)
{
	void* pData = usg::mem::Alloc(eHeap, eType, size, 4);
	return pData;
}

inline void* operator new(size_t size, size_t align, usg::MemAllocType eType, usg::MemType eHeap)
{
	void* pData = usg::mem::Alloc(eHeap, eType, size, align);
	return pData;
}


inline void* operator new[](size_t size, usg::MemAllocType eType, usg::MemType eHeap)
{
	return usg::mem::Alloc(eHeap, eType, size, 4);
}


#endif

#ifndef EASTL_USER_DEFINED_ALLOCATOR
// Used by STL allocator
inline void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line) NEW_THROW
{
	return usg::mem::Alloc(usg::g_uDefaultMemType, usg::ALLOC_OBJECT, size, sizeof(void*) * 2);
}

inline void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line) NEW_THROW
{
	return usg::mem::Alloc(usg::g_uDefaultMemType, usg::ALLOC_OBJECT, size, sizeof(void*) * 2);
}
#endif


#endif