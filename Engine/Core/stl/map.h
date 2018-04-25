#pragma once
#ifndef USAGI_STL_MAP_H
#define USAGI_STL_MAP_H

#include "Engine/Core/stl/usagi_eastl_allocator.h"
#include <EASTL/map.h>

namespace usg
{
	template <typename Key, typename T, typename Compare = eastl::less<Key>, typename Allocator = USAGISTLAllocatorType>
	using map = eastl::map<Key, T, Compare, Allocator>;
};

#endif // USAGI_STL_MAP_H