/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform independent debug rendering code
*****************************************************************************/
#pragma once

#ifndef USG_DEBUG_STATS_H
#define USG_DEBUG_STATS_H
#include "Engine/Common/Common.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Core/stl/Vector.h"
#include API_HEADER(Engine/Debug, DebugStats_ps.h)
#include "Engine/Debug/GlobalDebugStats.h"
#include "Engine/Core/String/FixedString.h"

namespace usg {

class ProfilingTimer;
class IDebugStatGroup;

namespace ai
{
	class IAgent;
}	//	namespace ai

#ifndef FINAL_BUILD
class DebugStats
{
public:
	DebugStats();
	virtual ~DebugStats();

	static DebugStats* Inst();

	void Init(GFXDevice* pDevice, DebugRender* pRender);
	void CleanUp(GFXDevice* pDevice);

	void PreDraw(GFXDevice* pDevice);
	void PostDraw(GFXDevice* pDevice);

	void RegisterGroup(IDebugStatGroup* pStarGroup);
	void DeregisterGroup(IDebugStatGroup* pStatGroup);
	void RegisterCPUTimer(const ProfilingTimer* pTimer) { m_globalStats.RegisterCPUTimer(pTimer); }
	
	virtual void Draw();

	void Update(float fElapsed);

	uint32 GetCurrentType() const { return m_uActiveType; }
	uint32 GetPage() const { return m_uActivePage; }

private:
	// Override this if you want to change the behaviour/ trigger different pages with different numbers
	virtual void UpdatePageNumber(bool bForward, bool bBack);
	virtual uint32 GetGamePages() const { return 0; }

	uint32		m_uActivePage;
	uint32		m_uActiveType;

	void SetPage(uint32 uGroup, uint32 uPage);
	uint32 GetTotalPageCount() const;


	static DebugStats* ms_pDebugStats;

	uint32 GetPageCount(uint32 uType);

	DebugRender*					m_pRender;
	GlobalDebugStats				m_globalStats;
	DebugStats_ps					m_platform;
	usg::vector<IDebugStatGroup*>	m_debugStats;	
	bool							m_bActive;

};

#else

class DebugStats
{
public:
	DebugStats() {}
	~DebugStats() {}

	void Init(GFXDevice* pDevice, DebugRender* pRender) {}
	void Update(float fElapsed) {}

	void PreDraw(GFXDevice* pDevice) {}
	void PostDraw(GFXDevice* pDevice) {}

private:
};

#endif	// FINAL_BUILD

} // namespace usagi

#endif // USG_DEBUG_STATS_H
