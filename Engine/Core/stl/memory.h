#pragma once
#ifndef USAGI_STL_MEMORY_H
#define USAGI_STL_MEMORY_H

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>

namespace usg
{
	using eastl::unique_ptr;
	using eastl::make_unique;
	using eastl::make_shared;
	using eastl::weak_ptr;
	using eastl::shared_ptr;
};

#endif // USAGI_STL_MEMORY_H