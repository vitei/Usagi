#include "Engine/Common/Common.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Physics/PhysXVehicle/VehicleSceneQueryData.h"
#include "VehicleCollider.h"
#include "Engine/Physics/PhysXVehicle/FrictionPairs.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Framework/ComponentLoadHandles.h"
#include <algorithm>
#include "Engine/Physics/PhysicsSceneData.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"
#include "Engine/Physics/CollisionData.pb.h"
#include "Engine/Physics/CollisionMask.pb.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Framework/TransformTool.h"
#include "Engine/Physics/PhysXMeshCache.h"
#include "Engine/Resource/CollisionModelResource.h"
#include "Engine/Core/stl/array.h"
#include "Engine/Core/stl/string.h"
#include "Engine/Scene/Model/ModelEvents.pb.h"

namespace usg
{
	struct WheelHelper
	{
		bool bImmovable;
		Vector3f vOffsetFromHub;

		WheelHelper()
		{
			bImmovable = false;
			vOffsetFromHub = V3F_ZERO;
		}
	};

	static physx::PxConvexMesh* CreateConvexMesh(const physx::PxVec3* verts, const physx::PxU32 numVerts, physx::PxPhysics& physics, physx::PxCooking& cooking)
	{
		// Create descriptor for convex mesh
		physx::PxConvexMeshDesc convexDesc;
		convexDesc.points.count = numVerts;
		convexDesc.points.stride = sizeof(physx::PxVec3);
		convexDesc.points.data = verts;
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		physx::PxConvexMesh* convexMesh = NULL;
		physx::PxDefaultMemoryOutputStream buf;
		if (cooking.cookConvexMesh(convexDesc, buf))
		{
			physx::PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
			convexMesh = physics.createConvexMesh(id);
		}

		return convexMesh;
	}

	static usg::pair<physx::PxConvexMesh*, physx::PxTransform> CreateChassisMesh(Required<VehicleCollider> vehicle, ComponentLoadHandles& handles, const physx::PxVec3 dims, physx::PxPhysics& physics, physx::PxCooking& cooking)
	{
		usg::pair<physx::PxConvexMesh*, physx::PxTransform> r{nullptr, physx::PxTransform(physx::PxIdentity)};
		if (vehicle->szCollisionModel[0] != 0)
		{
			Entity bodyBone = vehicle.GetEntity()->GetChildEntityByName(handles, "locatorBody");
			ASSERT(bodyBone != nullptr);
			auto& sceneRtd = *handles.pPhysicsScene;
			r.first = sceneRtd.pMeshCache->GetConvexMesh(handles, vehicle->szCollisionModel,"locatorBody");
			r.second = ToPhysXTransform(TransformTool::GetRelativeTransform(vehicle.GetEntity(), bodyBone, handles));
		}
		else
		{
			// Fallback solution is a box
			const float32 fYOffset = vehicle->fWheelRadius*2.0f + vehicle->vChassisExtents.y*0.5f;
			const physx::PxF32 x = dims.x*0.5f;
			const physx::PxF32 y = dims.y*0.5f;
			const physx::PxF32 z = dims.z*0.5f;
			const physx::PxVec3 vVertices[] =
			{
				physx::PxVec3(x,y + fYOffset,-z),
				physx::PxVec3(x,-y + fYOffset,-z),
				physx::PxVec3(-x,y + fYOffset,-z),
				physx::PxVec3(-x,-y + fYOffset,-z),
				physx::PxVec3(x,y + fYOffset,z*1.0f),
				physx::PxVec3(-x,y + fYOffset,z*1.0f),
				physx::PxVec3(-x,-y + fYOffset,z*1.0f),
				physx::PxVec3(x,-y + fYOffset,z*1.0f)
			};
			r.first = CreateConvexMesh(vVertices, ARRAY_SIZE(vVertices), physics, cooking);
		}
		return r;
	}

	static physx::PxMaterial* CreateMaterial(const PhysicMaterial& m, physx::PxPhysics* pPhysics)
	{
		auto pMat = pPhysics->createMaterial(m.fStaticFriction, m.fDynamicFriction, m.fBounciness);
		ApplyMaterial(pMat, m);
		return pMat;
	}

	static physx::PxRigidDynamic* CreateVehicleActor(Required<VehicleCollider> vehicle, ComponentLoadHandles& handles, const physx::PxVehicleChassisData& chassisData, physx::PxMaterial** wheelMaterials, physx::PxConvexMesh** wheelConvexMeshes, physx::PxMaterial** chassisMaterials, physx::PxConvexMesh* pChassisMesh, physx::PxTransform chassisTransform, const physx::PxU32 uNumChassisMeshes, physx::PxPhysics& physics)
	{
		ASSERT(uNumChassisMeshes == 1);
		Entity e = vehicle.GetEntity();

		Required<TransformComponent> trans;
		handles.GetComponent(e, trans);
		ASSERT(trans.IsValid());

		Required<CollisionMasks> masks;
		handles.GetComponent(e, masks);
		
		Required<RigidBody> rigidBody;
		handles.GetComponent(e, rigidBody);
		ASSERT(rigidBody->bDynamic);
		ASSERT(!rigidBody->bKinematic);
		OnLoaded(*GameComponents<RigidBody>::GetComponent(e), handles, false);

		auto& vehicleRtd = vehicle.GetRuntimeData();
		const uint32 uNumWheels = vehicle->uNumWheels;

		physx::PxRigidDynamic* pRigidBody = rigidBody.GetRuntimeData().pRigidActor->is<physx::PxRigidDynamic>();

		//Wheel and chassis simulation filter data.
		physx::PxFilterData wheelSimFilterData;
		wheelSimFilterData.word0 = COLLM_WHEEL;
		wheelSimFilterData.word1 = masks->uFilter | COLLM_VEHICLE;
		wheelSimFilterData.word3 = 0;

		//Wheel and chassis query filter data.
		//Optional: cars don't drive on other cars.
		physx::PxFilterData wheelQryFilterData;

		//Add all the wheel shapes to the actor.
		for (physx::PxU32 i = 0; i < uNumWheels; i++)
		{
			physx::PxConvexMeshGeometry geom(wheelConvexMeshes[i]);
			physx::PxShape* pWheelShape = pRigidBody->createShape(geom, *wheelMaterials[i]);
			pWheelShape->setQueryFilterData(wheelQryFilterData);
			pWheelShape->setSimulationFilterData(wheelSimFilterData);
			pWheelShape->setLocalPose(physx::PxTransform(physx::PxIdentity));
			physics::details::SetUserData(pWheelShape, &vehicleRtd);
#ifdef DEBUG_BUILD
			static const char* szWheelNames[] = { "Wheel0",	"Wheel1", "Wheel2", "Wheel3", "Wheel4", "Wheel5", "Wheel6", "Wheel7", "Wheel8", "Wheel9", "Wheel10", "Wheel11", "Wheel12",
				"Wheel13", "Wheel14", "Wheel15", "Wheel16", "Wheel17", "Wheel18", "Wheel19" };
			pWheelShape->setName(szWheelNames[i]);
#endif
			// Setup runtime date for the wheels so that they work with the Usagi collision system
			auto& rtd = vehicleRtd.wheelsData.wheel[i].rtd;
			rtd.pShape = pWheelShape;
			rtd.entity = vehicleRtd.wheelsData.wheel[i].bone != nullptr ? vehicleRtd.wheelsData.wheel[i].bone : vehicle.GetEntity();
			if (vehicleRtd.wheelsData.wheel[i].bone != nullptr)
			{
				auto pMasks = GameComponents<CollisionMasks>::Create(rtd.entity);
				pMasks->uGroup = wheelSimFilterData.word0;
				pMasks->uFilter = wheelSimFilterData.word1;
			}
			rtd.shapeAggregateEntity = vehicle.GetEntity();
			physx::PxMaterial* pMaterials[1];
			pWheelShape->getMaterials(pMaterials, 1);
			rtd.pMaterial = pMaterials[0];
		}

		//Add the chassis shapes to the actor.
		for (physx::PxU32 i = 0; i < uNumChassisMeshes; i++)
		{
			physx::PxShape* pChassisShape = pRigidBody->createShape(physx::PxConvexMeshGeometry(pChassisMesh), *chassisMaterials[i]);
			pChassisShape->setLocalPose(chassisTransform);
			pChassisShape->userData = &vehicleRtd;
			vehicleRtd.pShape = pChassisShape;
			vehicleRtd.pChassisShape = pChassisShape;
		}

		pRigidBody->setMass(chassisData.mMass);
		pRigidBody->setMassSpaceInertiaTensor(chassisData.mMOI);
		pRigidBody->setCMassLocalPose(physx::PxTransform(chassisData.mCMOffset, physx::PxQuat(physx::PxIdentity)));

		return pRigidBody;
	}

	static void ComputeWheelCenterActorOffsets(const physx::PxF32 wheelFrontZ, const physx::PxF32 wheelRearZ, const physx::PxVec3& chassisDims, const physx::PxF32 wheelWidth, const physx::PxF32 wheelRadius, const physx::PxU32 uNumWheels, physx::PxVec3* wheelCentreOffsets)
	{
		const physx::PxF32 uNumLeftWheels = uNumWheels / 2.0f;
		const physx::PxF32 deltaZ = (wheelFrontZ - wheelRearZ) / (uNumLeftWheels - 1.0f);
		for (physx::PxU32 i = 0; i < uNumWheels; i += 2)
		{
			wheelCentreOffsets[i + 0] = physx::PxVec3((-chassisDims.x + wheelWidth)*0.5f, wheelRadius, wheelRearZ + i*deltaZ*0.5f);
			wheelCentreOffsets[i + 1] = physx::PxVec3((+chassisDims.x - wheelWidth)*0.5f, wheelRadius, wheelRearZ + i*deltaZ*0.5f);
		}
	}

	static void SetupWheelsSimulationData(Required<VehicleCollider> vehicle,
		const physx::PxF32 wheelMOI,
		const usg::array<float, PhysicsConstants::VehicleMaxNumWheels>& wheelRadii,
		const usg::array<float, PhysicsConstants::VehicleMaxNumWheels>& wheelWidths,
		const usg::array<WheelHelper, PhysicsConstants::VehicleMaxNumWheels>& wheelHelpers,
		const physx::PxU32 numWheels,
		const physx::PxVec3* wheelCenterActorOffsets,
		const physx::PxVec3& chassisCMOffset,
		const physx::PxF32 chassisMass,
		physx::PxVehicleWheelsSimData* wheelsSimData)
	{
		//Set up the wheels.
		physx::PxVehicleWheelData wheels[PX_MAX_NB_WHEELS];
		{
			//Set up the wheel data structures with mass, moi, radius, width.
			//Increase the damping on the wheel.

			float fTotalWheelMass = vehicle->fWheelMass*numWheels;
			float fRadiusSum = 0;
			for (physx::PxU32 i = 0; i < numWheels; i++)
			{
				fRadiusSum += wheelRadii[i];
			}

			for (physx::PxU32 i = 0; i < numWheels; i++)
			{
				wheels[i].mMass = fTotalWheelMass * wheelRadii[i]/numWheels;
				wheels[i].mMOI = wheelMOI;
				wheels[i].mRadius = wheelRadii[i];
				wheels[i].mWidth = wheelWidths[i];
				wheels[i].mDampingRate = 1000.0f;
				wheels[i].mMaxHandBrakeTorque = 0;
				wheels[i].mMaxBrakeTorque = 0;
			}
		}

		//Set up the tires.
		physx::PxVehicleTireData tires[PX_MAX_NB_WHEELS];
		{
			//Set all tire types to "normal" type.
			for (physx::PxU32 i = 0; i < numWheels; i++)
			{
				tires[i].mType = TIRE_TYPE_NORMAL;
			}
		}

		//Set up the suspensions
		physx::PxVehicleSuspensionData suspensions[PX_MAX_NB_WHEELS];
		{
			//Compute the mass supported by each suspension spring.
			physx::PxF32 suspSprungMasses[PX_MAX_NB_WHEELS];
			PxVehicleComputeSprungMasses(numWheels, wheelCenterActorOffsets, chassisCMOffset, chassisMass, 1, suspSprungMasses);

			//Set the suspension data.
			for (physx::PxU32 i = 0; i < numWheels; i++)
			{
				suspensions[i].mMaxCompression = !wheelHelpers[i].bImmovable ? vehicle->fSpringMaxCompression : 0.0f;
				suspensions[i].mMaxDroop = !wheelHelpers[i].bImmovable ? vehicle->fSpringMaxDroop : 0.005f;
				suspensions[i].mSpringStrength = !wheelHelpers[i].bImmovable ? vehicle->fSpringStrength : 500000.0f;
				suspensions[i].mSpringDamperRate = vehicle->fSpringDamperRate;
				suspensions[i].mSprungMass = suspSprungMasses[i];
			}
		}

		//Set up the wheel geometry.
		physx::PxVec3 suspTravelDirections[PX_MAX_NB_WHEELS];
		physx::PxVec3 wheelCentreCMOffsets[PX_MAX_NB_WHEELS];
		physx::PxVec3 suspForceAppCMOffsets[PX_MAX_NB_WHEELS];
		physx::PxVec3 tireForceAppCMOffsets[PX_MAX_NB_WHEELS];
		{
			for (physx::PxU32 i = 0; i < numWheels; i++)
			{
				//Vertical suspension travel.
				suspTravelDirections[i] = physx::PxVec3(0, -1, 0);

				//Wheel center offset is offset from rigid body center of mass.
				wheelCentreCMOffsets[i] = wheelCenterActorOffsets[i] - chassisCMOffset;

				//Suspension force application point 0.3 metres below rigid body center of mass.
				suspForceAppCMOffsets[i] = physx::PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);

				//Tire force application point 0.3 metres below rigid body center of mass.
				tireForceAppCMOffsets[i] = physx::PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);
			}
		}

		//Set up the filter data of the raycast that will be issued by each suspension.
		physx::PxFilterData queryFilterData;

		//setupNonDrivableSurface(qryFilterData);

		//Set the wheel, tire and suspension data.
		//Set the geometry data.
		//Set the query filter data
		for (physx::PxU32 i = 0; i < numWheels; i++)
		{
			wheelsSimData->setWheelData(i, wheels[i]);
			wheelsSimData->setTireData(i, tires[i]);
			wheelsSimData->setSuspensionData(i, suspensions[i]);
			wheelsSimData->setSuspTravelDirection(i, suspTravelDirections[i]);
			wheelsSimData->setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
			wheelsSimData->setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
			wheelsSimData->setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
			wheelsSimData->setSceneQueryFilterData(i, queryFilterData);
			wheelsSimData->setWheelShapeMapping(i, i);
		}
	}

	// Inspect wheel bones and return number of wheels, their positions and radii.
	static uint32 FindWheels(Required<VehicleCollider> vehicle, usg::array<float, PhysicsConstants::VehicleMaxNumWheels>& wheelWidths, usg::array<float, PhysicsConstants::VehicleMaxNumWheels>& wheelRadii, usg::array<Vector3f, PhysicsConstants::VehicleMaxNumWheels>& wheelPositions, usg::array<WheelHelper, PhysicsConstants::VehicleMaxNumWheels>& wheelHelpers, ComponentLoadHandles& handles)
	{
		ASSERT(vehicle->szCollisionModel[0] != 0);
		CollisionModelResHndl collisionModelHandle = handles.pResourceMgr->GetCollisionModel(vehicle->szCollisionModel);

		auto getWheelDataFromBone = [](CollisionModelResHndl& res, const char* szBoneName, float& fWheelRadius, float& fWheelWidth) {
			const auto boneData = res->GetBoneData(utl::CRC32(szBoneName));
			for (const auto& bd : boneData)
			{
				fWheelRadius = std::max(bd.aabb.GetRadii().y, bd.aabb.GetRadii().z);
				fWheelWidth = bd.aabb.GetRadii().x;
				return;
			}
			ASSERT(false);
			return;
		};

		uint32 uWheelsFound = 0;
		auto wheelFinder = [&](Entity e, ComponentLoadHandles& handles)
		{
			Optional<Identifier> id;
			handles.GetComponent(e, id);
			if (id.Exists())
			{
				const bool bHub = str::CompareLen(id.Force()->name, "locatorHub", 10);
				const bool bImmovableHub = str::CompareLen(id.Force()->name, "locatorXHub", 11);
				if (bHub || bImmovableHub)
				{
					auto pWheelHub = GameComponents<VehicleWheel>::Create(e);
					pWheelHub->uIndex = uWheelsFound;
					pWheelHub->bIsHub = true;

					const auto wheelTrans = TransformTool::GetRelativeTransform(vehicle.GetEntity(), e, handles);
					wheelPositions[uWheelsFound] = wheelTrans.position;

					float fWheelRadius = 0;
					float fWheelWidth = 0;

					Entity bone = nullptr;
					auto wheelProcessor = [&](Entity wheelEntity, ComponentLoadHandles& handles)
					{
						Optional<Identifier> wheelId;
						handles.GetComponent(wheelEntity, wheelId);
						if (wheelEntity == e)
						{
							return;
						}
						if (id.Exists() && str::Find(wheelId.Force()->name,"Wheel") != nullptr)
						{
							getWheelDataFromBone(collisionModelHandle, wheelId.Force()->name, fWheelRadius, fWheelWidth);
							auto pWheelHub = GameComponents<VehicleWheel>::Create(wheelEntity);
							pWheelHub->uIndex = uWheelsFound;
							pWheelHub->bIsHub = false;
							bone = wheelEntity;
							const auto wheelTransFromHub = TransformTool::GetRelativeTransform(e, wheelEntity, handles);
							wheelHelpers[uWheelsFound].vOffsetFromHub = wheelTransFromHub.position;
						}
					};

					e->ProcessEntityRecursively(wheelProcessor, handles);
					if (!(fWheelWidth > 0 && fWheelRadius > 0))
					{
						// Get wheel extents from the hub instead
						getWheelDataFromBone(collisionModelHandle, id.Force()->name, fWheelRadius, fWheelWidth);
					}
					if (bone == nullptr)
					{
						bone = e;
					}
					ASSERT(fWheelWidth > 0 && fWheelRadius > 0);
					wheelRadii[uWheelsFound] = fWheelRadius;
					wheelWidths[uWheelsFound] = fWheelWidth;

					wheelHelpers[uWheelsFound].bImmovable = bImmovableHub;

					vehicle.GetRuntimeData().wheelsData.wheel[uWheelsFound].fRadius = fWheelRadius;
					vehicle.GetRuntimeData().wheelsData.wheel[uWheelsFound].vBaseOffset = wheelPositions[uWheelsFound];
					vehicle.GetRuntimeData().wheelsData.wheel[uWheelsFound].bone = bone;
					uWheelsFound++;
				}
			}
		};
		vehicle.GetEntity()->ProcessEntityRecursively(wheelFinder, handles);

		// Find the middle wheels, increase radii of all wheels by a value which makes the middle wheels touch ground
		// Also find the number of wheels touching ground in the default pose (when applying thrust force to the vehicle, the force is based on number of wheels touching the ground and we use
		// the current number of wheels touching the ground divided by this value to scale the thrust value).
		uint32 wheelsZOrdered[PhysicsConstants::VehicleMaxNumWheels];
		for (uint32 i = 0; i < uWheelsFound; i++) {
			wheelsZOrdered[i] = i;
		}
		std::sort(wheelsZOrdered, wheelsZOrdered + uWheelsFound, [wheelPositions](uint32 a, uint32 b) {
			return wheelPositions[a].z > wheelPositions[b].z;
		});
		const uint32 uMiddleIndex = wheelsZOrdered[uWheelsFound / 2];
		const float fMiddleY = wheelPositions[uMiddleIndex].y;
		const float fMiddleRadius = wheelRadii[uMiddleIndex];
		const float fMiddleWheelAboveGround = std::max(0.0f, fMiddleY - fMiddleRadius);

		auto& vehicleRtd = vehicle.GetRuntimeData();
		vehicleRtd.wheelsData.uNumWheelsOnGroundInDefaultPose = 0;
		for (uint32 i = 0; i < uWheelsFound; i++) {
			wheelRadii[i] += fMiddleWheelAboveGround;
			const float fWheelLowestY = wheelPositions[i].y - wheelRadii[i];
			if (fWheelLowestY < Math::EPSILON)
			{
				vehicleRtd.wheelsData.uNumWheelsOnGroundInDefaultPose++;
			}
		}
		ASSERT(vehicleRtd.wheelsData.uNumWheelsOnGroundInDefaultPose>=4 && vehicleRtd.wheelsData.uNumWheelsOnGroundInDefaultPose % 2 == 0);
		return uWheelsFound;
	}

	static physx::PxVehicleWheels* CreateVehicle(Required<VehicleCollider> vehicle, ComponentLoadHandles& handles)
	{
		const physx::PxVec3 chassisDims = ToPhysXVec3(vehicle->vChassisExtents);
		const physx::PxF32 wheelWidth = vehicle->fWheelWidth;
		const physx::PxF32 wheelRadius = vehicle->fWheelRadius;
		
		physx::PxU32 uNumWheels = vehicle->uNumWheels;

		usg::array<float, PhysicsConstants::VehicleMaxNumWheels> wheelRadii;
		usg::array<float, PhysicsConstants::VehicleMaxNumWheels> wheelWidths;
		usg::array<Vector3f, PhysicsConstants::VehicleMaxNumWheels> wheelPositions;
		usg::array<WheelHelper, PhysicsConstants::VehicleMaxNumWheels> wheelHelpers;
		
		ASSERT(uNumWheels > 0 || vehicle->szCollisionModel[0] != 0);
		if (vehicle->szCollisionModel[0] != 0)
		{
			uNumWheels = FindWheels(vehicle, wheelWidths, wheelRadii, wheelPositions, wheelHelpers, handles);
			ASSERT(uNumWheels >= 4 && uNumWheels <= PhysicsConstants::VehicleMaxNumWheels && uNumWheels % 2 == 0);
			vehicle.Modify().uNumWheels = uNumWheels;
		}
		else
		{
			for (auto& r : wheelRadii)
			{
				r = vehicle->fWheelRadius;
			}
			for (auto& w : wheelWidths)
			{
				w = vehicle->fWheelWidth;
			}
			vehicle.GetRuntimeData().wheelsData.uNumWheelsOnGroundInDefaultPose = uNumWheels;
			for (memsize i = 0; i < PX_MAX_NB_WHEELS; i++)
			{
				vehicle.GetRuntimeData().wheelsData.wheel[i].fRadius = vehicle->fWheelRadius;
				vehicle.GetRuntimeData().wheelsData.wheel[i].bone = nullptr;
			}
		}

		physx::PxPhysics* pPhysics = handles.pPhysicsScene->pPhysics;
		physx::PxCooking* pCooking = handles.pPhysicsScene->pCooking;

		Required<RigidBody> rigidBody;
		handles.GetComponent(vehicle.GetEntity(),rigidBody);
		ASSERT(rigidBody.IsValid());
		
		// Construct a physx actor with shapes for the chassis and wheels.
		// Set the rigid body mass, moment of inertia, and center of mass offset.
		physx::PxRigidDynamic* pVehicleActor = nullptr;
		{
			physx::PxConvexMesh* wheelConvexMeshes[PX_MAX_NB_WHEELS];
			physx::PxMaterial* wheelMaterials[PX_MAX_NB_WHEELS];
			if (vehicle->szCollisionModel[0] != 0)
			{
				for (physx::PxU32 i = 0; i < uNumWheels; i++)
				{
					wheelConvexMeshes[i] = handles.pPhysicsScene->pMeshCache->GetCylinderMesh(wheelHelpers[i].vOffsetFromHub, V3F_X_AXIS, wheelRadii[i], wheelWidths[i], 8);
					wheelMaterials[i] = CreateMaterial(vehicle->wheelMaterial, pPhysics);
				}
			}
			else
			{
				physx::PxConvexMesh* pWheelMesh = handles.pPhysicsScene->pMeshCache->GetCylinderMesh(V3F_ZERO, V3F_X_AXIS, vehicle->fWheelRadius, vehicle->fWheelWidth, 8);
				for (physx::PxU32 i = 0; i < uNumWheels; i++)
				{
					wheelConvexMeshes[i] = pWheelMesh;
					wheelMaterials[i] = CreateMaterial(vehicle->wheelMaterial, pPhysics);
				}
			}

			auto chassis = CreateChassisMesh(vehicle, handles, chassisDims, *pPhysics, *pCooking);
			physx::PxMaterial* pChassisMaterials[1] = { CreateMaterial(vehicle->chassisMaterial,pPhysics) };

			physx::PxVehicleChassisData rigidBodyData;
			rigidBodyData.mMOI = ToPhysXVec3(vehicle->vChassisMOI);
			rigidBodyData.mMass = rigidBody->fMass;
			rigidBodyData.mCMOffset = ToPhysXVec3(vehicle->vChassisCMOffset);
			pVehicleActor = CreateVehicleActor(vehicle, handles, rigidBodyData, wheelMaterials, wheelConvexMeshes, pChassisMaterials, chassis.first, chassis.second , 1, *pPhysics);

			const float fFinalMass = pVehicleActor->getMass();
			rigidBody.Modify().fMass = fFinalMass;

			auto pChassisShape = vehicle.GetRuntimeData().pChassisShape;
#ifndef FINAL_BUILD
			pChassisShape->setName("VehicleChassis");
#endif
			handles.pPhysicsScene->dirtyShapeList.insert((PhysXShapeRuntimeData*)&vehicle.GetRuntimeData());
		}

		physx::PxVehicleWheelsSimData* wheelsSimData = physx::PxVehicleWheelsSimData::allocate(uNumWheels);
		{

			physx::PxVec3 wheelCentreActorOffsets[PX_MAX_NB_WHEELS];
			if (vehicle->szCollisionModel[0] != 0)
			{
				for (memsize i = 0; i < uNumWheels; i++)
				{
					wheelCentreActorOffsets[i] = ToPhysXVec3(wheelPositions[i]);
				}
			}
			else
			{
				const physx::PxF32 fFrontZ = chassisDims.z*0.35f;
				const physx::PxF32 fRearZ = -chassisDims.z*0.35f;
				ComputeWheelCenterActorOffsets(fFrontZ, fRearZ, chassisDims, wheelWidth, wheelRadius, uNumWheels, wheelCentreActorOffsets);
				for (memsize i = 0; i < uNumWheels; i++)
				{
					vehicle.GetRuntimeData().wheelsData.wheel[i].vBaseOffset = ToUsgVec3(wheelCentreActorOffsets[i]);
				}
			}
			SetupWheelsSimulationData(vehicle, vehicle->fWheelMOI, wheelRadii, wheelWidths, wheelHelpers, uNumWheels, wheelCentreActorOffsets, ToPhysXVec3(vehicle->vChassisCMOffset), rigidBody->fMass, wheelsSimData);
		}

		physx::PxVehicleNoDrive* pVehicle = physx::PxVehicleNoDrive::allocate(uNumWheels);
		pVehicle->setup(pPhysics, pVehicleActor, *wheelsSimData);
		wheelsSimData->free();
		return pVehicle;
	}

	void OnVehicleColliderActivated(Component<Components::VehicleCollider>& c)
	{
		auto& rtd = c.GetRuntimeData();
		rtd.pVehicleDrive = nullptr;
		rtd.pChassisShape = nullptr;
		rtd.wheelsData.uNumWheelsOnGround = 0;
		for (auto& wheel : rtd.wheelsData.wheel)
		{
			wheel.bOnGround = false;
			wheel.uGroundMaterialMask = 0;
		}
	}

	void OnVehicleColliderLoaded(Component<Components::VehicleCollider>& c, ComponentLoadHandles& handles)
	{
		Required<RigidBody> rigidBody;
		handles.GetComponent(c.GetEntity(), rigidBody);
		ASSERT(rigidBody.IsValid() && "You are trying to add VehicleCollider to an entity which has no rigid body. Please add a dynamic rigid body to the entity.");

		const auto& vChassisDims = c->vChassisExtents;
		const Vector3f vChassisMOI((vChassisDims.y*vChassisDims.y + vChassisDims.z*vChassisDims.z)*rigidBody->fMass / 12.0f, (vChassisDims.x*vChassisDims.x + vChassisDims.z*vChassisDims.z)*0.8f*rigidBody->fMass / 12.0f, (vChassisDims.x*vChassisDims.x + vChassisDims.y*vChassisDims.y)*rigidBody->fMass / 12.0f);
		
		auto& collider = c.Modify();
		collider.vChassisMOI = vChassisMOI;
		collider.fWheelMOI = 0.5f*c->fWheelMass*c->fWheelRadius*c->fWheelRadius;

		auto& rtd = c.GetRuntimeData();
		rtd.pVehicleDrive = CreateVehicle(Required<VehicleCollider>(c),handles);
		rtd.entity = c.GetEntity();
		rtd.shapeAggregateEntity = c.GetEntity();
		
		handles.pPhysicsScene->vehicleData.vehicleList.push_back(rtd.pVehicleDrive);
	}

	void OnVehicleColliderDeactivated(Component<Components::VehicleCollider>& c, ComponentLoadHandles& handles)
	{
		auto& rtd = c.GetRuntimeData();
		auto it = std::find(handles.pPhysicsScene->vehicleData.vehicleList.begin(), handles.pPhysicsScene->vehicleData.vehicleList.end(), rtd.pVehicleDrive);
		ASSERT(it != handles.pPhysicsScene->vehicleData.vehicleList.end());
		handles.pPhysicsScene->vehicleData.vehicleList.erase(it);
		
		rtd.pVehicleDrive = nullptr;
		rtd.pChassisShape = nullptr;
	}

	namespace Systems
	{

		class SimulateVehicle : public System
		{
		public:

			struct Inputs
			{
				Required<VehicleCollider> vehicleCollider;
				Required<PhysicsScene, FromParents> scene;
				Required<RigidBody> rigidBody;
			};

			struct Outputs
			{
				Required<VehicleCollider> vehicleCollider;
			};

			DECLARE_SYSTEM(SYSTEM_FETCH_VEHICLE_SIMULATION_RESULTS)

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				const bool bDebugRender = false;

				auto& rtd = outputs.vehicleCollider.GetRuntimeData();
				const auto& sceneRtd = inputs.scene.GetRuntimeData().GetData();
				size_t uMyIndex = 0xffff;
				const size_t uNumVehicles = sceneRtd.vehicleData.vehicleList.size();
				for (size_t i = 0; i < uNumVehicles; i++)
				{
					if (sceneRtd.vehicleData.vehicleList[i] == inputs.vehicleCollider.GetRuntimeData().pVehicleDrive)
					{
						uMyIndex = i;
						break;
					}
				}

				if (uMyIndex != 0xffff)
				{
					const auto physxTrans = inputs.rigidBody.GetRuntimeData().pRigidActor->getGlobalPose();
					const auto& results = inputs.scene.GetRuntimeData().GetData().vehicleData.vehicleWheelQueryResults[uMyIndex];

					rtd.wheelsData.uNumWheelsOnGround = 0;
					for (size_t i = 0; i < inputs.vehicleCollider->uNumWheels; i++)
					{
						const auto& wheelRes = results.wheelQueryResults[i];
						rtd.wheelsData.uNumWheelsOnGround += 1-(wheelRes.isInAir & 1);
						rtd.wheelsData.wheel[i].bOnGround = !wheelRes.isInAir;
						if (wheelRes.tireSurfaceMaterial != nullptr)
						{
							ASSERT(wheelRes.tireSurfaceMaterial->userData != nullptr);
							const auto& m = *(PhysicMaterial*)(wheelRes.tireSurfaceMaterial->userData);
							rtd.wheelsData.wheel[i].uGroundMaterialMask = m.uFlags;
						}
						else
						{
							rtd.wheelsData.wheel[i].uGroundMaterialMask = 0;
						}
					}

					if (bDebugRender)
					{
						for (size_t i = 0; i < inputs.vehicleCollider->uNumWheels; i++)
						{
							const auto& wheelRes = results.wheelQueryResults[i];
							auto wheelTransform = physxTrans.transform(wheelRes.localPose);
							auto raycastBegin = wheelRes.suspLineStart;
							auto raycastEnd = wheelRes.suspLineStart + wheelRes.suspLineLength*wheelRes.suspLineDir;
							Color col = Color::Green;
							col.m_rgba[3] = 0.25f;
							col.m_rgba[0] = (rtd.wheelsData.wheel[i].uGroundMaterialMask % 256) / 255.0f;

							const auto qVehicleRotation = ToUsgQuaternionf(inputs.vehicleCollider.GetRuntimeData().pVehicleDrive->getRigidDynamicActor()->getGlobalPose().q);
							const auto vRight = V3F_X_AXIS*qVehicleRotation;
							const float32 fRadius = inputs.vehicleCollider.GetRuntimeData().wheelsData.wheel[i].fRadius;
							Debug3D::GetRenderer()->AddLine(ToUsgVec3(wheelTransform.p) - vRight*0.55f, ToUsgVec3(wheelTransform.p) + vRight*0.55f, Color::Red, 0.01f);
							Debug3D::GetRenderer()->AddSphere(ToUsgVec3(wheelTransform.p), fRadius, wheelRes.isInAir ? Color(1, 0, 0, 0.25f) : col);
							Debug3D::GetRenderer()->AddLine(ToUsgVec3(raycastBegin), ToUsgVec3(raycastEnd), Color::Red, 0.01f);

							if (!wheelRes.isInAir)
							{
								Debug3D::GetRenderer()->AddLine(ToUsgVec3(wheelRes.tireContactPoint), ToUsgVec3(wheelRes.tireContactPoint + wheelRes.tireContactNormal), Color::Blue, 0.01f);
							}
						}
					}
				}
			}
		};

		class UpdateVehicleWheels : public System
		{
		public:

			struct Inputs
			{
				Required<VehicleCollider, FromParents> vehicleCollider;
				Required<VehicleWheel> wheel;
				Required<PhysicsScene, FromParents> scene;
				Required<EventManagerHandle, FromParents> eventManager;
				Required<EntityID, FromParentWith<ModelComponent>> vehicleSelf;
			};

			struct Outputs
			{
				Required<TransformComponent> trans;
				Optional<InputScale> inputScale;
				Optional<UVRotation> uvRotation;
			};

			DECLARE_SYSTEM(SYSTEM_FETCH_VEHICLE_SIMULATION_RESULTS)

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				const auto& rtd = inputs.vehicleCollider.GetRuntimeData();
				const auto& sceneRtd = inputs.scene.GetRuntimeData().GetData();
				const size_t uNumVehicles = sceneRtd.vehicleData.vehicleList.size();
				for (size_t i = 0; i < uNumVehicles; i++)
				{
					if (sceneRtd.vehicleData.vehicleList[i] == inputs.vehicleCollider.GetRuntimeData().pVehicleDrive)
					{
						const auto& results = inputs.scene.GetRuntimeData().GetData().vehicleData.vehicleWheelQueryResults[i];
						if (inputs.wheel->bIsHub)
						{
							outputs.trans.Modify().position = ToUsgVec3(results.wheelQueryResults[inputs.wheel->uIndex].localPose.p);
							if (outputs.inputScale.Exists() && outputs.uvRotation.Exists())
							{
								const float fRot = inputs.vehicleCollider.GetRuntimeData().wheelsData.wheel[inputs.wheel->uIndex].fRotation*outputs.uvRotation.Force()->fMultiplier;
								outputs.uvRotation.Force().Modify().fRotation = fRot;
								RotateUV modifyUV = { fRot, outputs.uvRotation.Force()->identifier.uTexIndex, outputs.uvRotation.Force()->identifier.uMeshIndex, outputs.uvRotation.Force()->identifier.uUVCount };
								if (modifyUV.uMeshIndex != USG_INVALID_ID)
								{
									inputs.eventManager->handle->RegisterEventWithEntity(*inputs.vehicleSelf, modifyUV);
								}
							}
						}
						else
						{
							if (!inputs.vehicleCollider->bFakeWheelRotation)
							{
								auto qRot = ToUsgQuaternionf(results.wheelQueryResults[inputs.wheel->uIndex].localPose.q);
								Vector3f vAxis;
								float fAngle;
								qRot.GetAngleAxis(vAxis, fAngle);
								const float fXRot = fAngle;
								outputs.trans.Modify().rotation.SetFromEuler(0, fXRot, 0);
							}
							else
							{
								outputs.trans.Modify().rotation.SetFromEuler(0, inputs.vehicleCollider.GetRuntimeData().wheelsData.wheel[inputs.wheel->uIndex].fRotation, 0);
							}
						}
					}
				}
			}
		};

		
	}
}

#include GENERATED_SYSTEM_CODE(Engine/Physics/VehicleCollider.cpp)