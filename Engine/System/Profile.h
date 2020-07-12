/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Code related to the currently active player profile
*****************************************************************************/
#ifndef _USG_SYSTEM_PROFILE_H_
#define _USG_SYSTEM_PROFILE_H_

#include "Engine/Graphics/Textures/Texture.h"
#include OS_HEADER(Engine/System/, Profile_ps.h)

namespace usg {

class Profile
{
public:
	Profile();
	~Profile();

	virtual void Init(GFXDevice* pDevice);
	bool IsGuest();
	bool IsProfileSelected();
	bool DisplayProfileSelectScreen();
	const char* GetPlayerName();
	const Texture* GetPlayerImage();


private:
	PRIVATIZE_COPY(Profile)

	Profile_ps	m_platform;
};

}

#endif
