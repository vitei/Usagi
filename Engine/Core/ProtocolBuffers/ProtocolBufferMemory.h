/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
****************************************************************************/
#pragma once

#ifndef _USG_PROTOCOL_BUFFER_MEMORY_H__
#define _USG_PROTOCOL_BUFFER_MEMORY_H__

#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "Engine/Core/ProtocolBuffers/ProtocolBufferFields.h"

namespace usg {
class ProtocolBufferMemory
{
public:
	explicit ProtocolBufferMemory(const void * pDataStart, const uint32_t memorySize);

	template<typename T>
	bool Read(T* pDst)
	{
		ProtocolBufferFields<T>::PreLoad(pDst);
		bool result = pb_decode(&m_istream, ProtocolBufferFields<T>::Spec(), pDst);
		ProtocolBufferFields<T>::PostLoad(pDst);
		if (!result)
		{
#if defined(DEBUG_BUILD) && defined(PB_DEBUG)
			DEBUG_PRINT("Read error: %s\n", m_istream.errmsg ? m_istream.errmsg : "(none)");
#endif
			m_istream.errmsg = NULL;
		}
		return result;
	}

private:

	static bool pb_istream_from_memory_cb(pb_istream_t *stream, uint8_t *buf, size_t count);

	pb_istream_t pb_istream_from_memory(ProtocolBufferMemory * const pPBMem)
	{
		pb_istream_t out = { pb_istream_from_memory_cb, pPBMem, static_cast<size_t>(pPBMem->m_memorySize), NULL };
		return out;
	}

	pb_istream_t	m_istream;
	const uint8_t * m_pMemoryBase;
	uint32_t		m_memorySize;
	uint32_t		m_memoryPosition;
};

}

#endif // _USG_PROTOCOL_BUFFER_MEMORY_H__