/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
//	Description: Helper functions related to physics shapes
*****************************************************************************/

#pragma once


#include "Engine/Framework/Component.h"
#include "Engine/Maths/AABB.h"

namespace usg
{
	namespace Components
	{
		typedef struct _MeshCollider MeshCollider;
	}

	namespace physics
	{
		namespace details
		{
			AABB GetAABB(const RuntimeData<Components::MeshCollider>& meshColliderRtd);
		}

		template<typename T>
		AABB GetAABB(Required<Components::MeshCollider, T> meshCollider)
		{
			return details::GetAABB(meshCollider.GetRuntimeData());
		}

		template<typename T>
		AABB GetAABB(Optional<Components::MeshCollider, T> meshCollider)
		{
			ASSERT(meshCollider.Exists());
			return details::GetAABB(meshCollider.Force().GetRuntimeData());
		}
	}
}