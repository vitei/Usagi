/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "MessageDispatch.h"

namespace usg {

MessageDispatch::MessageDispatch()
{
	static const size_t POOL_SIZE = 256 * 1024;
	m_memPoolBuffer = mem::Alloc(MEMTYPE_NETWORK, ALLOC_COMPONENT, POOL_SIZE, 4U);
	m_memPool.Initialize(m_memPoolBuffer, POOL_SIZE);

	m_pbMessageCallbacks.SetAllocator(&m_memPool);
}

MessageDispatch::~MessageDispatch()
{
	MessageDispatchShutdown shutdown;
	shutdown.dispatch = this;
	FireMessage(shutdown);

	StringPointerHash< list<MessageCallback*>* >::Iterator it = m_pbMessageCallbacks.Begin();
	for(; !it.IsEnd(); ++it)
	{
		list<MessageCallback*>* callbackList = **it;
		if(callbackList != NULL)
		{
			m_memPool.Deallocate(callbackList);
		}
	}

	m_pbMessageCallbacks.Shutdown();
	m_memPool.FreeGroup(0);
	mem::Free(MEMTYPE_NETWORK, m_memPoolBuffer);
}

void MessageDispatch::FireMessage(uint32 type, ProtocolBufferReader& pb)
{
	string_crc key(type);
	list<MessageCallback*>* registeredCBs = m_pbMessageCallbacks.Get(key);
	if(registeredCBs != NULL)
	{
		for(auto it : *registeredCBs)
		{
			MessageCallback& cb = *it;
			cb(pb);
		}
	}
}

}

