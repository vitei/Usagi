/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Basic graphics initlization
*****************************************************************************/
#ifndef _USG_GRAPHICS_GFX_H_
#define _USG_GRAPHICS_GFX_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/Display.h"

namespace usg {

class Viewport;
class GFXDevice;

class GFX
{
public:
	static GFXDevice* Initialise();
	static void Reset();
	static void PostUpdate();
	static bool HasFocus() { return m_bHasFocus; }
	static void SetFocus(bool bFocus) { m_bHasFocus = bFocus; }
private:
	static bool	m_bHasFocus;
};

}

#endif
