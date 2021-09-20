/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Decoder delegate for reading into a U8String.
*****************************************************************************/

#include "Engine/Common/Common.h"
#include "PBU8String.h"
#include "Engine/Core/stl/string.h"
#include <pb_decode.h>
#include <pb_encode.h>

namespace usg
{
	template<int BUFFER_SIZE>
	struct PBU8String<BUFFER_SIZE>::WorkingData
	{
		ScratchRaw m_scratch;
		usg::string m_string;
	};

	template<int BUFFER_SIZE>
	bool PBU8String<BUFFER_SIZE>::decode(pb_istream_t *stream, const pb_field_t *field, void **arg)
	{
		ASSERT(*arg != NULL);
		WorkingData* pData = (WorkingData*)(*arg);
		ASSERT(pData != NULL);
		uint8* pScratch = (uint8*)pData->m_scratch.GetRawData();
		ASSERT(pScratch != NULL);
		memset(pScratch, 0, (BUFFER_SIZE + 1));

		while (stream->bytes_left) {
			size_t readSize = (stream->bytes_left > BUFFER_SIZE) ? BUFFER_SIZE : stream->bytes_left;
			bool success = pb_read(stream, pScratch, readSize);
			// ensure that the string is null-terminated
			pScratch[readSize] = 0;

			if (!success) { return false; }

			// copy what we've read...
			pData->m_string += usg::string((char*)pScratch);
			// and prepare the buffer for the next interation
			memset(pScratch, 0, readSize);
		}
		return true;
	}

	template<int BUFFER_SIZE>
	bool PBU8String<BUFFER_SIZE>::encode(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
	{
		ASSERT(*arg != NULL);
		PBU8String<BUFFER_SIZE>* self = (PBU8String<BUFFER_SIZE>*)arg;
		bool success = false;
		if (pb_encode_tag_for_field(stream, field))
		{
			success = pb_encode_string(stream, (uint8_t*)self->Get().c_str(), self->Get().length());
		}

		return success;
	}

	template<int BUFFER_SIZE>
	void PBU8String<BUFFER_SIZE>::PreLoad()
	{
		m_data->m_scratch.Init(BUFFER_SIZE + 1, 4);
		arg = m_data;
	}

	template<int BUFFER_SIZE>
	usg::string& PBU8String<BUFFER_SIZE>::Get()
	{
		return m_data->m_string;
	}

	template<int BUFFER_SIZE>
	const usg::string& PBU8String<BUFFER_SIZE>::Get() const
	{
		return m_data->m_string;
	}

	template<int BUFFER_SIZE>
	void PBU8String<BUFFER_SIZE>::PostLoad()
	{
		m_data->m_scratch.Free();
	}

	template<int BUFFER_SIZE>
	void PBU8String<BUFFER_SIZE>::PreSave()
	{ 
		arg = m_data; funcs.encode = encode;
	}

	template<int BUFFER_SIZE>
	void PBU8String<BUFFER_SIZE>::PostSave()
	{
		arg = nullptr; funcs.decode = decode;
	}

	template<int BUFFER_SIZE>
	PBU8String<BUFFER_SIZE>::~PBU8String()
	{
		vdelete m_data;
	}

	template<int BUFFER_SIZE>
	PBU8String<BUFFER_SIZE>::PBU8String() : m_data(vnew(ALLOC_OBJECT)WorkingData())
	{
		arg = nullptr; funcs.decode = decode;
	}

	template struct PBU8String<128>;	
}