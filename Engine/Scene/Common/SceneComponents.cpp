#include "Engine/Common/Common.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Framework/ModelMgr.h"
#include "Engine/Graphics/Shadows/ShadowCascade.h"
#include "Engine/Scene/Common/Decal.h"
#include "Engine/Scene/Camera/HMDCamera.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Scene/Common/GroundDecals.h"
#include "Engine/Graphics/Lights/Light.h"
#include "Engine/Graphics/Lights/LightMgr.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Scene/Common/SceneEvents.pb.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/ComponentLoadHandles.h"

namespace usg
{

	template<>
	void OnActivate<SceneComponent>(Component<SceneComponent>& p)
	{
		p.GetRuntimeData().pScene = nullptr;
	}

	template<>
	void OnLoaded<SceneComponent>(Component<SceneComponent>& p, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		if (!bWasPreviouslyCalled)
		{
			p.GetRuntimeData().pScene = vnew(ALLOC_OBJECT) Scene;
			handles.pScene = p.GetRuntimeData().pScene;
		}
	}

	template<>
	void OnDeactivate<SceneComponent>(Component<SceneComponent>& p, ComponentLoadHandles& handles)
	{
		if(p.GetRuntimeData().pScene)
		{
			p.GetRuntimeData().pScene->Cleanup(handles.pDevice);
			if (handles.pScene == p.GetRuntimeData().pScene)
			{
				handles.pScene = nullptr;
			}
			vdelete p.GetRuntimeData().pScene;
			p.GetRuntimeData().pScene = NULL;
		}
	}

	template<>
	void OnActivate<LightComponent>(Component<LightComponent>& p)
	{
		p.GetRuntimeData().pLight = NULL;
	}

	template<>
	void OnLoaded<LightComponent>(Component<LightComponent>& p, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
	{
		bool bLightNull = p.GetRuntimeData().pLight == NULL;

		if (bLightNull)
		{
			p.GetRuntimeData().pLight = handles.pScene->GetLightMgr().CreateLight(handles.pDevice, handles.pResourceMgr, p.GetData().spec);
			// We've just initialised to the default values
			p.GetRuntimeData().pLight->SwitchOn(false);	// Position may not be valid yet
			p.GetData().bFullIntensity = true;
		}
	}

	template<>
	void OnDeactivate<LightComponent>(Component<LightComponent>& p, ComponentLoadHandles& handles)
	{
		if (p.GetRuntimeData().pLight != NULL)
		{
			handles.pScene->GetLightMgr().RemoveLight(p.GetRuntimeData().pLight);
			p.GetRuntimeData().pLight = NULL;
		}
	}


	// FIXME: New resource management system so that none of this happens inside of the
	// systems
	template<>
	void OnLoaded<ShadowDecalComponent>(Component<ShadowDecalComponent>& p, ComponentLoadHandles& handles,
	                                    bool bWasPreviouslyCalled)
	{
		// TODO: check bWasPreviouslyCalled ?
		Required<usg::GroundDecalsHandle, FromSelfOrParents> Decals;
		handles.GetComponent(p.GetEntity(), Decals);

		// 20160129 n-heckel: This is a temporary work-around until levels are used for ModeGarage.
		if(Decals.IsValid())
		{
			p.GetRuntimeData().pDecal = Decals->pGroundDecals->GetShadowDecal(p->name);
			p.GetRuntimeData().vPrevTestPos.Assign(2000000.f, 2000000.f, 2000000.f);
		}
		else
		{
			p.GetRuntimeData().pDecal = NULL;
		}
	}

	template<>
	void OnDeactivate<ShadowDecalComponent>(Component<ShadowDecalComponent>& p, ComponentLoadHandles& handles)
	{
		Required<usg::GroundDecalsHandle, FromSelfOrParents> Decals;
		handles.GetComponent(p.GetEntity(), Decals);

		if(Decals.IsValid())
		{
			Decals->pGroundDecals->FreeShadowDecal(p.GetRuntimeData().pDecal);
			p.GetRuntimeData().pDecal = NULL;
		}
	}


	template<>
	void OnActivate<ModelMgrComponent>(Component<ModelMgrComponent>& p)
	{
		p.GetRuntimeData().pMgr = NULL;
	}

	template<>
	void OnLoaded<ModelMgrComponent>(Component<ModelMgrComponent>& p, ComponentLoadHandles& handles,
		bool bWasPreviouslyCalled)
	{
		if (!bWasPreviouslyCalled)
		{
			p.GetRuntimeData().pMgr = vnew(ALLOC_OBJECT) ModelMgr;
			p.GetRuntimeData().pMgr->Init(handles.pDevice, handles.pScene);
			handles.pModelMgr = p.GetRuntimeData().pMgr;
		}
	}

	template<>
	void OnDeactivate<ModelMgrComponent>(Component<ModelMgrComponent>& p, ComponentLoadHandles& handles)
	{
		if (p.GetRuntimeData().pMgr != NULL)
		{
			p.GetRuntimeData().pMgr->Destroy(handles.pDevice);
			if (handles.pModelMgr == p.GetRuntimeData().pMgr)
			{
				handles.pModelMgr = nullptr;
			}
			vdelete p.GetRuntimeData().pMgr;
			p.GetRuntimeData().pMgr = nullptr;
		}
	}

	template<>
	void OnLoaded<CameraComponent>(Component<CameraComponent>& p, ComponentLoadHandles& handles,
		bool bWasPreviouslyCalled)
	{
		bool bIsNull = p.GetRuntimeData().pCamera == NULL;

		if (bWasPreviouslyCalled && !bIsNull)
		{
			return;
		}

		usg::Matrix4x4 m = usg::Matrix4x4::Identity();
		p.GetRuntimeData().pCamera = vnew(ALLOC_OBJECT) usg::StandardCamera;
		p.GetRuntimeData().pCamera->SetUp(m, p->fAspectRatio, p->fFOV, p->fNearPlaneDist, p->fFarPlaneDist);
		p.GetRuntimeData().pCamera->SetRenderMask(p->uRenderMask);
		p.GetRuntimeData().pCamera->SetID(p->uCamId);

		handles.pScene->AddCamera(p.GetRuntimeData().pCamera);
		if(p->bForceSwitch)
		{
			handles.pScene->SetActiveCamera(p->uCamId, 0);
		}
		p.Modify().bActive = handles.pScene->GetViewContext(0)->GetCamera() == p.GetRuntimeData().pCamera;

	}

	template<>
	void OnActivate<CameraComponent>(Component<CameraComponent>& p)
	{
		p.GetRuntimeData().pCamera = nullptr;
	}

	template<>
	void OnDeactivate<CameraComponent>(Component<CameraComponent>& p, ComponentLoadHandles& handles)
	{
		if (p.GetRuntimeData().pCamera)
		{
			vdelete p.GetRuntimeData().pCamera;
			p.GetRuntimeData().pCamera = nullptr;
		}
	}


	template<>
	void OnLoaded<HMDCameraComponent>(Component<HMDCameraComponent>& p, ComponentLoadHandles& handles,
		bool bWasPreviouslyCalled)
	{
		bool bIsNull = p.GetRuntimeData().pCamera == NULL;

		if (bWasPreviouslyCalled && !bIsNull)
		{
			return;
		}

		p.GetRuntimeData().pCamera = vnew(ALLOC_OBJECT) HMDCamera;
		if (handles.pDevice)
		{
			p.GetRuntimeData().pCamera->Init(handles.pDevice->GetHMD(), p->fNearPlaneDist, p->fFarPlaneDist);
		}
		p.GetRuntimeData().pCamera->SetRenderMask(p->uRenderMask);
		p.GetRuntimeData().pCamera->SetID(p->uCamId);

		handles.pScene->AddCamera(p.GetRuntimeData().pCamera);
	}

	template<>
	void OnActivate<HMDCameraComponent>(Component<HMDCameraComponent>& p)
	{
		p.GetRuntimeData().pCamera = nullptr;
	}

	template<>
	void OnDeactivate<HMDCameraComponent>(Component<HMDCameraComponent>& p, ComponentLoadHandles& handles)
	{
		if (p.GetRuntimeData().pCamera)
		{
			handles.pScene->RemoveCamera(p.GetRuntimeData().pCamera);
			vdelete p.GetRuntimeData().pCamera;
			p.GetRuntimeData().pCamera = nullptr;
		}
	}

}
