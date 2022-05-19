/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Profile.h"

namespace usg
{

Profile::Profile()
{

}

Profile::~Profile()
{

}

void Profile::Init(GFXDevice* pDevice)
{
	m_platform.Init(pDevice);
}

bool Profile::IsGuest()
{
	return m_platform.IsGuest();
}

bool Profile::IsProfileSelected()
{
	return m_platform.IsProfileSelected();
}

bool Profile::DisplayProfileSelectScreen()
{
	return m_platform.DisplayProfileSelectScreen();
}

const char* Profile::GetPlayerName()
{
	return m_platform.GetPlayerName();
}

TextureHndl Profile::GetPlayerImage()
{
	return m_platform.GetPlayerImage();
}


}