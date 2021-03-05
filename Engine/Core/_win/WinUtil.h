/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Author: Alex Barnfield
//	Description: Windows specific hardware level functions
*****************************************************************************/

#include "Engine/Graphics/Device/Display.h"

namespace WINUTIL
{
	bool Init(HINSTANCE hInst);

	HINSTANCE	GetInstanceHndl();
	// FIXME: Not ideal
	WindHndl		GetWindow();
	void			SetWindow(WindHndl hndl);
	void			SetInFocus(bool bInFocus);
	bool			GetInFocus();

	WindHndl CreateDisplayWindow(WNDPROC wndProc, const char* szName, const usg::DisplayMode* pDisplaySettings, bool bHidden);
	void UpdateWindow(const usg::DisplayMode* pDisplaySettings);
	void OnLostFocus();
	void OnGainedFocus(uint32 uWidth, uint32 uHeight);

};
