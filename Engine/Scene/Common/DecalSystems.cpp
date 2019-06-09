/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Common/SceneEvents.pb.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Graphics/GPUUpdate.h"
#include "Engine/Scene/Common/GroundDecals.h"

namespace usg {

	namespace Systems
	{
		class SpawnDecals : public usg::System
		{
		public:
			struct Inputs
			{
				// FIXME: Add GPU events and have the device passed in on them
				Required<ActiveDevice, FromSelfOrParents> device;
				Required<usg::SimulationActive, FromSelfOrParents> simactive;
			};

			struct Outputs
			{
				Required<GroundDecalsHandle> groundDecalsHandle;
			};

			DECLARE_SYSTEM(usg::SYSTEM_TRANSFORM)
			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta);
			//static void OnCollision( const Inputs& inputs, Outputs& outputs, const ColliderInputs& colliderInputs, const Collision& collision );
			static void OnEvent(const Inputs& inputs, Outputs& outputs, const CreateDecalEvent& evt);
			static  void GPUUpdate(const Inputs& inputs, Outputs& outputs, GPUHandles* pGPUData);
		};


		void SpawnDecals::Run(const Inputs& inputs, Outputs& outputs, float fDelta)
		{
			outputs.groundDecalsHandle.Modify().pGroundDecals->Update(inputs.simactive->bActive ? fDelta : 0.0f);
		}

#if 0
		void GroundSystem::OnCollision(const Inputs& inputs, Outputs& outputs, const ColliderInputs& colliderInputs, const Collision& collision)
		{
			if (collision.material_flags & (CollisionQuadTree::MF_NOFIRE | CollisionQuadTree::MF_WATER))
				return;

			//DEBUG_PRINT( "OnCollision\n" );
			Vector3f vFrom, vTo;
			vFrom = collision.intersect_point - (collision.normal * 5.0f);
			vTo = collision.intersect_point + (collision.normal * 5.0f);
			outputs.groundDecalsHandle.Modify().pGroundDecals->AddDecal(inputs.device.GetRuntimeData().pDevice, vFrom, vTo);
		}
#endif

		void SpawnDecals::OnEvent(const Inputs& inputs, Outputs& outputs, const CreateDecalEvent& evt)
		{
			outputs.groundDecalsHandle.Modify().pGroundDecals->AddDecal(inputs.device.GetRuntimeData().pDevice, evt.vStartPos, evt.vEndPos, evt.fSize, evt.uType);
		}

		void SpawnDecals::GPUUpdate(const Inputs& inputs, Outputs& outputs, GPUHandles* pGPUData)
		{
			outputs.groundDecalsHandle.Modify().pGroundDecals->UpdateBuffers(pGPUData->pDevice);
		}

	}
}

#include GENERATED_SYSTEM_CODE(Engine/Scene/Common/DecalSystems.cpp)