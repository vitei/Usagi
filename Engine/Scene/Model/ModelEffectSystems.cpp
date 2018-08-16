/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Model/ModelEffectComponents.pb.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Graphics/Fog.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Scene/Model/ModelEffectEvents.pb.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Framework/EventManager.h"

namespace usg
{
	static const uint32 CameraFadeId = utl::CRC32("CameraFadeId");

	namespace Systems
	{

		// FIXME: Refactor so that visibility is cached and processed in the GPU update on models
		class FadeSystem : public usg::System
		{
		public:

			struct Inputs
			{
				Required<usg::ModelComponent> model;
				Required<Components::FadeComponent> fade;
				Required<usg::MatrixComponent> mtx;
				Required<SceneComponent, FromParents> scene;
			};

			struct Outputs
			{
				Required<usg::ModelComponent> model;
				Required<Components::FadeComponent>		  fade;
				Optional<usg::VisibilityComponent> visiblity;
			};

			DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)

			static float ComputeFadeValue(const Vector3f& vObjectPos, const Vector3f& vCamPos, const Vector3f& vCamLookDir, const Vector3f& vCamUpDir, float fModelRadius)
			{
				const float fDynamicRange = 255.0f; // Round return values so that we can skip updating the mesh if fade value changes minimally
				const float fMinFade = 60.0f / fDynamicRange;

				const float fFadeDist = 1.0f;
				const Vector3f vCameraPoint = vCamPos + fFadeDist*vCamLookDir;
				const Vector3f& vDiff = vObjectPos - vCameraPoint;

				if (DotProduct(vDiff, vCamLookDir) > fModelRadius)
				{
					// Fully visible
					return 1;
				}
				else if (DotProduct(vObjectPos-vCameraPoint, vCamLookDir) < -fModelRadius)
				{
					// Fully invisible
					return fMinFade;	// Causing bugs in shadows so returning min fade for now
				}

				const float fAlpha = Math::pi / 3;

				const Vector3f vLeftBoundNormal = -vCamLookDir*Quaternionf(vCamUpDir, Math::pi_over_2 + fAlpha);
				const Vector3f vRightBoundNormal = -vCamLookDir*Quaternionf(vCamUpDir, -Math::pi_over_2 - fAlpha);

				const float fDotLeft = DotProduct(vDiff, vLeftBoundNormal);
				const float fFadeLeft = Math::Clamp01(Math::InverseLerp(0.0f, fModelRadius, fDotLeft));
				if (fFadeLeft > 1 - Math::EPSILON)
				{
					return 1;
				}

				const float fDotRight = DotProduct(vDiff, vRightBoundNormal);
				const float fFadeRight = Math::Clamp01(Math::InverseLerp(0.0f, fModelRadius, fDotRight));
				if (fFadeRight > 1 - Math::EPSILON)
				{
					return 1;
				}

				const float fFade = Math::Max(fFadeLeft, fFadeRight);
				return (float)((int)(Math::Clamp(fFade, fMinFade,1.0f)*fDynamicRange + 0.5f)*1.0f/fDynamicRange);
			}


			static void	Run(const Inputs& inputs, Outputs& outputs, float fElapsed)
			{
				const FadeComponent& fadeIn = *inputs.fade;
				const float fPrevFade = inputs.fade->fCurrentAlpha * inputs.fade->fCurrentAlphaCameraMultip;
				const bool bWasFaded = fPrevFade < 1.0f - Math::EPSILON;
				outputs.fade.GetRuntimeData().Update(fElapsed);
				
				const Camera* pCamera = inputs.scene.GetRuntimeData().pScene->GetSceneCamera(0);
				if (inputs.fade->bFadeNearCamera)
				{
					const float fMeshCollisionSphereRadius = inputs.fade->fCameraFadeRadius > 0 ? inputs.fade->fCameraFadeRadius : inputs.model.GetRuntimeData().pModel->GetResource()->GetBounds().GetRadius();
					const float fCamFade = ComputeFadeValue(inputs.mtx->matrix.vPos().v3(), pCamera->GetPos().v3(), pCamera->GetFacing().v3(), pCamera->GetModelMatrix().vUp().v3(), fMeshCollisionSphereRadius);
					outputs.fade.GetRuntimeData().SetFade(CameraFadeId, fCamFade, 0.0f);
				}

				FadeComponent&	fadeOut = outputs.fade.Modify();
				fadeOut.fCurrentAlphaCameraMultip = outputs.fade.GetRuntimeData().GetCurrentFade();

				float fNewFade = inputs.fade->fCurrentAlpha * inputs.fade->fCurrentAlphaCameraMultip;
				bool bIsFaded = fNewFade < 1.0f - Math::EPSILON;

				float fFogValue = 1.0f;
				if (bIsFaded && fNewFade > Math::EPSILON)
				{
					// FIXME: 3DS hack, we should have proper fog. Remove when we're on better hardware
					Fog& fog = inputs.scene.GetRuntimeData().pScene->GetViewContext(0)->GetFog();
					float fFogRange = fog.GetMaxDepth() - fog.GetMinDepth();
					const float fDistance = Math::Clamp(inputs.mtx->matrix.vPos().GetDistanceFrom(pCamera->GetPos()) - fog.GetMinDepth(), 0.0f, fFogRange) / fFogRange;
					fFogValue = (1.0f - fDistance);
				}

				bool bSomethingChanged = false;
				if (bWasFaded && !bIsFaded)
				{
					outputs.model.GetRuntimeData().pModel->SetFade(false);
					bSomethingChanged = true;
				}
				else
				{
					if (fNewFade != fPrevFade)
					{
						UpdateFadeValue(inputs, outputs, fFogValue);
						bSomethingChanged = true;
					}
				}
			}

			static void		OnEvent(const Inputs& inputs, Outputs& outputs, const SetFadeMultiplier& evt)
			{
				for(pb_size_t i=0; i<inputs.fade->uIgnoreMessages_count; i++)
				{
					if(evt.uId == inputs.fade->uIgnoreMessages[i])
					{
						// We ignore this one
						return;
					}
				}
				FadeComponent& fade = outputs.fade.Modify();
				RuntimeData<FadeComponent>& fadeData = outputs.fade.GetRuntimeData();
				fadeData.SetFade(evt.uId, evt.fTargetFade, evt.fFadeTime);
			}

		private:
			static void UpdateFadeValue(const Inputs& inputs, Outputs& outputs, float fFogValue)
			{
				float fValue = inputs.fade->fCurrentAlpha * inputs.fade->fCurrentAlphaCameraMultip * fFogValue;

				usg::Model* pModel = outputs.model.GetRuntimeData().pModel;
				pModel->SetFade(fValue != 1.0f, fValue);
			}	
		};

		// For fading out something as it's about to be cleaned up
		class TriggerFadeSystem : public usg::System
		{
		public:
			struct Inputs
			{
				Required<usg::EntityID>		self;
				Required<TriggerFadeOut>	trigger;
				Required< Lifetime >		lifeTime;
				Required< MaxLifetime>		maxLifeTime;
				Required< EventManagerHandle, FromSelfOrParents > eventManager;
			};

			struct Outputs
			{
				Required< TriggerFadeOut > trigger;
			};

			DECLARE_SYSTEM(SYSTEM_HIERARCHY)	
			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				float fTimeToCleanup = inputs.maxLifeTime->fMaxLifetime - inputs.lifeTime->fLifetime;
				if (inputs.trigger->bTriggered == false && fTimeToCleanup < inputs.trigger->fFadeOutTime)
				{
					SetFadeMultiplier TriggerEffect;
					TriggerEffect.fFadeTime = inputs.trigger->fFadeOutTime;
					TriggerEffect.uId = utl::CRC32("TriggerFade");
					TriggerEffect.fTargetFade = 0.0f;
					outputs.trigger.Modify().bTriggered = true;
					inputs.eventManager->handle->RegisterEventWithEntity(inputs.self->id, TriggerEffect, ON_ENTITY | ON_CHILDREN);
				}
			}
		};
	}

}

#include GENERATED_SYSTEM_CODE(Engine/Scene/Model/ModelEffectSystems.cpp)

