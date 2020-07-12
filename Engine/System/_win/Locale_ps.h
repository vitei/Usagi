/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Class for getting (dummy) windows language and region settings
*****************************************************************************/
#ifndef _USG_LOCALE_PS_H_
#define _USG_LOCALE_PS_H_


#include "Engine/System/Localization.pb.h"

namespace usg
{

class Locale_ps
{
public:
	Locale_ps();
	~Locale_ps();

	void Init();
	Region GetRegion();
	Language GetLanguage();
private:
	PRIVATIZE_COPY(Locale_ps)
};

}

#endif
