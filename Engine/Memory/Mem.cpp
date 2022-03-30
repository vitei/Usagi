/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemHeap.h"
#include "Engine/Memory/DoubleStack.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Memory/Allocator.h"

namespace usg{

static MemHeap* 			g_mainHeap = nullptr;
static MemHeap* 			g_netHeap = nullptr;

// Only used if we aren't passed in allocator overrides
static HeapAllocator		g_mainHeapAlloc;
static HeapAllocator		g_netHeapAlloc;
static HeapAllocator		g_overflowAlloc;

static mem::AllocatorData	g_allocators;

MemType g_uDefaultMemType = MEMTYPE_STANDARD;

static const char* gszAllocStrings[] =
{
	"Objects",
	"Shader constants",
	"Collision data",
	"Array Pool",
	"Lists",
	"Fast Pool",
	"Geometry data",
	"Index data",
	"Internal gfx data",
	"Shaders",
	"Render targets",
	"Textures",
	"GFX hooks",
	"Command buffer",
	"Audio",
	"Debug",
	"Loading data",
	"Protocol buffer msg",
	"External allocations",
	"Components",
	"Entity",
	"System",
	"Layouts",
	"Scripts",
	"Network",
	"Shop",
	"Save data",
	"Screenshot",
	"String",
	"Container",
	"Alloc Font",
	"Resource Mgr",
	"StringPointerHash",
	"BehaviorTree",
	"Event",
	"Particles",
	"Animation",
	"Camera",
	"Mode",
	"Physics",
	"STL Default",
	"Don't use!!!!"
};

static_assert(ARRAY_SIZE(gszAllocStrings) == MemAllocType::ALLOC_TYPE_COUNT, "gszAllocStrings size does NOT match that of MemAllocType::ALLOC_TYPE_COUNT");

#if defined (PLATFORM_PC) || defined(PLATFORM_SWITCH)
namespace mem {
	static bool s_bConventionalMemManagement = true;
	void setConventionalMemManagement(bool b)
	{
		s_bConventionalMemManagement = b;
	}
}
#endif

static void InitaliseCommon()
{
	ScratchRaw::InitMemory(SCRATCH_HEAP_SIZE);
}

MemHeap* mem::GetMainHeap()
{
	return g_mainHeap;
}

MemHeap* mem::GetNetHeap()
{
	return g_netHeap;
}

void mem::InitialiseDefault()
{
	static MemHeap s_heapNet;
	static MemHeap s_MainHeap;

	s_bConventionalMemManagement = false;
	g_netHeap = &s_heapNet;
	g_netHeap->Initialize(NET_HEAP_SIZE);
	g_netHeapAlloc.Init(g_netHeap, 0);
	g_allocators.pAllocators[MEMTYPE_NETWORK] = &g_netHeapAlloc;

	InitaliseCommon();

	g_mainHeap = &s_MainHeap;
	g_mainHeap->Initialize(MAIN_HEAP_SIZE);
	g_mainHeapAlloc.Init(g_mainHeap, 0);
	g_allocators.pAllocators[MEMTYPE_STANDARD] = &g_mainHeapAlloc;

	mem::ps::InitialiseDefault(&g_allocators);
}

void mem::Cleanup()
{
	// FIXME: Free the heaps we've allocated!!!!!!!!!
	mem::ps::Cleanup();
	// Anything after this point was external to our memory management
	mem::setConventionalMemManagement(true);
}


void mem::InitialiseAllocators(AllocatorData* pInitData)
{
	for(uint32 i=0; i<MEMTYPE_COUNT; i++)
	{
		g_allocators.pAllocators[i] = pInitData->pAllocators[i];
	}
	InitaliseCommon();
}

Allocator* mem::GetAllocator(MemType eType)
{
	return g_allocators.pAllocators[eType];
}

static Allocator* GetGlobalAllocator(MemType eHeap)
{
	static constexpr memsize heapSizes[] =
	{
		MAIN_HEAP_SIZE, NET_HEAP_SIZE
	};

	static MemHeap heaps[MEMTYPE_COUNT];
	static bool b_allocatorInitialized[MEMTYPE_COUNT] = { false };

	if (!b_allocatorInitialized)
	{
		// Init...
	}
	return g_allocators.pAllocators[eHeap];
}

static uint8 s_nexHackBuffer[512];
static uint8* s_pNexBufferPointer = s_nexHackBuffer;

void* mem::Alloc(MemType eHeap, MemAllocType eType, memsize uSize, uint32 uAlign, bool bNotifyGPU)
{
#ifdef PLATFORM_PC
	if (usg::mem::s_bConventionalMemManagement)
	{
		return _aligned_malloc(uSize, uAlign);
	}
	else
#endif
	{
#if 0
		if (eType == ALLOC_DONT_USE)
		{
			void* pReturn = s_pNexBufferPointer;
			s_pNexBufferPointer += uSize;

			auto iPtrDiff = s_pNexBufferPointer - s_nexHackBuffer;
			if (iPtrDiff >= (decltype(iPtrDiff))(s_nexHackBuffer))
			{
				ASSERT(false && "You are doing too much DONT_ALLOW allocations.");
			}
			return pReturn;
		}
	#endif
		return g_allocators.pAllocators[eHeap]->Alloc(eType, uSize, uAlign);
	}
}

void mem::Free(MemType eHeap, void* pData, bool bNotifyGPU)
{
#ifdef PLATFORM_PC
	if (usg::mem::s_bConventionalMemManagement)
	{
		_aligned_free(pData);
	}
	else
#endif
	{
		auto iPtrDiff = (uint8*)pData - s_nexHackBuffer;
		if (iPtrDiff < ARRAY_SIZE(s_nexHackBuffer))
		{
			return;
		}
		g_allocators.pAllocators[eHeap]->Free(pData);
	}
}

void* mem::ReAlloc(void* pData, uint32 uSize)
{
#ifdef PLATFORM_PC
	if (usg::mem::s_bConventionalMemManagement)
	{
		return realloc(pData, uSize);
	}
	else
#endif
	{
		return MemHeap::Reallocate(pData, uSize);
	}
}

void mem::Free(void* pData)
{
#ifdef PLATFORM_PC
	if (usg::mem::s_bConventionalMemManagement)
	{
		_aligned_free(pData);
	}
	else
#endif
	{
		auto iPtrDiff = (uint8*)pData - s_nexHackBuffer;
		if (iPtrDiff < ARRAY_SIZE(s_nexHackBuffer))
		{
			return;
		}
		MemHeap::Deallocate(pData);
	}
}

void mem::AddMemoryTag()
{
	mem::ps::AddMemoryTag();
	g_mainHeap->SetStatic(false);
}

void mem::FreeToLastTag()
{
	mem::ps::FreeToLastTag();
	g_mainHeap->SetStatic(true);
}


const char* mem::GetAllocString(MemAllocType eType)
{
	return gszAllocStrings[eType];
}

}

void* operator new(size_t size) NEW_THROW
{
	return usg::mem::Alloc(usg::MEMTYPE_STANDARD, usg::ALLOC_DONT_USE, size, 4);
}

void operator delete(void* ptr) NEW_THROW
{
	usg::mem::Free(ptr);

}

void* operator new[](size_t size)
{
#if defined (PLATFORM_PC) || defined(PLATFORM_SWITCH)
	//if (usg::mem::s_bConventionalMemManagement)
	{
		// Only for External programs(e.g. Ayataka, ...) use
		return usg::mem::Alloc(usg::MEMTYPE_STANDARD, usg::ALLOC_OBJECT, size, 4);
	}
#else
	ASSERT(false);	// Not allowing the global allocator
	return nullptr;
#endif
}

void operator delete[](void* ptr)
{
	usg::mem::Free(ptr);
}