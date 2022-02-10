#pragma once
#ifndef USAGI_STL_MEMORY_H
#define USAGI_STL_MEMORY_H

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include "Engine/Core/stl/usagi_eastl_allocator.h"


namespace usg
{
	using eastl::unique_ptr;
	using eastl::make_unique;
	using eastl::weak_ptr;
	using eastl::shared_ptr;

	template <typename T, typename... Args>
	shared_ptr<T> make_shared(Args&&... args)
	{
		// allocate with the default allocator.
		return eastl::allocate_shared<T>(USAGISTLAllocatorType(EASTL_SHARED_PTR_DEFAULT_NAME), eastl::forward<Args>(args)...);
	}
};

#endif // USAGI_STL_MEMORY_H