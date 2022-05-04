/****************************************************************************
//	Usagi Engine, Copyright Vitei, Inc. 2013
//	Description: Calculate modifications of model bones and UVs based on
//	sketal and material animations
*****************************************************************************/
#ifndef _USG_SCENE_MODEL_MODEL_ANIM_PLAYER_H_
#define _USG_SCENE_MODEL_MODEL_ANIM_PLAYER_H_


#include "Engine/Core/Utility.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Resource/SkeletalAnimationResource.h"
#include "Engine/Scene/Model/SkeletalAnimation.h"
#include "Engine/Scene/Model/AnimationChain.pb.h"
#include "Engine/Scene/Model/AnimationMotion.h"

namespace usg {

class Model;
class ModelResource;

class ModelAnimPlayer
{
public:
	ModelAnimPlayer();
	~ModelAnimPlayer();

	// Give a root bone name if only updating a sub-element
	bool Init(const SkeletonResource* pSkeleton, const char* szAnimChain, bool bDefaultToBindPose);

	void Update(float fElapsed);
	const SkeletalAnimationResource::Transform* GetTransform(uint32 uBoneIndex);
	bool ControlsBone(uint32 uBoneIndex) { return m_pBoneInfo[uBoneIndex].bReferencedByAnim; }
	void SetCondition(const char* szName, bool bValue);
	void SetCondition(uint32 uNameHash, bool bValue);
	bool GetCondition(const char* conditionName);
	bool GetCondition(uint32 uNameHash);
	void TriggerEvent(const char* szName);
	void SetModifierValue(const char* szName, float fValue);
	bool IsVisible();
	void AddQuaternionWeight(const Quaternionf& src, float fWeight, Quaternionf& inOut);
	bool IsInState(const char* szName) const;

private:
	void CleanUp();
	uint32 GetConditionIndex(const char* szName);
	uint32 GetConditionIndex(uint32 uNameHash);
	uint32 GetEventIndex(const char* szName);
	uint32 GetConditionOrEventIndex(const char* szName);
	void SetConditionWithIndex(uint32 uCondition, bool bValue);
	bool GetIndex(const char* szBoneName, uint32 &indexOut);
	void ClearWeighting();
	void CalculateWeighting();
	uint32 GetStateIndex(SkeletalAnim::AnimChain &chain, const char* szName);
	void ApplyBoneTransforms(SkeletalAnimation* pAnim);
	void ApplyBoneTransformsNoWeights(SkeletalAnimation* pAnim);
	void ApplyBlendedAnim();
	float GetCubicBlend(float fLerp);

	PRIVATIZE_COPY(ModelAnimPlayer)

		enum
	{
		MAX_ANIMS_PER_BONE = 3,
		MAX_TRANSITIONS = SkeletalAnim::State::transitions_max_count,
		LABEL_LENGTH = 64,
		MAX_STATES = SkeletalAnim::AnimChain::states_max_count,
		MAX_CONDITIONS = SkeletalAnim::AnimChain::conditions_max_count + SkeletalAnim::AnimChain::events_max_count,
		MAX_MODIFIERS = SkeletalAnim::AnimChain::modifiers_max_count,
	};

	enum
	{
		STATE_FLAG_HIDDEN = (1<<0)
	};

	struct BoneInfo
	{
		float	fTotalWeighting;
		bool	bReferencedByAnim;
		SkeletalAnimationResource::Transform trans;
	};

	struct Transition
	{
		uint32						uSourceState;	// Convenience so we don't have to find the owner to reverse the anim
		uint32						uTargetState;
		uint32						uConditionMask;		// Mask of the conditions we care about
		uint32						uConditionValues;	// The value those conditions should be set to
		SkeletalAnim::BlendType 	eBlendType;
		SkeletalAnim::CrossFade 	eCrossFade;
		float						fBlendTime;
		bool						bWaitOnAnim;
	};

	void StartTransition(Transition& trans);
	void TryToTransition();

	struct StateInfo
	{
		char					szName[LABEL_LENGTH];
		Transition				transitions[MAX_TRANSITIONS];
		AnimationMotion*		pAnimation;
		uint32					uTransitions;
		bool					bAnimValid;	// Should ensure this, but don't have the necessary anims yet
		uint32					uStateFlags;
	};

	struct Context
	{
		StateInfo*			pActiveState;
		Transition*			pActiveTransition;
		uint32				uSubIndex;	// For referencing which anim in a chain we have triggered
		float				fStateTime;
		bool				bReverse;
	};

	struct ConditionInfo
	{
		char				szName[LABEL_LENGTH];
		uint32				uNameHash;
	};

	SkeletalAnim::Modifier		m_modifiers[MAX_MODIFIERS];
	uint32						m_uModifiers;
	list<SkeletalAnimation*>	m_activeAnims;	// Only valid during the update loop for now
	ConditionInfo				m_conditions[MAX_CONDITIONS];
	Context						m_context;
	bool						m_bRecalculateWeighting;
	StateInfo					m_states[MAX_STATES];
	uint32						m_uStateCount;
	uint32						m_uConditionCount;
	uint32						m_uEventOffset;
	uint32						m_uConditionValues;	// A mask of all the currently active, disabled conditions
	uint32						m_uEventClearMask;
	BoneInfo*					m_pBoneInfo;
	const SkeletonResource*		m_pResource;
};

inline bool ModelAnimPlayer::IsVisible()
{
	if (m_context.pActiveState)
	{
		return ((m_context.pActiveState->uStateFlags & STATE_FLAG_HIDDEN) == 0);
	}

	return true;
}

inline void ModelAnimPlayer::AddQuaternionWeight(const Quaternionf& src, float fWeight, Quaternionf& inOut)
{
	if (((inOut.x * src.x + inOut.y * src.y + inOut.z * src.z + inOut.w * src.w) < 0.0f))
	{
		fWeight = -fWeight;
	}
	inOut.x += src.x * fWeight;
	inOut.y += src.y * fWeight;
	inOut.z += src.z * fWeight;
	inOut.w += src.w * fWeight;
}

}

#endif	// #ifndef _USG_SCENE_MODEL_MODEL_ANIM_PLAYER_H_

