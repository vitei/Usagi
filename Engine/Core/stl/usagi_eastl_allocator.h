/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
****************************************************************************/
#pragma once

#ifndef USG_ENGINE_CORE_STL_USAGI_EASTL_ALLOCATOR_H
#define USG_ENGINE_CORE_STL_USAGI_EASTL_ALLOCATOR_H

#ifdef EASTL_USER_DEFINED_ALLOCATOR // USAGI implementation of the EASTL allocator
#include <EASTL/allocator.h>

	namespace eastl
	{
		template<usg::MemAllocType ALLOC_TYPE>
		class EASTL_API usagi_eastl_allocator
		{
		public:
			EASTL_ALLOCATOR_EXPLICIT usagi_eastl_allocator(const char* pName = EASTL_NAME_VAL(EASTL_ALLOCATOR_DEFAULT_NAME));
			usagi_eastl_allocator(const usagi_eastl_allocator& x);
			usagi_eastl_allocator(const usagi_eastl_allocator& x, const char* pName);

			usagi_eastl_allocator& operator=(const usagi_eastl_allocator& x);

			void* allocate(size_t n, int flags = 0);
			void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0);
			void  deallocate(void* p, size_t n);

			const char* get_name() const;
			void        set_name(const char* pName);

		protected:
#if EASTL_NAME_ENABLED
			const char* mpName; // Debug name, used to track memory.
#endif
		};

		template<usg::MemAllocType ALLOC_TYPE>
		bool operator==(const usagi_eastl_allocator<ALLOC_TYPE>& a, const usagi_eastl_allocator<ALLOC_TYPE>& b);

		template<usg::MemAllocType ALLOC_TYPE>
		bool operator!=(const usagi_eastl_allocator<ALLOC_TYPE>& a, const usagi_eastl_allocator<ALLOC_TYPE>& b);

		template<usg::MemAllocType ALLOC_TYPE>
		inline usagi_eastl_allocator<ALLOC_TYPE>::usagi_eastl_allocator(const char* EASTL_NAME(pName))
		{
			#if EASTL_NAME_ENABLED
				mpName = pName ? pName : EASTL_ALLOCATOR_DEFAULT_NAME;
			#endif
		}

		template<usg::MemAllocType ALLOC_TYPE>
		inline usagi_eastl_allocator<ALLOC_TYPE>::usagi_eastl_allocator(const usagi_eastl_allocator& EASTL_NAME(alloc))
		{
			#if EASTL_NAME_ENABLED
				mpName = alloc.mpName;
			#endif
		}

		template<usg::MemAllocType ALLOC_TYPE>
		inline usagi_eastl_allocator<ALLOC_TYPE>::usagi_eastl_allocator(const usagi_eastl_allocator&, const char* EASTL_NAME(pName))
		{
			#if EASTL_NAME_ENABLED
				mpName = pName ? pName : EASTL_ALLOCATOR_DEFAULT_NAME;
			#endif
		}

		template<usg::MemAllocType ALLOC_TYPE>
		inline usagi_eastl_allocator<ALLOC_TYPE>& usagi_eastl_allocator<ALLOC_TYPE>::operator=(const usagi_eastl_allocator& EASTL_NAME(alloc))
		{
			#if EASTL_NAME_ENABLED
				mpName = alloc.mpName;
			#endif
			return *this;
		}

		template<usg::MemAllocType ALLOC_TYPE>
		inline const char* usagi_eastl_allocator<ALLOC_TYPE>::get_name() const
		{
			#if EASTL_NAME_ENABLED
				return mpName;
			#else
				return EASTL_ALLOCATOR_DEFAULT_NAME;
			#endif
		}

		template<usg::MemAllocType ALLOC_TYPE>
		inline void usagi_eastl_allocator<ALLOC_TYPE>::set_name(const char* EASTL_NAME(pName))
		{
			#if EASTL_NAME_ENABLED
				mpName = pName;
			#endif
		}

		template<usg::MemAllocType ALLOC_TYPE>
		inline void* usagi_eastl_allocator<ALLOC_TYPE>::allocate(size_t n, int flags)
		{
			#if EASTL_NAME_ENABLED
				#define pName mpName
			#else
				#define pName EASTL_ALLOCATOR_DEFAULT_NAME
			#endif

			#if EASTL_DLL
				return allocate(n, EASTL_SYSTEM_ALLOCATOR_MIN_ALIGNMENT, 0, flags);
			#elif (EASTL_DEBUGPARAMS_LEVEL <= 0)
				// This level would uses alloc as-is
				return usg::mem::Alloc(usg::g_uDefaultMemType, ALLOC_TYPE, n, sizeof(void*) * 2);
			#elif (EASTL_DEBUGPARAMS_LEVEL == 1)
				// This level would use pName
				return usg::mem::Alloc(usg::g_uDefaultMemType, ALLOC_TYPE, n, sizeof(void*) * 2);
			#else
				// This level would use pName, __FILE__ and __LINE__
				return usg::mem::Alloc(usg::g_uDefaultMemType, ALLOC_TYPE, n, sizeof(void*) * 2);
			#endif
		}

		template<usg::MemAllocType ALLOC_TYPE>
		inline void* usagi_eastl_allocator<ALLOC_TYPE>::allocate(size_t n, size_t alignment, size_t offset, int flags)
		{
			#if EASTL_DLL
				// We currently have no support for implementing flags when 
				// using the C runtime library operator new function. The user 
				// can use SetDefaultAllocator to override the default allocator.
				EA_UNUSED(offset); EA_UNUSED(flags);

				size_t adjustedAlignment = (alignment > EA_PLATFORM_PTR_SIZE) ? alignment : EA_PLATFORM_PTR_SIZE;

				void* p = new char[n + adjustedAlignment + EA_PLATFORM_PTR_SIZE];
				void* pPlusPointerSize = (void*)((uintptr_t)p + EA_PLATFORM_PTR_SIZE);
				void* pAligned = (void*)(((uintptr_t)pPlusPointerSize + adjustedAlignment - 1) & ~(adjustedAlignment - 1));

				void** pStoredPtr = (void**)pAligned - 1;
				EASTL_ASSERT(pStoredPtr >= p);
				*(pStoredPtr) = p;

				EASTL_ASSERT(((size_t)pAligned & ~(alignment - 1)) == (size_t)pAligned);

				return pAligned;
			#elif (EASTL_DEBUGPARAMS_LEVEL <= 0)
				// This level would uses alloc as-is along with offset
				return usg::mem::Alloc(usg::g_uDefaultMemType, ALLOC_TYPE, n, static_cast<uint32>(alignment));
			#elif (EASTL_DEBUGPARAMS_LEVEL == 1)
				// This level would use pName and offset
				return usg::mem::Alloc(usg::g_uDefaultMemType, ALLOC_TYPE, n, static_cast<uint32>(alignment));
			#else
				// This level would use pName, offset, __FILE__ and __LINE__
				return usg::mem::Alloc(usg::g_uDefaultMemType, ALLOC_TYPE, n, static_cast<uint32>(alignment));
			#endif

			#undef pName  // See above for the definition of this.
		}

		template<usg::MemAllocType ALLOC_TYPE>
		inline void usagi_eastl_allocator<ALLOC_TYPE>::deallocate(void* p, size_t)
		{
			#if EASTL_DLL
				if (p != nullptr)
				{
					void* pOriginalAllocation = *((void**)p - 1);
					delete[](char*)pOriginalAllocation;
				}
			#else
				delete[](char*)p;
			#endif
		}

		template<usg::MemAllocType ALLOC_TYPE>
		inline bool operator==(const usagi_eastl_allocator<ALLOC_TYPE>&, const usagi_eastl_allocator<ALLOC_TYPE>&)
		{
			return true; // All allocators are considered equal, as they merely use global new/delete.
		}

		template<usg::MemAllocType ALLOC_TYPE>
		inline bool operator!=(const usagi_eastl_allocator<ALLOC_TYPE>&, const usagi_eastl_allocator<ALLOC_TYPE>&)
		{
			return false; // All allocators are considered equal, as they merely use global new/delete.
		}


	} // namespace eastl

	#ifndef USAGISTLAllocatorType
		#define USAGISTLAllocatorType eastl::usagi_eastl_allocator<ALLOC_STL_DEFAULT>
	#endif
#else
#ifndef USAGISTLAllocatorType
	#define USAGISTLAllocatorType eastl::allocator
#endif
#endif // EASTL_USER_DEFINED_ALLOCATOR

#endif // USG_ENGINE_CORE_STL_USAGI_EASTL_ALLOCATOR_H