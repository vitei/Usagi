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
#include "AnimationBase.h"

namespace usg
{

	AnimationBase::AnimationBase()
	{
		Reset();
	}

	AnimationBase::~AnimationBase()
	{
		
	}

	void AnimationBase::Reset()
	{
		m_bActive = false;
		m_fPlaybackSpeed = 1.0f;
		m_fActiveFrame = 0.0f;
		m_fPlaybackSpeed = 1.0f;
	}

	void AnimationBase::Play()
	{
		m_fActiveFrame = 0.0f;
		m_bActive = true;
	}

	void AnimationBase::UpdateInt(float fElapsed, float fFrameRate, float fFrameCount, bool bLoop)
	{
		if (!m_bActive)
			return;

		const float fAnimSpeed = fFrameRate;
		m_fActiveFrame += (fElapsed * m_fPlaybackSpeed * fAnimSpeed);

		// We've passed the end (playing the anim forward)
		if (m_fActiveFrame > fFrameCount)
		{
			ASSERT(m_fPlaybackSpeed * fElapsed > 0.0f);
			if (bLoop)
			{
				m_fActiveFrame = 0.0f;
			}
			else
			{
				m_fActiveFrame = fFrameCount;
				m_bActive = false;
			}
		}

		// We've passed the beginning (playing the anim backward)
		if (m_fActiveFrame < 0.0f)
		{
			ASSERT(m_fPlaybackSpeed * fElapsed < 0.0f);
			if (bLoop)
			{
				m_fActiveFrame = fFrameCount;
			}
			else
			{
				m_fActiveFrame = 0.0f;
				m_bActive = false;
			}
		}
	}



}