/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Decoder delegate for allocating from Fast Pools and
//	             referencing members using a List.
*****************************************************************************/
#ifndef _USG_CORE_PB_LIST_FROM_FAST_POOL_H_
#define _USG_CORE_PB_LIST_FROM_FAST_POOL_H_

#include <pb.h>
#include <pb_decode.h>

#include "Engine/Memory/FastPool.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Core/ProtocolBuffers/PBDecoderDelegate.h"

namespace usg {

template <typename T>
class PBListFromFastPool : public PBDecoderDelegate<T>
{
public:
	PBListFromFastPool() : m_pool(NULL) {}

	typedef List<T> AccessorType;
	AccessorType data;
	typedef PBListFromFastPool<T> WorkingData;
	typedef typename AccessorType::Iterator Iterator;

	void SetPool(FastPool<T>* pool) { m_pool = pool; }

	void init(void** arg)
	{
		*arg = this;
	}

	void finalise(void** arg)
	{
	}

	void increment()
	{
	}

	T* alloc()
	{
		ASSERT(m_pool != NULL && "You must set pool using SetPool before loading.");
		return m_pool->Alloc();
	}

private:
	FastPool<T> *m_pool;
};

}

#endif //_USG_CORE_PB_LIST_FROM_FAST_POOL_H_

