#pragma once

#include "Engine/Common/Common.h"

namespace usg
{
	namespace details
	{
		uint32 GenerateSystemId();
	}

	template<typename S>
	uint32 GetSystemId()
	{
		static const uint32 uSystemID = details::GenerateSystemId();
		return uSystemID;
	}
}
