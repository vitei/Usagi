/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Class to manage region and language settings
*****************************************************************************/
#ifndef _USG_LOCALE_H_
#define _USG_LOCALE_H_


#include "Engine/System/Localization.pb.h"
#include OS_HEADER(Engine/System/, Locale_ps.h)

namespace usg {

class Locale
{
public:
	Locale();
	~Locale();

	virtual void Init();
	Region GetRegion();
	Language GetLanguage();

	inline void SetDebugRegion(Region region) { m_debugRegion = region; }
	inline void SetDebugLanguage(Language language) { m_debugLanguage = language; }
private:
	PRIVATIZE_COPY(Locale)

	Locale_ps m_platform;
	Region m_debugRegion;
	Language m_debugLanguage;
};

}

#endif
