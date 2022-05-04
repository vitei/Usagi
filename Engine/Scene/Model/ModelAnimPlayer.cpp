/****************************************************************************
//	Usagi Engine, Copyright Vitei, Inc. 2013
 ****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Model/AnimationChain.pb.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "ModelAnimPlayer.h"

namespace usg
{

	ModelAnimPlayer::ModelAnimPlayer() :
		m_activeAnims(6)
	{
		m_uStateCount = 0;
		m_uConditionCount = 0;
		m_uConditionValues = 0;
		m_pBoneInfo = nullptr;
		for (uint32 uState = 0; uState < MAX_STATES; uState++)
		{
			StateInfo& destState = m_states[uState];
			destState.szName[0] = '\0';
			destState.uTransitions = 0;
		}
	}

	ModelAnimPlayer::~ModelAnimPlayer()
	{
		CleanUp();
	}

	void ModelAnimPlayer::CleanUp()
	{
		for (uint32 uState = 0; uState < m_uStateCount; uState++)
		{
			StateInfo& destState = m_states[uState];
			if (destState.pAnimation)
			{
				destState.pAnimation->Reset();

				vdelete destState.pAnimation;
				destState.pAnimation = NULL;
			}

			destState.szName[0] = '\0';
			destState.uTransitions = 0;

		}

		if (m_pBoneInfo)
		{
			vdelete[] m_pBoneInfo;
			m_pBoneInfo = nullptr;
		}

		m_uStateCount = 0;
		m_uConditionCount = 0;
		m_uConditionValues = 0;
		m_context.pActiveState = NULL;
		m_context.pActiveTransition = NULL;
		m_context.uSubIndex = 0;
		m_context.bReverse = false;
		m_context.fStateTime = 0.0f;
		m_bRecalculateWeighting = true;

	}


	uint32 ModelAnimPlayer::GetStateIndex(SkeletalAnim::AnimChain &chain, const char* szName)
	{
		for (uint32 uState = 0; uState < chain.states_count; uState++)
		{
			const SkeletalAnim::State& state = chain.states[uState];
			if (str::Compare(szName, state.stateName))
			{
				return uState;
			}
		}
		ASSERT(false);
		return USG_INVALID_ID;
	}

	uint32 ModelAnimPlayer::GetConditionOrEventIndex(const char* szName)
	{
		for (uint32 i = 0; i < m_uConditionCount; i++)
		{
			if (str::Compare(m_conditions[i].szName, szName))
			{
				return i;
			}
		}
		// We don't have such a condition
		ASSERT(false);
		return USG_INVALID_ID;
	}

	uint32 ModelAnimPlayer::GetConditionIndex(const char* szName)
	{
		for (uint32 i = 0; i < m_uEventOffset; i++)
		{
			if (str::Compare(m_conditions[i].szName, szName))
			{
				return i;
			}
		}
		// We don't have such a condition
		ASSERT(false);
		return USG_INVALID_ID;
	}

	uint32 ModelAnimPlayer::GetConditionIndex(uint32 uNameHash)
	{
		for (uint32 i = 0; i < m_uEventOffset; i++)
		{
			if (m_conditions[i].uNameHash == uNameHash)
			{
				return i;
			}
		}
		// We don't have such a condition
		ASSERT(false);
		return USG_INVALID_ID;
	}

	uint32 ModelAnimPlayer::GetEventIndex(const char* szName)
	{
		for (uint32 i = m_uEventOffset; i < m_uConditionCount; i++)
		{
			if (str::Compare(m_conditions[i].szName, szName))
			{
				return i;
			}
		}
		// We don't have such a condition
		ASSERT(false);
		return USG_INVALID_ID;
	}

	bool ModelAnimPlayer::GetCondition(const char* szName)
	{
		uint32 uIndex = GetConditionIndex(szName);
		if (uIndex != USG_INVALID_ID)
		{
			return (m_uConditionValues & (1<<uIndex)) > 0;
		}
		ASSERT(false);
		return false;
	}

	bool ModelAnimPlayer::GetCondition(uint32 uNameHash)
	{
		uint32 uIndex = GetConditionIndex(uNameHash);
		if (uIndex != USG_INVALID_ID)
		{
			return (m_uConditionValues & (1 << uIndex)) > 0;
		}
		ASSERT(false);
		return false;
	}

	void ModelAnimPlayer::SetCondition(uint32 uNameHash, bool bValue)
	{
		uint32 uIndex = GetConditionIndex(uNameHash);
		if (uIndex != USG_INVALID_ID)
		{
			SetConditionWithIndex(uIndex, bValue);
		}
	}

	void ModelAnimPlayer::SetCondition(const char* szName, bool bValue)
	{
		uint32 uIndex = GetConditionIndex(szName);
		if(uIndex != USG_INVALID_ID)
		{
			SetConditionWithIndex(uIndex, bValue);
		}
	}

	void ModelAnimPlayer::TriggerEvent(const char* szName)
	{
		uint32 uIndex = GetEventIndex(szName);
		if (uIndex != USG_INVALID_ID)
		{
			SetConditionWithIndex(uIndex, true);
		}
	}

	void ModelAnimPlayer::SetConditionWithIndex(uint32 uCondition, bool bValue)
	{
		ASSERT(uCondition < m_uConditionCount);
		if (bValue)
		{
			m_uConditionValues |= (1<<uCondition);
		}
		else
		{
			m_uConditionValues &= ~(1 << uCondition);
		}
	}

	bool ModelAnimPlayer::Init(const SkeletonResource* pSkeleton, const char* szAnimChainName, bool bDefaultToBindPose)
	{
		// Make sure to reset any previous data, we can re-use these anim players
		CleanUp();

		m_pResource = pSkeleton;
		// First determine all the bones potentially updated by the anim set
		ProtocolBufferFile* pFile = ResourceMgr::Inst()->GetBufferedFile(szAnimChainName);
		if (!pFile)
			return false;
		// TODO: Move me onto the scratch as I'm likely to get quite large
		SkeletalAnim::AnimChain *pChain;
		ScratchObj<SkeletalAnim::AnimChain>  chainMem(pChain, 1);
		
		bool bReadSucceeded = pFile->Read(pChain);

		// First set-up our bones
		m_pBoneInfo = vnew(ALLOC_ANIMATION) BoneInfo[pSkeleton->GetBoneCount()];
		for (uint32 uBone = 0; uBone < pSkeleton->GetBoneCount(); uBone++)
		{
			m_pBoneInfo[uBone].bReferencedByAnim = false;
			m_pBoneInfo[uBone].fTotalWeighting = 0.0f;
		}

		// Now the conditions
		m_uEventClearMask = 0;
		m_uConditionValues = 0;
		m_uConditionCount = 0;
		for (uint32 uCondition = 0; uCondition < pChain->conditions_count; uCondition++)
		{
			m_uConditionCount++;
			str::Copy(m_conditions[uCondition].szName, pChain->conditions[uCondition].name, sizeof(m_conditions[uCondition].szName));
			m_conditions[uCondition].uNameHash = utl::CRC32(m_conditions[uCondition].szName);
			SetConditionWithIndex(uCondition, pChain->conditions[uCondition].value);
		}

		m_uEventOffset = m_uConditionCount;

		for (uint32 uEvent = 0; uEvent < pChain->events_count; uEvent++)
		{
			uint32 uIndex = m_uConditionCount;
			m_uConditionCount++;
			str::Copy(m_conditions[uIndex].szName, pChain->events[uEvent].name, sizeof(m_conditions[uIndex].szName));
			m_conditions[uIndex].uNameHash = utl::CRC32(m_conditions[uIndex].szName);
			SetConditionWithIndex(uIndex, pChain->events[uEvent].value);
			m_uEventClearMask |= (1<< uIndex);
		}

		// So we can do an &=
		m_uEventClearMask = ~m_uEventClearMask;

		// And now our animation sets
		uint32 uAnimCount = 0;

		// Clear the bones
		if (bDefaultToBindPose)
		{
			for (uint32 uBone = 0; uBone < m_pResource->GetBoneCount(); uBone++)
			{
				const SkeletonResource::Bone* pBone = pSkeleton->GetBoneByIndex(uBone);
				if (bDefaultToBindPose)
				{
					m_pBoneInfo[uBone].trans.vPos = pBone->vTranslate;
					m_pBoneInfo[uBone].trans.vScale = pBone->vScale;
					Matrix4x4 mRot = Matrix4x4::Identity();
					mRot.MakeRotate(pBone->vRotate.x, pBone->vRotate.y, pBone->vRotate.z);
					m_pBoneInfo[uBone].trans.qRot = mRot;
				}
			}
		}

		m_uModifiers = pChain->modifiers_count;
		for (uint32 i = 0; i < m_uModifiers; i++)
		{
			m_modifiers[i] = pChain->modifiers[i];
		}

		// First figure out the number of skeletal animations we need
		for (uint32 uState = 0; uState < pChain->states_count; uState++)
		{
			const SkeletalAnim::State& state = pChain->states[uState];
			StateInfo& destState = m_states[uState];
			str::Copy(destState.szName, state.stateName, sizeof(destState.szName));
			destState.uStateFlags = 0;
			if (state.bHidden)
				destState.uStateFlags |= STATE_FLAG_HIDDEN;
			m_uStateCount++;

			const SkeletalAnim::Animation animation = state.anim;
			bool bMotionValid = (state.eMotionType != SkeletalAnim::MOTION_TYPE_ANIM && state.eMotionType != SkeletalAnim::MOTION_TYPE_NONE || animation.animName[0] != '\0');

			if (bMotionValid)
			{
				switch (state.eMotionType)
				{
					case SkeletalAnim::MOTION_TYPE_ANIM:
					{
						SingleAnimationMotion* pMotion = vnew(ALLOC_ANIMATION) SingleAnimationMotion();
						pMotion->SetModifiers(m_modifiers, m_uModifiers);
						pMotion->Init(pSkeleton, state.anim);
						destState.pAnimation = pMotion;
						break;
					}
					case SkeletalAnim::MOTION_TYPE_1D_BLEND:
					{
						Blend1DAnimationMotion* pMotion = vnew(ALLOC_ANIMATION) Blend1DAnimationMotion();
						pMotion->SetModifiers(m_modifiers, m_uModifiers);
						pMotion->Init(pSkeleton, state.motion);
						destState.pAnimation = pMotion;
						break;
					}
					case SkeletalAnim::MOTION_TYPE_2D_BLEND:
					{
						// TODO: Implement me
						ASSERT(false);
						destState.pAnimation = NULL;
						break;
					}
					case SkeletalAnim::MOTION_TYPE_NONE:
					{
						// Using for not having skeletal anims
						destState.pAnimation = NULL;
						break;
					}
					
					default:
						destState.pAnimation = NULL;
						ASSERT(false);
				}
				AnimationMotion* pAnimDest= destState.pAnimation;


				for (uint32 uBone = 0; uBone < m_pResource->GetBoneCount(); uBone++)
				{
					m_pBoneInfo[uBone].bReferencedByAnim |= pAnimDest ? pAnimDest->IsBoneReferenced(uBone) : false;
				}

				destState.bAnimValid = true;
			}
			else
			{
				destState.pAnimation = NULL;
				destState.bAnimValid = false;
			}

			for (uint32 uTransition = 0; uTransition < state.transitions_count; uTransition++)
			{
				const SkeletalAnim::Transition& transition = state.transitions[uTransition];
				Transition& destTransition = destState.transitions[uTransition];
				destTransition.uTargetState = GetStateIndex(*pChain, transition.targetState);
				destTransition.uSourceState = uState;	// Convenience so we can easily reverse an anim
				destTransition.fBlendTime = transition.blendInTime;
				destTransition.eBlendType = transition.blendType;
				destTransition.eCrossFade = transition.fadeType;
				destTransition.uConditionMask = 0;
				destTransition.uConditionValues = 0;
				destTransition.bWaitOnAnim = transition.bWaitOnAnim;
				// Set up the conditions necessary to enter these transitions
				for (uint32 uCondition = 0; uCondition < transition.conditions_count; uCondition++)
				{
					const SkeletalAnim::Condition& condition = transition.conditions[uCondition];
					uint32 uIndex = GetConditionOrEventIndex(condition.name);
					if (uIndex != USG_INVALID_ID)
					{
						destTransition.uConditionMask |= (1<<uIndex);
						if (condition.value)
						{
							destTransition.uConditionValues |= (1 << uIndex);
						}
					}
				}
		
				destState.uTransitions++;
			}
		}

		// Set up our initial state
		m_context.pActiveState = &m_states[GetStateIndex(*pChain, pChain->entryState)];
		if (m_context.pActiveState->bAnimValid)
		{
			m_context.pActiveState->pAnimation->Play();
		}
		m_context.pActiveTransition = NULL;	// Don't start off in a transition
		m_context.bReverse = false;

		return true;

	}


	void ModelAnimPlayer::StartTransition(Transition& trans)
	{
		m_context.pActiveTransition = &trans;
		m_context.fStateTime = 0.0f;
		m_context.uSubIndex = 0;
		m_context.bReverse = false;
		if (trans.fBlendTime == 0.0f)
		{
			m_context.pActiveTransition = NULL;
			m_context.pActiveState = &m_states[trans.uTargetState];
			if (m_context.pActiveState->bAnimValid)
			{
				m_context.pActiveState->pAnimation->Play();
			}
		}
		else
		{
			if (m_context.pActiveState && m_context.pActiveState->bAnimValid)
			{
				if (trans.eCrossFade == SkeletalAnim::FADE_FROZEN)
				{
					m_context.pActiveState->pAnimation->Stop();
				}
				else if (trans.eCrossFade == SkeletalAnim::FADE_REVERSE)
				{
					// Reverse the current animation, such as for closing doors
					m_context.pActiveState->pAnimation->SetSpeedMultiplier(-1.0f);
				}
			}
			m_context.pActiveTransition = &trans;
			m_context.fStateTime = 0.0f;
			m_context.bReverse = false;
			m_context.pActiveState = NULL;	// We consider ourselves to be *in* a transition so don't hold the active state

			// Start to play the destination state so we can blend into it
			m_states[trans.uTargetState].pAnimation->Play();
			// TODO: Add the option to stop the source animation so we blend out of a still pose
		}
		m_bRecalculateWeighting = true;
	}


	void ModelAnimPlayer::ClearWeighting()
	{
		for (uint32 uBone = 0; uBone < m_pResource->GetBoneCount(); uBone++)
		{
			m_pBoneInfo[uBone].fTotalWeighting = 0.0f;
		}
	}

	void ModelAnimPlayer::CalculateWeighting()
	{
		ClearWeighting();

		for (SkeletalAnimation* pAnim : m_activeAnims)
		{
			for (uint32 i = 0; i < m_pResource->GetBoneCount(); i++)
			{
				if (m_pBoneInfo[i].bReferencedByAnim)
				{
					m_pBoneInfo[i].fTotalWeighting += pAnim->GetWeighting();
				}
			}
		}

		// TODO: Apply the animation weighting to the bones used by them
		m_bRecalculateWeighting = false;
	}

	void ModelAnimPlayer::ApplyBlendedAnim()
	{
		CalculateWeighting();

		for (uint32 uBone = 0; uBone < m_pResource->GetBoneCount(); uBone++)
		{
			// FIXME: Animations should contrain unmodified bones for lerping perposes
			if (m_pBoneInfo[uBone].fTotalWeighting > 0.0f)
			{
				m_pBoneInfo[uBone].trans.vPos = Vector3f::ZERO;
				m_pBoneInfo[uBone].trans.qRot.Assign(0.0f, 0.0f, 0.0f, 0.0f);
				m_pBoneInfo[uBone].trans.vScale = Vector3f::ZERO;
			}
		}

		// Now got through the list of active anims and apply them based on their weighting
		for (SkeletalAnimation* pAnim : m_activeAnims)
		{
			if (pAnim->GetWeighting() > 0.0f)
			{
				ApplyBoneTransforms(pAnim);
			}
		}

		for (uint32 uBone = 0; uBone < m_pResource->GetBoneCount(); uBone++)
		{
			// FIXME: Animations should contrain unmodified bones for lerping perposes
			if (m_pBoneInfo[uBone].fTotalWeighting > 0.0f)
			{
				m_pBoneInfo[uBone].trans.qRot.Normalise();
			}
		}
	}

	void ModelAnimPlayer::ApplyBoneTransformsNoWeights(SkeletalAnimation* pAnim)
	{
		for (uint32 i = 0; i < m_pResource->GetBoneCount(); i++)
		{
			if (m_pBoneInfo[i].bReferencedByAnim)
			{
				pAnim->GetTransform(i, m_pBoneInfo[i].trans);
			}
		}
	}

	void ModelAnimPlayer::ApplyBoneTransforms(SkeletalAnimation* pAnim)
	{
		SkeletalAnimationResource::Transform trans;
		for (uint32 i = 0; i < pAnim->GetNumberOfTargets(); i++)
		{
			if (m_pBoneInfo[i].bReferencedByAnim)
			{
				float fFrac = pAnim->GetWeighting() / m_pBoneInfo[i].fTotalWeighting;
				pAnim->GetTransform(i, trans);
				m_pBoneInfo[i].trans.vPos += (trans.vPos * fFrac);
				// TODO: Confirm it's ok to blend the rotation in this manner
				AddQuaternionWeight(trans.qRot, fFrac, m_pBoneInfo[i].trans.qRot);
				m_pBoneInfo[i].trans.vScale += (trans.vScale * fFrac);
			}
		}
	}

	void ModelAnimPlayer::TryToTransition()
	{
		if (m_context.pActiveState)
		{
			AnimationMotion* pAnim = m_context.pActiveState->bAnimValid ? m_context.pActiveState->pAnimation : NULL;
			bool bAnimFinished = !pAnim || !pAnim->IsPlaying();
			StateInfo* pState = m_context.pActiveState;
			for (uint32 i = 0; i < pState->uTransitions; i++)
			{
				if( bAnimFinished || !pState->transitions[i].bWaitOnAnim )
				if ((pState->transitions[i].uConditionMask & m_uConditionValues) == pState->transitions[i].uConditionValues)
				{
					// Go to the next state
					StartTransition(pState->transitions[i]);
					break;
				}
			}
		}
	}

	bool ModelAnimPlayer::IsInState(const char* szName) const
	{
		if (m_context.pActiveState)
		{
			return str::Compare(m_context.pActiveState->szName, szName);
		}

		return false;
	}

	float ModelAnimPlayer::GetCubicBlend(float fU)
	{
		float fV = 1 - fU;
		float fBezier = ((3.f * fV*(fU*fU)) + (fU*fU*fU));
		return fBezier;
	}

	void ModelAnimPlayer::Update(float fElapsed)
	{
		m_context.fStateTime += fElapsed;

		TryToTransition();

		m_activeAnims.clear();

		// TODO: This is a VERY simple no blended chained animation system but we could easily expand this to allow for blending
		// FIXME: Not good logic, only valid whilst we are testing with a single animation
		bool bCanTransition = true;

		if(m_context.pActiveTransition)
		{
			ASSERT(m_context.pActiveTransition != NULL);
			bCanTransition = false;	// We aren't allowing transitioning from inside a transition
			Transition* pTransition = m_context.pActiveTransition;
			float fLerp = m_context.fStateTime / pTransition->fBlendTime;

			if (fLerp >= 1.0f)
			{
				m_context.pActiveState = &m_states[pTransition->uTargetState];
				m_context.pActiveTransition = NULL;
			}
			else
			{
				AnimationMotion* pFrom = m_states[pTransition->uSourceState].pAnimation;
				AnimationMotion* pTo = m_states[pTransition->uTargetState].pAnimation;

				if (pTransition->eBlendType == SkeletalAnim::BLEND_CUBIC)
				{
					fLerp = GetCubicBlend(fLerp);
				}

				pFrom->Update(fElapsed, (1.0f - fLerp));
				pTo->Update(fElapsed, fLerp);
				pFrom->AppendToAnimList(m_activeAnims);
				pTo->AppendToAnimList(m_activeAnims);
			}
		}

		if (m_context.pActiveState)
		{
			AnimationMotion* pAnim = m_context.pActiveState->bAnimValid ? m_context.pActiveState->pAnimation : NULL;
			if (pAnim)
			{
				pAnim->Update(fElapsed, 1.0f);
				pAnim->AppendToAnimList(m_activeAnims);			
			}
		}

//		if (IsVisible())
		{
			switch (m_activeAnims.size())
			{
			case 0:
				// Do nothing
				break;
			case 1:
				ApplyBoneTransformsNoWeights(*m_activeAnims.begin());
				break;
			default:
				ApplyBlendedAnim();
				break;
			}
		}


		// Clear out the events
		m_uConditionValues &= m_uEventClearMask;
	}


	const SkeletalAnimationResource::Transform* ModelAnimPlayer::GetTransform(uint32 uBoneIndex)
	{
		ASSERT(uBoneIndex != USG_INVALID_ID);
		// TODO: What to do about the default pose?

		return &m_pBoneInfo[uBoneIndex].trans;
	}
	

	bool ModelAnimPlayer::GetIndex(const char* szBoneName, uint32 &indexOut)
	{
		// TODO: Perhaps we should use different indices than the skeleton, holding information only for bones which are manipulated by the model
		indexOut = m_pResource->GetBoneIndex(szBoneName);
		return m_pBoneInfo[indexOut].bReferencedByAnim;
	}


	void ModelAnimPlayer::SetModifierValue(const char* szName, float fValue)
	{
		for (uint32 uModifier = 0; uModifier < m_uModifiers; uModifier++)
		{
			SkeletalAnim::Modifier& modifier = m_modifiers[uModifier];
			if (str::Compare(szName, modifier.name))
			{
				modifier.value = fValue;
				return;
			}
		}
		// We don't have such a modifier value
		ASSERT(false);
	}

}