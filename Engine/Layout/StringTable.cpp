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

	for (auto itr = m_table.keyStrings.Begin(); !itr.IsEnd(); ++itr)
	{
		bInsertSucceeded = m_hashtable.Insert((*itr).key, &(*itr));

		ASSERT(bInsertSucceeded);
		m_keystringCount++;
	}
}

Keystring* StringTable::Find(const char* szKey) const
{
	return m_hashtable.Get(szKey);
}


usg::Keystring* StringTable::Find(uint32 crc) const
{
	string_crc lookupCRC(crc);
	return m_hashtable.Get(lookupCRC);
}

void StringTable::CreatePathToStringsFile(U8String& path, const char* szBasename, Region region,
                                          Language language)
{
	const char* regionName = NameForRegion(region);
	path.ParseString("VPB/%s_%s_%s.vpb", szBasename, regionName, NameForLanguage(language));
}

} // namespace usg
