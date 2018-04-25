/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
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



}