#ifndef PUGI_UTIL_H
#define PUGI_UTIL_H

#include <stdio.h>
#include <stdarg.h>

#include "lib/pugixml-1.2/src/pugixml.hpp"

inline pugi::xpath_node_set evaluateXpathQuery( const pugi::xpath_node& node, const char* queryString, ... )
{
	char xpath[512];
	va_list vlist;
	va_start( vlist, queryString );
	vsnprintf( xpath, sizeof( xpath ) - 1, queryString, vlist );

	pugi::xpath_query query( xpath );

	va_end( vlist );

	return query.evaluate_node_set( node );
}

inline const char* getAttributeString( const pugi::xml_node& node, const char* name )
{
	pugi::xml_attribute attr = node.attribute( name );
	if( !attr.empty() ) {
		return attr.as_string();
	}
	return NULL;
}

#endif // PUGI_UTIL_H
