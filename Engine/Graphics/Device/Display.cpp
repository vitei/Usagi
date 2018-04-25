/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/Display.h"

namespace usg {

void Display::Initialise(usg::GFXDevice* pDevice, WindHndl hndl, struct DeviceResource& res)
{
	m_platform.Initialise(pDevice, hndl);
}

void Display::CleanUp(usg::GFXDevice* pDevice)
{
	m_platform.CleanUp(pDevice);
}


void Display::Present()
{
	m_platform.Present();
}

}
