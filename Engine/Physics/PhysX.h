#ifndef _USG_PHYSICS_PHYSICS_PHYSX_H_
#define _USG_PHYSICS_PHYSICS_PHYSX_H_

#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector3f.h"
#include "Engine/Maths/Quaternionf.h"
#include "Engine/Framework/Component.h"
#include "PxPhysicsAPI.h"

namespace usg
{
	class SystemCoordinator;

	namespace Components
	{
		typedef struct _PhysicsScene PhysicsScene;
		typedef struct _TransformComponent TransformComponent;
		typedef struct _MatrixComponent MatrixComponent;
		typedef struct _CollisionMasks CollisionMasks;
		typedef struct _RigidBody RigidBody;
	}

	namespace physics
	{
		constexpr float32 DefaultTimeStep = 1.0f / 60.0f;

		void SetTimeStep(float32 fStep);
		float32 GetTimeStep();

		Components::TransformComponent ToUsgTransform(const physx::PxTransform& t);

		void init();
		void deinit();
	}

	typedef struct _PhysicMaterial PhysicMaterial;

	struct PhysicsConstants
	{
		static constexpr uint32 MaxNumAsyncRaycastsPerFrame = 256;
		static constexpr uint32 RaycastWorkBufferSize = 4096;
		static constexpr uint32 VehicleMaxNumWheels = PX_MAX_NB_WHEELS;
	};

	class PhysXErrorCallback : public physx::PxErrorCallback
	{
	public:

		virtual ~PhysXErrorCallback() {}

		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
		{
			DEBUG_PRINT("PhysX Error %d: %s (%s:%d)\n", code, message, file, line);
		}

	};

	class PhysXAllocator : public physx::PxAllocatorCallback
	{
	public:
		void* allocate(size_t size, const char*, const char*, int)
		{
			void* ptr = mem::Alloc(MEMTYPE_STANDARD, ALLOC_PHYSICS, size, 16, false);
			PX_ASSERT((reinterpret_cast<size_t>(ptr) & 15) == 0);
			return ptr;
		}

		void deallocate(void* ptr)
		{
			mem::Free(ptr);
		}
	};

	enum class ShapeBitmask : uint32
	{
		DisableImpulses = (uint32)(1 << 30),
		CCD = (uint32)(1 << 29),
		EnableContactModification = (uint32)(1 << 28),
	};

	// Conversion functions between usg and PhysX types

	static_assert(sizeof(physx::PxVec3) == sizeof(Vector3f), "Size mismatch between usg::Vector3f and physx::PxVec3");
	static_assert(sizeof(physx::PxQuat) == sizeof(Quaternionf), "Size mismatch between usg::Quaternionf and physx::PxQuat");

	inline const physx::PxVec3& ToPhysXVec3(const Vector3f& v)
	{
		return *(const physx::PxVec3*)(&v);
	}

	inline const Vector3f& ToUsgVec3(const physx::PxVec3& v)
	{
		return *(const Vector3f*)(&v);
	}

	inline const physx::PxQuat& ToPhysXQuaternion(const Quaternionf& q)
	{
		return *(const physx::PxQuat*)(&q);
	}

	inline const Quaternionf& ToUsgQuaternionf(const physx::PxQuat& q)
	{
		return *(const Quaternionf*)(&q);
	}

	physx::PxTransform ToPhysXTransform(const Components::TransformComponent& trans);
	physx::PxTransform ToPhysXTransform(const Components::MatrixComponent& matrix);

	// Physics world initialization / deinitialization
	
	void InitPhysicsScene(Component<usg::Components::PhysicsScene>& p);
	void DeinitPhysicsScene(Component<usg::Components::PhysicsScene>& p);

	// Material handling

	void ApplyMaterial(physx::PxMaterial* pMaterial, const PhysicMaterial& m);

	// Misc

	void UpdateSimulationFilter(physx::PxShape* pShape, const usg::Components::CollisionMasks* pMasks, const usg::Components::RigidBody* pBody, Entity aggregateEntity);

	void GenerateOnCollisionSignals(SystemCoordinator& systemCoordinator, Required<usg::Components::PhysicsScene> scene);

}

#endif