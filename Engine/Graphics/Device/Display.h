/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Represents a final destination screen, or window (not a viewport)
*****************************************************************************/
#pragma once

#ifndef USG_GRAPHICS_DISPLAY_H
#define USG_GRAPHICS_DISPLAY_H

#include "Engine/Graphics/Color.h"
#include API_HEADER(Engine/Graphics/Device, Display_ps.h)

namespace usg {

class RenderTarget;

struct DisplaySettings
{
	static const int s_nameLength = 64;
	// Always zero on most platforms, PC uses the co-ordinates to represent different monitors
	uint32		uX;	
	uint32		uY;
	uint32		uWidth;
	uint32		uHeight;
	WindHndl	hardwareHndl;
	bool		bWindowed;
	char		name[s_nameLength];
};

class Display
{
public:
	Display() {}
	~Display() {}

	void Initialise(usg::GFXDevice* pDevice, WindHndl hndl, struct DeviceResource& res);
	void Cleanup(usg::GFXDevice* pDevice);
	bool GetDisplayDimensions(uint32 &xOut, uint32 &yOut, bool bOrient) { return m_platform.GetDisplayDimensions(xOut, yOut, bOrient); }
	bool GetActualDimensions(uint32 &xOut, uint32 &yOut, bool bOrient) { return m_platform.GetActualDimensions(xOut, yOut, bOrient); }
	void Present();
	const RenderPassHndl& GetRenderPass() { return m_platform.GetRenderPass(); }
	void ScreenShot(const char* szFileName) { m_platform.ScreenShot(szFileName); }
	void Resize(usg::GFXDevice* pDevice, uint32 uWidth, uint32 uHeight) { m_platform.Resize(pDevice, uWidth, uHeight); }
    void Resize(usg::GFXDevice* pDevice) { m_platform.Resize(pDevice); }
	void Minimized(usg::GFXDevice* pDevice) { m_platform.Minimized(pDevice); }

	Display_ps& GetPlatform() { return m_platform; }
	WindHndl GetHandle() const { return m_window; }

private:
	PRIVATIZE_COPY(Display)

	Display_ps	m_platform;
	WindHndl	m_window;
};

} // namespace usagi

#endif // USG_GRAPHICS_DISPLAY_H
