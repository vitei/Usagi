/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Profile_ps.h"

namespace usg
{

Profile_ps::Profile_ps()
{

}

Profile_ps::~Profile_ps()
{

}

void Profile_ps::Init(GFXDevice* pDevice)
{
}

bool Profile_ps::IsGuest()
{
	return true;
}

bool Profile_ps::IsProfileSelected()
{
	return true;
}

bool Profile_ps::DisplayProfileSelectScreen()
{
	return true;
}

const char* Profile_ps::GetPlayerName()
{
	return "Doomguy";
}

const TextureHndl Profile_ps::GetPlayerImage()
{
	return TextureHndl();

}

}