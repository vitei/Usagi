/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_ENGINE_STDALLOCATOR_H_
#define _USG_ENGINE_STDALLOCATOR_H_
#include "Engine/Common/Common.h"
#include <memory>

namespace usg {

	// This template allows you to use STL containers inside Usagi

	template<typename T, uint32 ALLOC_TYPE>
	struct StdAllocator : public std::allocator<T> {
		inline typename std::allocator<T>::pointer allocate(typename std::allocator<T>::size_type n, typename std::allocator<void>::const_pointer = 0) {
			return reinterpret_cast<typename std::allocator<T>::pointer>(usg::mem::Alloc(MEMTYPE_STANDARD, (MemAllocType)ALLOC_TYPE, sizeof(T)*n));
		}

		inline void deallocate(typename std::allocator<T>::pointer p, typename std::allocator<T>::size_type n) {
			usg::mem::Free(MEMTYPE_STANDARD, p, false);
		}

		template<typename U>
		struct rebind {
			typedef StdAllocator<U, ALLOC_TYPE> other;
		};

		StdAllocator() {}

		template<class Other>
		StdAllocator(const StdAllocator<Other, ALLOC_TYPE>& _Right) {}
	};

}
#endif
