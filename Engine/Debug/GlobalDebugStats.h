/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Stats which should always be available regardless of the mode
*****************************************************************************/
#pragma once
#ifndef USG_DEBUG_GLOBAL_DEBUG_STATS_H
#define USG_DEBUG_GLOBAL_DEBUG_STATS_H

#include "Engine/Debug/Rendering/IDebugStatGroup.h"
#include "Engine/Core/String/FixedString.h"


namespace usg
{

	class ProfilingTimer;

	class GlobalDebugStats : public IDebugStatGroup
	{
	public:
		GlobalDebugStats();
		~GlobalDebugStats();

		void Update(float fElapsed) override;

		void Draw(DebugRender* pRender) override;
		void PreDraw(GFXDevice* pDevice) override;
		void PostDraw(GFXDevice* pDevice) override;

		uint32 GetPageCount() const { return PAGE_COUNT; }
		void AppendWarnings(usg::string& string) override;

		void SetThreadActivity(int iIdx, const char* szActivitys);
		void RegisterCPUTimer(const ProfilingTimer* pTimer) { m_pCpuTimer = pTimer; }

		enum GLOBAL_PAGES
		{
			PAGE_MAIN = 0,
			PAGE_TIMING,
			PAGE_MEMORY,
			PAGE_PHYSICS,
			//	PAGE_THREADS,
			PAGE_COUNT
		};

	private:
		void DrawTimingPage(DebugRender* pRender);
		void DrawThreadsPage(DebugRender* pRender);
		void DrawMemoryPage(DebugRender* pRender, const MemHeap* pHeap, const char* szName);
		float DrawMemoryStat(DebugRender* pRenderer, const MemHeap* pHeap, MemAllocType eType, float fPos, float fMaxSize);

		void UpdateFPS(float fElapsed);



		enum 
		{
			WARNING_CPU_HEAVY = 0,
			WARNING_GPU_HEAVY,
			WARNING_MEMORY,
			WARNING_COUNT
		};

		static const uint32 NUM_THREADS = 16;

		const ProfilingTimer*	m_pCpuTimer;
		LabelString				m_currentThread[NUM_THREADS];
		LabelString				m_lastThread[NUM_THREADS];
		Warning					m_warnings[WARNING_COUNT];
		float					m_fTotalElapsed;
		float					m_fMaxElapsed;
		uint32					m_uFrameCounter;
		uint32					m_uFrameRate;
		float					m_fCPUTime;
		float					m_fGPUTime;
		float					m_fTargetCPUTime;
		float					m_fTargetGPUTime;
	};

}

#endif // USG_DEBUG_STAT_GROUP_H
