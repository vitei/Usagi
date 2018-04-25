/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/GFX.h"

#include OS_HEADER(Engine/Graphics/Device, OpenGLContext.h)


namespace usg {

static GFXDevice* s_pDevice = NULL;
bool GFX::m_bHasFocus = true;

GFXDevice* GFX::Initialise()
{
	ASSERT(!s_pDevice);
	s_pDevice = vnew(ALLOC_OBJECT) GFXDevice();
	s_pDevice->Init();

	return s_pDevice;
}


void GFX::Reset()
{
	vdelete s_pDevice;
}

void GFX::PostUpdate()
{
	// process windows messages
#ifdef PLATFORM_PC
	MSG msg;
	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage (&msg, NULL, 0, 0))
		{
			//g_exiting = true;
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif
    
    
    
#ifdef PLATFORM_OSX
    glfwPollEvents();
#endif

}

}




