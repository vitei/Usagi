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
	
	class DebugShapeRenderer
	{
	protected:
		static void RenderDebugShape(const DebugRenderData& debug, const usg::Components::Sphere& sphere, const Optional<MatrixComponent>& matrix)
		{
#ifndef FINAL_BUILD
			if (DebugStats::Inst()->GetCurrentType() != debug.page) { return; }

			const Vector3f centre = matrix.Exists()
				? matrix.Force()->matrix.TransformVec3(sphere.centre)
				: sphere.centre;

			DEBUG_PRINT("Rendering sphere at %0.02f %0.02f %0.02f \n", centre.x, centre.y, centre.z);
			Debug3D::GetRenderer()->AddSphere(centre, sphere.radius, debug.color);
#endif
		}
	};

	template<typename Shape>
	class RenderDebug : public System, public DebugShapeRenderer
	{
	public:
		typedef Shape ShapeTP;
		struct Inputs
		{
			Optional<MatrixComponent> matrix;

			Required<DebugRenderData> debug;
			Required<ShapeTP>         shape;
		};

		static const SystemCategory CATEGORY = SYSTEM_DEFAULT_PRIORITY;
		static const char* Name()
		{
			static const char* const name = "RenderDebug"; // Would be good to include the shape name
			return name;
		}

		static uint32 GetSystemId()
		{
			return usg::GetSystemId<RenderDebug<Shape> >();
		}

		static bool GetInputOutputs(ComponentGetter& GetComponent, Inputs& inputs, Outputs& outputs)
		{
			return GetComponent(inputs.matrix) &&
				GetComponent(inputs.debug) &&
				GetComponent(inputs.shape);
		}

		static void Run(const Inputs& in, Outputs&, float)
		{
			RenderDebugShape(*in.debug, *in.shape, in.matrix);
		}
	};

	namespace Systems {
		typedef RenderDebug<usg::Components::Sphere> RenderDebugSphere;
	}

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

#include GENERATED_SYSTEM_CODE(Engine/Debug/DebugSystems.cpp)
