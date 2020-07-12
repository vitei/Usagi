/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_SKELETAL_ANIMATION_RESOURCE_H_
#define _USG_SKELETAL_ANIMATION_RESOURCE_H_
#include "Engine/Maths/Quaternionf.h"
#include "Engine/Maths/Vector3f.h"
#include "Engine/Scene/Model/SkeletalAnimation.pb.h"
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Core/stl/vector.h"

namespace usg {

class Model;

class SkeletalAnimationResource : public ResourceBase
{
public:
	typedef exchange::BoneAnimationFrame Transform;


	SkeletalAnimationResource();
	virtual ~SkeletalAnimationResource();

	bool Load(const char* szName);
	
	// Note reference index is the index of the bones referenced by the animation, not the index within the skeleton
	void GetTransform(uint32 uReferenceIndex, float fFrame, const exchange::BoneAnimationFrame& bindPose, exchange::BoneAnimationFrame& transform) const;
	sint32 GetBoneReference(const char* szBoneName) const;

	float GetFrameRate() const { return m_header.frameRate; }
	bool IsLoop() const { return m_header.isLoop; }
	uint32 GetFrameCount() const { return m_header.frameCount-1; }

	const static ResourceType StaticResType = ResourceType::SKEL_ANIM;

private:
	const exchange::BoneAnimationFrame* GetAnimFrame(uint32 uReferenceIndex, uint32 uFrame) const;

	exchange::SkeletalAnimationHeader	 m_header;
	exchange::BoneAnimationDescription*	 m_pBoneDescriptions;
	exchange::BoneAnimationFrame*		 m_pBoneAnimFrames;
	
};

}

#endif // _USG_SKELETAL_ANIMATION_RESOURCE_H_
