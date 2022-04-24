/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USAGI_SCENE_COMMON_SCENE_COMPONENTS_H_
#define _USAGI_SCENE_COMMON_SCENE_COMPONENTS_H_

#include "Engine/Framework/Component.h"
#include "Engine/Physics/CollisionMeshHitResult.h"
#include "Engine/Graphics/Lights/LightMgr.h"


namespace usg
{
	class Scene;
	class Light;
	class ShadowCascade;
	class Decal;
	class ModelMgr;

	template<>
	struct RuntimeData<usg::Components::SceneComponent>
	{
		usg::Scene*				pScene;

		usg::LightMgr::QualitySettings	shadowQuality;
		bool							bNeedsShadowUpdate = false;
	};

	template<>
	struct RuntimeData<usg::Components::LightComponent>
	{
		Light* 			pLight;
	};

	template<>
	struct RuntimeData<usg::Components::ShadowDecalComponent>
	{
		usg::Decal*				pDecal;
		usg::Vector3f			vPrevTestPos;
		CollisionMeshHitResult	hitResult;
	};

	template<>
	struct RuntimeData<usg::Components::ModelMgrComponent>
	{
		ModelMgr* 			pMgr;
	};

	class StandardCamera;
	class HMDCamera;

	template<>
	struct RuntimeData<CameraComponent>
	{
		StandardCamera* pCamera;
	};

	template<>
	struct RuntimeData<HMDCameraComponent>
	{
		HMDCamera* pCamera;
	};
}

#endif
