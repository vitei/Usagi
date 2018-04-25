#pragma once

#include "Engine/Core/stl/usagi_eastl_allocator.h"
#include <EASTL/deque.h>

namespace usg
{
	template<typename ValueType, typename Allocator = USAGISTLAllocatorType>
	using deque = eastl::deque<ValueType, Allocator>;
};