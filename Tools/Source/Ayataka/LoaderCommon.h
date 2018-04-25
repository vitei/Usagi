#ifndef _LOADER_COMMON_H_
#define _LOADER_COMMON_H_

namespace LoaderCommon {

inline bool isNumber( char c ) {
	return ( ( '0' <= c && c <= '9' ) || c == '-' || c == '.' || c == 'e' || c == 'E' );
}
	
inline bool proceedToNextNum( const char** p )
{
	bool bEndOf = false;
	bool bSearchBlank = true;
	bool bContinue = true;
	while( bContinue ) {
		char c = **p;
		if( isNumber( c ) ) {
			if( !bSearchBlank ) {
				bContinue = false;
			}
		}
		else if( c == '\0' ) {
			bEndOf = true;
			bContinue = false;
		}
		else {
			bSearchBlank = false;
		}

		if( bContinue ) {
			( *p )++;
		}
	}

	return bEndOf;
}

inline void loadUIntStream( uint32_t* pOut, const char* pString, uint32_t size )
{
	const char* p = pString;
	if( !isNumber( *p ) ) {
		proceedToNextNum( &p );
	}

	pOut[0] = static_cast<uint32_t>( atoi( p ) );
	for( uint32_t n = 1; n < size; ++n ) {
		proceedToNextNum( &p );
		pOut[n] = static_cast<uint32_t>( atoi( p ) );
	}
}

template <class ListType>
inline void loadNumberStream(ListType* pOut, const char* pString, uint32_t size )
{
	const char* p = pString;
	if( !isNumber( *p ) )
	{
		proceedToNextNum( &p );
	}

	const char* pInitialPoint = p;

	pOut[0] = static_cast<ListType>( atof( p ) );
	uint32_t n;
	for( n = 1; n < size; ++n )
	{
		bool bEndOfString = proceedToNextNum( &p );

		// if the given size exceeds the number of values that pString has, back to initial value and repeat
		if( bEndOfString ) {
			p = pInitialPoint;
		}
		pOut[n] = static_cast<ListType>( atof( p ) );
	}
}

inline uint32_t checkVectorColumnNum( const char* pNodeName )
{
	if( startsWith( pNodeName, "Vector1" ) ) {
		return 1;
	}
	else if( startsWith( pNodeName, "Vector2" ) ) {
		return 2;
	}
	else if( startsWith( pNodeName, "Vector3" ) ) {
		return 3;
	}
	else if( startsWith( pNodeName, "Vector4" ) ) {
		return 4;
	}
	assert( false );
	return 0;
}

}

#endif // _LOADER_COMMON_H_
