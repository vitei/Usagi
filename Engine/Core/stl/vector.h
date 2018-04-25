#pragma once
#ifndef USAGI_STL_VECTOR_H
#define USAGI_STL_VECTOR_H

#include "Engine/Core/stl/usagi_eastl_allocator.h"
#include <EASTL/vector.h>

namespace usg
{
	template<typename T, typename Allocator = USAGISTLAllocatorType>
	using vector = eastl::vector<T, Allocator>;
};

#endif // USAGI_STL_VECTOR_H