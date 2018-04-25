#pragma once
#ifndef USAGI_STL_TYPE_TRAITS_H
#define USAGI_STL_TYPE_TRAITS_H

#include <EASTL/type_traits.h>

namespace usg
{
	using eastl::enable_if;
	using eastl::disable_if;
	using eastl::is_scalar;
}

#endif // USAGI_STL_TYPE_TRAITS_H