#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <assert.h>
#include <string.h>

#define SAFE_DELETE( a ) if( a ) { vdelete a; a = NULL; }
#define SAFE_DELETE_ARRAY( a ) if( a ) { vdelete [] a; a = NULL; }

#include "common/debug.h"


static const uint32_t DEFAULT_ALIGNMENT = 16;

inline size_t calcAlignedSize( size_t size, size_t align ) {
	if( size & ( align - 1 ) ) {
		size = ( size & ~( align - 1 ) ) + align;
	}
	return size;
}

inline void* offsetAddress( void* pAddr, size_t offset ) {
	return reinterpret_cast<void*>( reinterpret_cast<size_t>( pAddr ) + offset );
}

inline void* offsetAndAlignAddress( void* pAddr, int offset, size_t align ) {
	void* p = offsetAddress( pAddr, offset );
	size_t ret = calcAlignedSize( reinterpret_cast<size_t>( p ), align );
	return reinterpret_cast<void*>( ret );
}

inline void turnOnFlag( uint32_t& attr, uint32_t flag ) {
	attr |= flag;
}

inline void turnOffFlag( uint32_t& attr, uint32_t flag ) {
	attr &= ~flag;
}

inline void setFlag( uint32_t& attr, uint32_t flag, bool bOn ) {
	if( bOn ) {
		turnOnFlag( attr, flag );
	}
	else {
		turnOffFlag( attr, flag );
	}
}

inline bool IsFlagOn( uint32_t attr, uint32_t flag ) {
	return ( attr & flag ) != 0;
}

inline uint32_t searchIndex( const char* p, const char* pSymbols[], uint32_t totalNum, uint32_t colNum = 1 )
{
	uint32_t row = 0;
	for( uint32_t index = 0; index < totalNum; ++index ) {
		if( strcmp( p, pSymbols[index] ) == 0 ) {
			row = index / colNum;
			break;
		}
	}

	return row;
}

#define SEARCH_INDEX( p, pSymbols ) searchIndex( p, pSymbols, ARRAY_SIZE(pSymbols), 1 )

namespace aya {
	inline void* Alloc( size_t size, uint32_t align = 4U ) {
		return usg::mem::Alloc( usg::MEMTYPE_STANDARD, usg::ALLOC_OBJECT, size, align );
	}

	inline void Free( void* p ) {
		if( p ) {
			usg::mem::Free( usg::MEMTYPE_STANDARD, p );
		}
	}
}

#endif // COMMON_H
