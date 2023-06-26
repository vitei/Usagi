/****************************************************************************
//	Usagi Engine, Copyright Vitei, Inc. 2013
//	Description: Maintains the mapping between an animation resource and a
//	skeletons bone indices, also tracks the current frame
*****************************************************************************/
#ifndef _USG_SCENE_MODEL_SKELETAL_ANIMATION_H_
#define _USG_SCENE_MODEL_SKELETAL_ANIMATION_H_


#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Resource/SkeletalAnimationResource.h"
#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Scene/Model/AnimationBase.h"

namespace usg {

class Model;
class ModelResource;

// Animations may not reference all the bones in a skeleton so we have to keep track
// of which they do
class SkeletalAnimation : public AnimationBase
{
	typedef AnimationBase Inherited;
public:
	SkeletalAnimation();
	virtual ~SkeletalAnimation();

	bool Init(const SkeletonResource* pSkeleton, const char* szAnimName);
	virtual void Reset();
	
	void Update(float fElapsed);
	//void Reverse() { m_fPlaybackSpeed = -m_fPlaybackSpeed;  }
	const char* GetName() { return m_name.c_str(); }

	uint32 GetNumberOfTargets() { return m_uBoneCount;  }
	void GetTransform(uint32 uIndex, exchange::BoneAnimationFrame& transform) const;
	bool IsBoneReferenced(uint32 uIndex) { return m_pBoneInfo[uIndex].sBoneRefIndex >= 0; }
	void SetLoop(bool bLoop) { m_bLoop = bLoop; }

private:
	struct BoneInfo
	{
		sint32						 sBoneRefIndex;	// -1 if not referenced by anim
		exchange::BoneAnimationFrame transform;
	};

	usg::string							m_name;
	BoneInfo*							m_pBoneInfo;
	uint32								m_uBoneCount;
	const SkeletonResource*				m_pResource;
	SkeletalAnimationResHndl			m_pAnimResource;
	bool								m_bLoop;
};

}

#endif	// #ifndef _USG_SCENE_MODEL_SKELETAL_ANIMATION_H_

