/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A dictionary that maps ASCII keys to a protobuf message
//  containing localized text and formatting information.
*****************************************************************************/
#ifndef _USG_STRING_TABLE_H_
#define _USG_STRING_TABLE_H_ 

#include "Engine/Core/Containers/StringPointerHash.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Layout/Keystrings.pb.h"
#include "Engine/System/Localization.pb.h"

namespace usg {

class StringTable : public Singleton<StringTable>
{
public:
	StringTable();
	virtual ~StringTable();

	struct KeyString
	{
		const KeyStringDef* pStr;
		const TextStyle* pStyle;
	};

	void Init(const char* szFilename, Region region, Language language);
	KeyString Find(const char* szKey) const;
	KeyString Find(uint32 crc) const;
private:
	void CreatePathToStringsFile(U8String& path, const char* szBasename,
	                             Region region, Language language);
	StringPointerHash<KeyStringDef*> m_hashtable;
	StringPointerHash<TextStyle*> m_styleHashtable;
	KeystringTable m_table;
	uint32 m_keystringCount;
};

}

#endif  // __USG_STRING_TABLE_H__
