/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/System/Locale.h"
#include "Engine/System/LocalizationUtil.h"

using namespace usg;

Locale::Locale()
{
}

Locale::~Locale()
{
}

void Locale::Init()
{
	m_platform.Init();
	m_debugRegion = m_platform.GetRegion();
	m_debugLanguage = m_platform.GetLanguage();

#ifndef FINAL_BUILD
	string filename;
	bool bDidFindOverrideFile = false;

	for (uint32 uRegion = _Region_MIN; uRegion <= _Region_MAX; uRegion++)
	{
		for (uint32 uLanguage = _Language_MIN; uLanguage <= _Language_MAX; uLanguage++)
		{
			filename = str::ParseString("%s_%s", NameForRegion((Region)uRegion),
			                     NameForLanguage((Language)uLanguage));

			if (!bDidFindOverrideFile &&
			    (File::FileStatus(filename.c_str()) == FILE_STATUS_VALID))
			{
				m_debugRegion = (Region)uRegion;
				m_debugLanguage = (Language)uLanguage;
				DEBUG_PRINT("[LOCALE] Overrode locale with value: %s\n", filename.c_str());
				bDidFindOverrideFile = true;
				break;
			}
		}

		if (bDidFindOverrideFile)
		{
			break;
		}
	}
#endif
}

Region Locale::GetRegion()
{
#ifndef FINAL_BUILD
	return m_debugRegion;
#else
	return m_platform.GetRegion();
#endif
}

Language Locale::GetLanguage()
{
#ifndef FINAL_BUILD
	return m_debugLanguage;
#else
	return m_platform.GetLanguage();
#endif
}
