#pragma once


#include "Engine/Physics/PhysX.h"

namespace usg
{

	enum
	{
		DRIVABLE_SURFACE = 0xffff0000,
		UNDRIVABLE_SURFACE = 0x0000ffff
	};

	void setupDrivableSurface(physx::PxFilterData& filterData);

	void setupNonDrivableSurface(physx::PxFilterData& filterData);

	physx::PxQueryHitType::Enum WheelRaycastPreFilter(physx::PxFilterData filterData0, physx::PxFilterData filterData1, const void* constantBlock, physx::PxU32 constantBlockSize, physx::PxHitFlags& queryFlags);

	class VehicleSceneQueryData
	{
	public:
		VehicleSceneQueryData();
		~VehicleSceneQueryData();

		//Allocate scene query data for up to maxNumVehicles and up to maxNumWheelsPerVehicle with numVehiclesInBatch per batch query.
		static VehicleSceneQueryData* Allocate(const physx::PxU32 uMaxNumVehicles, const physx::PxU32 maxNumWheelsPerVehicle, const physx::PxU32 numVehiclesInBatch, physx::PxAllocatorCallback& allocator);

		//Free allocated buffers.
		void Free(physx::PxAllocatorCallback& allocator);

		//Create a PxBatchQuery instance that will be used for a single specified batch.
		static physx::PxBatchQuery* SetUpBatchedSceneQuery(const physx::PxU32 batchId, const VehicleSceneQueryData& vehicleSceneQueryData, physx::PxScene* scene);

		//Return an array of scene query results for a single specified batch.
		physx::PxRaycastQueryResult* GetRaycastQueryResultBuffer(const physx::PxU32 batchId);

		//Get the number of scene query results that have been allocated for a single batch.
		physx::PxU32 GetRaycastQueryResultBufferSize() const;

	private:

		//Number of raycasts per batch
		physx::PxU32 mNumRaycastsPerBatch;

		//One result for each wheel.
		physx::PxRaycastQueryResult* mSqResults;

		//One hit for each wheel.
		physx::PxRaycastHit* mSqHitBuffer;

		//Filter shader used to filter drivable and non-drivable surfaces
		physx::PxBatchQueryPreFilterShader mPreFilterShader;
	};

}