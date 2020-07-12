#pragma once
#ifndef USAGI_STL_STACK_H
#define USAGI_STL_STACK_H

#include "Engine/Core/stl/usagi_eastl_allocator.h"
#include <EASTL/stack.h>

namespace usg
{
	template<typename T, typename Allocator = USAGISTLAllocatorType>
	using stack = eastl::stack<T, Allocator>;
};

#endif // USAGI_STL_VECTOR_H