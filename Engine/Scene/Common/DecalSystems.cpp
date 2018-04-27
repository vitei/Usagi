/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Scene/Common/SceneEvents.pb.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"
#include "Engine/Physics/PhysicsComponents.pb.h"
#include "Engine/Graphics/GPUUpdate.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Physics/CollisionQuadTree.pb.h"
#include "Engine/Scene/Common/GroundDecals.h"
#include "Engine/Physics/CollisionQuadTreeHandle.h"

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

		class ShadowDecalSystem : public usg::System
		{
		public:

			struct Inputs
			{
				Required<ShadowDecalComponent>					shadow;
				Required<usg::MatrixComponent>					matrix;
				Required<usg::GroundDecalsHandle, FromParents>	groundDecals;
				Required<usg::QuadTreeHandle, FromParents>		quadTree;
				Optional<usg::ModelComponent>					model;
				Required<usg::SceneComponent, FromSelfOrParents>		scene;
			};

			struct Outputs
			{
				Required<ShadowDecalComponent> shadow;
			};

			DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)

			static  void GPUUpdate(const Inputs& inputs, Outputs& outputs, GPUHandles* pGPUData);

		private:
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


		float TestGroundHeight(const usg::CollisionQuadTree* pTree, const usg::Matrix4x4& mtx, const usg::Vector3f &spos, usg::CollisionMeshHitResult &hit, const float radius)
		{
			ASSERT(pTree != NULL);

			const usg::CollisionQuadTree::Node* pNode = pTree->GetRootNode();
			usg::Vector3f dir = -V3F_Y_AXIS;
			usg::Vector3f to = spos + (dir * radius);

			return pTree->ClipLine(pNode, spos, to, dir, radius, true, hit, usg::CollisionQuadTree::MF_ALL & ~(usg::CollisionQuadTree::MF_WATER | usg::CollisionQuadTree::MF_NOCOLLISION));
		}


		void ShadowDecalSystem::GPUUpdate(const Inputs& inputs, Outputs& outputs, GPUHandles* pGPUData)
		{
			usg::Vector3f pos = inputs.matrix->matrix.vPos().v3();

			// TODO: These variables should be per object type
			float fLength = inputs.shadow->fMaxHeight;
			const float fRadiusTol = inputs.shadow->fUpdateDistance;
			pos += usg::V3F_Y_AXIS;

			// Do nothing if we haven't moved enough
			if (inputs.shadow.GetRuntimeData().vPrevTestPos.GetSquaredDistanceFrom(pos) > fRadiusTol*fRadiusTol)
			{
				float fH = TestGroundHeight(inputs.quadTree->m_quadtree, inputs.matrix->matrix, pos, outputs.shadow.GetRuntimeData().hitResult, fLength);

				if (fH < fLength)
				{
					// FIXME: Move into a shadow system
					// Index Buffer
					const uint32 TRIANGLES_NUM = 20;
					const uint32 indicesMax = TRIANGLES_NUM * 3;
					uint16 indices[indicesMax];

					float radius = inputs.shadow->fTestRadius + fRadiusTol;
					uint32 indicesNum = inputs.quadTree->m_quadtree->setupGroundPatchIndices(indices, indicesMax, radius, outputs.shadow.GetRuntimeData().hitResult);

					for (int i = indicesNum; i < indicesMax; ++i) {
						indices[i] = indices[indicesNum];
					}

					usg::Sphere boundingSphere;
					boundingSphere.SetPos(outputs.shadow.GetRuntimeData().hitResult.vIntersectPoint);
					boundingSphere.SetRadius(radius);


					Required<usg::GroundDecalsHandle, FromSelfOrParents> Decals;
					GetComponent(inputs.matrix.GetEntity(), Decals);
					RuntimeData<ShadowDecalComponent>& decal = outputs.shadow.GetRuntimeData();

					decal.pDecal->AddToScene(inputs.scene.GetRuntimeData().pScene, true);
					decal.pDecal->SetContents(pGPUData->pDevice, &boundingSphere, Decals->pGroundDecals->GetVertexBuffer(), indices, indicesNum);
					decal.vPrevTestPos = pos;
				}
			}

			// TODO: Cache the orthographic matrix
			float32 fNear = 0.01f;
			float32 fFar = 10.0f;
			usg::Matrix4x4 projMatrix, viewMatrix;

			float fWidth = inputs.shadow->vScale.x;
			float fHeight = inputs.shadow->vScale.y;

			projMatrix.Orthographic(-fWidth, fWidth, -fHeight, fHeight, fNear, fFar);

			if (inputs.shadow->bRotateWithObject)
			{
				viewMatrix.CameraMatrix(
					inputs.matrix->matrix.vRight(),
					inputs.matrix->matrix.vFace(),
					-inputs.matrix->matrix.vUp(),
					inputs.matrix->matrix.vPos()
					);
			}
			else
			{
				viewMatrix.CameraMatrix(
					V4F_X_AXIS,
					V4F_Z_AXIS,
					-V4F_Y_AXIS,
					inputs.matrix->matrix.vPos()
					);
			}



			outputs.shadow.GetRuntimeData().pDecal->SetMatrix(projMatrix, viewMatrix);
			float fOpacity = 1.0f;
			// TODO: Fix to use the visibility component and don't update if we shouldn't be drawing anyway
			if (inputs.model.Exists())
			{
				fOpacity = inputs.model.Force().GetRuntimeData().pModel->GetAlpha();
			}
			outputs.shadow.GetRuntimeData().pDecal->SetOpacity(fOpacity);
			outputs.shadow.GetRuntimeData().pDecal->AddToScene(inputs.scene.GetRuntimeData().pScene, fOpacity > Math::EPSILON);
		}
		
	}
}

#include GENERATED_SYSTEM_CODE(Engine/Scene/Common/DecalSystems.cpp)