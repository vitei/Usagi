/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PROTOCOL_BUFFER_READER_H__
#define _USG_PROTOCOL_BUFFER_READER_H__

#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>


#include "Engine/Core/ProtocolBuffers/ProtocolBufferFields.h"

namespace usg {

// Similar to ProtocolBufferFile, but constructed from a buffer held in memory.
class ProtocolBufferReader
{
public:
	ProtocolBufferReader(uint8* buffer, size_t buf_size)
	: m_istream (pb_istream_from_buffer(buffer, buf_size)) {}
	~ProtocolBufferReader(void) {}

	template<typename T>
	bool Read(T* pDst)
	{
		ProtocolBufferFields<T>::PreLoad(pDst);
		bool result = pb_decode(&m_istream, ProtocolBufferFields<T>::Spec(), pDst);
		ProtocolBufferFields<T>::PostLoad(pDst);
		if(!result)
		{
			DEBUG_PRINT("Read error: %s\n", m_istream.errmsg ? m_istream.errmsg : "(none)");
			m_istream.errmsg = NULL;
		}
		return result;
	}

private:
	pb_istream_t m_istream;
};

}

#endif //_USG_PROTOCOL_BUFFER_READER_H__

