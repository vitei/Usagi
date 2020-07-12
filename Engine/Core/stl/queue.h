#pragma once

#include "Engine/Core/stl/usagi_eastl_allocator.h"
#include <EASTL/queue.h>

namespace usg
{
	template<typename ValueType, typename Allocator = USAGISTLAllocatorType>
	using queue = eastl::queue<ValueType, eastl::deque<ValueType, Allocator, DEQUE_DEFAULT_SUBARRAY_SIZE(ValueType)> >;
};