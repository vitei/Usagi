#pragma once

#include "Engine/Common/Common.h"
#include "Engine/Physics/PhysX.h"

namespace usg
{
	enum
	{
		SURFACE_TYPE_TARMAC,
		MAX_NUM_SURFACE_TYPES
	};

	enum
	{
		TIRE_TYPE_NORMAL = 0,
		TIRE_TYPE_WORN,
		MAX_NUM_TIRE_TYPES
	};

	physx::PxVehicleDrivableSurfaceToTireFrictionPairs* CreateFrictionPairs(Required<usg::Components::PhysicsScene> scene);
}
