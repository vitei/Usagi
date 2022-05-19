/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Player sign in data (for now dummy, but games for windows etc)
*****************************************************************************/
#ifndef _USG_SYSTEM_PROFILE_PS_H_
#define _USG_SYSTEM_PROFILE_PS_H_

#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Resource/ResourceDecl.h"

namespace usg {

class Profile_ps
{
public:
	Profile_ps();
	~Profile_ps();

	void Init(GFXDevice* pDevice);
	bool IsGuest();
	bool IsProfileSelected();
	bool DisplayProfileSelectScreen();
	const char* GetPlayerName();
	const TextureHndl GetPlayerImage();


private:
	PRIVATIZE_COPY(Profile_ps)

	const char*		m_szName;
};

}

#endif
