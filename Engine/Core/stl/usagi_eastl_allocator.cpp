#include "Engine/Common/Common.h"

#ifdef EASTL_USER_DEFINED_ALLOCATOR //  USAGI implementation of the EASTL allocator
#include <EASTL/internal/config.h>
#include <EASTL/allocator.h>

namespace eastl
{
	/// gDefaultAllocator
	/// Default global allocator instance. 
	EASTL_API allocator   gDefaultAllocator;
	EASTL_API allocator* gpDefaultAllocator = &gDefaultAllocator;

	EASTL_API allocator* GetDefaultAllocator()
	{
		return gpDefaultAllocator;
	}

	EASTL_API allocator* SetDefaultAllocator(allocator* pAllocator)
	{
		allocator* const pPrevAllocator = gpDefaultAllocator;
		gpDefaultAllocator = pAllocator;
		return pPrevAllocator;
	}
} // namespace eastl

#endif // EASTL_USER_DEFINED_ALLOCATOR