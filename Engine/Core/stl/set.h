#pragma once
#ifndef USAGI_STL_SET_H
#define USAGI_STL_SET_H

#include "Engine/Core/stl/usagi_eastl_allocator.h"
#include <EASTL/set.h>
#include "Engine/Core/stl/utility.h"

namespace usg
{
	template <typename Key, typename Compare = eastl::less<Key>, typename Allocator = USAGISTLAllocatorType>
	using set = eastl::set<Key, Compare, Allocator>;
};

#endif // USAGI_STL_SET_H