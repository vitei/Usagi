/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
//	Description: Helper functions related to physics shapes
*****************************************************************************/

#include "Engine/Common/Common.h"
#include "Engine/Physics/PhysicsShapes.h"
#include "Engine/Physics/PhysicsComponents.h"

namespace usg
{
	namespace physics
	{
		namespace details
		{
			AABB GetAABB(const RuntimeData<Components::MeshCollider>& meshColliderRtd)
			{
				ASSERT(meshColliderRtd.pConvexMesh != nullptr || meshColliderRtd.pTriangleMesh != nullptr);
				AABB r;
				const auto physxAABB = meshColliderRtd.pConvexMesh ? meshColliderRtd.pConvexMesh->getLocalBounds() : meshColliderRtd.pTriangleMesh->getLocalBounds();
				r.SetMinMax(ToUsgVec3(physxAABB.minimum), ToUsgVec3(physxAABB.maximum));
				return r;
			}
		}
	}
}