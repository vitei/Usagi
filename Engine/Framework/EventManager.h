/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// EventManager
// A manager for events.

#pragma once

#ifndef USAGI_FRAMEWORK_EVENT_MANAGER_H
#define USAGI_FRAMEWORK_EVENT_MANAGER_H

#include "Engine/Memory/MemHeap.h"
#include "Engine/Core/stl/list.h"
#include "Engine/Framework/Event.pb.h"
#include "Engine/Framework/Script/ExecuteScript.h"
#include "Engine/Framework/SystemCoordinator.h"
#include "Event.h"

namespace usg
{
	class Messenger;
	class MessageDispatch;

	template<typename T>
	void RegisterLuaEventTransmitters(lua_State* L);

class EventManager
{
public:
	typedef list< TriggerableEvent* > EventQueue;
	EventManager();
	~EventManager();
	void Clear();
	void SetMessenger(Messenger* messenger);
	void SetupListener(const uint32 uEventId, MessageDispatch& dispatch);

	void TriggerEvents(SystemCoordinator& systemCoordinator, Entity rootEntity, uint32 uFrame);
	void TriggerPreRunEvents(SystemCoordinator& systemCoordinator, Entity rootEntity, uint32 uFrame);

	void TriggerEventsForEntity(SystemCoordinator& systemCoordinator, Entity e, Entity rootEntity);
	void RegisterEntitiesRemoved(Entity* pEntities, uint32 uCount);
	static double GetTimeNow();


	template<typename EventType>
	void RegisterEvent(const EventType& evt, typename Event<EventType>::ExtraData extra = nullptr)
	{
		// FIXME: Make me 0.0f for immediate run
		RegisterEventAtTime(evt, 0, extra);
	}

	template<typename EventType>
	void RegisterEventWithEntity(Entity e, const EventType& evt, uint32 targets = ON_ENTITY, typename EventOnEntity<EventType>::ExtraData extra = nullptr)
	{
		RegisterEventWithEntityAtTime(e, evt, targets, 0, extra);
	}

	void RegisterEventWithEntityAtTime(Entity e, const uint32 uEventId, const void* pEventRawData, const memsize uDataSize, uint32 uTargets, float64 fTime);

	template<typename EventType>
	void RegisterEventWithEntity(EntityID e, const EventType& evt, uint32 targets = ON_ENTITY, typename EventOnEntity<EventType>::ExtraData extra = nullptr)
	{
		RegisterEventWithEntity(e.id, evt, targets, extra);
	}

	template<typename EventType>
	void RegisterNetworkEvent(const EventType& evt, bool bIncludeSelf = true, bool bWithoutDelay = false)
	{
		ASSERT(m_messenger != nullptr);
		EventHeader hdr;
		EventHeader_init(&hdr);
		hdr.time = bWithoutDelay ? 0.0 : GetServerTimePrecise();
		RegisterNetworkEventInt(hdr, ProtocolBufferFields<EventType>::ID, (const void*)&evt, sizeof(evt), bIncludeSelf);
	}

	template<typename EventType>
	void RegisterNetworkEventWithEntityAtTime(const sint64 nuid, const EventType& evt, uint32 targets = ON_ENTITY, const double dTime = 0.0, bool bIncludeSelf = true)
	{
		ASSERT(m_messenger != nullptr);
		EventHeader hdr;
		EventHeader_init(&hdr);
		hdr.has_target = true;
		hdr.target.nuid = nuid;
		hdr.target.targets = targets;
		hdr.time = dTime;
		RegisterNetworkEventInt(hdr, ProtocolBufferFields<EventType>::ID, (const void*)&evt, sizeof(evt), bIncludeSelf);
	}

	template<typename EventType>
	void RegisterNetworkEventWithEntity(const sint64 nuid, const EventType& evt, uint32 targets = ON_ENTITY, bool bIncludeSelf = true, bool bWithoutDelay = false)
	{
		ASSERT(m_messenger != nullptr);
		EventHeader hdr;
		EventHeader_init(&hdr);
		hdr.has_target = true;
		hdr.target.nuid = nuid;
		hdr.target.targets = targets;
		hdr.time = bWithoutDelay ? 0.0 : GetServerTimePrecise();
		RegisterNetworkEventInt(hdr, ProtocolBufferFields<EventType>::ID, (const void*)&evt, sizeof(evt), bIncludeSelf);
	}

	template<typename EventType>
	void RegisterNetworkEventWithEntity(const NetworkUID nuid, const EventType& evt, uint32 targets = ON_ENTITY, bool bIncludeSelf = true)
	{
		RegisterNetworkEventWithEntity(nuid.gameUID, evt, targets, bIncludeSelf);
	}

	template<typename EventType>
	void RegisterPreRunEvent(const EventType& evt, typename Event<EventType>::ExtraData extra = nullptr)
	{
		void* buffer = m_heap.Allocate(sizeof(Event<EventType>), 4, 0, ALLOC_EVENT);
		ASSERT(buffer != nullptr);
		Event<EventType>* wrappedEvent = new (buffer) Event<EventType>(evt, 0, extra);
		AddToPreRunEventQueue(wrappedEvent);
	}

	template<typename EventType>
	void RegisterEventAtTime(const EventType& evt, double t, typename Event<EventType>::ExtraData extra = nullptr)
	{
		void* buffer = m_heap.Allocate(sizeof(Event<EventType>), 4, 0, ALLOC_EVENT);
		ASSERT(buffer != nullptr);
		Event<EventType>* wrappedEvent = new (buffer) Event<EventType>(evt, t, extra);
		AddToEventQueue(wrappedEvent);
	}

	template<typename EventType>
	void RegisterEventWithEntityAtTime(Entity e, const EventType& evt, uint32 targets, double t, typename EventOnEntity<EventType>::ExtraData extra = nullptr)
	{
		void* buffer = m_heap.Allocate(sizeof(EventOnEntity<EventType>), 4, 0, ALLOC_EVENT);
		ASSERT(buffer != nullptr);
		ASSERT(e != nullptr);
		EventOnEntity<EventType>* wrappedEvent = new (buffer) EventOnEntity<EventType>(e, evt, targets, t, extra);
		AddToEventQueue(wrappedEvent);
	}

	template<typename EventType>
	void RegisterEventWithEntityAtTime(EntityID e, const EventType& evt, uint32 targets, double t, typename EventOnEntity<EventType>::ExtraData extra = nullptr)
	{
		RegisterEventWithEntityAtTime(e.id, evt, targets, t, extra);
	}

private:
	static const uint32 HEAP_SIZE = 65536;

	void TriggerEventsInt(EventQueue& Queue, SystemCoordinator& systemCoordinator, Entity rootEntity, uint32 uFrame);


	Messenger* m_messenger;
	MemHeap m_heap;
	void* m_pMem;
	EventQueue m_eventQueue;
	EventQueue m_preRunEventQueue;

	bool m_bRegisteredDispatch;

	void AddToEventQueue(TriggerableEvent* evt);
	void AddToPreRunEventQueue(TriggerableEvent* evt);
	float64 GetServerTimePrecise();

	void RegisterNetworkEventInt(const EventHeader& hdr, const uint32 uEventId, const void* pData, const memsize uDataSize, bool bIncludeSelf);

	struct MessageHandler;
	friend struct MessageHandler;
};

template<typename EVENT, bool GENERATE, bool SEND, bool RECEIVE>
struct InitHelper
{
	void operator()(lua_State* pLua, SystemCoordinator& systemCoordinator)
	{
		ASSERT(false);
	}
};

template<typename EVENT>
struct InitHelper<EVENT, true, true, true>
{
	void operator()(lua_State* pLua, SystemCoordinator& systemCoordinator)
	{
		InitLuaType<EVENT>(pLua);
		RegisterLuaEventTransmitters<EVENT>(pLua);

		SignalRunner runner;
		OnEventSignal< EVENT >::template FillSignalRunner<ExecuteScript>(runner, usg::GetSystemId<ExecuteScript>());
		systemCoordinator.RegisterSystemWithSignal(usg::GetSystemId<ExecuteScript>(), OnEventSignal< EVENT >::ID, runner);
	}
};

template<typename EVENT>
struct InitHelper<EVENT, true, false, false>
{
	void operator()(lua_State* pLua, SystemCoordinator& systemCoordinator)
	{
		InitLuaType<EVENT>(pLua);
	}
};


template<typename EVENT>
struct InitHelper<EVENT, true, true, false>
{
	void operator()(lua_State* pLua, SystemCoordinator& systemCoordinator)
	{
		InitLuaType<EVENT>(pLua);
		RegisterLuaEventTransmitters<EVENT>(pLua);
	}
};

template<typename EVENT>
struct InitHelper<EVENT, true, false, true>
{
	void operator()(lua_State* pLua, SystemCoordinator& systemCoordinator)
	{
		InitLuaType<EVENT>(pLua);
		
		SignalRunner runner;
		OnEventSignal< EVENT >::template FillSignalRunner<ExecuteScript>(runner, usg::GetSystemId<ExecuteScript>());
		systemCoordinator.RegisterSystemWithSignal(usg::GetSystemId<ExecuteScript>(), OnEventSignal< EVENT >::ID, runner);
	}
};

template<typename EVENT>
struct InitHelper<EVENT, false, false, false> { void operator()(lua_State* pLua, SystemCoordinator& systemCoordinator) { } };

// This is not the most intuitive place to put this code, but it avoids a
// complicated nest of includes to leaving it here...
template<typename EVENT>
struct SystemCoordinator::SignalHelper< OnEventSignal<EVENT> >
{
	static void RegisterSignal(SystemCoordinator& systemCoordinator)
	{
		if (systemCoordinator.m_pMessageDispatch != nullptr)
		{
			ASSERT(systemCoordinator.m_pEventManager != nullptr);
			systemCoordinator.m_pEventManager->SetupListener(ProtocolBufferFields<EVENT>::ID, *systemCoordinator.m_pMessageDispatch);
		}
		InitHelper<EVENT, LuaSerializer<EVENT>::GENERATE, LuaSerializer<EVENT>::SEND, LuaSerializer<EVENT>::RECEIVE> helper;
		helper(systemCoordinator.m_pLuaVM->Root(), systemCoordinator);
	}
};

namespace Components
{
	struct EventManagerHandle
	{
		EventManagerHandle() : handle(nullptr) {}
		EventManager& operator*() { return *handle; }
		EventManager* operator->() { return handle; }

		EventManager* handle;
	};
}

template<>
inline constexpr char* NamePB<EventManagerHandle>()
{
	return "EventManagerHandle";
}

} // namespace usg

#endif // USAGI_FRAMEWORK_EVENT_MANAGER_H
