#ifndef _PB_UTIL_H_
#define _PB_UTIL_H_

#include <pb.h>

#include "common.h"
#include "endian.h"

inline void swapEndianPB( void* p, const pb_field_t fields[] );

inline void swapEndianPB_Field( void* p, const pb_field_t field )
{
	int arrayNum = 1;
	if( ( field.type & PB_HTYPE_MASK ) == PB_HTYPE_REPEATED ) {
		arrayNum = field.array_size;
	}

	switch( field.type & PB_LTYPE_MASK ) {
		case PB_LTYPE_VARINT:  /* int32, int64, enum, bool */
		case PB_LTYPE_UVARINT: /* uint32, uint64 */
		case PB_LTYPE_SVARINT: /* sint32, sint64 */
		case PB_LTYPE_FIXED32: /* fixed32, sfixed32, float */
			switch( field.data_size ) {
				case 2:
					swapEndian<uint16_t>( p, field.data_size * arrayNum );
					break;
				case 4:
					swapEndian<uint32_t>( p, field.data_size * arrayNum );
					break;
				default:
					ASSERT_MSG( 0, "Data size of this field %d is wrong.", field.data_size ); // 64bit data-type is not supported yet
			}
			break;

		case PB_LTYPE_SUBMESSAGE:
			for( int i = 0; i < arrayNum; ++i ) {
				swapEndianPB( p, reinterpret_cast<const pb_field_t*>( field.ptr ) );
				p = offsetAddress( p, field.data_size );
			}
			break;

		case PB_LTYPE_FIXED64: /* fixed64, sfixed64, double */
			ASSERT_MSG( 0, "64bit data-type is not supported yet." );
			break;
	}
}

inline void swapEndianPB( void* p, const pb_field_t fields[] )
{
	int i = 0;
	while( fields[i].tag != 0 ) {
		p = offsetAddress( p, fields[i].data_offset ); // offset head

		swapEndianPB_Field( p, fields[i] );

		int step = 0;
		switch( fields[i].type & PB_HTYPE_MASK ) {
			case PB_HTYPE_REQUIRED:
				step = fields[i].data_size;
				break;

			case PB_HTYPE_REPEATED:
				step = fields[i].data_size * fields[i].array_size;
				break;

			default:
				ASSERT_MSG( 0, "error" ); // others are not supported yet.
				break;
		}
		p = offsetAddress( p, step );
		++i;
	}
}

#define SET_XML_ATTR_INTO_PB( destName, attrName, type ) SET_XML_ATTR_INTO_PB_IMPL( pb, destName, node, attrName, type )

#define SET_XML_ATTR_INTO_PB_IMPL( dest, destName, src, attrName, type ) \
{ \
	pugi::xml_attribute attrTemp = src.attribute( attrName ); \
	if( !attrTemp.empty() ) { \
		dest.destName = attrTemp.as_##type(); \
	} \
}

#endif // !_PB_UTIL_H_
