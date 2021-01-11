#pragma once
#ifndef USAGI_STL_STACK_H
#define USAGI_STL_STACK_H

#include "Engine/Core/stl/usagi_eastl_allocator.h"
#include "vector.h"
#include <EASTL/stack.h>

namespace usg
{
	template<typename T, typename Container = usg::vector<T>>
	using stack = eastl::stack<T, Container>;
};

#endif // USAGI_STL_STACK_H