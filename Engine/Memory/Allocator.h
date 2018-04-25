/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_ENGINE_MEMORY_ALLOCATOR_H
#define USG_ENGINE_MEMORY_ALLOCATOR_H
#include "Engine/Common/Common.h"

namespace usg{

	// When updating these values be sure to update the string (in Mem.cpp)

	enum MemAllocType
	{
		ALLOC_OBJECT = 0,
		ALLOC_SHADER_CONSTANTS,
		ALLOC_COLLISION,
		ALLOC_POOL,
		ALLOC_LIST,
		ALLOC_FASTPOOL,
		ALLOC_GEOMETRY_DATA,
		ALLOC_INDEX_DATA,
		ALLOC_GFX_INTERNAL,
		ALLOC_GFX_SHADER,
		ALLOC_GFX_RENDER_TARGET,
		ALLOC_GFX_TEXTURE,
		ALLOC_GFX_CALLBACK,
		ALLOC_GFX_COMMAND_BUFFER,
		ALLOC_AUDIO,
		ALLOC_DEBUG,
		ALLOC_LOADING,
		ALLOC_PROTOCOL_MESSAGE,
		ALLOC_EXTERNAL_ALLOCATION,
		ALLOC_COMPONENT,
		ALLOC_ENTITY,
		ALLOC_SYSTEM,
		ALLOC_LAYOUT,
		ALLOC_SCRIPT,
		ALLOC_NETWORK,
		ALLOC_SHOP,
		ALLOC_SAVEDATA,
		ALLOC_SCREENSHOT,
		ALLOC_STRING,
		ALLOC_CONTAINER,
		ALLOC_FONT,
		ALLOC_RESOURCE_MGR,
		ALLOC_STRING_POINTER_HASH,
		ALLOC_BEHAVIOR_TREE,
		ALLOC_EVENT,
		ALLOC_PARTICLES,
		ALLOC_ANIMATION,
		ALLOC_CAMERA,
		ALLOC_MODE,
		ALLOC_PHYSICS,
		ALLOC_STL_DEFAULT,
		ALLOC_DONT_USE,

		// The number of types
		ALLOC_TYPE_COUNT	
	};
}

namespace usg
{

	class DoubleStack;
	class MemHeap;

	class Allocator
	{
	public:
		Allocator() {}
		~Allocator() {};

		virtual void* Alloc(MemAllocType eType, memsize uSize, uint32 uAlign) = 0;
		virtual void Free(void* pMem) = 0;

	private:

	};

	class HeapAllocator : public Allocator
	{
	public:
		HeapAllocator();
		~HeapAllocator();

		void Init(MemHeap* pHeap, uint32 uGroup);
		void SetGroup(uint32 uGroup);

		uint32 GetGroup() { return m_uGroup; }
		void FreeGroup();

		virtual void* Alloc(MemAllocType eType, memsize uSize, uint32 uAlign=4);
		virtual void Free(void* pMem);

	protected:
		MemHeap*	m_pHeap;
		uint32		m_uGroup;
	};

	class StackAllocator : public Allocator
	{
	public:
		StackAllocator();
		~StackAllocator();

		void Init(DoubleStack* pStack, bool bFront);
		virtual void* Alloc(MemAllocType eType, memsize uSize, uint32 uAlign=4);
		virtual void Free(void* pMem);

	protected:
		DoubleStack* m_pStack;
		bool m_bFront;
	};

}

#endif // USG_ENGINE_MEMORY_ALLOCATOR_H  
