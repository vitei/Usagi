#pragma once

#include "Engine/Common/Common.h"
#include "Engine/Framework/SystemCoordinator.h"

namespace usg
{
	class EntityLoader;

	namespace Components
	{
		struct EntityLoaderHandle
		{
			EntityLoaderHandle() : pHandle(nullptr) {}
			EntityLoader& operator*() { return *pHandle; }
			EntityLoader* operator->() { return pHandle; }

			EntityLoader* pHandle;
		};
	}

	template<>
	inline constexpr char * NamePB<EntityLoaderHandle>()
	{
		return "EntityLoaderHandle";
	}

}