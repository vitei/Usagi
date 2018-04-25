#include "Animation.h"

#include <string.h>

#include "common.h"

namespace exchange {

static void copyString( char* pDest, size_t destLen, const char* pSrc, size_t srcLen )
{
	if( srcLen < destLen ) {
		strncpy( pDest, pSrc, srcLen );
		pDest[srcLen] = '\0';
	}
}

Animation::Animation()
{
	usg::exchange::SkeletalAnimationHeader_init( &m_header );
}

void Animation::SetName( const char* p )
{
	copyString( m_header.name, ARRAY_SIZE(m_header.name ), p, strlen( p ) + 1 );
}


const char* Animation::GetName( void )
{
	return m_header.name;
}


void Animation::InitTiming(uint32 uFrameCount, float fFrameRate)
{
	m_header.frameRate = fFrameRate;
	m_header.frameCount = uFrameCount;
	m_animFrames.resize(uFrameCount);
}


usg::exchange::BoneAnimationDescription* Animation::AddBone(const char* szBoneName, bool bControlsRot, bool bControlsPos, bool bControlsScale)
{
	// Can't add a bone after allocating the space
	if (!GetBone(szBoneName))
	{
		usg::exchange::BoneAnimationDescription info;
		usg::exchange::BoneAnimationDescription_init(&info);
		copyString(info.targetName, ARRAY_SIZE(info.targetName), szBoneName, strlen(szBoneName) + 1);
		info.controlsRot = bControlsRot;
		info.controlsPos = bControlsPos;
		info.controlsScale = bControlsScale;
		m_boneInfo.push_back(info);

		return &m_boneInfo.back();
	}
	ASSERT(false);
	return nullptr;
}

void Animation::AllocateAnimBones()
{
	for (auto itr = m_animFrames.begin(); itr != m_animFrames.end(); itr++)
	{
		(*itr).boneFrameInfo.resize(m_boneInfo.size());
	}
	m_header.referencedBones = (uint32_t)m_boneInfo.size();
}


usg::exchange::BoneAnimationDescription* Animation::GetBone(const char* szBoneName)
{
	int boneIndex = GetBoneIndex(szBoneName);
	if (boneIndex >= 0)
	{
		return &m_boneInfo[boneIndex];
	}
	return nullptr;
}

int Animation::GetBoneIndex(const char* szBoneName)
{
	for (int i = 0; i < m_boneInfo.size(); i++)
	{
		if (strcmp( m_boneInfo[i].targetName, szBoneName) == 0)
		{
			return i;
		}
	}
	return -1;
}


usg::exchange::BoneAnimationFrame* Animation::GetBoneAnimFrame(const char* szBoneName, uint32 uFrame)
{
	int boneIndex = GetBoneIndex(szBoneName);
	if (boneIndex >= 0 && uFrame < m_animFrames.size())
	{
		return &m_animFrames[uFrame].boneFrameInfo[boneIndex];
	}

	ASSERT(false);
	return nullptr;
}


void Animation::Export(aya::string path)
{
	FILE* fp = fopen(path.c_str(), "wb");
	if (!fp) {
		// open failed
		return;
	}

	// TODO: Endian issues
	fwrite(&m_header, sizeof(m_header), 1, fp);
	fwrite(m_boneInfo.data(), sizeof(usg::exchange::BoneAnimationDescription), m_boneInfo.size(), fp);
	for (uint32 i = 0; i < m_animFrames.size(); i++)
	{
		fwrite(m_animFrames[i].boneFrameInfo.data(), sizeof(usg::exchange::BoneAnimationFrame), m_animFrames[i].boneFrameInfo.size(), fp);
	}

	fclose(fp);
}

}
