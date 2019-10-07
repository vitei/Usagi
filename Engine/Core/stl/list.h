#pragma once
#ifndef USAGI_STL_LIST_H
#define USAGI_STL_LIST_H

#include "Engine/Core/stl/usagi_eastl_allocator.h"
#include <EASTL/list.h>

namespace usg
{
	template<typename T, typename Allocator = USAGISTLAllocatorType>
	using list = eastl::list<T, Allocator>;
};

#endif // USAGI_STL_VECTOR_H