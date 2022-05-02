/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/Signal.h"
#include "Engine/Framework/SystemCategories.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Physics/CollisionData.pb.h"
#include "Engine/Debug/DebugStats.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Debug/DebugComponents.pb.h"
#include "Engine/Debug/DebugCamera.h"
#include "Engine/Debug/DebugEvents.pb.h"
#include "Engine/Scene/Common/SceneEvents.pb.h"
#include "Engine/Scene/Camera/StandardCamera.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Debug/DebugComponents.pb.h"
#include "Engine/Framework/SystemCoordinator.h"

namespace usg
{
	

	namespace Systems
	{


		class DebugPauseEventHandler : public usg::System
		{
		public:
			struct Outputs
			{
				Required<usg::SimulationActive> sim;
				Required<usg::SceneComponent> scene;
			};

			DECLARE_SYSTEM(usg::SYSTEM_PRE_EARLY)
			static void OnEvent(const Inputs& inputs, Outputs& outputs, const RequestDebugCameraState_Internal& debugCamera)
			{
#ifdef DEBUG_BUILD
				outputs.sim.Modify().bActive = !debugCamera.bEnable;
				if (debugCamera.bEnable)
				{
					outputs.scene.GetRuntimeData().pScene->SetActiveCamera(utl::CRC32("DebugCam"), 0);
				}
#endif
			}
		};



		class UpdateDebugCamera : public System
		{
		public:
			struct Inputs
			{
				Required<DebugCameraComponent> debug;
				Required< SystemTimeComponent, FromParents> systemTime;
				Required< SceneComponent, FromParents> scene;
				Required< EntityID, FromParentWith<SceneComponent> > sceneEntity;

				Required<EventManagerHandle, FromSelfOrParents> eventManager;
			};

			struct Outputs
			{
				Required<DebugCameraComponent> debug;
				Required<MatrixComponent>      matrix;
			};

			DECLARE_SYSTEM(SYSTEM_DEFAULT_PRIORITY)

			static void Run(const Inputs& in, Outputs& outputs, float dt)
			{
				if (in.debug.GetRuntimeData().pDebugCam->GetActive())
				{
					DebugCamera* pDebugCam = outputs.debug.GetRuntimeData().pDebugCam;
					pDebugCam->Update(in.systemTime->fSystemElapsed);
					outputs.matrix.Modify().matrix = pDebugCam->GetModelMat();
				}
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::RequestDebugCameraState& event)
			{

				if (event.bEnable)
				{
					outputs.debug.Modify().uPrevCamID = inputs.scene.GetRuntimeData().pScene->GetSceneCamera(0)->GetID();
					outputs.debug.GetRuntimeData().pDebugCam->SetMatrix(inputs.scene.GetRuntimeData().pScene->GetSceneCamera(0)->GetModelMatrix());
				}
				else
				{
					EnableCamera enableCamera;
					enableCamera.uCameraID = inputs.debug->uPrevCamID;
					enableCamera.uContext = 0;
					inputs.eventManager->handle->RegisterEvent(enableCamera);
				}
				outputs.debug.GetRuntimeData().pDebugCam->SetActive(event.bEnable);
				RequestDebugCameraState_Internal intEvt;
				intEvt.bEnable = event.bEnable;
				inputs.eventManager->handle->RegisterEventWithEntity(inputs.sceneEntity->id, intEvt);
			}
		};

	}


}

#include GENERATED_SYSTEM_CODE(Engine/Debug/DebugSystems.cpp)
