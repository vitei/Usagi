/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "ResourceDictionary.h"
#include "Engine/Core/Utility.h"


uint32_t crc32( const char* p )
{
	return utl::CRC32(p);
}

namespace usg {

NameDataHash*	ResourceDictionary::m_NameDataHashes = NULL;
uint32			ResourceDictionary::m_NameDataHashNum = 0;

void ResourceDictionary::init( void )
{
	ASSERT( m_NameDataHashes == NULL && m_NameDataHashNum == 0 );

	const char* name = "nameDataHash.bin";
	ASSERT( File::FileStatus( name ) == FILE_STATUS_VALID);

	ProtocolBufferFile file( name );
	usg::NameDataHashHeader header;
	bool isRead = file.Read( &header );
	ASSERT( isRead );

	m_NameDataHashNum = header.hashNum;
	m_NameDataHashes = vnew( ALLOC_RESOURCE_MGR ) usg::NameDataHash[m_NameDataHashNum];

	for( uint32 i = 0; i < m_NameDataHashNum; ++i ) {
		isRead = isRead && file.Read( &m_NameDataHashes[i] );
		ASSERT( isRead );
	}
}


void ResourceDictionary::cleanup( void )
{
	if( m_NameDataHashes ) {
		vdelete m_NameDataHashes;
		m_NameDataHashes = NULL;
	}
}

NameHash ResourceDictionary::calcNameHash( const char* name )
{
	return crc32( name );
}

DataHash ResourceDictionary::searchDataHashByName( const char* name )
{
	NameHash crc = calcNameHash( name );
	return searchDataHashByName( crc );
}


usg::DataHash ResourceDictionary::searchDataHashByName( NameHash nameHash )
{
	ASSERT( m_NameDataHashes && m_NameDataHashNum > 0 );

	// s_NameDataHashes is ascendingly ordered by nameCRC
	int head = 0, tail = m_NameDataHashNum;
	while( tail - head > 1 ) {
		int center = head + ( ( tail - head ) / 2 );
		if( nameHash < m_NameDataHashes[center].nameCRC ) {
			// Search lower side
			tail = center;
		}
		else if( m_NameDataHashes[center].nameCRC < nameHash ) {
			// Search higher side
			head = center + 1;
		}
		else {
			// found
			return m_NameDataHashes[center].dataHash;
		}
	}

	if( head >= tail ) {
		return 0;
	}

	if( m_NameDataHashes[head].nameCRC == nameHash ) {
		return m_NameDataHashes[head].dataHash;
	}
	else if( m_NameDataHashes[tail].nameCRC == nameHash ) {
		return m_NameDataHashes[tail].dataHash;
	}

	return 0;
}

}