#include "Engine/Common/Common.h"
#include "Engine/Physics/PhysXVehicle/VehicleSceneQueryData.h"
#include "Engine/Physics/CollisionMask.pb.h"
#include "Engine/Physics/CollisionQuadTree.h"

namespace usg
{

	physx::PxQueryHitType::Enum WheelRaycastPreFilter(physx::PxFilterData filterData0, physx::PxFilterData filterData1, const void* constantBlock, physx::PxU32 constantBlockSize, physx::PxHitFlags& queryFlags)
	{
		const bool bIsDrivableSurface = (filterData1.word0 & COLLM_DRIVABLE_SURFACE) != 0;
		return bIsDrivableSurface ? physx::PxQueryHitType::eBLOCK : physx::PxQueryHitType::eNONE;
	}

	VehicleSceneQueryData::VehicleSceneQueryData() : mNumRaycastsPerBatch(0), mSqResults(nullptr), mSqHitBuffer(nullptr), mPreFilterShader(WheelRaycastPreFilter)
	{

	}

	VehicleSceneQueryData::~VehicleSceneQueryData()
	{

	}

	void VehicleSceneQueryData::Free(physx::PxAllocatorCallback& allocator)
	{
		allocator.deallocate(this);
	}

	VehicleSceneQueryData* VehicleSceneQueryData::Allocate(const physx::PxU32 maxNumVehicles, const physx::PxU32 maxNumWheelsPerVehicle, const physx::PxU32 numVehiclesInBatch, physx::PxAllocatorCallback& allocator)
	{
		static_assert(0 == (sizeof(physx::PxRaycastQueryResult) & 15),"");
		static_assert(0 == (sizeof(physx::PxRaycastHit) & 15),"");

		const physx::PxU32 sqDataSize = ((sizeof(VehicleSceneQueryData) + 15) & ~15);

		const physx::PxU32 maxNumWheels = maxNumVehicles*maxNumWheelsPerVehicle;

		const physx::PxU32 size = sqDataSize + sizeof(physx::PxRaycastQueryResult)*maxNumWheels + sizeof(physx::PxRaycastHit)*maxNumWheels;
		physx::PxU8* buffer = (physx::PxU8*)allocator.allocate(size, NULL, NULL, 0);

		VehicleSceneQueryData* sqData = new(buffer) VehicleSceneQueryData();
		buffer += sqDataSize;

		sqData->mNumRaycastsPerBatch = numVehiclesInBatch*maxNumWheelsPerVehicle;

		sqData->mSqResults = (physx::PxRaycastQueryResult*)buffer;
		buffer += sizeof(physx::PxRaycastQueryResult)*maxNumWheels;

		sqData->mSqHitBuffer = (physx::PxRaycastHit*)buffer;
		buffer += sizeof(physx::PxRaycastHit)*maxNumWheels;

		for (physx::PxU32 i = 0; i < maxNumVehicles; i++)
		{
			for (physx::PxU32 j = 0; j < maxNumWheelsPerVehicle; j++)
			{
				ASSERT((size_t)(sqData->mSqResults + i*maxNumWheelsPerVehicle + j) < (size_t)buffer);
				ASSERT((size_t)(sqData->mSqHitBuffer + i*maxNumWheelsPerVehicle + j) < (size_t)buffer);
				new(sqData->mSqResults + i*maxNumWheelsPerVehicle + j) physx::PxRaycastQueryResult();
				new(sqData->mSqHitBuffer + i*maxNumWheelsPerVehicle + j) physx::PxRaycastHit();
			}
		}

		return sqData;
	}

	physx::PxBatchQuery* VehicleSceneQueryData::SetUpBatchedSceneQuery(const physx::PxU32 uBatchId, const VehicleSceneQueryData& vehicleSceneQueryData, physx::PxScene* scene)
	{
		const physx::PxU32 maxNumRaycastsInBatch = vehicleSceneQueryData.mNumRaycastsPerBatch;
		physx::PxBatchQueryDesc sqDesc(maxNumRaycastsInBatch, 0, 0);
		sqDesc.queryMemory.userRaycastResultBuffer = vehicleSceneQueryData.mSqResults + uBatchId*maxNumRaycastsInBatch;
		sqDesc.queryMemory.userRaycastTouchBuffer = vehicleSceneQueryData.mSqHitBuffer + uBatchId*maxNumRaycastsInBatch;
		sqDesc.queryMemory.raycastTouchBufferSize = maxNumRaycastsInBatch;
		sqDesc.preFilterShader = vehicleSceneQueryData.mPreFilterShader;
		return scene->createBatchQuery(sqDesc);
	}

	physx::PxRaycastQueryResult* VehicleSceneQueryData::GetRaycastQueryResultBuffer(const physx::PxU32 batchId)
	{
		return (mSqResults + batchId*mNumRaycastsPerBatch);
	}

	physx::PxU32 VehicleSceneQueryData::GetRaycastQueryResultBufferSize() const
	{
		return mNumRaycastsPerBatch;
	}

}