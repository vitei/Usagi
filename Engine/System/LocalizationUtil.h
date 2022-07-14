/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Utility functions for working with localized resources
*****************************************************************************/
#ifndef _LOCALIZATION_UTIL_H_
#define _LOCALIZATION_UTIL_H_

#include "Engine/System/Localization.pb.h"

namespace usg
{

inline const char* NameForRegion(Region region)
{
	const char* szRegions[_Region_ARRAYSIZE] = {"JP", "US", "EU", "CN"};
	ASSERT(region < _Region_ARRAYSIZE);

	return szRegions[region];
}

inline const char* NameForLanguage(Language language)
{
	const char* szLanguages[_Language_ARRAYSIZE] =
		{"Japanese", "English", "French", "Spanish", "German", "Italian", "Dutch", "Chinese"};
	ASSERT(language < _Language_ARRAYSIZE);

	return szLanguages[language];
}

inline const char* CodeForLanguage(Language language)
{
	const char* szLanguages[_Language_ARRAYSIZE] =
		{"ja", "en", "fr", "es", "de", "it", "nl"};
	ASSERT(language < _Language_ARRAYSIZE);

	return szLanguages[language];
}

}

#endif
