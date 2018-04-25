#include "Engine/Common/Common.h"
#include "Engine/Physics/PhysicsTools.h"
#include "Engine/Physics/PhysX.h"
#include "Engine/Physics/PhysicsStructs.pb.h"

namespace usg
{
	namespace physics
	{
		namespace details
		{

			uint32 FetchMaterialFlags(physx::PxShape* pShape, uint32 uInternalFaceIndex)
			{
				constexpr uint32 InvalidFaceIndex = 0xffffFFFF;
				ASSERT(pShape != nullptr);
				if (uInternalFaceIndex != InvalidFaceIndex)
				{
					const auto& pMaterial = pShape->getMaterialFromInternalFaceIndex(uInternalFaceIndex);
					if (pMaterial != nullptr && pMaterial->userData != nullptr)
					{
						const auto& m = *(PhysicMaterial*)pMaterial->userData;
						return m.uFlags;
					}
					else
					{
						return (pShape->getSimulationFilterData().word3 & 0xffff);
					}
				}
				return (pShape->getSimulationFilterData().word3 & 0xffff);
			}

		}
	}
}