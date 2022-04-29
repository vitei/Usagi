/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/

#pragma once

#include "Engine/Core/stl/set.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Core/stl/map.h"
#include "Engine/Maths/MovingAverage.h"
#include "Engine/Physics/PhysicsStructs.pb.h"
#include "Engine/Physics/PhysX.h"
#include "Engine/Physics/RaycastHitBase.h"
#include "Engine/Physics/Debug/PhysicsDebugStats.h"

namespace usg
{
	namespace Components
	{
		typedef struct _PhysicsScene PhysicsScene;
	}

	class PhysXMeshCache;
	class ContactReportCallback;
	class VehicleSceneQueryData;
	struct ActorRuntimeData;
	struct PhysXShapeRuntimeData;

	namespace physics
	{
		struct PhysicsSceneData
		{
			uint32 uTick;

			physx::PxFoundation* pPhysicsFoundation;
			physx::PxPhysics* pPhysics;
			physx::PxCooking* pCooking;
			physx::PxScene* pScene;
			PhysXMeshCache* pMeshCache;

			volatile bool bSimulationRunning;

			set<ActorRuntimeData*> addActorList;
			set<ActorRuntimeData*> dirtyActorList;
			set<PhysXShapeRuntimeData*> dirtyShapeList;
			set<physx::PxActor*> removeActorList;
			set<physx::PxAggregate*> addAggregateList;
			set<physx::PxAggregate*> removeAggregateList;

			PhysicsDebugStats debugStats;

			struct
			{
				physx::PxBatchQuery* pRaycastBatchQuery;
				vector<pair<uint32, pair<uint32, Entity>>> pendingRequests;
				struct
				{
					vector<RaycastHitBase> hits;
					vector<Entity> entities;
					vector<uint8> workBuffer;
				} workData;
			} raycastData;

			struct
			{
				VehicleSceneQueryData* pVehicleSceneQueryData;
				physx::PxBatchQuery* pVehicleBatchQuery;
				physx::PxVehicleDrivableSurfaceToTireFrictionPairs* pFrictionPairs;
				vector<physx::PxVehicleWheels*> vehicleList;

				vector<physx::PxWheelQueryResult> wheelQueryResults;
				vector<physx::PxVehicleWheelQueryResult> vehicleWheelQueryResults;
			} vehicleData;

			struct CollisionData
			{
				uint32 uTick;
				uint32 uNotifyFirstTick;
				uint32 uNotifySecondTick;
				bool bFirstContact;
				uint32 uMaterialFlags;

				struct Contact
				{
					static constexpr uint32 NotifyFirst = 1;
					static constexpr uint32 NotifySecond = 2;
					Vector3f vNormal;
					Vector3f vIntersectionPoint;
					Vector3f vImpulse;
					float fDepth;
					uint32 uMaterialFlags[2];
					uint32 uBitmask;
				};
				vector<Contact> contacts;
			};

			map<pair<Entity, Entity>, CollisionData> collisions;

			struct TriggerData
			{
				Entity triggerEntity;
				Entity triggererEntity;
				bool bEntered;
			};
			vector<TriggerData> triggerSignals;

			struct SharedMaterial
			{
				physx::PxMaterial* pPhysXMaterial = 0;
				PhysicMaterial material;
			};
			map<uint32, SharedMaterial> sharedMaterials;

#ifndef FINAL_BUILD
			struct
			{
				bool bDebugRenderOnNextFrame = false;
				Vector3f vDebugRenderCenter;

				// Profiling data
				MovingAverage<float, 16> fFetchResultsTime;
			} diagnostics;
#endif
		};

		// Registers a shared material. Returns hash of the material.
		uint32 RegisterSharedMaterial(Required<Components::PhysicsScene> scene, const PhysicMaterial& m);

		// Get pointer to a shared material using its hash value.
		PhysicsSceneData::SharedMaterial* GetSharedMaterial(Required<Components::PhysicsScene> scene, uint32 uMaterialHash);
	}
}
