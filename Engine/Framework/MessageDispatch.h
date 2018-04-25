/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// Handles message dispatch -- allows you to register callback functions
// which should be fired when certain net messages come in.

#ifndef NET_MESSAGE_DISPATCH_H
#define NET_MESSAGE_DISPATCH_H

#include "Engine/Core/Containers/StringPointerHash.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Core/ProtocolBuffers.h"
#include "MessageWithHeader.h"

#define CALL_METHOD(obj, methodPtr) ((obj).*(methodPtr))

namespace usg {

struct MessageCallback;
class MessageDispatch
{
public:
	MessageDispatch();
	~MessageDispatch();

	template<typename T>              void RegisterCallback(void (*cb)(const T&));
	template<typename T, typename C>  void RegisterCallback(C* object, void (C::*cb)(const T&));
	template<typename T, typename CB> void RegisterCallback(CB cb);

private:
	PRIVATIZE_COPY(MessageDispatch)

	// Only the Messenger and NetClient can call FireMessage directly
	void FireMessage(uint32 type, ProtocolBufferReader& pb);
	template <typename T> void FireMessage(T& pb);
	friend class Messenger;
	friend class NetClient;

	StringPointerHash< List<MessageCallback>* >  m_pbMessageCallbacks;

	void *m_memPoolBuffer;
	MemHeap m_memPool;
};

// Called when MessageDispatch is destroyed.
// Anything holding a pointer to a MessageDispatch which lives beyond a Mode's lifetime
// should register a callback for this message and use it to set the pointer to NULL.
struct MessageDispatchShutdown
{
	MessageDispatch* dispatch;
};

struct MessageCallback
{
	virtual void operator()(ProtocolBufferReader& pb)=0;
};

template<typename T>
struct MessageCallbackDecoded : public MessageCallback
{
	virtual void operator()(ProtocolBufferReader& pb)=0;
	virtual void operator()(T& pb)=0;
};

template<typename T, typename CallbackType>
struct MessageCallbackGen : public MessageCallbackDecoded<T>
{
	MessageCallbackGen(CallbackType cb) : m_cb(cb) {};
	virtual void operator()(ProtocolBufferReader& pb)
	{
		T pbData;
		bool result = pb.Read(&pbData);
		ASSERT(result);
		m_cb(pbData);
	}

	virtual void operator()(T& pb)
	{
		m_cb(pb);
	}

private:
	CallbackType m_cb;
};

template<typename HeaderType, typename PayloadType, typename CallbackType>
struct MessageCallbackGen<MessageWithHeader<HeaderType, PayloadType>, CallbackType> : public MessageCallbackDecoded< MessageWithHeader<HeaderType, PayloadType> >
{
	MessageCallbackGen(CallbackType cb) : m_cb(cb) {};
	virtual void operator()(ProtocolBufferReader& pb)
	{
		bool result;

		MessageWithHeader<HeaderType, PayloadType> pbData;
		result = pb.Read(&pbData.hdr);
		ASSERT(result);
		result = pb.Read(&pbData.body);
		ASSERT(result);
		m_cb(pbData);
	}

	virtual void operator()(MessageWithHeader<HeaderType, PayloadType>& pb)
	{
		m_cb(pb);
	}

private:
	CallbackType m_cb;
};

template<class C, typename T>
struct MethodCallback
{
	C* obj;
	void (C::*cb)(const T&);
	MethodCallback(C* _obj, void (C::*_cb)(const T&)) : obj(_obj), cb(_cb) {}
	void operator()(const T& pMsg) { CALL_METHOD(*obj, cb)(pMsg); }
};

template<typename T>
void MessageDispatch::RegisterCallback(void (*cb)(const T&))
{
	RegisterCallback<T, void (*)(const T&)>(cb);
}

template<typename T, typename C>
void MessageDispatch::RegisterCallback(C* object, void (C::*cb)(const T&))
{
	RegisterCallback<T>(MethodCallback<C, T>(object, cb));
}

template<typename T, typename CallbackType>
void MessageDispatch::RegisterCallback(CallbackType cb)
{
	void* bytes = m_memPool.Allocate(sizeof(MessageCallbackGen<T, CallbackType>), 4, 0, ALLOC_NETWORK);
	MessageCallbackGen<T, CallbackType>* callback = new (bytes) MessageCallbackGen<T, CallbackType>(cb);

	string_crc key(ProtocolBufferFields<T>::ID);
	List<MessageCallback>* registeredCBs = m_pbMessageCallbacks.Get(key);

	if(registeredCBs == NULL)
	{
		void* listBytes = m_memPool.Allocate(sizeof(List<MessageCallback>), 4, 0, ALLOC_NETWORK);
		registeredCBs = new(listBytes) List<MessageCallback>(16);
		m_pbMessageCallbacks.Insert(key, registeredCBs);
	}

	registeredCBs->AddToEnd(callback);
}

template<typename T>
void MessageDispatch::FireMessage(T& pb)
{
	string_crc key(ProtocolBufferFields<T>::ID);
	List<MessageCallback>* registeredCBs = m_pbMessageCallbacks.Get(key);
	if(registeredCBs != NULL)
	{
		for(List<MessageCallback>::Iterator it = registeredCBs->Begin(); !it.IsEnd(); ++it)
		{
			MessageCallbackDecoded<T>& cb = *(MessageCallbackDecoded<T>*)*it;
			cb(pb);
		}
	}
}

}

#endif //NET_MESSAGE_DISPATCH_H

