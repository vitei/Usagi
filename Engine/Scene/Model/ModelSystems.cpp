/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Framework/SystemCategories.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/ExclusionCheck.h"
#include "Engine/Resource/SkeletalAnimationResource.h"
#include "Engine/Scene/Model/ModelAnimPlayer.h"
#include "Engine/Scene/Model/Bone.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Model/ModelRenderNodes.h"
#include "Engine/Scene/Model/UVMapper.h"
#include "Engine/Scene/Model/ModelEvents.pb.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Core/stl/utility.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Graphics/GPUUpdate.h"


namespace usg
{

	class BoneTransformInterface
	{
	protected:

		template<typename INPUTS, typename OUTPUTS>
		static void LateUpdateInt(const INPUTS& inputs, OUTPUTS& outputs)
		{
			outputs.bone.GetRuntimeData().pBone->SetTransform(inputs.mtx->matrix, inputs.mtx.Dirty());
		}

		template<typename INPUTS, typename OUTPUTS>
		static  void GPUUpdateInt(const INPUTS& inputs, OUTPUTS& outputs, GPUHandles* pGPUData)
		{
			outputs.bone.GetRuntimeData().pBone->UpdateConstants(pGPUData->pDevice);
		}

	};

	namespace Systems
	{
		class UpdateModel : public System
		{
		public:
			struct Inputs
			{
				Required<MatrixComponent>	matrix;
				Required<ModelComponent>	model;
				Optional<VisibilityComponent, FromSelfOrParents> visibility;
		    };

			struct Outputs
			{
				Required<ModelComponent>							model;
		    };

			DECLARE_SYSTEM(SYSTEM_POST_TRANSFORM)
		    static void LateUpdate(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				if (inputs.visibility.Exists() && inputs.model.GetRuntimeData().pModel->ShouldDraw() != inputs.visibility.Force()->bVisible)
				{
					outputs.model.GetRuntimeData().pModel->AddToScene(inputs.visibility.Force()->bVisible);
				}
				outputs.model.GetRuntimeData().pModel->SetTransform(inputs.matrix->matrix);
				
			}

			static  void GPUUpdate(const Inputs& inputs, Outputs& outputs, GPUHandles* pGPUData)
			{
				outputs.model.GetRuntimeData().pModel->GPUUpdate(pGPUData->pDevice);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::RotateUV& event)
			{
				// TODO: Maybe we should be caching this UV mapper?
				for (uint32 i = 0; i < event.uUVCount; i++)
				{
					UVMapper* pMapper = outputs.model.GetRuntimeData().pModel->GetUVMapper(event.uMeshIndex, event.uTexIndex + i);
					pMapper->SetUVRotation(event.fRotation);
				}
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::TranslateUV& event)
			{
				// TODO: Maybe we should be caching this UV mapper?
				for (uint32 i = 0; i < event.uUVCount; i++)
				{
					UVMapper* pMapper = outputs.model.GetRuntimeData().pModel->GetUVMapper(event.uMeshIndex, event.uTexIndex + i);
					pMapper->SetUVTranslation(event.vTranslation);
				}
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::ScaleModel& event)
			{
				outputs.model.GetRuntimeData().pModel->SetScale(event.fScale);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::OverrideModelVectorEvent& event)
			{
				outputs.model.GetRuntimeData().pModel->OverrideVariable(event.varName, event.vVector, 0);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::OverrideModelScalarEvent& event)
			{
				outputs.model.GetRuntimeData().pModel->OverrideVariable(event.varName, event.fScalar, 0);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::OverrideModelBoolEvent& event)
			{
				outputs.model.GetRuntimeData().pModel->OverrideVariable(event.varName, event.bBool, 0);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::UpdateModelRenderMask& event)
			{
				outputs.model.GetRuntimeData().pModel->SetRenderMask(event.uRenderMask);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const usg::AddSubModelRenderMask& evt)
			{
				outputs.model.GetRuntimeData().pModel->SetRenderMask( (outputs.model.GetRuntimeData().pModel->GetRenderMask() | evt.uAddRenderMask) & ~evt.uSubRenderMask);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::SetMeshRenderLayer& event)
			{
				Model::RenderMesh* Mesh = outputs.model.GetRuntimeData().pModel->GetRenderMesh(event.meshName);
				if (Mesh)
				{
					Mesh->SetLayer(event.eLayer);
				}
			}
		};
		
		class TranslateModelUVs : public System
		{
		public:
			struct Inputs
			{
				Required< EventManagerHandle, FromSelfOrParents > eventManager;
				Required< EntityID, FromParentWith<ModelComponent> > target;
				Required< UVTranslation >		translation;
				Required<InputScale, FromSelfOrParents>	multiplier;
				Required<usg::SimulationActive, FromParents> simactive;
			};

			struct Outputs
			{
				 Required< UVTranslation > translation;
			};	

			DECLARE_SYSTEM(SYSTEM_SCENE)
			static  void	Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				if (!inputs.simactive->bActive)
					return;

				float fMultiplier = inputs.multiplier->fValue;
				Vector2f vTranslation = inputs.translation->vTranslation + (inputs.translation->vMultiplier * fMultiplier);
				vTranslation.x = Math::WrapValue(vTranslation.x, -10.0f, 10.0f);
				vTranslation.y = Math::WrapValue(vTranslation.y, -10.0f, 10.0f);
				outputs.translation.Modify().vTranslation = vTranslation;
				Events::TranslateUV modifyUV = { vTranslation, inputs.translation->identifier.uTexIndex, inputs.translation->identifier.uMeshIndex, inputs.translation->identifier.uUVCount };
				if (modifyUV.uMeshIndex != USG_INVALID_ID)
				{
					inputs.eventManager->handle->RegisterEventWithEntity(*inputs.target, modifyUV);
				}
			}
		};

		class RotateModelUVs : public System
		{
		public:
			struct Inputs
			{
				Required< EventManagerHandle, FromSelfOrParents > eventManager;
				Required< EntityID, FromParentWith<ModelComponent> > target;
				Required<usg::SimulationActive, FromParents> simactive;
				Required< UVRotation > rotation;
				// TODO: Templatize this similar to the collisions so that we can base this on any
				// modifier we choose
				Required<InputScale, FromParents> multiplier;
			};

			struct Outputs
			{
				 Required< UVRotation > rotation;
			};

			DECLARE_SYSTEM(SYSTEM_SCENE)
			static  void	Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				if (!inputs.simactive->bActive)
					return;

				float fMultiplier = inputs.multiplier->fValue;
				outputs.rotation.Modify().fRotation = inputs.rotation->fRotation + fMultiplier * (inputs.rotation->fMultiplier);
				Events::RotateUV modifyUV = {inputs.rotation->fRotation, inputs.rotation->identifier.uTexIndex, inputs.rotation->identifier.uMeshIndex, inputs.rotation->identifier.uUVCount };
				if (modifyUV.uMeshIndex != USG_INVALID_ID)
				{
					inputs.eventManager->handle->RegisterEventWithEntity(*inputs.target, modifyUV);
				}
			}
		};

		class UpdateModelAnimation : public System
		{
		public:
			struct Inputs
			{
				Required<ModelAnimComponent>		anim;
				Required<usg::SimulationActive, FromParents> simactive;
		    };

			struct Outputs
			{
				Required<ModelAnimComponent>		anim;
				Optional<VisibilityComponent>		visibility;
				Optional<ModelComponent>			model;
		    };

			DECLARE_SYSTEM(SYSTEM_EARLY)
		    static void	Run(const Inputs& inputs, Outputs& outputs, float fDelta)
		    {
				if (!inputs.simactive->bActive)
					return;

		    	// FIXME: Should ideally be frame based rather than frame based
				outputs.anim.GetRuntimeData().pAnimPlayer->Update(fDelta);

				if (outputs.visibility.Exists())
				{
					outputs.visibility.Force().Modify().bVisible = inputs.anim.GetRuntimeData().pAnimPlayer->IsVisible();
				}
		    }

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ::usg::Events::ModifyAnimationConditionEvent& event)
			{
    			ModelAnimPlayer* player = outputs.anim.GetRuntimeData().pAnimPlayer;
				player->SetCondition(event.nameHash, event.value);
			}

		};

		class AnimateBone : public System
		{
		public:
			struct Inputs
			{
				Required<BoneComponent, FromSelf>								bone;
				Required<ModelAnimComponent, FromUnderCompInc<ModelComponent> >	anim;
		    };

			struct Outputs
			{
				Required<TransformComponent>		transform;
				Optional<ScaleComponent>			scale;
		    };

			DECLARE_SYSTEM(SYSTEM_POST_GAMEPLAY)

		    static void	Run(const Inputs& inputs, Outputs& outputs, float fDelta)
		    {
		    	if (inputs.anim.GetRuntimeData().pAnimPlayer->ControlsBone(inputs.bone->uIndex))
				{
					const SkeletalAnimationResource::Transform* pTransform = inputs.anim.GetRuntimeData().pAnimPlayer->GetTransform(inputs.bone->uIndex);
					TransformComponent& trans = outputs.transform.Modify();
					trans.position = pTransform->vPos;
					trans.rotation = pTransform->qRot;
					if (outputs.scale.Exists())
					{
					/*	if (inputs.anim->applyBindPose)
						{
							outputs.scale.Force().Modify().scale = pTransform->vScale * inputs.bone->m_scale;
						}
						else*/
						{
							outputs.scale.Force().Modify().scale = pTransform->vScale;
						}
						
					}

					/*if (inputs.anim->applyBindPose)
					{
						trans.position += inputs.bone->m_translate;
						// FIXME: Apply the bind pose rotation too!
					}*/
				}
		    }
		};


		class UpdateBone : public System, protected BoneTransformInterface
		{
		public:
			struct Inputs
			{
				Required<MatrixComponent, FromSelfOrParents>			mtx;
				Required<TransformComponent, FromSelf>					tran;
		    };

			struct Outputs
			{
				Required<BoneComponent>							bone;
		    };

			DECLARE_SYSTEM(SYSTEM_POST_TRANSFORM)

			static void LateUpdate(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				LateUpdateInt(inputs, outputs);
			}

			static  void GPUUpdate(const Inputs& inputs, Outputs& outputs, GPUHandles* pGPUData)
			{
				GPUUpdateInt(inputs, outputs, pGPUData);
			}
		};

		// Billboard bones need to be updated but have no transform components
		class UpdateBillboardedBone : public System, protected BoneTransformInterface
		{
		public:
			struct Inputs
			{
				Required<MatrixComponent, FromSelfOrParents>			mtx;
				Required<Billboard, FromSelf>							bill;
			};

			struct Outputs
			{
				Required<BoneComponent>							bone;
			};

			DECLARE_SYSTEM(SYSTEM_POST_TRANSFORM)

			static void LateUpdate(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				LateUpdateInt(inputs, outputs);
			}

			static  void GPUUpdate(const Inputs& inputs, Outputs& outputs, GPUHandles* pGPUData)
			{
				GPUUpdateInt(inputs, outputs, pGPUData);
			}
		};

		class CalculateBillboardMatrix : public System
		{
		public:
			struct Inputs
			{
				Required<MatrixComponent, FromParentWith<IntermediateBone> >	parentMtx;
				Required<SceneComponent, FromParents>							scene;
				Required<Billboard, FromSelf>									billboard;
		    };

			struct Outputs
			{
				Required<MatrixComponent, FromSelf>							mtx;
		    };

			DECLARE_SYSTEM(SYSTEM_POST_TRANSFORM)

			static void BillboardMatrix(const Matrix4x4& in, const Vector4f& vFace, Matrix4x4& out, bool bRecaulateY)
			{
				Vector4f vY(in.vUp());
				Vector4f vZ(vFace);

				vY.NormaliseIfNZero(V4F_X_AXIS);

				Vector4f vX;
				vX = CrossProduct(vY, vZ).NormaliseIfNZero(in.vFace());

				if (bRecaulateY)
				{
					vY = CrossProduct(vFace,vX);
				}
				else
				{
					vZ = CrossProduct(vX,vY);
				}

				out._11 = vX.x;
				out._12 = vX.y;
				out._13 = vX.z;
				out._14 = 0.0f;

				out._21 = vY.x;
				out._22 = vY.y;
				out._23 = vY.z;
				out._24 = 0.0f;

				out._31 = vZ.x;
				out._32 = vZ.y;
				out._33 = vZ.z;
				out._34 = 0.0f;

				out._41 = in._41;
				out._42 = in._42;
				out._43 = in._43;
				out._44 = 1.0f;
			}


		static void ScreenBillboardMatrix(const Matrix4x4& in, const Matrix4x4& mInvView, const Vector4f& vYAxis, const Vector4f& vZAxis, Matrix4x4& out)
			{
				Vector4f vY = vYAxis.GetNormalised();

				Vector4f vX;
				vX = CrossProduct(vY, vZAxis).GetNormalised();
				vY = CrossProduct(vZAxis,vX);

				out._11 = vX.x;
				out._12 = vX.y;
				out._13 = vX.z;
				out._14 = 0.0f;

				out._21 = vY.x;
				out._22 = vY.y;
				out._23 = vY.z;
				out._24 = 0.0f;

				out._31 = vZAxis.x;
				out._32 = vZAxis.y;
				out._33 = vZAxis.z;
				out._34 = 0.0f;

				out._41 = in._41;
				out._42 = in._42;
				out._43 = in._43;
				out._44 = 1.0f;

				out = out * mInvView;

				out.SetPos(in.vPos());
			}

		    static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
		    {
				const Camera* pCamera = inputs.scene.GetRuntimeData().pScene->GetSceneCamera(0);
				const Matrix4x4& mParent = inputs.parentMtx->matrix;
				const Matrix4x4& mCamera = pCamera->GetModelMatrix();
				Matrix4x4& mOut = outputs.mtx.Modify().matrix;
				switch(inputs.billboard->mode)
				{
				case usg::exchange::Bone_BillboardMode_OFF:
					ASSERT(false);	// Why are we running this system?
					mOut = mParent;	// Return the matrix as is
					break;
				case usg::exchange::Bone_BillboardMode_WORLD:
				{
					BillboardMatrix(mParent, -mCamera.vFace(), mOut, true);
				}
				break;
				case usg::exchange::Bone_BillboardMode_SCREEN:
					{
						Matrix4x4 mInvViewMat;
						Matrix4x4 mViewMat;
						GetViewMatrix(inputs, false, mViewMat);
						mViewMat.GetInverse(mInvViewMat);
						Vector4f vViewPos = -(mParent.vPos() * mViewMat);
						vViewPos.NormaliseIfNZero(V4F_Z_AXIS);

						ScreenBillboardMatrix(mParent, mInvViewMat, V4F_Y_AXIS, vViewPos, mOut);
					}
					break;
				case usg::exchange::Bone_BillboardMode_WORLDVIEWPOINT:
					{
						Vector4f vFace = -(mParent.vPos() - mCamera.vPos());
						vFace.NormaliseIfNZero(V4F_Z_AXIS);
						BillboardMatrix(mParent, vFace, mOut, false);
					}
					break;
				case usg::exchange::Bone_BillboardMode_SCREENVIEWPOINT:
					{
						Matrix4x4 mInvViewMat;
						Matrix4x4 mViewMat;
						GetViewMatrix(inputs, false, mViewMat);
						mViewMat.GetInverse(mInvViewMat);
						Vector4f vViewPos = -(mParent.vPos() * mViewMat );
						vViewPos.NormaliseIfNZero(V4F_Z_AXIS);

						ScreenBillboardMatrix(mParent, mInvViewMat, pCamera->GetModelMatrix().vUp() , vViewPos, mOut);
					}
					break;
				case usg::exchange::Bone_BillboardMode_YAXIALVIEWPOINT:
					{
						Vector4f vFace = -(mParent.vPos() - mCamera.vPos());
						vFace.NormaliseIfNZero(V4F_Z_AXIS);
						BillboardMatrix(mParent, vFace, mOut, false);
					}
					break;
				case usg::exchange::Bone_BillboardMode_YAXIAL:
					{
						usg::Matrix4x4  mMat = mParent;
						mMat.SetUp(V4F_Y_AXIS);

						BillboardMatrix(mMat, -mCamera.vFace(), mOut, false);
					}
					break;

				default:
					ASSERT_MSG( false, "Unexpected billboard mode: %d", inputs.billboard->mode);
				};
		    }

		    static void GetViewMatrix(const Inputs& inputs, bool bInverse, Matrix4x4& out)
		    {
				out = inputs.scene.GetRuntimeData().pScene->GetSceneCamera(0)->GetViewMatrix();
		    }
		};
	}
}

#include GENERATED_SYSTEM_CODE(Engine/Scene/Model/ModelSystems.cpp)