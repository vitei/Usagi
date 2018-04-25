/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
****************************************************************************/
#include "Engine/Common/Common.h"
#include "ProtocolBufferMemory.h"
#include "Engine/Memory/MemUtil.h"

namespace usg {

ProtocolBufferMemory::ProtocolBufferMemory(const void * pMemoryBase, const uint32_t memorySize)
	: m_istream()
	, m_pMemoryBase(reinterpret_cast<const uint8_t *>(pMemoryBase))
	, m_memorySize(memorySize)
	, m_memoryPosition(0)
{
	m_istream = pb_istream_from_memory(this);
}

bool ProtocolBufferMemory::pb_istream_from_memory_cb(pb_istream_t *stream, uint8_t *buf, size_t count)
{
	ProtocolBufferMemory* pPBMem = (ProtocolBufferMemory*)stream->state;
	const uint8_t* pSrc = pPBMem->m_pMemoryBase + pPBMem->m_memoryPosition;

	MemCpy(buf, pSrc, count);

	pPBMem->m_memoryPosition += static_cast<uint32>(count);

	size_t bytes_read = count;
	return bytes_read == count;
}

}
