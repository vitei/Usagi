#pragma once
#ifndef USAGI_STL_STRING_H
#define USAGI_STL_STRING_H

#include "Engine/Core/stl/usagi_eastl_allocator.h"
#include <EASTL/string.h>

namespace usg
{
	template <typename T, typename Allocator = USAGISTLAllocatorType>
	using basic_string = eastl::basic_string<T, Allocator>;

	using string   = usg::basic_string<char>;
	using wstring  = usg::basic_string<wchar_t>;
	using string8  = basic_string<char8_t>;
	using string16 = basic_string<char16_t>;
	using string32 = basic_string<char32_t>;
};

#endif // USAGI_STL_STRING_H