/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A model for collisions
*****************************************************************************/
#ifndef _USG_PHYSICS_PHYSICS_COMPONENTS_H_
#define _USG_PHYSICS_PHYSICS_COMPONENTS_H_

#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Physics/PhysX.h"
#include "Engine/Framework/ComponentEntity.h"
#include "Engine/Physics/PhysicsComponents.pb.h"
#include <utility>

namespace usg
{
	class PhysXMeshCache;
	class VehicleSceneQueryData;
	class ContactReportCallback;

	namespace physics
	{
		struct PhysicsSceneData;
	}

	struct PhysXShapeRuntimeData
	{
		physx::PxMaterial* pMaterial;
		physx::PxShape* pShape;
		Entity shapeAggregateEntity;
		Entity entity;
		void release();
	};

	struct ActorRuntimeData
	{
		enum class Bitmask
		{
			InScene = 1
		};
		uint32 uBitmask;
		physx::PxActor* pActor;
	};

	template<>
	struct RuntimeData<usg::Components::PhysicsScene>
	{
		physics::PhysicsSceneData* pSceneData;
		physics::PhysicsSceneData& GetData()
		{
			return *pSceneData;
		}
		const physics::PhysicsSceneData& GetData() const
		{
			return *pSceneData;
		}
	};

	template<>
	struct RuntimeData<usg::Components::RigidBody> : public ActorRuntimeData
	{
		physx::PxRigidActor* pRigidActor;
	};

	template<>
	struct RuntimeData<usg::Components::SphereCollider> : public PhysXShapeRuntimeData
	{

	};

	template<>
	struct RuntimeData<usg::Components::BoxCollider> : public PhysXShapeRuntimeData
	{

	};

	template<>
	struct RuntimeData<usg::Components::ConeCollider> : public PhysXShapeRuntimeData
	{

	};
	
	template<>
	struct RuntimeData<usg::Components::CylinderCollider> : public PhysXShapeRuntimeData
	{
		physx::PxConvexMesh* pConvexMesh;
	};

	template<>
	struct RuntimeData<usg::Components::VehicleCollider> : public PhysXShapeRuntimeData
	{
		physx::PxVehicleWheels* pVehicleDrive;
		physx::PxShape* pChassisShape;

		struct
		{
			// Number of wheels touching the ground on current frame.
			uint32 uNumWheelsOnGround;

			// Number of wheels touching the ground if the vehicle is placed on a flat plane and no forces are being applied.
			// The value is constant and computed when the vehicle is constructed.
			uint32 uNumWheelsOnGroundInDefaultPose;

			struct {
				bool bOnGround;
				float fRadius;

				PhysXShapeRuntimeData rtd;
				Entity bone;

				// Offset from thee actor center in the default pose
				Vector3f vBaseOffset;

				// If you are using bFakeWheelRotation for the vehicle collider, you must update this every time with the fake value of your choice.
				float fRotation = 0.0f;

				// Material flags of the shape that the wheel is currently touching.
				uint32 uGroundMaterialMask;
			} wheel[PhysicsConstants::VehicleMaxNumWheels];
		} wheelsData;
	};

	template<>
	struct RuntimeData<usg::Components::MeshCollider> : public PhysXShapeRuntimeData
	{
		physx::PxConvexMesh* pConvexMesh;
		physx::PxTriangleMesh* pTriangleMesh;
	};

	template<>
	struct RuntimeData<usg::Components::HeightFieldCollider> : public PhysXShapeRuntimeData
	{
		physx::PxHeightField* pHeightfield;
		physx::PxHeightFieldSample* pSamples;
	};

	template<>
	struct RuntimeData<usg::Components::PrismaticJoint>
	{
		physx::PxPrismaticJoint* pJoint;
	};

	template<>
	struct RuntimeData<usg::Components::RevoluteJoint>
	{
		physx::PxRevoluteJoint* pJoint;
	};

	template<>
	struct RuntimeData<usg::Components::FixedJoint>
	{
		physx::PxFixedJoint* pJoint;
	};

	template<>
	struct RuntimeData<usg::Components::PhysicsAggregate>
	{
		physx::PxAggregate* pAggregate;
	};

	namespace physics
	{
		namespace details
		{
			uint32 GetMaterialHash(const PhysicMaterial& m);

			inline Vector3f GetVelocityAtPosition(const RuntimeData<Components::RigidBody>& rbRtd, const Vector3f& vWorldPosition)
			{
				return rbRtd.pRigidActor->is<physx::PxRigidBody>() ? ToUsgVec3(physx::PxRigidBodyExt::getVelocityAtPos(*rbRtd.pRigidActor->is<physx::PxRigidBody>(), ToPhysXVec3(vWorldPosition))) : Vector3f::ZERO;
			}

			inline PhysXShapeRuntimeData* GetUserDataFromPhysXShape(physx::PxShape* pShape)
			{
				ASSERT(pShape != nullptr);
				return (PhysXShapeRuntimeData*)pShape->userData;
			}

			inline void SetUserData(physx::PxShape* pShape, PhysXShapeRuntimeData* pRTD)
			{
				ASSERT(pShape != nullptr);
				pShape->userData = pRTD;
			}
		}

		template<typename T>
		Vector3f GetLinearVelocity(Optional<RigidBody, T> rigidBody)
		{
			ASSERT(rigidBody.Exists());
			if (rigidBody.Force()->bDynamic)
			{
				physx::PxRigidDynamic* pRigidDynamic = rigidBody.Force().GetRuntimeData().pRigidActor->template is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				return ToUsgVec3(pRigidDynamic->getLinearVelocity());
			}
			return Vector3f::ZERO;
		}

		template<typename T>
		Vector3f GetLinearVelocity(Required<RigidBody, T> rigidBody)
		{
			if (rigidBody->bDynamic)
			{
				physx::PxRigidDynamic* pRigidDynamic = rigidBody.GetRuntimeData().pRigidActor->template is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				return ToUsgVec3(pRigidDynamic->getLinearVelocity());
			}
			return Vector3f::ZERO;
		}

		template<typename T>
		float GetLinearDamping(Required<RigidBody, T> rigidBody)
		{
			if (rigidBody->bDynamic)
			{
				physx::PxRigidDynamic* pRigidDynamic = rigidBody.GetRuntimeData().pRigidActor->template is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				return pRigidDynamic->getLinearDamping();
			}
			return 0.0f;
		}

		template<typename T>
		Vector3f GetAngularVelocity(Required<RigidBody, T> rigidBody)
		{
			if (rigidBody->bDynamic)
			{
				physx::PxRigidDynamic* pRigidDynamic = rigidBody.GetRuntimeData().pRigidActor->template is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				return ToUsgVec3(pRigidDynamic->getAngularVelocity());
			}
			return usg::Vector3f::ZERO;
		}

		template<typename T>
		Vector3f GetVelocityAtPosition(Optional<RigidBody, T> rigidBody, const Vector3f& vWorldPosition)
		{
			ASSERT(rigidBody.Exists());
			return details::GetVelocityAtPosition(rigidBody.Force().GetRuntimeData(), vWorldPosition);
		}

		template<typename T>
		Vector3f GetVelocityAtPosition(Required<RigidBody, T> rigidBody, const Vector3f& vWorldPosition)
		{
			return details::GetVelocityAtPosition(rigidBody.GetRuntimeData(), vWorldPosition);
		}

	}
}


#endif
