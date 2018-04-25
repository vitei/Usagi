/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Decoder delegate for reading into plain C arrays.
*****************************************************************************/
#ifndef _USG_CORE_PB_CHUNKED_ARRAY_H_
#define _USG_CORE_PB_CHUNKED_ARRAY_H_

#include <pb.h>
#include <pb_decode.h>
#include "Engine/Memory/Mem.h"
#include "Engine/Common/Common.h"
#include "Engine/Core/ProtocolBuffers/PBDecoderDelegate.h"

namespace usg {

template <typename T, MemAllocType MEM_ALLOC_TYPE = ALLOC_PROTOCOL_MESSAGE, int CHUNK_SIZE = 256>
class PBChunkedArray : public PBDecoderDelegate<T>
{
public:
	struct ArrayWrapper
	{
		ArrayWrapper() : array(NULL), count(0) {}
		// BIG MEMORY LEAK, FIXME DANI
		~ArrayWrapper() { if(array!=NULL) vdelete[] array; array=NULL; }
		T* array;
		size_t count;

		      T& operator[](int i)       { return array[i]; }
		const T& operator[](int i) const { return array[i]; }

		class Iterator
		{
		public:
			Iterator(T* pStart, size_t count, size_t startPos = 0)
			: m_pStart(pStart), m_count(count), m_pos(startPos) {}
			Iterator& operator++() { m_pos++; return *this; }
			Iterator& operator--() { m_pos--; return *this; }

			const T& operator*() const { return m_pStart[m_pos]; }
			      T& operator*()       { return m_pStart[m_pos]; }
			bool     IsEnd()           { return m_pos >= m_count; }

		private:
			T*	m_pStart;
			size_t m_count;
			size_t m_pos;
		};

		Iterator Begin() const { return Iterator(array, count); }
		Iterator End()   const { return Iterator(array, count, count); }
	};

	typedef ArrayWrapper AccessorType;
	typedef typename ArrayWrapper::Iterator Iterator;
	AccessorType data;

	void init(void** arg)
	{
		ScratchRaw::Init(arg, sizeof(WorkingData), 4);
		WorkingData *decoder = (WorkingData*)*arg;
		decoder->m_outputBuffer = NULL;
		decoder->m_owner = this;
		decoder->m_numRead = 0;
	}

	void finalise(void** arg)
	{
		WorkingData *decoder = (WorkingData*)*arg;
		decoder->commit();

		data.array = decoder->m_outputBuffer;
		data.count = decoder->m_numRead;
		ScratchRaw::Free(arg);
		*arg = &data;
	}

	struct WorkingData
	{
		WorkingData() : m_numRead(0), m_outputBuffer(NULL) {}

		// TODO Right now we use (m_numRead % CHUNK_SIZE) to work out the offset
		//      into the array.  We're getting away with this, but it's kind of a bug
		//      waiting to happen, in case we ever call commit() before we've reached
		//      CHUNK_SIZE and before we've finished reading all the data.
		size_t m_numRead;
		T      m_scratch[CHUNK_SIZE];
		T*     m_outputBuffer;

		PBChunkedArray<T, MEM_ALLOC_TYPE, CHUNK_SIZE>* m_owner;

		T* alloc()
		{
			size_t numReadThisChunk = m_numRead % CHUNK_SIZE;
			if(m_numRead > 0 && numReadThisChunk == 0)
				commit();

			return &m_scratch[numReadThisChunk];
		}

		void InitializeField(T* field_to_init)
		{
			m_owner->InitializeField(field_to_init);
		}

		void increment()
		{
			m_numRead++;
		}

		void commit()
		{
			if(m_numRead > 0)
			{
				if(m_outputBuffer == NULL)
				{
					m_outputBuffer = vnew(MEM_ALLOC_TYPE) T[m_numRead];
					memcpy(m_outputBuffer, m_scratch, sizeof(T) * m_numRead);
				}
				else
				{
					size_t numReadThisChunk = m_numRead % CHUNK_SIZE;
					if(numReadThisChunk == 0) { numReadThisChunk = CHUNK_SIZE; }

					size_t chunkOffset = m_numRead - numReadThisChunk;

					// This is kind of nasty but YOLO
					T* newBuffer = vnew(MEM_ALLOC_TYPE) T[m_numRead];
					memcpy(newBuffer, m_outputBuffer, sizeof(T) * chunkOffset);
					memcpy(&newBuffer[chunkOffset], m_scratch, sizeof(T) * numReadThisChunk);
					vdelete[] m_outputBuffer;
					m_outputBuffer = newBuffer;
				}
			}
		}
	};
};

}

#endif //_USG_CORE_PB_CHUNKED_ARRAY_H_

