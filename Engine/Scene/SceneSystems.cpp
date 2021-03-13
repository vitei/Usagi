/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Framework/SystemCategories.h"
#include "Engine/Scene/Common/SceneEvents.pb.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Scene/Camera/HMDCamera.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Graphics/Lights/ProjectionLight.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "Engine/Graphics/Lights/PointLight.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Graphics/Shadows/ShadowCascade.h"
#include "Engine/Graphics/GPUUpdate.h"
#include "Engine/Framework/EventManager.h"


namespace usg
{
	namespace Systems
	{
		class ClearScene : public System 
		{
		public:
			struct Outputs
			{
				Required<SceneComponent> sceneComp;
			};

			DECLARE_SYSTEM(SYSTEM_PRE_EARLY)

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				outputs.sceneComp.GetRuntimeData().pScene->PreUpdate();
			}
		};
		class UpdateScene : public System
		{
		public:
			struct Inputs
			{
				Required<SceneComponent> sceneComp;
				Required<ActiveDevice> device;
				Required<SimulationActive, FromSelfOrParents> simactive;
			};

			struct Outputs
			{
				Required<SceneComponent> sceneComp;
			};

			DECLARE_SYSTEM(SYSTEM_SCENE)

			static void LateUpdate(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				Scene* scene = outputs.sceneComp.GetRuntimeData().pScene;
				// FIXME: This should get removed out and put into the matrix hierarchy system
				if (inputs.simactive->bActive)
				{
					scene->TransformUpdate(fDelta);
				}
				else
				{
					scene->TransformUpdate(0.0f);
				}
			}
			static  void GPUUpdate(const Inputs& inputs, Outputs& outputs, GPUHandles* pGPUData)
			{
				Scene* scene = outputs.sceneComp.GetRuntimeData().pScene;
				scene->Update(pGPUData->pDevice);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::SetViewContextMask& event)
			{
				Scene* scene = outputs.sceneComp.GetRuntimeData().pScene;
				ViewContext* view = scene->GetViewContext(event.uContext);
				ASSERT(view!=NULL);
				if (view)
				{
					uint32 uMask = view->GetRenderMask();
					uMask |= event.uEnableBits;
					uMask &= ~event.uDisableBits;
					view->SetRenderMask(uMask);
				}
			}

			static void OnEvent(const Inputs& in, Outputs& out, const EnableCamera& evt)
			{
				Scene* scene = out.sceneComp.GetRuntimeData().pScene;
				scene->SetActiveCamera(evt.uCameraID, evt.uContext);
			}
		};

		class UpdateSceneLight : public System
		{
		public:

			struct Inputs
			{
				Required< Components::MatrixComponent,
				          FromSelfOrParents > mtx;
				Required< LightComponent > light;
				Optional< LightIntensityComponent > intensity;
			};

			struct Outputs
			{
				Required< LightComponent > light;
			};

			DECLARE_SYSTEM(SYSTEM_POST_TRANSFORM)
			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				if (inputs.light.GetRuntimeData().pLight)
				{
					const LightComponent lightIn = *inputs.light;
					Light* pLight = outputs.light.GetRuntimeData().pLight;
					switch (pLight->GetType())
					{
					case LIGHT_TYPE_SPOT:
						pLight->SetDirection( Vector4f(inputs.mtx->matrix.TransformVec3(inputs.light->spec.direction, 0.0f), 0.0f) );
						pLight->SetPosition(inputs.mtx->matrix.vPos());
						break;
					case LIGHT_TYPE_POS:
						pLight->SetPosition(inputs.mtx->matrix.vPos());
						break;
					case LIGHT_TYPE_PROJ:
					{
						ProjectionLight* pProj = (ProjectionLight*)pLight;
						pProj->SetModelMatrix(inputs.mtx->matrix);
						break;
					}
					default:
						break;
					}
					pLight->SwitchOn(true);

					// TODO: This should really be handled with light animations, or moved into a seperate system as we 
					// may want to control the intensity of a light manually with an intensity component
					if (inputs.intensity.Exists())
					{
						const float fIntensity = inputs.intensity.Force()->fIntensity;
						float fFrac = 1.0f;
						if (fIntensity < 1.0f || !inputs.light->bFullIntensity)
						{		
							if (fIntensity > 0.0f)
							{
								pLight->SetAmbient(lightIn.spec.base.ambient * fIntensity);
								pLight->SetDiffuse(lightIn.spec.base.diffuse * fIntensity);
								pLight->SetSpecularColor(lightIn.spec.base.specular * fIntensity);
								// TODO: Make these virtual functions so that we don't have to know
								if (lightIn.spec.base.kind == LightKind_POINT)
								{
									PointLight* pPoint = (PointLight*)pLight;
									pPoint->SetRange(lightIn.spec.atten.fNear * fIntensity, lightIn.spec.atten.fFar * fIntensity);
								}
							}
							else
							{
								pLight->SwitchOn(false);
							}

							outputs.light.Modify().bFullIntensity = fIntensity == 1.0f;
						}
					}

				}
			}
		};

		class UpdateLightFade : public System
		{
		public:

			struct Inputs
			{
				Required< LightFadeComponent > fade;
				Required< LightIntensityComponent > intensity;
				Required< Lifetime > lifeTime;
				Required< MaxLifetime> maxLifeTime;
			};

			struct Outputs
			{
				Required< LightIntensityComponent > intensity;
			};

			DECLARE_SYSTEM(SYSTEM_HIERARCHY)	// Needs to run before the light
			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				float fLifeTime = inputs.lifeTime->fLifetime;
				float fFadeOutStart = inputs.maxLifeTime->fMaxLifetime - inputs.fade->fFadeOutTime;
				float fFade = 1.0f;
				if (fLifeTime < inputs.fade->fFadeInTime)
				{
					fFade = fLifeTime / inputs.fade->fFadeInTime;
				}
				else if (fLifeTime > fFadeOutStart)
				{
					float fFadeTime = fLifeTime - fFadeOutStart;
					fFade = Math::Max(1.0f - (fFadeTime / inputs.fade->fFadeOutTime), 0.0f);
				}

				if (fFade != inputs.intensity->fIntensity)
				{
					outputs.intensity.Modify().fIntensity = fFade;
				}
			}
		};


		class UpdateHMDCameraInternals : public usg::System
		{
		public:
			struct Inputs
			{
				usg::Required<usg::MatrixComponent> mtx;
			};

			struct Outputs
			{
				usg::Required<usg::HMDCameraComponent> cam;
			};

			DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)

			static void LateUpdate(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				outputs.cam.GetRuntimeData().pCamera->SetModelMatrix(inputs.mtx->matrix);
				outputs.cam.GetRuntimeData().pCamera->Update();
			}

		};


		class UpdateSceneCamera : public usg::System
		{
		public:
			struct Inputs
			{
				usg::Required<usg::CameraComponent> cam;
				usg::Required<usg::MatrixComponent> mtx;
			};

			struct Outputs
			{
				usg::Required<usg::CameraComponent> cam;
			};

			DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)

			static void LateUpdate(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				const Vector3f vPos = inputs.mtx->matrix.TransformVec3(Vector3f(0.0f, 0.0f, 0.0f), 1.0f);
				const auto& vUp = inputs.mtx->matrix.vUp().v3().GetNormalised();
				const auto& vForward = inputs.mtx->matrix.vFace().v3().GetNormalised();
				auto& cameraRtd = outputs.cam.GetRuntimeData();
				ASSERT(cameraRtd.pCamera != nullptr);
				Matrix4x4 mtx;
				mtx.LookAt(vPos, vPos + vForward, vUp);
				cameraRtd.pCamera->SetUp(mtx, outputs.cam->fAspectRatio, outputs.cam->fFOV, outputs.cam->fNearPlaneDist, outputs.cam->fFarPlaneDist);
				cameraRtd.pCamera->SetRenderMask(outputs.cam->uRenderMask);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const usg::SetAspectRatio& evt)
			{
				outputs.cam.Modify().fAspectRatio = evt.fAspectRatio;
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const usg::SetFieldOfView& evt)
			{
				outputs.cam.Modify().fFOV = evt.fFOV;
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const usg::AddSubCameraRenderMask& evt)
			{
				if(evt.uCameraID == 0 || outputs.cam.GetRuntimeData().pCamera->GetID() == evt.uCameraID)
				{
					outputs.cam.Modify().uRenderMask = (outputs.cam->uRenderMask | evt.uAddMasks ) & ~evt.uSubMasks;
				}
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const usg::EnableCamera& evt)
			{
				if (evt.uCameraID == outputs.cam->uCamId)
				{
					outputs.cam.Modify().bActive = true;
				}
				else
				{
					outputs.cam.Modify().bActive = false;
				}
			}

		};


	}

}

#include GENERATED_SYSTEM_CODE(Engine/Scene/SceneSystems.cpp)