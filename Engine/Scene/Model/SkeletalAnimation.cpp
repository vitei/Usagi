/****************************************************************************
//	Usagi Engine, Copyright � Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Model/AnimationChain.pb.h"
#include "SkeletalAnimation.h"

namespace usg
{

	SkeletalAnimation::SkeletalAnimation()
	{
		m_pBoneInfo = NULL;
		Reset();
	}

	SkeletalAnimation::~SkeletalAnimation()
	{
		Reset();
	}

	void SkeletalAnimation::Reset()
	{
		Inherited::Reset();
		m_uBoneCount = 0;
		if (m_pBoneInfo)
		{
			vdelete[] m_pBoneInfo;
			m_pBoneInfo = NULL;
		}
	}

	bool SkeletalAnimation::Init(const SkeletonResource* pSkeleton, const char* szAnimName)
	{
		m_name = szAnimName;
		m_name += ".vskla";
		m_pAnimResource = ResourceMgr::Inst()->GetSkeletalAnimation(m_name.c_str());
		if (!m_pAnimResource)
			return false;
	
		m_uBoneCount = pSkeleton->GetBoneCount();

		m_pBoneInfo = vnew(ALLOC_ANIMATION) BoneInfo[m_uBoneCount];
		
		// First init the bones
		for (uint32 i = 0; i < m_uBoneCount; i++)
		{
			const SkeletonResource::Bone* pBone = pSkeleton->GetBoneByIndex(i);
			m_pBoneInfo[i].sBoneRefIndex = m_pAnimResource->GetBoneReference(pBone->name.c_str());
			m_pBoneInfo[i].transform.vPos = pBone->vTranslate;
			m_pBoneInfo[i].transform.vScale = pBone->vScale;
			Matrix4x4 mRot = Matrix4x4::Identity();
			mRot.MakeRotate(pBone->vRotate.x, pBone->vRotate.y, pBone->vRotate.z);
			m_pBoneInfo[i].transform.qRot = mRot;
		}


		return true;

	}


	void SkeletalAnimation::Update(float fElapsed)
	{
		UpdateInt(fElapsed, m_pAnimResource->GetFrameRate(), (float)m_pAnimResource->GetFrameCount(), m_pAnimResource->IsLoop());
	}

	void SkeletalAnimation::GetTransform(uint32 uIndex, exchange::BoneAnimationFrame& transform) const
	{
		ASSERT(uIndex < m_uBoneCount);
		const BoneInfo* pBone = &m_pBoneInfo[uIndex];

		if (pBone->sBoneRefIndex >= 0)
		{
			m_pAnimResource->GetTransform((uint32)pBone->sBoneRefIndex, m_fActiveFrame, pBone->transform, transform);
		}
		else
		{
			transform = pBone->transform;
		}
	}
}