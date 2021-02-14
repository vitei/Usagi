#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdio.h>
#include <math.h>
#include <vector>

#include "common.h"
#include "StringUtil.h"
#include "Engine/Scene/Model/SkeletalAnimation.pb.h"


namespace exchange {


class Animation
{
public:
	Animation();
	virtual ~Animation() {}

	void SetName( const char* p );
	const char* GetName( void );


	void InitTiming(uint32 uFrameCount, float fFrameRate);

	// Confirms no such bone is already present
	usg::exchange::BoneAnimationDescription* AddBone(const char* szBoneName, bool bControlsRot = true, bool bControlsPos = true, bool bControlsScale = false);
	void AllocateAnimBones();
	usg::exchange::BoneAnimationDescription* GetBone(const char* szBoneName);
	int GetBoneIndex(const char* szBoneName);

	usg::exchange::BoneAnimationFrame* GetBoneAnimFrame(const char* szBoneName, uint32 uFrame);

    usg::exchange::SkeletalAnimationHeader& Header( void ) { return m_header; }
	const usg::exchange::SkeletalAnimationHeader& Header( void ) const { return m_header; }

	uint32 GetFrameCount() const { return m_header.frameCount; }

	void Export(aya::string path);

	bool ValidAnim() const { return m_boneInfo.size() > 0; }

private:
    usg::exchange::SkeletalAnimationHeader m_header;
	
	std::vector<usg::exchange::BoneAnimationDescription>	m_boneInfo;

	struct AnimationFrame
	{
		std::vector<usg::exchange::BoneAnimationFrame> boneFrameInfo;
	};

	std::vector<AnimationFrame>	m_animFrames;
};

}

#endif // MESH_H
