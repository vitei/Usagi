/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An bone used within model skeletons
*****************************************************************************/
#pragma once

#ifndef USG_GRAPHICS_SCENE_BONE_H
#define USG_GRAPHICS_SCENE_BONE_H
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Scene/SceneConstantSets.h"

namespace usg {

class Model;
class ModelResource;

class Bone
{
public:
	Bone();
	~Bone();

	void Init(GFXDevice* pDevice, Bone* pParent, const SkeletonResource::Bone* pBone);
	void CleanUp(GFXDevice* pDevice);
	void SetTransform(const Matrix4x4& mat, bool bUpdateConsts = true)
	{
		m_pTransformNode->SetMatrix(mat);
		if (bUpdateConsts /*&& m_pRenderGroup*/)
		{
			// FIXME: Only do for referenced base nodes
			SceneConsts::ModelTransform* pTransform = m_boneConstants.Lock<SceneConsts::ModelTransform>();
			pTransform->mModelMat = mat;
			m_boneConstants.Unlock();
		}
	}

	void UpdateConstants(GFXDevice* pDevice)
	{
		// TODO: Only perform if we are visible
		m_boneConstants.UpdateData(pDevice);
	}

	// Used by a fully skinned animation
	const ConstantSet* GetConstantSet() const { return &m_boneConstants;  }
	const Matrix4x4& GetLocalTransform() const { return m_pTransformNode->GetLocalMatrix(); }

	// Note, may currently be a frame behind, may need a post transform update for objects dependent on this
	// frames matrix
	const Matrix4x4& GetWorldMatrix() const { return m_pTransformNode->GetMatrix(); }
	const Matrix4x4& GetInverseBindMatrix() const { return m_pResource->mInvBindMatrix;  }
	const usg::Sphere& GetLocalColSphere() const { return m_pResource->cColSphere;  }
	void AttachRenderNode(Scene* pScene, RenderNode* pNode, uint8 uLod = 0, bool bDynamic = false);
	void RemoveRenderNode(Scene* pScene, RenderNode* pNode);
	void RemoveRenderNodes(Scene* pScene);

	void CreateTransformNode(Scene* pScene, TransformNode* pModelNode);
	void RemoveTransformNode(Scene* pScene);
	TransformNode* GetTransformNode() { return m_pTransformNode; }
	RenderGroup* GetRenderGroup() { return m_pRenderGroup; }

	bool IsValid() { return (m_pResource != NULL); }
	const SkeletonResource::Bone* GetResource() const { return m_pResource; }

private:
	PRIVATIZE_COPY(Bone)


	const SkeletonResource::Bone*	m_pResource;
	Bone*							m_pParent;
	// TODO: Really need one of these per view if billboarded
	TransformNode*					m_pTransformNode;
	ConstantSet						m_boneConstants;
	RenderGroup*					m_pRenderGroup;
	bool							m_bWorldTransDirty;
};

} // namespace usg

#endif	// USG_GRAPHICS_SCENE_BONE_H
