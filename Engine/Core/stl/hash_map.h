#pragma once
#ifndef USAGI_STL_HASH_MAP_H
#define USAGI_STL_HASH_MAP_H

#include "Engine/Core/stl/usagi_eastl_allocator.h"
#include <EASTL/hash_map.h>

namespace usg
{
	template <typename Key, typename T, typename Hash = eastl::hash<Key>, typename Predicate = eastl::equal_to<Key>,
		typename Allocator = USAGISTLAllocatorType, bool bCacheHashCode = false>
	using hash_map = eastl::hash_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>;
};

#endif // USAGI_STL_HASH_MAP_H