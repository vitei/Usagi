/****************************************************************************
//	Usagi Engine, Copyright � Vitei, Inc. 2013
//	Description: Classes for the various animation state motion types
*****************************************************************************/
#ifndef _USG_SCENE_MODEL_MODEL_ANIM_MOTION_H_
#define _USG_SCENE_MODEL_MODEL_ANIM_MOTION_H_

#include "Engine/Core/stl/list.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Resource/SkeletalAnimationResource.h"
#include "Engine/Scene/Model/SkeletalAnimation.h"
#include "Engine/Scene/Model/AnimationChain.pb.h"

namespace usg
{

	class Model;
	class ModelResource;


	class AnimationMotion
	{
	public:
		AnimationMotion();
		virtual ~AnimationMotion() {}

		void SetModifiers(const SkeletalAnim::Modifier* pModifiers, uint32 uModifierCount);
		virtual void Update(float fElapsed, float fWeighting) = 0;
		virtual SkeletalAnimation* GetActiveAnim(uint32 uIndex) = 0;
		virtual uint32 GetActiveAnimCount() const = 0;
		virtual SkeletalAnimation* GetAnim(uint32 uIndex) = 0;
		virtual void SetSpeedMultiplier(float fMultiplier) = 0;
		void AppendToAnimList(list<SkeletalAnimation*>& list);
		void Reset();
		bool IsBoneReferenced(uint32 uBoneId);
		void Play();
		void Stop();
		bool IsPlaying();

	protected:
		virtual uint32 GetAnimCount() = 0;
		const SkeletalAnim::Modifier*	m_pModifiers;
		uint32							m_uModifierCount;
	};

	class SingleAnimationMotion : public AnimationMotion
	{
	public:
		SingleAnimationMotion();
		virtual ~SingleAnimationMotion();


		void Init(const SkeletonResource* pResource, const SkeletalAnim::Animation& anim);
		virtual void Update(float fElapsed, float fWeighting);
		virtual SkeletalAnimation* GetActiveAnim(uint32 uIndex) { ASSERT(uIndex == 0); return &m_animation; }
		virtual SkeletalAnimation* GetAnim(uint32 uIndex) { ASSERT(uIndex == 0); return &m_animation; }
		virtual uint32 GetActiveAnimCount() const { return 1; }
		virtual void SetSpeedMultiplier(float fMultiplier);
		virtual uint32 GetAnimCount() { return 1; }

	private:
		SkeletalAnimation	m_animation;
		float				m_fSpeed;
	};


	class BlendAnimationMotion : public AnimationMotion
	{
	public:
		BlendAnimationMotion();
		virtual ~BlendAnimationMotion();


		virtual void Init(const SkeletonResource* pResource, const SkeletalAnim::Motion& motion);
		virtual void Update(float fElapsed, float fWeighting);
		virtual SkeletalAnimation* GetActiveAnim(uint32 uIndex) { return m_pActiveAnims[uIndex]; }
		virtual SkeletalAnimation* GetAnim(uint32 uIndex) { return &m_pMotions[uIndex].animation; }
		virtual uint32 GetActiveAnimCount() const { return m_uActiveAnims; }
		virtual void SetSpeedMultiplier(float fMultiplier);
		virtual uint32 GetAnimCount() { return m_uMotionCount; }

	protected:

		enum
		{
			MAX_ACTIVE_ANIMS = 3	// 1D blend is 2, 2d is 3
		};
		struct Motion
		{
			SkeletalAnimation	animation;
			float				fWeightX;
			float				fWeightY;
			float				fPlaybackSpeed;
		};

		SkeletalAnimation*	m_pActiveAnims[MAX_ACTIVE_ANIMS];
		Motion*				m_pMotions;
		uint32				m_uMotionCount;
		uint32				m_uActiveAnims;
	};

	class Blend1DAnimationMotion : public BlendAnimationMotion
	{
		typedef BlendAnimationMotion Inherited;
	public:
		Blend1DAnimationMotion();
		virtual ~Blend1DAnimationMotion();

		virtual void Init(const SkeletonResource* pResource, const SkeletalAnim::Motion& motion);
		virtual void Update(float fElapsed, float fWeighting);

	private:
		uint32 m_uModifierIndexX;

	};

	// GEA p587
#if 0
	class Blend2DAnimationMotion : public BlendAnimationMotion
	{

	};
#endif

}

#endif	// #ifndef _USG_SCENE_MODEL_MODEL_ANIM_MOTION_H_

