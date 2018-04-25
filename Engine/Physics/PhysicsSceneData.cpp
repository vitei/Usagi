/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Framework/Component.h"
#include "Engine/Physics/PhysicsSceneData.h"
#include "Engine/Physics/PhysicsComponents.pb.h"

namespace usg
{
	namespace physics
	{
		uint32 RegisterSharedMaterial(Required<Components::PhysicsScene> scene, const PhysicMaterial& m)
		{
			auto& data = scene.GetRuntimeData().GetData();
			const uint32 uHash = details::GetMaterialHash(m);
			if (!data.sharedMaterials.count(uHash))
			{
				auto& sharedMaterial = data.sharedMaterials[uHash];
				sharedMaterial.material = m;
				sharedMaterial.pPhysXMaterial = data.pPhysics->createMaterial(0, 0, 0);
				ApplyMaterial(sharedMaterial.pPhysXMaterial, sharedMaterial.material);
			}
			return uHash;
		}

		PhysicsSceneData::SharedMaterial* GetSharedMaterial(Required<Components::PhysicsScene> scene, uint32 uMaterialHash)
		{
			auto& data = scene.GetRuntimeData().GetData();
			if (data.sharedMaterials.count(uMaterialHash))
			{
				return &data.sharedMaterials[uMaterialHash];
			}
			return nullptr;
		}
	}
}
