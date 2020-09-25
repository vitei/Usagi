/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Stats which should always be available regardless of the mode
*****************************************************************************/
#pragma once
#ifndef USG_DEBUG_SCENE_DEBUG_STATS_H
#define USG_DEBUG_SCENE_DEBUG_STATS_H

#include "Engine/Debug/Rendering/IDebugStatGroup.h"
#include "Engine/Core/String/FixedString.h"
#include "Engine/Graphics/Color.h"

namespace usg
{
	class ProfilingTimer;
	class Debug3D;
	class Scene;

	class SceneDebugStats : public IDebugStatGroup
	{
	public:
		SceneDebugStats();
		~SceneDebugStats();

		void Init(Debug3D* pDebug3D, Scene* pScene);
		void Cleanup(GFXDevice* pDevice);
		void Update(float fElapsed) override;

		void Draw(DebugRender* pRender) override;
		void PreDraw(GFXDevice* pDevice) override;
		void PostDraw(GFXDevice* pDevice) override;
		void SetActive(bool bActive) override;
		void SetPage(uint32 uPage) override;

		uint32 GetPageCount() const { return PAGE_COUNT; }

		void RegisterTimer( const char* szName, const ProfilingTimer* pTimer, Color color = Color( 0.0f, 1.0f, 0.0f ), float fOffset = 0.0f );
		void SetBehaviorTreeDebugData(const LabelString* apsBehaviors, uint32* auLayers, uint32 uCount, bool bCanSeeTarget);
		void SetAgentNavigationData(usg::Vector3f* path, uint32 uPathCount, const usg::Vector3f& vPos, const usg::Vector3f& vTargetPos, const usg::Vector3f& vTarget, bool bHasTarget);

	private:
		float DrawTimerStat(DebugRender* pRender, uint32 i, float fPos, float fMinPass = 0.0f);
		void DrawScenePage(DebugRender* pRender);
		void DrawBehaviorTreePage(DebugRender* pRender);
		void DrawTimerPage(DebugRender* pRender);

		enum GLOBAL_PAGES
		{
			PAGE_SCENE = 0,
			PAGE_BEHAVIOUR_TREE,
			PAGE_TIMERS,
			PAGE_COUNT
		};

		enum
		{
			MAX_PROFILE_TIMERS = 240
		};

		static const uint32 NUM_BH = 64;
		static const uint32 MAX_BH_SIZE = 32;

		Debug3D*		m_pDebug3D;
		Scene*			m_pScene;

		LabelString		m_behaviors[NUM_BH];

		bool			m_bAgentHasTarget;

		uint32			m_layer[NUM_BH];

		usg::Vector3f	m_avPath[32];
		usg::Vector3f	m_vAgentPos;
		usg::Vector3f	m_vAgentTargetPos;
		usg::Vector3f	m_vTarget;

		uint32			m_uBehaviorCounter;
		uint32			m_uPathCounter;

		bool			m_bCanSeeTarget;


		struct TimerInfo
		{
			const ProfilingTimer*	pTimer;
			const char*				szName;
			Color					color;
			float 					fMaxFrame;
			float					fOffset;
		};

		TimerInfo				m_timers[MAX_PROFILE_TIMERS];

		uint32					m_uTimers;

	};

}

#endif // USG_DEBUG_STAT_GROUP_H
