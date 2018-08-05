/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, RasterizerState.h)
#include API_HEADER(Engine/Graphics/Device, AlphaState.h)
#include API_HEADER(Engine/Graphics/Device, DepthStencilState.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include OS_HEADER(Engine/Graphics/Device, OpenGLContext.h)
#include OS_HEADER(Engine/Core/Timer, TimeTracker.h)
#include "Engine/Core/String/String_Util.h"

namespace usg {

OpenGLContext	GFXDevice_ps::m_sContext;


GFXDevice_ps::GFXDevice_ps()
{
	m_pParent = NULL;
	m_uQueryId = 0;
	m_fGPUTime = 0.0f;
	for (uint32 i = 0; i < GFX_NUM_DYN_BUFF; i++)
	{
		m_uPerformanceQueries[i] = 0;
	}
}

GFXDevice_ps::~GFXDevice_ps()
{
	glDeleteQueries(GFX_NUM_DYN_BUFF, m_uPerformanceQueries);
}

void GFXDevice_ps::Init(GFXDevice* pParent)
{  
	m_pParent = pParent;
	m_uDisplayCount = 0;

	// TODO: Move this out into platform specific code
	DISPLAY_DEVICE DispDev;
	ZeroMemory(&DispDev, sizeof(DISPLAY_DEVICE));
	DispDev.cb = sizeof(DISPLAY_DEVICE);

	// Pass in a value over than zero to the second parameter to enumerate graphics cards
	for (DWORD uDeviceIndex = 0; m_uDisplayCount < MAX_DISPLAY_COUNT && EnumDisplayDevices(NULL, uDeviceIndex, &DispDev, 0); uDeviceIndex++)
	{
		if (!(DispDev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
		{
			DisplaySettings* pSettings = &m_diplayInfo[m_uDisplayCount];
			str::Copy(pSettings->name, DispDev.DeviceString, USG_IDENTIFIER_LEN);
			DEVMODE dm;
			ZeroMemory(&dm, sizeof(dm));
			dm.dmSize = sizeof(dm);
			if (EnumDisplaySettingsEx(DispDev.DeviceName, ENUM_CURRENT_SETTINGS, &dm, 0) == FALSE)
			{
				BOOL bReturn = EnumDisplaySettingsEx(DispDev.DeviceName, ENUM_REGISTRY_SETTINGS, &dm, 0);
				ASSERT(bReturn==TRUE);
			}

			HMONITOR hm = 0;
			MONITORINFO mi;
			ZeroMemory(&mi, sizeof(mi));
			mi.cbSize = sizeof(mi);
			if (DispDev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
			{
				// display is enabled. only enabled displays have a monitor handle
				POINT pt = { dm.dmPosition.x, dm.dmPosition.y };
				hm = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
				if (hm)
				{
					GetMonitorInfo(hm, &mi);
					pSettings->uX = mi.rcMonitor.left;
					pSettings->uY = mi.rcMonitor.top;
					pSettings->uWidth = mi.rcMonitor.right - mi.rcMonitor.left;
					pSettings->uHeight = Math::Abs(mi.rcMonitor.top - mi.rcMonitor.bottom);
					pSettings->bWindowed = false;	// No meaning for init data
					pSettings->hardwareHndl = NULL;	// This represents a display, so there is no parent window
					m_uDisplayCount++;
				}
			}
		}
	}

	if (glGenQueries)
	{
		glGenQueries(GFX_NUM_DYN_BUFF, m_uPerformanceQueries);
	}
}

uint32 GFXDevice_ps::GetHardwareDisplayCount()
{
	return m_uDisplayCount;
}

const DisplaySettings* GFXDevice_ps::GetDisplayInfo(uint32 uIndex)
{
	if (uIndex < m_uDisplayCount)
	{
		return &m_diplayInfo[uIndex];
	}
	
	return NULL;
}


void GFXDevice_ps::Begin()
{
	static int frameCount = 0;

	if (frameCount > 3)
	{
		uint64 timeElapsed;
		glGetQueryObjectui64v(m_uPerformanceQueries[m_uQueryId], GL_QUERY_RESULT, &timeElapsed);
		m_fGPUTime = (float)timeElapsed / 1000000.0f;
	}
	m_uQueryId = (m_uQueryId + 1) % GFX_NUM_DYN_BUFF;
	for (uint32 i = 0; i < m_pParent->GetValidDisplayCount(); i++)
	{
		m_pParent->GetDisplay(i)->GetPlatform().SwapBuffers(m_pParent);
	}
	glBeginQuery(GL_TIME_ELAPSED, m_uPerformanceQueries[m_uQueryId]);
	frameCount++;
}

void GFXDevice_ps::End()
{
	glEndQuery(GL_TIME_ELAPSED);
}


void GFXDevice_ps::InitOGLContext(WindHndl hndl, uint32 uSyncInterval)
{
	m_sContext.Init(hndl, uSyncInterval);

	glClear(GL_COLOR_BUFFER_BIT);
	m_sContext.PerformBufferSwap(::GetDC(hndl));
}


}