#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#ifndef LEVEL_EDITOR_BUILD
#include "OwnSTLDecl.h"

typedef std::vector< char, aya::Allocator<char> > CharVector;

#include <limits>
#include <algorithm>

#include "common.h"
#else
#include <limits>
#include <algorithm>

namespace aya {
	typedef std::string string;
	typedef std::vector< aya::string > StringVector;
}
typedef std::vector< char > CharVector;
#endif

#define MAX_STRING 512

inline bool startsWith( const aya::string& target, const aya::string& s ) {
	return s.compare( target.substr( 0, s.length() ) ) == 0;
}

inline bool endsWith( const aya::string& target, const aya::string& s ) {
	return s.compare( target.substr( target.length() - s.length() ) ) == 0;
}

inline bool compare( const char* str1, const char* str2 ) {
	return strcmp( str1, str2 ) == 0;
}

inline void remove( aya::string& target, size_t begin, size_t length ) {
	target.erase( begin, length );
}

inline bool contains( const aya::string& target, const aya::string& s ) {
	return target.find( s ) != aya::string::npos;
}

inline aya::StringVector split( const aya::string& target, const aya::string& splitter ) {
	aya::StringVector splitted;

	size_t length = target.length();
	size_t begin = 0;
	while( begin < length ) {
		size_t end = target.find( splitter, begin );

		// not found
		if( end == aya::string::npos ) {
			splitted.push_back( target.substr( begin, target.length() - begin ) );
			break;
		}

		splitted.push_back( target.substr( begin, end - begin ) );
		begin = end + splitter.length();
	}
	return splitted;
}

inline void replace( aya::string& target, const aya::string& from, const aya::string& to ) {
	size_t length = target.length();
	size_t begin = 0;
	while( begin < length ) {
		size_t pos = target.find( from, begin );

		if( pos == aya::string::npos ) {
			break;
		}

		target.replace( pos, from.length(), to );
		begin = pos + to.length();
	}
}

inline void removeAll( aya::StringVector& vec, const aya::string s ) {
	aya::StringVector::iterator itr = vec.begin();
	while( itr != vec.end() ) {
		if( s.compare( *itr ) == 0 ) {
			itr = vec.erase( itr );
		}
		else {
			++itr;
		}
	}
}

inline void removeBlank( aya::StringVector& vec ) {
	aya::StringVector::iterator itr = vec.begin();
	while( itr != vec.end() ) {
		if( (*itr).length() == 0 ) {
			itr = vec.erase( itr );
		}
		else {
			++itr;
		}
	}
}

inline aya::StringVector split( const aya::string& target, const CharVector& splitters ) {
	aya::StringVector result;

	size_t targetLength = target.length();
	size_t begin = 0;
	while( begin < targetLength ) {
		size_t end = aya::string::npos;

		CharVector::const_iterator itr = splitters.begin();
		while( itr < splitters.end() ) {
			size_t found = target.find( *itr, begin );
			if( found != aya::string::npos ) {
				if( end == aya::string::npos ) {
					end = found;
				}
				else {
					end = std::min( found, end );
				}
			}
			++itr;
		}

		// not found
		if( end == aya::string::npos ) {
			result.push_back( target.substr( begin, target.length() - begin ) );
			break;
		}

		result.push_back( target.substr( begin, end - begin ) );
		begin = end + 1;
	}

	return result;
}

inline aya::StringVector splitStream( aya::string target )
{
	CharVector splitters;
	splitters.push_back( '\r' );
	splitters.push_back( '\n' );
	splitters.push_back( '\t' );
	splitters.push_back( ' ' );

	return split( target, splitters );
}

inline aya::string dirpath( aya::string path )
{
	aya::string pathString( path );
	size_t pos = pathString.rfind( '/' );
	return pathString.substr( 0, pos );
}

#define STRING_COPY( dest, src ) strncpy( dest, src, ARRAY_SIZE( dest ) - 1 )

#endif // STRINGUTIL_H
