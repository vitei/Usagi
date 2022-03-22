/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "SkeletalAnimationResource.h"
#include "Engine/Scene/Model/Bone.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"

namespace usg
{
	SkeletalAnimationResource::SkeletalAnimationResource() :
		ResourceBase(StaticResType)
	{

	}
	
	SkeletalAnimationResource::~SkeletalAnimationResource()
	{
		m_pBoneAnimFrames = nullptr;
		m_pBoneDescriptions = nullptr;
	}

	bool SkeletalAnimationResource::Load(const char* szName)
	{
		File file(szName);

		if (!file.Read(sizeof(m_header), &m_header)) {
			return false;
		}


		m_pBoneDescriptions = vnew(ALLOC_ANIMATION) exchange::BoneAnimationDescription[m_header.referencedBones];
		m_pBoneAnimFrames = vnew(ALLOC_ANIMATION) exchange::BoneAnimationFrame[m_header.frameCount * m_header.referencedBones];

		file.Read(m_header.referencedBones * sizeof(m_pBoneDescriptions[0]), m_pBoneDescriptions);
		file.Read(m_header.referencedBones * m_header.frameCount * sizeof(m_pBoneAnimFrames[0]), m_pBoneAnimFrames);

		SetupHash(szName);

		return true;
	}


	bool SkeletalAnimationResource::Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData)
	{
		// TODO: Switch to persistent data? Need to consider alignment if we do
		const uint8* pSrc = (uint8*)pData;

		memcpy(&m_header, pSrc, sizeof(m_header));
		pSrc += sizeof(m_header);

		m_pBoneDescriptions = vnew(ALLOC_ANIMATION) exchange::BoneAnimationDescription[m_header.referencedBones];
		m_pBoneAnimFrames = vnew(ALLOC_ANIMATION) exchange::BoneAnimationFrame[m_header.frameCount * m_header.referencedBones];

		memsize boneDescSize = m_header.referencedBones * sizeof(m_pBoneDescriptions[0]);
		memcpy(m_pBoneDescriptions, pSrc, boneDescSize);
		pSrc += boneDescSize;

		memsize animFramesSize = m_header.referencedBones * m_header.frameCount * sizeof(m_pBoneAnimFrames[0]);
		memcpy(m_pBoneAnimFrames, pSrc, animFramesSize);

		SetupHash(pFileHeader->szName);

		return true;
	}

	sint32 SkeletalAnimationResource::GetBoneReference(const char* szBoneName) const
	{
		for (uint32 i = 0; i < m_header.referencedBones; i++)
		{
			if (str::Compare(m_pBoneDescriptions[i].targetName, szBoneName))
			{
				return (sint32)i;
			}
		}

		return (-1);
	}


	const exchange::BoneAnimationFrame* SkeletalAnimationResource::GetAnimFrame(uint32 uBoneIndex, uint32 uFrame) const
	{
		if (uBoneIndex >= m_header.referencedBones || uFrame >= m_header.frameCount)
		{
			ASSERT(false);
			return nullptr;
		}
		exchange::BoneAnimationFrame* pFullFrame = &m_pBoneAnimFrames[uFrame * m_header.referencedBones];
		return &pFullFrame[uBoneIndex];
	}

	void SkeletalAnimationResource::GetTransform(uint32 uBoneIndex, float fFrame, const exchange::BoneAnimationFrame& bindPose, exchange::BoneAnimationFrame& transform) const
	{
		if (uBoneIndex >= m_header.referencedBones)
		{
			ASSERT(false);
			return;
		}
		const exchange::BoneAnimationDescription* pDesc = &m_pBoneDescriptions[uBoneIndex];
		uint32 uBaseFrame = (uint32)fFrame;
		float fRemainder = fFrame - (float)uBaseFrame;
		if (fRemainder == 0.0f || uBaseFrame == m_header.frameCount-1)
		{
			const exchange::BoneAnimationFrame* pFrame = GetAnimFrame(uBoneIndex, uBaseFrame);
			transform.qRot = pDesc->controlsRot ? pFrame->qRot : bindPose.qRot;
			transform.vPos = pDesc->controlsPos ? pFrame->vPos : bindPose.vPos;
			transform.vScale = pDesc->controlsScale ? pFrame->vScale : bindPose.vScale;
		}
		else
		{
			const exchange::BoneAnimationFrame* pFrame = GetAnimFrame(uBoneIndex, uBaseFrame);
			const exchange::BoneAnimationFrame* pNextFrame = GetAnimFrame(uBoneIndex, uBaseFrame+1);

			transform.qRot = pDesc->controlsRot ? Slerp(pFrame->qRot, pNextFrame->qRot, fRemainder) : bindPose.qRot;
			transform.vPos = pDesc->controlsPos ? Lerp(pFrame->vPos, pNextFrame->vPos, fRemainder) : bindPose.vPos;
			transform.vScale = pDesc->controlsScale ? Lerp(pFrame->vScale, pNextFrame->vScale, fRemainder) : bindPose.vScale;
		}
	}

}