/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Core/Thread/Thread.h"
#include "Engine/Memory/DoubleStack.h"
#include "Engine/HID/Input.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/SceneContext.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "Engine/Core/Timer/ProfilingTimer.h"
#include "Engine/Scene/Scene.h"
#include "DebugStats.h"
#include OS_HEADER(Engine/Core/Timer, TimeTracker.h)

#ifndef FINAL_BUILD

namespace usg {

DebugStats* DebugStats::ms_pDebugStats = nullptr;
DebugStats::DebugStats()
{
	m_uActivePage	= 0;
	m_uActiveType = 0;
	
	m_pRender = nullptr;

	ms_pDebugStats = this;

	m_debugStats.push_back(&m_globalStats);
	m_debugStats.push_back(&m_platform);

}

DebugStats::~DebugStats()
{

}

DebugStats* DebugStats::Inst()
{
	return ms_pDebugStats;
}

void DebugStats::Init(GFXDevice* pDevice, DebugRender* pRender)
{
	m_pRender = pRender;
	m_platform.Init(pDevice);
}

void DebugStats::Cleanup(GFXDevice* pDevice)
{
	if (m_pRender)
	{
		
	}
}

void DebugStats::Draw()
{
	m_debugStats[m_uActiveType]->Draw(m_pRender);

	usg::string warningString;
	Color warningCol(1.0f, 0.2f, 0.2f, 1.0);
	float fLineNo = 0.0f;
	
	for (usg::vector<IDebugStatGroup*>::const_iterator it = m_debugStats.begin(); it != m_debugStats.end(); ++it)
	{
		(*it)->AppendWarnings(warningString);
	}

	if (warningString.length() > 0)
	{
		m_pRender->AddString(warningString.c_str(), 0.8f, fLineNo, warningCol);
	}


}

void DebugStats::UpdatePageNumber(bool bForward, bool bBack)
{
	if (bForward)
	{
		m_uActivePage = m_uActivePage + 1;
		if (m_uActivePage >= GetPageCount(m_uActiveType))
		{
			m_uActivePage = 0;
			do
			{
				m_uActiveType = (m_uActiveType + 1) % m_debugStats.size();
			} while (GetPageCount(m_uActiveType) == 0);
		}
	}

	if (bBack)
	{
		if (m_uActivePage > 0)
		{
			m_uActivePage = m_uActivePage - 1;
		}
		else
		{
			do
			{
				m_uActiveType = (m_uActiveType - 1) % m_debugStats.size();
			} while (GetPageCount(m_uActiveType) == 0);

			m_uActivePage = GetPageCount(m_uActiveType) - 1;
		}
	}
	if(bForward || bBack)
	{
		SetPage(m_uActiveType, m_uActivePage);
	}
}

void DebugStats::Update(float fElapsed)
{
	Gamepad* pGamepad = Input::GetGamepad(0);
	if(pGamepad)
	{
		UpdatePageNumber(pGamepad->GetButtonDown(GAMEPAD_BUTTON_SELECT), false);
	}

	usg::vector<IDebugStatGroup*>::iterator it;
	for (it = m_debugStats.begin(); it != m_debugStats.end(); ++it)
	{
		(*it)->Update(fElapsed);
	}
}

void DebugStats::SetPage(uint32 uGroup, uint32 uPage)
{
	//ASSERT(GetPageCount(eType) < uPage);
	if (uGroup != m_uActiveType)
	{
		m_debugStats[m_uActiveType]->SetActive(false);
	}

	m_uActivePage = uPage;
	m_uActiveType = uGroup;

	m_debugStats[uGroup]->SetPage(uPage);
}

uint32 DebugStats::GetTotalPageCount() const
{
	uint32 uPages = 0;
	usg::vector<IDebugStatGroup*>::const_iterator it;
	for (it = m_debugStats.begin(); it != m_debugStats.end(); ++it)
	{
		uPages += (*it)->GetPageCount();
	}
	return uPages;
}

uint32 DebugStats::GetPageCount(uint32 uGroup)
{
	return m_debugStats[uGroup]->GetPageCount();
}



void DebugStats::RegisterGroup(IDebugStatGroup* pStatGroup)
{
	m_debugStats.push_back(pStatGroup);
	pStatGroup->SetOwner(this);
}

void DebugStats::DeregisterGroup(IDebugStatGroup* pStatGroup)
{
	uint32 uGroup = 0;
	usg::vector<IDebugStatGroup*>::const_iterator it;
	for (it = m_debugStats.begin(); it != m_debugStats.end(); it++, uGroup++)
	{
		if (*it == pStatGroup)
		{
			if (uGroup == m_uActiveType)
			{
				m_uActivePage = 0;
				m_uActiveType = uGroup;

				m_debugStats[uGroup]->SetPage(0);
			}

			(*it)->ClearOwner();
			m_debugStats.erase(it);
			break;
		}
	}
}

void DebugStats::PreDraw(GFXDevice* pDevice)
{
	usg::vector<IDebugStatGroup*>::const_iterator it;
	for (it = m_debugStats.begin(); it != m_debugStats.end(); it++)
	{
		(*it)->PreDraw(pDevice);
	}
}

void DebugStats::PostDraw(GFXDevice* pDevice)
{
	usg::vector<IDebugStatGroup*>::const_iterator it;
	for (it = m_debugStats.begin(); it != m_debugStats.end(); it++)
	{
		(*it)->PostDraw(pDevice);
	}

}

}

#endif
