/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Stats which should always be available regardless of the mode
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Core/Timer/ProfilingTimer.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Scene.h"
#include "SceneDebugStats.h"

namespace usg {

	SceneDebugStats::SceneDebugStats() :
		IDebugStatGroup(),
		m_pDebug3D(nullptr),
		m_pScene(nullptr),
		m_uBehaviorCounter(0),
		m_uPathCounter(0),
		m_uTimers(0)
	{

	}

	SceneDebugStats::~SceneDebugStats()
	{

	}

	void SceneDebugStats::Init(Debug3D* pDebug3D, Scene* pScene)
	{
		m_pDebug3D = pDebug3D;
		m_pScene = pScene;

		m_uBehaviorCounter = 0;
		m_bAgentHasTarget = false;
	}

	void SceneDebugStats::Cleanup(GFXDevice* pDevice)
	{
	
	}

	void SceneDebugStats::Update(float fElapsed)
	{

	}

	void SceneDebugStats::Draw(DebugRender* pRender)
	{
		switch (m_uActivePage)
		{
		case PAGE_SCENE:
			DrawScenePage(pRender);
			break;
		case PAGE_BEHAVIOUR_TREE:
			DrawBehaviorTreePage(pRender);
			break;
		case PAGE_TIMERS:
			DrawTimerPage(pRender);
			break;
		default:
			ASSERT(false);
		}
	}

	void SceneDebugStats::PreDraw(GFXDevice* pDevice)
	{

	}

	void SceneDebugStats::PostDraw(GFXDevice* pDevice)
	{

	}

	void SceneDebugStats::SetActive(bool bActive)
	{
		if (!bActive)
		{
			for (uint32 i = 0; i < m_uTimers; i++)
			{
				m_timers[i].fMaxFrame = 0.0f;
			}
		}
		IDebugStatGroup::SetActive(bActive);

		if (!bActive)
		{
			m_pScene->ShowBounds(false);
		}
	}


	void SceneDebugStats::SetPage(uint32 uPage)
	{
		m_pScene->ShowBounds(uPage == PAGE_SCENE);
		IDebugStatGroup::SetPage(uPage);
	}


	void SceneDebugStats::DrawTimerPage(DebugRender* pRender)
	{
		Color cTitleCol(1.0f, 0.0f, 0.0f, 1.0f);
		float fPos = 1.0f;

		pRender->AddString("Profiling timers", 0.0f, 0.0f, cTitleCol);
		static float fMinTime = 0.1f;
		float fMinPass = 30.f;
		uint32 uVisible = 0;

		for (uint32 i = 0; i < m_uTimers; i++)
		{
			if (m_timers[i].szName && (m_timers[i].pTimer->GetTimerTotal() > 0))
			{
				m_timers[i].fMaxFrame = Math::Max(m_timers[i].pTimer->GetTotalMilliSeconds(), m_timers[i].fMaxFrame);

				if (m_timers[i].fMaxFrame > fMinTime)
				{
					fPos++;
					fMinPass = DrawTimerStat(pRender, i, fPos, fMinPass);
					uVisible++;
				}
			}
		}

		if (uVisible > 19)
		{
			fMinTime = fMinPass;
		}

		// ruler
		//const float fLineHeight = m_pRender->GetLineHeight();
		//const float boldness = 0.001f;
		//m_pRender->AddRect( barBegin, fPos * fLineHeight, boldness, m_uTimers * fLineHeight, Color( 1.0f, 1.0f, 1.0f ) );
		//m_pRender->AddRect( barBegin + barLength, fPos * fLineHeight, boldness, m_uTimers * fLineHeight, Color( 1.0f, 1.0f, 1.0f ) );
	}


	float SceneDebugStats::DrawTimerStat(DebugRender* pRender, uint32 i, float fPos, float fMinPass)
	{
		Color cTitleCol(1.0f, 0.0f, 0.0f, 1.0f);

		const float oneFrame = (1000.0f / 30.0f);

		const float barBegin = 0.135f;
		const float barLength = 0.55f;

		float fProfileOffset = 0.3f;
		float fSpikeProfileOffset = 0.5f;

		usg::string tmpString;

		// name
		pRender->AddString(m_timers[i].szName, m_timers[i].fOffset, fPos, m_timers[i].color);

		// msec
		float fAverage = m_timers[i].pTimer->GetTotalMilliSeconds();
		tmpString = str::ParseString("%.2fms", fAverage);
		pRender->AddString(tmpString.c_str(), fProfileOffset, fPos, m_timers[i].color);

		// spike msec
		float fAverageSpike = m_timers[i].fMaxFrame;
		tmpString = str::ParseString("%.2fms", fAverageSpike);
		pRender->AddString(tmpString.c_str(), fSpikeProfileOffset, fPos, m_timers[i].color);

		Color backGroundCol = m_timers[i].color;
		backGroundCol.a() = 0.35f;
		// bar
		float percent = fAverage / oneFrame;
		float maxPercent = fAverageSpike / oneFrame;
		pRender->AddBar(fPos, barBegin, barLength * maxPercent, backGroundCol);
		pRender->AddBar(fPos, barBegin, barLength * percent, m_timers[i].color);

		return Math::Min(fMinPass, m_timers[i].fMaxFrame);
	}

	void SceneDebugStats::DrawScenePage(DebugRender* pRender)
	{
		Color cTitleCol(1.0f, 0.0f, 0.0f, 1.0f);
		Color cTextCol(0.0f, 1.0f, 0.0f, 1.0f);
		Color cStatCol(0.0f, 1.0f, 1.0f, 1.0f);

		float fPos = 2.0f;
		pRender->AddString("Scene Info", 0.0f, 0.0f, cTitleCol);
		usg::string tmpString;

		List<SceneContext>& sceneContexts = m_pScene->GetSceneContexts();

		uint32 uContextCount = sceneContexts.GetSize();
		tmpString = str::ParseString("Number of scenes: %d", uContextCount);
		pRender->AddString(tmpString.c_str(), 0.0f, fPos, cTextCol);

		fPos += 2.0f;
		uint32 uContext = 0;
		for (List<SceneContext>::Iterator it = sceneContexts.Begin(); !it.IsEnd(); ++it)
		{
			if ((*it)->IsActive())
			{
				tmpString = str::ParseString("Scene Context: %d showing %d / %d cullable items", uContext, (*it)->GetVisiblePVSCount(), m_pScene->GetPVSCount());
				pRender->AddString(tmpString.c_str(), 0.0f, fPos, cStatCol);
				fPos += 1.0f;
			}
			uContext++;
		}
	}

	void SceneDebugStats::DrawBehaviorTreePage(DebugRender* pRender)
	{
		Color cTargetColText(1.0f, 0.0f, 0.0f, 1.0f);
		Color cPosColText(0.0f, 1.0f, 0.0f, 1.0f);
		Color cDestinationColText(0.0f, 0.0f, 1.0f, 1.0f);
		Color cBHText(1.0f, 1.0f, 0.0f, 1.0f);

		Color cTargetCol(1.0f, 0.0f, 0.0f, 0.4f);
		Color cPosCol(0.0f, 1.0f, 0.0f, 0.5f);
		Color cDestinationCol(0.0f, 0.0f, 1.0f, 0.5f);

		string tmp;
		float fLine = 1.0f;
		tmp = str::ParseString("Pos: %.2f, %.2f, %.2f", m_vAgentPos.x, m_vAgentPos.y, m_vAgentPos.z);
		pRender->AddString(tmp.c_str(), 0.0f, fLine++, cPosColText);
		tmp = str::ParseString("Destination Pos: %.2f, %.2f, %.2f", m_vAgentTargetPos.x, m_vAgentTargetPos.y, m_vAgentTargetPos.z);
		pRender->AddString(tmp.c_str(), 0.0f, fLine++, cDestinationColText);

		if (m_bAgentHasTarget)
		{
			tmp = str::ParseString("Enemy Target: %.2f, %.2f, %.2f", m_vTarget.x, m_vTarget.y, m_vTarget.z);
			m_pDebug3D->AddSphere(m_vTarget, 3.0f, cTargetCol);
		}
		else
		{
			tmp = str::ParseString("Enemy Target: None");
		}

		pRender->AddString(tmp.c_str(), 0.0f, fLine++, cTargetColText);

		m_pDebug3D->AddSphere(m_vAgentPos, 3.0f, cPosCol);
		m_pDebug3D->AddSphere(m_vAgentTargetPos, 2.0f, cDestinationCol);

		tmp = str::ParseString("CanSeeTarget: %s", m_bCanSeeTarget ? "Yes" : "No");
		pRender->AddString(tmp.c_str(), 0.0f, fLine++, cTargetColText);

		for (uint32 i = 0; i < m_uBehaviorCounter; i++)
		{
			pRender->AddString(m_behaviors[i].CStr(), (float)(m_layer[i]) * 0.01f, (float)(i + ((uint32)fLine + 1)), cBHText);
		}
	}

	void SceneDebugStats::SetBehaviorTreeDebugData(const LabelString* apsBehaviors, uint32* auLayers, uint32 uCount, bool bCanSeeTarget)
	{
		if (m_uActivePage != (uint32)PAGE_BEHAVIOUR_TREE)
		{
			return;
		}

		m_bCanSeeTarget = bCanSeeTarget;

		for (uint32 i = 0; i < uCount; i++)
		{
			m_behaviors[i].Set(apsBehaviors[i].CStr());
			m_layer[i] = auLayers[i];
		}

		m_uBehaviorCounter = uCount;
	}

	void SceneDebugStats::SetAgentNavigationData(usg::Vector3f* path, uint32 uPathCount, const usg::Vector3f& vPos, const usg::Vector3f& vTargetPos, const usg::Vector3f& vTarget, bool bHasTarget)
	{
		if (m_uActivePage != (uint32)PAGE_BEHAVIOUR_TREE)
		{
			return;
		}

		m_vAgentPos = vPos;
		m_vAgentTargetPos = vTargetPos;
		m_vTarget = vTarget;
		m_bAgentHasTarget = bHasTarget;
		m_uPathCounter = uPathCount;
		MemCpy(m_avPath, path, sizeof(usg::Vector3f) * 32);
	}

	void SceneDebugStats::RegisterTimer(const char* szName, const ProfilingTimer* pTimer, Color color, float fOffset)
	{
		ASSERT(m_uTimers < MAX_PROFILE_TIMERS);
		if (!szName)
			return;

		m_timers[m_uTimers].szName = szName;
		m_timers[m_uTimers].pTimer = pTimer;
		m_timers[m_uTimers].fMaxFrame = 0;
		m_timers[m_uTimers].color = color;
		m_timers[m_uTimers].fOffset = fOffset;
		m_uTimers++;
	}

}
