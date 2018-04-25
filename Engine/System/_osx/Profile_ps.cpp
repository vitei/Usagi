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
	ResourceMgr::Inst()->GetTexture(pDevice, "macmii");
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
	return "Steve Jobs";
}

const Texture* Profile_ps::GetPlayerImage()
{
	return m_pTexture;

}

}