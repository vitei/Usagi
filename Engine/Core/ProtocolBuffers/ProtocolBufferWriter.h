/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PROTOCOL_BUFFER_WRITER_H__
#define _USG_PROTOCOL_BUFFER_WRITER_H__

#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>


#include "Engine/Core/ProtocolBuffers/ProtocolBufferFields.h"

namespace usg {

class ProtocolBufferWriter
{
public:
	ProtocolBufferWriter(uint8* buffer, size_t buf_size)
	: m_ostream (pb_ostream_from_buffer(buffer, buf_size)) {}
	~ProtocolBufferWriter(void) {}

	template<typename T>
	bool Write(T* pSrc)
	{
		ProtocolBufferFields<T>::PreSave(pSrc);
		bool result = pb_encode(&m_ostream, ProtocolBufferFields<T>::Spec(), pSrc);
		ProtocolBufferFields<T>::PostSave(pSrc);
		ASSERT(m_ostream.bytes_written < m_ostream.max_size);
		if(result) { uint8_t zero = 0; WriteRaw(&zero, 1); }
		else       { DEBUG_PRINT("Write error: %s\n", m_ostream.errmsg); }
		return result;
	}

	void WriteRaw(const uint8* buffer, size_t buf_size)
	{
		ASSERT(buf_size < m_ostream.max_size - m_ostream.bytes_written);
		pb_write(&m_ostream, buffer, buf_size);
	}

	uint32 GetPos() { return (uint32)m_ostream.bytes_written; }

private:
	pb_ostream_t m_ostream;
};

}

#endif //_USG_PROTOCOL_BUFFER_WRITER_H__

