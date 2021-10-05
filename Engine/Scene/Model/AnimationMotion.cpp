/****************************************************************************
//	Usagi Engine, Copyright � Vitei, Inc. 2013
 ****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Scene/Model/AnimationChain.pb.h"
#include "Engine/Core/String/String_Util.h"
#include "AnimationMotion.h"

namespace usg
{

	AnimationMotion::AnimationMotion()
	{
		m_pModifiers = NULL;
		m_uModifierCount = 0;
	}

	void AnimationMotion::AppendToAnimList(list<SkeletalAnimation*>& list)
	{
		for (uint32 i = 0; i < GetActiveAnimCount(); i++)
		{
			list.push_back(GetActiveAnim(i));
		}
	}

	void AnimationMotion::Reset()
	{
		for (uint32 i = 0; i < GetAnimCount(); i++)
		{
			GetAnim(i)->Reset();
		}
	}

	bool AnimationMotion::IsBoneReferenced(uint32 uBoneId)
	{
		bool bReferenced = false;
		for (uint32 i = 0; i < GetAnimCount(); i++)
		{
			bReferenced |= GetAnim(i)->IsBoneReferenced(uBoneId);
		}
		return bReferenced;
	}

	void AnimationMotion::Play()
	{
		for (uint32 i = 0; i < GetAnimCount(); i++)
		{
			GetAnim(i)->Play();
		}
	}

	void AnimationMotion::Stop()
	{
		for (uint32 i = 0; i < GetAnimCount(); i++)
		{
			GetAnim(i)->Stop();
		}
	}

	bool AnimationMotion::IsPlaying()
	{
		bool bIsPlaying = false;
		for (uint32 i = 0; i < GetAnimCount(); i++)
		{
			bIsPlaying |= GetAnim(i)->IsPlaying();
		}
		return bIsPlaying;
	}

	void AnimationMotion::SetModifiers(const SkeletalAnim::Modifier* pModifiers, uint32 uModifierCount)
	{
		m_pModifiers = pModifiers;
		m_uModifierCount = uModifierCount;
	}

	SingleAnimationMotion::SingleAnimationMotion()
	{

	}
	
	SingleAnimationMotion::~SingleAnimationMotion()
	{

	}


	void SingleAnimationMotion::Init(const SkeletonResource* pResource, const SkeletalAnim::Animation& anim)
	{
		m_animation.Init(pResource, anim.animName);
		m_fSpeed = anim.playbackSpeed;
		m_animation.SetSpeed(m_fSpeed);
	}

	void SingleAnimationMotion::Update(float fElapsed, float fWeighting)
	{
		m_animation.Update(fElapsed);
		m_animation.SetWeighting(fWeighting);
	}


	void SingleAnimationMotion::SetSpeedMultiplier(float fMultiplier)
	{
		m_animation.SetSpeed(fMultiplier * m_fSpeed);
	}

	BlendAnimationMotion::BlendAnimationMotion()
	{
		for (uint32 i = 0; i < MAX_ACTIVE_ANIMS; i++)
		{
			m_pActiveAnims[i] = NULL;
		}
		m_pMotions = NULL;
		m_uMotionCount = 0;
		m_uActiveAnims = 0;
	}

	BlendAnimationMotion::~BlendAnimationMotion()
	{
		if (m_pMotions != NULL)
		{
			vdelete[] m_pMotions;
			m_pMotions = NULL;
		}
	}


	void BlendAnimationMotion::Init(const SkeletonResource* pResource, const SkeletalAnim::Motion& motion)
	{
		m_pMotions = vnew(ALLOC_ANIMATION) Motion[motion.anim_count];
		for (uint32 i = 0; i < motion.anim_count; i++)
		{
			m_pMotions[i].fWeightX = motion.anim[i].fLerpX;
			m_pMotions[i].fWeightY = motion.anim[i].fLerpY;
			m_pMotions[i].animation.Init(pResource, motion.anim[i].animName);
			m_pMotions[i].fPlaybackSpeed = motion.anim[i].playbackSpeed;
			m_pMotions[i].animation.SetSpeed(m_pMotions[i].fPlaybackSpeed);
			m_pMotions[i].animation.SetWeighting(0.0f);
		}
		m_uMotionCount = motion.anim_count;
	}

	void BlendAnimationMotion::Update(float fElapsed, float fWeighting)
	{
		for (uint32 i = 0; i < m_uMotionCount; i++)
		{
			m_pMotions[i].animation.Update(fElapsed);
		}
		// Weighting is controlled in the overriding parent code
	}

	void BlendAnimationMotion::SetSpeedMultiplier(float fMultiplier)
	{
		for (uint32 i = 0; i < m_uMotionCount; i++)
		{
			m_pMotions[i].animation.SetSpeed(m_pMotions[i].fPlaybackSpeed * fMultiplier);
		}
	}



	Blend1DAnimationMotion::Blend1DAnimationMotion()
	{
		m_uModifierIndexX = USG_INVALID_ID;
	}

	Blend1DAnimationMotion::~Blend1DAnimationMotion()
	{

	}

	void Blend1DAnimationMotion::Init(const SkeletonResource* pResource, const SkeletalAnim::Motion& motion)
	{
		ASSERT(motion.parameterX[0] != '\0');	// We need a valid modifier
		for (uint32 i = 0; i < m_uModifierCount; i++)
		{
			if (str::Compare(motion.parameterX, m_pModifiers[i].name))
			{
				m_uModifierIndexX = i;
				break;
			}
		}
		ASSERT(m_uModifierIndexX != USG_INVALID_ID);

		Inherited::Init(pResource, motion);
	}

	void Blend1DAnimationMotion::Update(float fElapsed, float fWeighting)
	{
		Inherited::Update(fElapsed, fWeighting);

		float fModifier = m_pModifiers[m_uModifierIndexX].value;
		uint32 uClosestOver = USG_INVALID_ID;
		uint32 uClosestUnder = USG_INVALID_ID;

		// Now find the upper and lower weighted animations
		// FIXME: When the tool is spitting these out we could assume they are ordered
		for (uint32 i = 0; i < m_uMotionCount; i++)
		{
			float fMotionWeight = m_pMotions[i].fWeightX;
			if(fMotionWeight <= fModifier)
			{
				if(uClosestUnder == USG_INVALID_ID || fMotionWeight > m_pMotions[uClosestUnder].fWeightX)
				{
					uClosestUnder = i;
				}
			}

			if (fMotionWeight >= fModifier)
			{
				if (uClosestOver == USG_INVALID_ID || fMotionWeight < m_pMotions[uClosestOver].fWeightX)
				{
					uClosestOver = i;
				}
			}
		}
		if (uClosestOver == uClosestUnder)
		{
			// We were exactly on a bounadary
			uClosestOver = USG_INVALID_ID;
		}

		// Now apply the weighting accordingly
		if (uClosestOver == USG_INVALID_ID || uClosestUnder == USG_INVALID_ID)
		{
			ASSERT(uClosestUnder != USG_INVALID_ID || uClosestOver != USG_INVALID_ID);
			m_uActiveAnims = 1;
			uint32 uIndex = uClosestUnder == USG_INVALID_ID ? uClosestOver : uClosestUnder;
			m_pActiveAnims[0] = &m_pMotions[uIndex].animation;
			m_pActiveAnims[0]->SetWeighting(fWeighting);
		}
		else
		{
			// Do a lerp
			Motion& lower = m_pMotions[uClosestUnder];
			Motion& upper = m_pMotions[uClosestOver];

			m_pActiveAnims[0] = &lower.animation;
			m_pActiveAnims[1] = &upper.animation;

			float fDist = upper.fWeightX - lower.fWeightX;
			float fLerp = (fModifier - lower.fWeightX) / fDist;

			m_pActiveAnims[0]->SetWeighting(fWeighting * (1.0f - fLerp));
			m_pActiveAnims[1]->SetWeighting(fWeighting * fLerp);

			m_uActiveAnims = 2;
		}
	}

}