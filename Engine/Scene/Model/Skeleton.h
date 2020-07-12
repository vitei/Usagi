/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An animatable skeleton for a model
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_SKELETON_H_
#define _USG_GRAPHICS_SCENE_SKELETON_H_

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Scene/SceneConstantSets.h"

namespace usg {

class Model;
class ModelResource;
class Bone;

class Skeleton
{
public:
	Skeleton();
	Skeleton(GFXDevice* pDevice, Scene* pScene, Model* pModel, bool bAutoTransform);
	~Skeleton();

	bool Load( GFXDevice* pDevice, Scene* pScene, Model* pModel, bool bAutoTransform);
	void CleanUp(GFXDevice* pDevice);
	void SetInUse(Scene* pScene, Model* pModel, bool bInUse);
	// TODO: Loading and processing of animations
	//void SetAnimation()

	void Update(float fElapsed);
	uint32 GetBoneCount() const { return m_pResource->GetBoneCount(); }
	const SkeletonResource* GetResource() const { return m_pResource;  }

	Bone* GetBone(uint32 uIndex);
	const Bone* GetBone(uint32 uIndex) const;
	const Bone* GetBone(const U8String& name) const;
	Bone* GetBone(const U8String& name);

private:
	PRIVATIZE_COPY(Skeleton)

	uint32						m_uBoneCount;
	Bone*						m_pBones;
	Matrix4x4					m_transform;
	const SkeletonResource*		m_pResource;
	bool						m_bAutoTransform;
	bool						m_bInUse;
};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_H_
