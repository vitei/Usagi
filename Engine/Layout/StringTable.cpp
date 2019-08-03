/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Layout/StringTable.h"
#include "Engine/Common/Common.h"
#include "Engine/Core/ProtocolBuffers.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Memory/Mem.h"
#include "Engine/System/LocalizationUtil.h"

namespace usg {

StringTable::StringTable()
: m_hashtable(),
 m_keystringCount(0)
{
	m_hashtable.SetAllocator(mem::GetMainHeap());
	m_styleHashtable.SetAllocator(mem::GetMainHeap());
}

StringTable::~StringTable() 
{
	m_keystringCount = 0;
}

void StringTable::Init(const char* szFilename, Region region, Language language)
{
	U8String path;
	CreatePathToStringsFile(path, szFilename, region, language);
	ASSERT(File::FileStatus(path.CStr()) == FILE_STATUS_VALID);
	ProtocolBufferFile file(path.CStr());

	m_keystringCount = 0;
	file.Read(&m_table);
	bool bInsertSucceeded = false;

	for (auto itr = m_table.textStyles.Begin(); !itr.IsEnd(); ++itr)
	{
		m_styleHashtable.Insert((*itr).name, &(*itr));
	}

	for (auto itr = m_table.keyStrings.Begin(); !itr.IsEnd(); ++itr)
	{
		bInsertSucceeded = m_hashtable.Insert((*itr).key, &(*itr));

		ASSERT(bInsertSucceeded);
		m_keystringCount++;
	}
}

StringTable::KeyString StringTable::Find(const char* szKey) const
{
	KeyString keyString;
	keyString.pStr = m_hashtable.Get(szKey);
	if (keyString.pStr)
	{
		keyString.pStyle = m_styleHashtable.Get(keyString.pStr->styleCRC);
	}
	return keyString;
}


StringTable::KeyString StringTable::Find(uint32 crc) const
{
	KeyString keyString;
	keyString.pStr = m_hashtable.Get(crc);
	if (keyString.pStr)
	{
		keyString.pStyle = m_styleHashtable.Get(keyString.pStr->styleCRC);
	}
	return keyString;
}

void StringTable::CreatePathToStringsFile(U8String& path, const char* szBasename, Region region,
                                          Language language)
{
	const char* regionName = NameForRegion(region);
	path.ParseString("VPB/%s_%s_%s.vpb", szBasename, regionName, NameForLanguage(language));
}

} // namespace usg
