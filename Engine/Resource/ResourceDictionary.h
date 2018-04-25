/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef ResourceList_h__
#define ResourceList_h__

#include "Engine/Resource/NameDataHash.pb.h"

namespace usg {
	typedef uint32 NameHash;
	typedef uint32 DataHash;

	class ResourceDictionary
	{
	public:
		static void init( void );
		static void cleanup( void );
		static NameHash calcNameHash( const char* name );
		static DataHash searchDataHashByName( const char* name );
		static DataHash searchDataHashByName( NameHash nameHash );
	private:
		static NameDataHash*	m_NameDataHashes;
		static uint32			m_NameDataHashNum;
	};
}

#endif // ResourceList_h__
