#ifndef ENDIAN_H
#define ENDIAN_H

// Endian swap macro
#define SWAP_ENDIAN_16( n ) ( ( ( n & 0x00ff ) << 8 ) | ( ( n & 0xff00 ) >> 8 ) )
#define SWAP_ENDIAN_32( n ) ( ( ( n & 0x000000ff ) << 24 ) | ( ( n & 0x0000ff00 ) << 8 ) | ( ( n & 0x00ff0000 ) >> 8 ) | ( ( n & 0xff000000 ) >> 24 ) )

inline void swapEndian16( void* p ) {
	*reinterpret_cast<short*>(p) = SWAP_ENDIAN_16( *reinterpret_cast<short*>(p) );
}
inline void swapEndian32( void* p ) {
	*reinterpret_cast<int*>(p) = SWAP_ENDIAN_32( *reinterpret_cast<int*>(p) );
}

template<typename T>
inline void swapEndian( void* p, size_t size ) {
	size_t typesize = sizeof(T);
	ASSERT( size % typesize == 0 );
	size_t num = size / typesize;

	T* pTemp = reinterpret_cast<T*>( p );
	switch( typesize ) {
	case 1: break;

	case 2: {
		for( size_t i = 0; i < num; ++i ) {
			swapEndian16( pTemp );
			++pTemp;
		}
	} break;

	case 4: {
		for( size_t i = 0; i < num; ++i ) {
			swapEndian32( pTemp );
			++pTemp;
		}
	} break;

	default:
		ASSERT( 0 );
	}
}

template <typename T>
inline void swapEndian_Struct( T* p, bool b ) {
	if( !b ) { return; }

	swapEndian<uint32_t>( reinterpret_cast<void*>( p ), sizeof(T) );
}

template <typename T>
inline void swapEndian_Stream( void* p, size_t size, bool b ) {
	if( !b ) { return; }

	swapEndian<T>( p, size );
}

inline size_t calcOffset( void* current, void* base ) {
	return reinterpret_cast<size_t>( current ) - reinterpret_cast<size_t>( base );
}

#endif // ENDIAN_H
