/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_GFXDEVICE_
#define _USG_GRAPHICS_PC_GFXDEVICE_
#include "Engine/Common/Common.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Graphics/Device/Display.h"
#include API_HEADER(Engine/Graphics/Device, GLSLShader.h)
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class AlphaState;
class RasterizerState;
class DepthStencilState;
class Viewport;
class GFXDevice;
class Display;

class GFXDevice_ps
{
public:
	GFXDevice_ps();
	~GFXDevice_ps();

	void Init(GFXDevice* pParent);

	uint32 GetHardwareDisplayCount();
	const DisplaySettings* GetDisplayInfo(uint32 uIndex);

	void PostInit() {} 

	void Begin();
	void End();
	float GetGPUTime() const { return m_fGPUTime; }

	GFXContext* CreateDeferredContext(uint32 uSizeMul) { ASSERT(false); return NULL; }

	void FinishedStaticLoad() {  }
	void ClearDynamicResources() {  }
	bool Is3DEnabled() const { return false; }
	void WaitIdle() {}

	// Bit of a hack to get around this legacy opengl stuff, we need a window that matches the pixel format of all the windows we intend to use
	// to set up our original opengl context. Must be called first
	static void InitOGLContext(WindHndl hndl, uint32 uSyncInterval = 2);

	static OpenGLContext& GetOGLContext() { return m_sContext; }

private:
	enum
	{
		MAX_DISPLAY_COUNT = 4	// TODO: Remove hardcoding
	};

	GFXDevice*		m_pParent;
	DisplaySettings m_diplayInfo[MAX_DISPLAY_COUNT];
	uint32			m_uDisplayCount;
	static OpenGLContext	m_sContext;

	GLuint		m_uPerformanceQueries[GFX_NUM_DYN_BUFF];
	uint32		m_uQueryId;
	float		m_fGPUTime;
};

}

#endif