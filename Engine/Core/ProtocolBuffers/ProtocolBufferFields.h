/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PROTOCOL_BUFFER_FIELDS_H__
#define _USG_PROTOCOL_BUFFER_FIELDS_H__

#include <pb.h>

namespace usg {

// Specialise this template class to set the field spec for a particular Protocol Buffer type
enum { INVALID_PB_ID = 0xffffffff }; // This will break if the CRC32 of a protocol buffer's name
                                     // is the same, but that's no different from any other checksum clash

	struct ProtocolBufferFieldsBase
	{
		static const pb_field_t * Spec() { return NULL; }
		static const char * Name() { static const char * nonPB = "Non-protocol buffer component"; return nonPB; }
		static void Init(void*) {}
		static void PreLoad(void*) {}
		static void PostLoad(void*) {}
		static void PreSave(void*) {}
		static void PostSave(void*) {}
	};

	template<typename T> struct ProtocolBufferFields : public ProtocolBufferFieldsBase {
		enum { ID = INVALID_PB_ID };
	};

	template<typename T>
	constexpr char * NamePB()
	{
		return ProtocolBufferFields<T>::Name;
	}

	template<typename T> void PreLoadPB(T* msg) { ProtocolBufferFields<T>::PreLoad(msg); }
	template<typename T> void PostLoadPB(T* msg) { ProtocolBufferFields<T>::PostLoad(msg); }
	template<typename T> void PreSavePB(T* msg) { ProtocolBufferFields<T>::PreSave(msg); }
	template<typename T> void PostSavePB(T* msg) { ProtocolBufferFields<T>::PostSave(msg); }

}

#endif //_USG_PROTOCOL_BUFFER_FIELDS_H__
