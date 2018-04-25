#include "Engine/Common/Common.h"
#include "Engine/Physics/PhysXVehicle/FrictionPairs.h"
#include "Engine/Physics/PhysicsComponents.pb.h"
#include "Engine/Physics/PhysicsSceneData.h"

namespace usg
{
	// TODO: create a VPB file which can give friction multipliers for each material flag

	static physx::PxF32 gTireFrictionMultipliers[MAX_NUM_SURFACE_TYPES][MAX_NUM_TIRE_TYPES] =
	{
		//NORMAL,	WORN
		{ 1.0f,		0.1f }//TARMAC
	};

	physx::PxVehicleDrivableSurfaceToTireFrictionPairs* CreateFrictionPairs(Required<usg::Components::PhysicsScene> scene)
	{
		physx::PxPhysics* pPhysics = scene.GetRuntimeData().GetData().pPhysics;
		physx::PxMaterial* pDefaultMaterial = pPhysics->createMaterial(1.00f, 3.0f, 0.1f);

		physx::PxVehicleDrivableSurfaceType surfaceTypes[1];
		surfaceTypes[0].mType = SURFACE_TYPE_TARMAC;

		const physx::PxMaterial* surfaceMaterials[1];
		surfaceMaterials[0] = pDefaultMaterial;

		physx::PxVehicleDrivableSurfaceToTireFrictionPairs* surfaceTirePairs = physx::PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(MAX_NUM_TIRE_TYPES, MAX_NUM_SURFACE_TYPES);

		surfaceTirePairs->setup(MAX_NUM_TIRE_TYPES, MAX_NUM_SURFACE_TYPES, surfaceMaterials, surfaceTypes);

		for (physx::PxU32 i = 0; i < MAX_NUM_SURFACE_TYPES; i++)
		{
			for (physx::PxU32 j = 0; j < MAX_NUM_TIRE_TYPES; j++)
			{
				surfaceTirePairs->setTypePairFriction(i, j, gTireFrictionMultipliers[i][j]);
			}
		}
		return surfaceTirePairs;
	}
}
