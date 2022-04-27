/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceMgr.h"
#include "PhysicsComponents.h"
#include "Engine/Physics/PhysX.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/ComponentLoadHandles.h"
#include "Engine/Resource/CollisionModelResource.h"
#include "Engine/Physics/PhysXMeshCache.h"
#include "Engine/Physics/VehicleCollider.h"
#include "Engine/Physics/PhysicsSceneData.h"
#include "Engine/Framework/TransformTool.h"
#include <EASTL/string.h>

namespace usg
{
	static_assert(sizeof(CollisionModelResource::IndexType)==sizeof(uint32),"PhysX and Usagi CollisionModelResource are using different index type.");

	// Helper functions

	namespace physics
	{
		namespace details
		{
			uint32 GetMaterialHash(const PhysicMaterial& m)
			{
				return utl::CRC32(&m, sizeof(PhysicMaterial));
			}
		}
	}

	template<typename ShapeType>
	static void OnActivateShape(Component<ShapeType>& c)
	{
		auto& rtd = c.GetRuntimeData();
		rtd.shapeAggregateEntity = nullptr;
		rtd.entity = nullptr;
		rtd.pShape = nullptr;
		rtd.pMaterial = nullptr;
	}

	template<typename ShapeType>
	static void OnDeactivateShape(Component<ShapeType>& c, ComponentLoadHandles& handles)
	{
		auto& rtd = c.GetRuntimeData();
		rtd.release();

		if (handles.pPhysicsScene->dirtyShapeList.count(&rtd))
		{
			handles.pPhysicsScene->dirtyShapeList.erase(&rtd);
		}

		rtd.shapeAggregateEntity = nullptr;
		rtd.entity = nullptr;
		rtd.pShape = nullptr;
	}
	
	void PhysXShapeRuntimeData::release()
	{
		ASSERT(pMaterial != nullptr);
		pMaterial->release();
		pMaterial = nullptr;
	}

	TransformComponent GetTransform(Entity e, ComponentLoadHandles& handles)
	{
		Optional<TransformComponent> trans;
		handles.GetComponent(e, trans);
		if (trans.Exists())
		{
			return *trans.Force();
		}
		TransformComponent r;
		TransformComponent_init(&r);
		return r;
	}

	// Get an entity's transform relative to some not necessarily direct parent (pass nullptr as parent to get global pose)
	static physx::PxTransform GetRelativeTransform(Entity parent, Entity child, ComponentLoadHandles& handles)
	{
		if (parent == nullptr)
		{
			// Use the matrix component to initialize static bodies with no transformcomponent
			Optional<TransformComponent, FromSelfOrParents> transFromSelfOrParents;
			handles.GetComponent(child, transFromSelfOrParents);
			if (!transFromSelfOrParents.Exists())
			{
				Optional<MatrixComponent, FromSelfOrParents> mtxFromSelfOrParents;
				handles.GetComponent(child, mtxFromSelfOrParents);
				if (mtxFromSelfOrParents.Exists())
				{
					Quaternionf qRot = mtxFromSelfOrParents.Force()->matrix;
					const Vector3f vPos = mtxFromSelfOrParents.Force()->matrix.vPos().v3();
					physx::PxTransform t;
					t.p = ToPhysXVec3(vPos);
					t.q = ToPhysXQuaternion(qRot);
					ASSERT(t.q.isUnit());
					return t;
				}
			}
		}

		const TransformComponent usgTrans = GetTransform(child, handles);
		if (!usgTrans.bInheritFromParent)
		{
			return ToPhysXTransform(usgTrans);
		}
		physx::PxTransform trans = ToPhysXTransform(usgTrans);
		Entity e = child;
		while (e->GetParentEntity() != parent)
		{
			physx::PxTransform parentTrans = ToPhysXTransform(GetTransform(e->GetParentEntity(), handles));
			trans = parentTrans.transform(trans);
			e = e->GetParentEntity();
		}
		// Some propagated error
		trans.q = trans.q.getNormalized();
		return trans;
	}

	void ApplyMaterial(physx::PxMaterial* pMaterial, const PhysicMaterial& m)
	{
		pMaterial->setDynamicFriction(m.fDynamicFriction);
		pMaterial->setStaticFriction(m.fStaticFriction);
		pMaterial->setRestitution(m.fBounciness);
		pMaterial->setFrictionCombineMode((physx::PxCombineMode::Enum)m.eFrictionCombineMode);
		pMaterial->setRestitutionCombineMode((physx::PxCombineMode::Enum)m.eRestitutionCombineMode);
		pMaterial->userData = (void*)&m;
	}

	// The Scene

	template<>
	void OnActivate<PhysicsScene>(Component<PhysicsScene>& c)
	{
		auto& rtd = c.GetRuntimeData();
		rtd.pSceneData = nullptr;
	}

	template<>
	void OnDeactivate<PhysicsScene>(Component<PhysicsScene>& c, ComponentLoadHandles& handles)
	{
		DeinitPhysicsScene(c);
	}

	template <>
	void OnLoaded<PhysicsScene>(Component<PhysicsScene>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}
		InitPhysicsScene(c);
	}

	// Rigid Body

	static void OnRigidBodyLoaded(Required<RigidBody>& c, ComponentLoadHandles& handles)
	{
		auto& rtd = c.GetRuntimeData();

		if (rtd.pRigidActor != nullptr)
		{
			return;
		}

		auto& sceneRuntimeData = *handles.pPhysicsScene;

		ASSERT(!c->bDynamic || GameComponents<TransformComponent>::GetComponentData(c.GetEntity()) != nullptr && "Entities with dynamic rigid bodies must have TransformComponent");
		const physx::PxTransform initialTransform = GetRelativeTransform(nullptr, c.GetEntity(), handles);
		
		if (c->bDynamic)
		{
			physx::PxRigidDynamic* pRigidBody = sceneRuntimeData.pPhysics->createRigidDynamic(initialTransform);
			ASSERT(pRigidBody != nullptr);
			pRigidBody->setMass(c->fMass);
			pRigidBody->setMassSpaceInertiaTensor(ToPhysXVec3(c->vInertiaTensor));
			pRigidBody->setAngularDamping(c->fAngularDamping);
			pRigidBody->setLinearDamping(c->fLinearDamping);
			pRigidBody->setSolverIterationCounts(c->uSolverMinPositionsIters, c->uSolverMinVelocityIters);
			rtd.pRigidActor = pRigidBody;
			GameComponents<DynamicBodyTag>::Create(c.GetEntity());
			if (c->bDisableSleep)
			{
				pRigidBody->setSleepThreshold(0);
			}
			else
			{
				pRigidBody->setSleepThreshold(c->fSleepThreshold);
				pRigidBody->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
			}
			if (c->bKinematic)
			{
				pRigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
				GameComponents<KinematicBodyTag>::Create(c.GetEntity());
			}
			if (c->bEnableCCD)
			{
				ASSERT_MSG(!c->bKinematic, "CCD not supported on kinematic bodies.");
				pRigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
			}
			if (c->bDisableGravity)
			{
				pRigidBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
			}
		}
		else
		{
			ASSERT_MSG(!c->bKinematic, "Kinematic bodies must be dynamic.");
			ASSERT_MSG(!c->bEnableCCD, "CCD is applied to dynamic bodies.");
			ASSERT_MSG(!c->bDisableGravity, "Gravity is applied to dynamic bodies.");
			rtd.pRigidActor = sceneRuntimeData.pPhysics->createRigidStatic(initialTransform);
		}
		ASSERT(rtd.pRigidActor != nullptr);
		rtd.pRigidActor->userData = GameComponents<RigidBody>::GetComponent(c.GetEntity());
		rtd.pActor = rtd.pRigidActor;

		Optional<Identifier> id;
		handles.GetComponent(c.GetEntity(), id);
		if (id.Exists())
		{
			rtd.pActor->setName(id.Force()->name);
		}

		Optional<PhysicsAggregate, FromSelfOrParents> aggregate;
		handles.GetComponent(c.GetEntity(), aggregate);
		if (aggregate.Exists() && !c->bDoNotAttachToParentAggregate)
		{
			auto& aggregateComponent = *GameComponents<PhysicsAggregate>::GetComponent(aggregate.Force().GetEntity());
			if (aggregateComponent.GetRuntimeData().pAggregate == nullptr)
			{
				OnLoaded(aggregateComponent, handles, false);
			}
			physx::PxAggregate* pAggregate = aggregateComponent.GetRuntimeData().pAggregate;
			ASSERT(pAggregate != nullptr);
			pAggregate->addActor(*rtd.pActor);
			ASSERT(rtd.pActor->getAggregate() == pAggregate);
		}


		// Go through the parents and inform them physics needs the transform updated
		// FIXME: This should be done in data by the hierarchy processor rather than at runtime
		Entity parent = c.GetEntity();
		while (parent = parent->GetParentEntity())
		{
			Optional<TransformComponent> transform;
			Optional<RigidBodyTransformUpdate> parentMarker;
			handles.GetComponent(parent, transform);
			handles.GetComponent(parent, parentMarker);
			if (transform.Exists() && !parentMarker.Exists())
			{
				GameComponents<RigidBodyTransformUpdate>::Create(parent);
			}
		}
		
		sceneRuntimeData.addActorList.insert(&rtd);
		sceneRuntimeData.dirtyActorList.insert(&rtd);
	}

	template <>
	void OnLoaded<RigidBody>(Component<RigidBody>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}

		Required<RigidBody, FromSelf> rb;
		handles.GetComponent(c.GetEntity(), rb);
		OnRigidBodyLoaded(rb, handles);
	}

	template<>
	void OnActivate<RigidBody>(Component<RigidBody>& c)
	{
		c.Modify().vInertiaTensor = Vector3f(1, 1, 1);

		auto& rtd = c.GetRuntimeData();
		rtd.pRigidActor = nullptr;
		rtd.pActor = nullptr;
		rtd.pActor = nullptr;
		rtd.uBitmask = 0;
	}

	template<>
	void OnDeactivate<RigidBody>(Component<RigidBody>& c, ComponentLoadHandles& handles)
	{
		auto& rtd = c.GetRuntimeData();
		if (rtd.pRigidActor != nullptr)
		{
			ASSERT(rtd.pActor != nullptr);

			if (handles.pPhysicsScene->addActorList.count(&rtd))
			{
				handles.pPhysicsScene->addActorList.erase(&rtd);
			}
			else
			{
				handles.pPhysicsScene->removeActorList.insert(rtd.pRigidActor);
			}

			handles.pPhysicsScene->dirtyActorList.erase(&rtd);

			rtd.pRigidActor = nullptr;
			rtd.pActor = nullptr;
		}
		rtd.uBitmask = 0;
	}

	// Generic Shape Functions
	template<typename T>
	static void AddShape(Required<RigidBody, T> rigidBody, physx::PxShape* pShape, ComponentLoadHandles& handles)
	{
		physx::PxRigidActor* pActor = rigidBody.GetRuntimeData().pRigidActor;
		ASSERT(pActor != nullptr && "PhysX actor pointer uninitialized. Are you attempting to add a shape to a rigid body for which OnLoaded has not yet been called?");
		pActor->attachShape(*pShape);

		Optional<VehicleCollider> vehicle;
		handles.GetComponent(rigidBody.GetEntity(), vehicle);
		if (vehicle.Exists())
		{
			return;
		}
		
		if (rigidBody->bDynamic && pActor->is<physx::PxRigidBody>())
		{
			physx::PxRigidBody* pRigidBody = static_cast<physx::PxRigidBody*>(rigidBody.GetRuntimeData().pRigidActor);
			if (rigidBody->fDensity > Math::EPSILON)
			{
				physx::PxRigidBodyExt::updateMassAndInertia(*pRigidBody, rigidBody->fDensity, 0, false);
				rigidBody.Modify().fMass = pRigidBody->getMass();
			}
			else
			{
				physx::PxRigidBodyExt::updateMassAndInertia(*pRigidBody, 1.0f, 0, false);
				pRigidBody->setMass(rigidBody->fMass);
			}
			const physx::PxVec3 vTensor = pRigidBody->getMassSpaceInertiaTensor().multiply(ToPhysXVec3(rigidBody->vInertiaTensor));
			pRigidBody->setMassSpaceInertiaTensor(vTensor);
		}
	}

	template<typename SearchMask>
	static void EnsureRigidBodyLoaded(Optional<RigidBody, SearchMask> rigidBody, ComponentLoadHandles& handles)
	{
		if (rigidBody.Exists() && rigidBody.Force().GetRuntimeData().pRigidActor == nullptr)
		{
			Required<RigidBody, FromSelf> rigidBodyFromSelf;
			handles.GetComponent(rigidBody.Force().GetEntity(), rigidBodyFromSelf);
			ASSERT(rigidBodyFromSelf.IsValid());
			OnRigidBodyLoaded(rigidBodyFromSelf, handles);
		}
	}

	template<typename SearchMask>
	static void EnsureRigidBodyLoaded(Required<RigidBody, SearchMask> rigidBody, ComponentLoadHandles& handles)
	{
		if (rigidBody.GetRuntimeData().pRigidActor == nullptr)
		{
			Required<RigidBody, FromSelf> rigidBodyFromSelf;
			handles.GetComponent(rigidBody.GetEntity(), rigidBodyFromSelf);
			ASSERT(rigidBodyFromSelf.IsValid());
			OnRigidBodyLoaded(rigidBodyFromSelf, handles);
		}
	}

	template<typename ShapeType>
	static void OnShapeLoaded(Component<ShapeType>&c, const physx::PxGeometry& geometry, ComponentLoadHandles& handles)
	{
		PhysXShapeRuntimeData& rtd = c.GetRuntimeData();

		rtd.pMaterial = handles.pPhysicsScene->pPhysics->createMaterial(PhysicMaterial_fStaticFriction_default, PhysicMaterial_fDynamicFriction_default, PhysicMaterial_fBounciness_default);
		rtd.pMaterial->userData = (void*)(uint64)0xdeadc0de;

		ApplyMaterial(rtd.pMaterial, c->material);

		physx::PxShape* pShape = handles.pPhysicsScene->pPhysics->createShape(geometry, *rtd.pMaterial, true);
		rtd.pShape = pShape;

		Entity shapeAggregateEntity = nullptr;
		Entity actorEntity = nullptr;
		Optional<RigidBody, FromSelfOrParents> rigidBody;
		handles.GetComponent(c.GetEntity(), rigidBody);
		ASSERT((rigidBody.Exists() || c->bIsTrigger) && "A shape must be attached to a rigid body or be a trigger.");
		if (rigidBody.Exists())
		{
			Optional<PhysicsAggregate, FromSelfOrParents> aggregate;
			handles.GetComponent(c.GetEntity(), aggregate);
			if (aggregate.Exists() && !rigidBody.Force()->bDoNotAttachToParentAggregate)
			{
				shapeAggregateEntity = aggregate.Force().GetEntity();
			}
			else
			{
				shapeAggregateEntity = rigidBody.Force().GetEntity();
			}
			actorEntity = rigidBody.Force().GetEntity();
		}
		else if (c->bIsTrigger)
		{
			shapeAggregateEntity = c.GetEntity();
			actorEntity = shapeAggregateEntity;
		}
		ASSERT(shapeAggregateEntity != nullptr);
		rtd.shapeAggregateEntity = shapeAggregateEntity;
		rtd.entity = c.GetEntity();

		EnsureRigidBodyLoaded(rigidBody, handles);
		
		physics::details::SetUserData(pShape, &c.GetRuntimeData());

		if (c.GetEntity() != actorEntity)
		{
			const physx::PxTransform transformRelativeToActor = GetRelativeTransform(actorEntity,c.GetEntity(), handles);
			pShape->setLocalPose(physx::PxTransform(ToPhysXVec3(c->vCenter)).transform(transformRelativeToActor));
		}
		else
		{
			pShape->setLocalPose(physx::PxTransform(ToPhysXVec3(c->vCenter)));
		}

		Required<RigidBody> rigidBodyFromSelf;
		if (rigidBody.Exists())
		{
			handles.GetComponent(actorEntity, rigidBodyFromSelf);
		}
		else
		{
			// No rigid body found. In this case, the shape must be a trigger.
			ASSERT(c->bIsTrigger);
			auto pRigidBody = GameComponents<RigidBody>::Create(shapeAggregateEntity);
			RigidBody_init(pRigidBody);
			pRigidBody->bDynamic = false;

			handles.GetComponent(shapeAggregateEntity, rigidBodyFromSelf);
			OnRigidBodyLoaded(rigidBodyFromSelf, handles);
		}
		if (c->bIsTrigger)
		{
			pShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			pShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		}
		

		Optional<Identifier> id;
		handles.GetComponent(c.GetEntity(), id);
		if (id.Exists())
		{
			pShape->setName(id.Force()->name);
		}
		AddShape(rigidBodyFromSelf, pShape, handles);
		handles.pPhysicsScene->dirtyActorList.insert(&rigidBodyFromSelf.GetRuntimeData());
		handles.pPhysicsScene->dirtyShapeList.insert(&rtd);
	}

	// Sphere Collider

	template <>
	void OnLoaded<SphereCollider>(Component<SphereCollider>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}

		OnShapeLoaded(c, physx::PxSphereGeometry(c->fRadius), handles);
	}

	template<>
	void OnActivate<SphereCollider>(Component<SphereCollider>& c)
	{
		OnActivateShape(c);
	}

	template<>
	void OnDeactivate<SphereCollider>(Component<SphereCollider>& c, ComponentLoadHandles& handles)
	{
		OnDeactivateShape(c, handles);
	}

	// Box Collider

	template <>
	void OnLoaded<BoxCollider>(Component<BoxCollider>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}

		OnShapeLoaded(c, physx::PxBoxGeometry(ToPhysXVec3(c->vExtents)), handles);
	}

	template<>
	void OnActivate<BoxCollider>(Component<BoxCollider>& c)
	{
		OnActivateShape(c);
	}

	template<>
	void OnDeactivate<BoxCollider>(Component<BoxCollider>& c, ComponentLoadHandles& handles)
	{
		OnDeactivateShape(c, handles);
	}

	// Cone Collider

	template <>
	void OnLoaded<ConeCollider>(Component<ConeCollider>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}

		auto pScene = handles.pPhysicsScene;

		Vector3f vToCircle;
		Vector3f vOtherVec = Vector3f::Z_AXIS;
		if (DotProduct(c->vDirection, Vector3f::Z_AXIS) > 0.99f)
		{
			vOtherVec = Vector3f::X_AXIS;
		}
		vToCircle = CrossProduct(c->vDirection, vOtherVec).GetNormalised()*c->fRadius;

		vector<physx::PxVec3> vertices;
		const size_t uCircleVertexCount = 1 + (size_t)(c->fRadius * 3);
		vertices.reserve(1 + uCircleVertexCount);
		physx::PxConvexMeshDesc convexDesc;
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
		convexDesc.points.stride = sizeof(physx::PxVec3);

		vertices.push_back(ToPhysXVec3(c->vCenter));
		for (size_t uVertexIndex = 0; uVertexIndex < uCircleVertexCount; uVertexIndex++)
		{
			const float fAngle = (float)uVertexIndex / (float)uCircleVertexCount*Math::two_pi;
			const Vector3f vOffset = vToCircle*Quaternionf(c->vDirection, fAngle);
			const Vector3f vVertex = c->vCenter + c->vDirection*c->fLength + vOffset;
			vertices.push_back(ToPhysXVec3(vVertex));
		}

		convexDesc.points.count = static_cast<uint32>(vertices.size());
		convexDesc.points.data = &vertices[0];

		physx::PxDefaultMemoryOutputStream buf;
		physx::PxConvexMeshCookingResult::Enum result;
		if (!pScene->pCooking->cookConvexMesh(convexDesc, buf, &result))
		{
			ASSERT(false && "Failed to generate convex mesh from cone geometry.");
			return;
		}
		physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		physx::PxConvexMesh* pConvexMesh = pScene->pPhysics->createConvexMesh(input);
		OnShapeLoaded(c, physx::PxConvexMeshGeometry(pConvexMesh), handles);
		pConvexMesh->release();
	}

	template<>
	void OnActivate<ConeCollider>(Component<ConeCollider>& c)
	{
		OnActivateShape(c);
	}

	template<>
	void OnDeactivate<ConeCollider>(Component<ConeCollider>& c, ComponentLoadHandles& handles)
	{
		OnDeactivateShape(c, handles);
	}

	// Cylinder Collider

	template <>
	void OnLoaded<CylinderCollider>(Component<CylinderCollider>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}

		const memsize uCircleVertexCount = c->has_uCircleVertices ? c->uCircleVertices : Math::Min((memsize)16, 1 + (memsize)(c->fRadius * 5));
		physx::PxConvexMesh* pConvexMesh = handles.pPhysicsScene->pMeshCache->GetCylinderMesh(c->vCenter, c->vDirection, c->fRadius, c->fHeight, (uint32)uCircleVertexCount);
		ASSERT(pConvexMesh != nullptr);
		if (pConvexMesh != nullptr)
		{
			c.GetRuntimeData().pConvexMesh = pConvexMesh;
			OnShapeLoaded(c, physx::PxConvexMeshGeometry(pConvexMesh), handles);
		}
	}

	template<>
	void OnActivate<CylinderCollider>(Component<CylinderCollider>& c)
	{
		OnActivateShape(c);
		c.GetRuntimeData().pConvexMesh = nullptr;
	}

	template<>
	void OnDeactivate<CylinderCollider>(Component<CylinderCollider>& c, ComponentLoadHandles& handles)
	{
		OnDeactivateShape(c, handles);
		c.GetRuntimeData().pConvexMesh = nullptr;
	}

	// Heightfield Collider
	template <>
	void OnLoaded<HeightFieldCollider>(Component<HeightFieldCollider>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}

		// FIXME: Once working this should be cached like the CollisionMesh as we may well re-use terrain sections
		auto& rtd = c.GetRuntimeData();
		rtd.pSamples = (physx::PxHeightFieldSample*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_PHYSICS, sizeof(physx::PxHeightFieldSample) * (c->uColumns * c->uRows));

		physx::PxHeightFieldDesc hfDesc;
		hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
		hfDesc.nbColumns = c->uColumns;
		hfDesc.nbRows = c->uRows;
		hfDesc.samples.data = rtd.pSamples;
		hfDesc.samples.stride = sizeof(physx::PxHeightFieldSample);

		auto& sceneRuntimeData = *handles.pPhysicsScene;

		rtd.pHeightfield = sceneRuntimeData.pCooking->createHeightField(hfDesc, sceneRuntimeData.pPhysics->getPhysicsInsertionCallback());

		Required<RigidBody, FromSelf> rb;
		OnShapeLoaded(c, physx::PxHeightFieldGeometry(rtd.pHeightfield, (physx::PxMeshGeometryFlags)0, c->fHeightScale, c->fColumnScale, c->fRowScale), handles);
	}

	template<>
	void OnActivate<HeightFieldCollider>(Component<HeightFieldCollider>& c)
	{
		auto& rtd = c.GetRuntimeData();
		rtd.pHeightfield = nullptr;
		rtd.pSamples = nullptr;
		OnActivateShape(c);
	}

	template<>
	void OnDeactivate<HeightFieldCollider>(Component<HeightFieldCollider>& c, ComponentLoadHandles& handles)
	{
		auto& rtd = c.GetRuntimeData();
		OnDeactivateShape(c, handles);
		if (rtd.pHeightfield)
		{
			rtd.pHeightfield->release();
			mem::Free(rtd.pSamples);
			rtd.pSamples = nullptr;
			rtd.pHeightfield = nullptr;
		}
	}

	// Joint Shared functions

	template<typename JointType, typename PhysXJointType>
	void OnJointLoaded(PhysXJointType* pJoint, Component<JointType>& c)
	{
		if (c->bCanBreak)
		{
			const auto& joinBreakForce = c->breakForce;
			pJoint->setBreakForce(joinBreakForce.fLinear, joinBreakForce.fAngular);
		}

		if (c->bEnableProjection)
		{
			pJoint->setProjectionLinearTolerance(c->fProjectionLinearTolerance);
			pJoint->setProjectionAngularTolerance(c->fProjectionAngularTolerance);
			pJoint->setConstraintFlag(physx::PxConstraintFlag::ePROJECTION, true);
		}
		pJoint->userData = c.GetEntity();
	}

	// Revolute Joint

	template<>
	void OnActivate<RevoluteJoint>(Component<RevoluteJoint>& c)
	{
		auto& rtd = c.GetRuntimeData();
		rtd.pJoint = nullptr;
	}

	template<>
	void OnDeactivate<RevoluteJoint>(Component<RevoluteJoint>& c, ComponentLoadHandles& handles)
	{

	}

	template <>
	void OnLoaded<RevoluteJoint>(Component<RevoluteJoint>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}

		// Note: currently only parent-child joints supported. Consider adding support for joints between sibling rigid bodies.

		Required<usg::RigidBody, FromSelf> myRigidBody;
		Required<usg::RigidBody, FromParents> myParentsRigidBody;
		handles.GetComponent(c.GetEntity(), myRigidBody);
		handles.GetComponent(c.GetEntity(), myParentsRigidBody);
		EnsureRigidBodyLoaded(myRigidBody, handles);
		EnsureRigidBodyLoaded(myParentsRigidBody, handles);
		
		physx::PxRigidActor* pActor1 = myParentsRigidBody.GetRuntimeData().pRigidActor;
		physx::PxRigidActor* pActor2 = myRigidBody.GetRuntimeData().pRigidActor;
		ASSERT(pActor1 != nullptr && pActor2 != nullptr);

		physx::PxTransform t1(physx::PxIdentity);

		usg::Matrix4x4 mMat;

		mMat.SetRight(usg::Vector4f(c->vAxis, 0.0f));
		mMat.SetUp( usg::Vector4f(c->vFace, 0.0f) );

		usg::Vector4f vCross = usg::CrossProduct(mMat.vRight(), mMat.vUp());

		mMat.SetFace(vCross);

		Quaternionf q1;
		//q1.MakeVectorRotation(Vector3f::X_AXIS, c->vAxis);
		q1 = mMat;
		q1.Normalise();

		TransformComponent trans;
		trans = TransformTool::GetRelativeTransform(myParentsRigidBody.GetEntity(), myRigidBody.GetEntity(), handles);

		t1.q = ToPhysXQuaternion(trans.rotation);
		t1.p = ToPhysXVec3(trans.position);

		t1.q = t1.q * ToPhysXQuaternion(q1);

		Required<TransformComponent> myTrans;
		handles.GetComponent(c.GetEntity(), myTrans);

		// FIXME: The code to handle inverting the transforms doesn't handle nested
		// entities. Although there's no good reason to inherit
		myTrans.Modify().bInheritFromParent = false;
		physx::PxTransform t2(physx::PxIdentity);
		Quaternionf q2;
		//q2.MakeVectorRotation(Vector3f::X_AXIS, c->vAxis);
		q2 = mMat;
		
		t2.q = ToPhysXQuaternion(q2);
		
		auto pJoint = physx::PxRevoluteJointCreate(*handles.pPhysicsScene->pPhysics, pActor1, t1, pActor2, t2);
		c.GetRuntimeData().pJoint = pJoint;

		pJoint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_ENABLED, c->bEnableMotor);
		if (c->fMinAngleDegrees > -180.f || c->fMaxAngleDegrees < 180.f)
		{
			physx::PxJointAngularLimitPair limit(Math::DegToRad(c->fMinAngleDegrees), Math::DegToRad(c->fMaxAngleDegrees), 0.1f);
			pJoint->setLimit(limit);
			pJoint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, true);
		}

		OnJointLoaded(pJoint, c);
	}

	// Fixed Joint

	template<>
	void OnActivate<FixedJoint>(Component<FixedJoint>& c)
	{
		auto& rtd = c.GetRuntimeData();
		rtd.pJoint = nullptr;
	}

	template<>
	void OnDeactivate<FixedJoint>(Component<FixedJoint>& c, ComponentLoadHandles& handles)
	{

	}

	template <>
	void OnLoaded<FixedJoint>(Component<FixedJoint>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}

		// Note: currently only parent-child joints supported. Consider adding support for joints between sibling rigid bodies.

		Required<usg::RigidBody, FromSelf> myRigidBody;
		Optional<usg::RigidBody, FromParents> myParentsRigidBody;
		handles.GetComponent(c.GetEntity(), myRigidBody);
		handles.GetComponent(c.GetEntity(), myParentsRigidBody);
		EnsureRigidBodyLoaded(myRigidBody, handles);
		if (myParentsRigidBody.Exists())
		{
			EnsureRigidBodyLoaded(myParentsRigidBody, handles);
		}

		physx::PxRigidActor* pActor1 = myParentsRigidBody.Exists() ? myParentsRigidBody.Force().GetRuntimeData().pRigidActor : nullptr;
		physx::PxRigidActor* pActor2 = myRigidBody.GetRuntimeData().pRigidActor;
		ASSERT(pActor2 != nullptr);

		physx::PxTransform t1(physx::PxIdentity);
		TransformComponent trans;
		if (myParentsRigidBody.Exists())
		{
			trans = TransformTool::GetRelativeTransform(myParentsRigidBody.Force().GetEntity(), myRigidBody.GetEntity(), handles);
		}
		else
		{
			trans = TransformTool::GetRelativeTransform(nullptr, myRigidBody.GetEntity(), handles);
		}

		t1.q = ToPhysXQuaternion(trans.rotation);
		t1.p = ToPhysXVec3(trans.position);


		Required<TransformComponent> myTrans;
		handles.GetComponent(c.GetEntity(), myTrans);
		physx::PxTransform t2(physx::PxIdentity);

		auto pJoint = physx::PxFixedJointCreate(*handles.pPhysicsScene->pPhysics, pActor1, t1, pActor2, t2);
		c.GetRuntimeData().pJoint = pJoint;

		OnJointLoaded(pJoint, c);
	}

	// Prismatic Joint

	template<>
	void OnActivate<PrismaticJoint>(Component<PrismaticJoint>& c)
	{
		auto& rtd = c.GetRuntimeData();
		rtd.pJoint = nullptr;
	}

	template<>
	void OnDeactivate<PrismaticJoint>(Component<PrismaticJoint>& c, ComponentLoadHandles& handles)
	{

	}

	template <>
	void OnLoaded<PrismaticJoint>(Component<PrismaticJoint>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}

		// Note: currently only parent-child joints supported. Consider adding support for joints between sibling rigid bodies.

		Required<usg::RigidBody, FromSelf> myRigidBody;
		Optional<usg::RigidBody, FromParents> myParentsRigidBody;
		handles.GetComponent(c.GetEntity(), myRigidBody);
		handles.GetComponent(c.GetEntity(), myParentsRigidBody);
		EnsureRigidBodyLoaded(myRigidBody, handles);
		if (myParentsRigidBody.Exists())
		{
			EnsureRigidBodyLoaded(myParentsRigidBody, handles);
		}

		physx::PxRigidActor* pActor1 = myParentsRigidBody.Exists() ? myParentsRigidBody.Force().GetRuntimeData().pRigidActor : nullptr;
		physx::PxRigidActor* pActor2 = myRigidBody.GetRuntimeData().pRigidActor;
		ASSERT(pActor2 != nullptr);

		const TransformComponent myTransformRelativeToConnectedParent = TransformTool::GetRelativeTransform(myParentsRigidBody.Exists() ? myParentsRigidBody.Force().GetEntity() : nullptr, myRigidBody.GetEntity(), handles);
		physx::PxTransform t1(physx::PxIdentity);
		t1.p = ToPhysXVec3(myTransformRelativeToConnectedParent.position);
		Quaternionf q1;
		q1.MakeVectorRotation(Vector3f::X_AXIS, c->vAxis);
		t1.q = ToPhysXQuaternion(q1);

		physx::PxTransform t2(physx::PxIdentity);
		t2.q = ToPhysXQuaternion(q1);

		auto pJoint = physx::PxPrismaticJointCreate(*handles.pPhysicsScene->pPhysics, pActor1, t1, pActor2, t2);
		c.GetRuntimeData().pJoint = pJoint;

		OnJointLoaded(pJoint, c);
	}

	// Aggregate

	template<>
	void OnActivate<PhysicsAggregate>(Component<PhysicsAggregate>& c)
	{
		auto& rtd = c.GetRuntimeData();
		rtd.pAggregate = nullptr;
	}

	template<>
	void OnDeactivate<PhysicsAggregate>(Component<PhysicsAggregate>& c, ComponentLoadHandles& handles)
	{
		auto& scene = *handles.pPhysicsScene;
		auto pAggregate = c.GetRuntimeData().pAggregate;
		if (scene.addAggregateList.count(pAggregate))
		{
			scene.addAggregateList.erase(pAggregate);
		}
		else
		{
			scene.removeAggregateList.insert(pAggregate);
		}
	}

	template <>
	void OnLoaded<PhysicsAggregate>(Component<PhysicsAggregate>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}

	
		c.GetRuntimeData().pAggregate = handles.pPhysicsScene->pPhysics->createAggregate(c->uMaxNumBodies, c->bEnableSelfCollisions);
		handles.pPhysicsScene->addAggregateList.insert(c.GetRuntimeData().pAggregate);
	}

	// Mesh Collider

	template <>
	void OnLoaded<MeshCollider>(Component<MeshCollider>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}
		
		auto& rtd = c.GetRuntimeData();

		physx::PxCooking* pCooking = handles.pPhysicsScene->pCooking;
		ASSERT(pCooking != nullptr);
		physx::PxPhysics* pPhysics = handles.pPhysicsScene->pPhysics;
		ASSERT(pPhysics != nullptr);

		Required<RigidBody, FromSelfOrParents> rigidBody;
		handles.GetComponent(c.GetEntity(), rigidBody);
		if (rigidBody->bDynamic && !c->bConvex && !rigidBody->bKinematic)
		{
			// Triangle Mesh collider can only be attached to static actors (this is a limitation of PhysX). Split the mesh into convex submeshes if you REALLY need such a complex
			// collision model in a dynamic entity.
			// The one exception to this rule is kinematic actors
			ASSERT(false);
			c.Modify().bConvex = true;
		}

		if (c->szPakName[0] != '\0')
		{
			usg::string fullName = "Models/";
			fullName += c->szPakName;
			handles.pResourceMgr->LoadPackage(handles.pDevice, fullName.c_str());
		}

		PhysXMeshCache* pMeshCache = handles.pPhysicsScene->pMeshCache;
		if (c->bConvex)
		{
			physx::PxConvexMesh* pConvexMesh = pMeshCache->GetConvexMesh(handles, c->szCollisionModel, c->szMeshName);
			rtd.pConvexMesh = pConvexMesh;
			OnShapeLoaded(c, physx::PxConvexMeshGeometry(pConvexMesh, physx::PxMeshScale(c->fMeshScale)), handles);
			rtd.pShape->setName(c->szCollisionModel);
		}
		else
		{
			ASSERT(c->szMeshName[0]==0 && "Non-convex collision models with bones not supported as of now");
			if (c->szCollisionModel[0])
			{
				ASSERT(rtd.pTriangleMesh == nullptr);
				rtd.pTriangleMesh = pMeshCache->GetTriangleMesh(handles, c->szCollisionModel, c->bFlipNormals);
			}
			else
			{
				ASSERT(rtd.pTriangleMesh != nullptr && "If you do not specify collision mesh filename, you must programmatically create the PxTriangleMesh between OnActivate and OnLoaded.");
			}
			OnShapeLoaded(c, physx::PxTriangleMeshGeometry(rtd.pTriangleMesh, physx::PxMeshScale(c->fMeshScale)), handles);
			if (c->szCollisionModel[0])
			{
				rtd.pShape->setName(c->szCollisionModel);
			}
		}
	}

	template<>
	void OnActivate<MeshCollider>(Component<MeshCollider>& c)
	{
		auto& rtd = c.GetRuntimeData();
		rtd.pConvexMesh = nullptr;
		rtd.pTriangleMesh = nullptr;
		OnActivateShape(c);
	}

	template<>
	void OnDeactivate<MeshCollider>(Component<MeshCollider>& c, ComponentLoadHandles& handles)
	{
		auto& rtd = c.GetRuntimeData();
		OnDeactivateShape(c, handles);
		rtd.pConvexMesh = nullptr;
		rtd.pTriangleMesh = nullptr;
	}

	// Vehicle Collider

	template <>
	void OnLoaded<VehicleCollider>(Component<VehicleCollider>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (bWasPreviouslyCalled)
		{
			return;
		}
		OnVehicleColliderLoaded(c, handles);
		const memsize uWheelCount = c->uNumWheels;
		for (memsize i = 0; i < uWheelCount; i++)
		{
			auto& wheel = c.GetRuntimeData().wheelsData.wheel[i];
			handles.pPhysicsScene->dirtyShapeList.insert((PhysXShapeRuntimeData*)&wheel.rtd);
		}
	}

	template<>
	void OnActivate<VehicleCollider>(Component<VehicleCollider>& c)
	{
		auto& rtd = c.GetRuntimeData();
		OnActivateShape(c);
		OnVehicleColliderActivated(c);
	}

	template<>
	void OnDeactivate<VehicleCollider>(Component<VehicleCollider>& c, ComponentLoadHandles& handles)
	{
		auto& rtd = c.GetRuntimeData();
		OnDeactivateShape(c, handles);
		OnVehicleColliderDeactivated(c, handles);
	}

}

