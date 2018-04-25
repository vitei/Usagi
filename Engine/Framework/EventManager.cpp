#include "Engine/Common/Common.h"
#include "EventManager.h"
#include "Engine/Framework/SystemCoordinator.h"
#include "Engine/Network/NetTime.h"
#include "Engine/Framework/Messenger.h"
#include "Engine/Core/stl/vector.h"

namespace usg
{
	struct EventManager::MessageHandler
	{
		void operator()(const MessageWithHeader<EventHeader, WrappedEvent>& evt);
		MessageHandler(EventManager& _evtManager) : evtManager(_evtManager) {}
	private:
		void ProcessWrappedEvent(const WrappedEvent& evt, const EventHeader& header);
		EventManager& evtManager;
	};

	void EventManager::SetupListener(const uint32 uEventId, MessageDispatch& dispatch)
	{
		if (!m_bRegisteredDispatch)
		{
			MessageHandler msgHandler(*this);
			dispatch.RegisterCallback< MessageWithHeader<EventHeader, WrappedEvent> >(msgHandler);
			m_bRegisteredDispatch = true;
		}
	}

	template<>
	void RegisterComponent<usg::Components::EventManagerHandle>(SystemCoordinator& systemCoordinator)
	{
		systemCoordinator.RegisterComponent<::usg::Components::EventManagerHandle>();
	}

	void EventManager::AddToEventQueue(TriggerableEvent* evt)
	{
		m_eventQueue.AddToEnd(evt);
	}

	float64 EventManager::GetServerTimePrecise()
	{
		return NetTime::GetServerTimePrecise();
	}

	void EventManager::MessageHandler::ProcessWrappedEvent(const WrappedEvent& evt, const EventHeader& header)
	{
		if (header.has_target)
		{
			struct GenericEventOnNetworkEntity : public EventOnNetworkEntityBase
			{
				uint32 m_data[WrappedEvent::uData_max_count];

				GenericEventOnNetworkEntity(sint64 iNUID, const WrappedEvent& evt, uint32 uTargets, double fTime) : 
					EventOnNetworkEntityBase(evt.uMessageId, fTime, (void*)&m_data, iNUID, uTargets)
				{
					usg::MemCpy(m_data, evt.uData, evt.uDataSize);
				}
			};

			void* buffer = evtManager.m_heap.Allocate(sizeof(GenericEventOnNetworkEntity), 4, 0, ALLOC_EVENT);
			ASSERT(buffer != nullptr);
			GenericEventOnNetworkEntity* pGenericEvent = new (buffer) GenericEventOnNetworkEntity(header.target.nuid, evt, header.target.targets, header.time);
			evtManager.m_eventQueue.AddToEnd(pGenericEvent);
		}
		else
		{
			struct GenericEvent : public EventBase
			{
				uint32 m_data[WrappedEvent::uData_max_count];

				GenericEvent(const WrappedEvent& evt, double fTime) :
					EventBase(evt.uMessageId, fTime, (void*)&m_data)
				{
					usg::MemCpy(m_data, evt.uData, evt.uDataSize);
				}
			};

			void* buffer = evtManager.m_heap.Allocate(sizeof(GenericEvent), 4, 0, ALLOC_EVENT);
			ASSERT(buffer != nullptr);
			GenericEvent* pGenericEvent = new (buffer) GenericEvent(evt, header.time);
			evtManager.m_eventQueue.AddToEnd(pGenericEvent);
		}
	}

	void EventManager::MessageHandler::operator()(const MessageWithHeader<EventHeader, WrappedEvent>& evt)
	{
		ProcessWrappedEvent(evt.body, evt.hdr);
	}

	void EventManager::RegisterNetworkEventInt(const EventHeader& hdr, const uint32 uEventId, const void* pData, const memsize uDataSize, bool bIncludeSelf)
	{
		MessageWithHeader<EventHeader, WrappedEvent> msg;
		EventHeader_init(&msg.hdr);
		msg.hdr = hdr;
		msg.body.uMessageId = uEventId;
		const memsize uWords = ((uDataSize + 3) & ~0x03) / 4;
		ASSERT(uWords * 4 >= uDataSize && (uDataSize % 4 != 0 || uDataSize == uWords*4));
		msg.body.uData_count = (uint32)uWords;
		msg.body.uDataSize = (uint32)uDataSize;
		ASSERT(uWords <= msg.body.uData_max_count && "You are creating network events of crazy size...");
		usg::MemCpy(msg.body.uData, pData, uDataSize);
		m_messenger->SendToAll(msg, true, bIncludeSelf);
	}

	void EventManager::RegisterEventWithEntityAtTime(Entity e, const uint32 uEventId, const void* pEventRawData, const memsize uDataSize, uint32 uTargets, float64 fTime)
	{
		struct GenericEventOnEntity : public EventOnEntityBase
		{
			usg::vector<uint8> m_data;

			GenericEventOnEntity(Entity e, const uint32 uEventId, const void* pData, memsize uDataSize, uint32 uTargets, double fTime) :
				EventOnEntityBase(uEventId, fTime, nullptr, e, uTargets)
			{
				m_data.resize(uDataSize);
				MemCpy(&m_data[0], pData, uDataSize);
				this->pData = &m_data[0];
			}

			~GenericEventOnEntity() override
			{
				
			}
		};

		void* buffer = m_heap.Allocate(sizeof(GenericEventOnEntity), 4, 0, ALLOC_EVENT);
		ASSERT(buffer != nullptr);
		GenericEventOnEntity* wrappedEvent = new (buffer) GenericEventOnEntity(e, uEventId, pEventRawData, uDataSize, uTargets, fTime);
		AddToEventQueue(wrappedEvent);

	}
}

using namespace usg;

EventManager::EventManager()
	: m_messenger(nullptr)
	, m_heap()
	, m_pMem(mem::Alloc(MEMTYPE_STANDARD, ALLOC_COMPONENT, HEAP_SIZE))
	, m_eventQueue()
	, m_bRegisteredDispatch(false)
{
	m_heap.Initialize(m_pMem, HEAP_SIZE);
}

EventManager::~EventManager()
{
	for (EventQueue::Iterator eventIterator = m_eventQueue.Begin(); !eventIterator.IsEnd();)
	{
		TriggerableEvent* evt = *eventIterator;
		m_heap.Deallocate(evt);
	}
	m_eventQueue.Clear();

	mem::Free(m_pMem);
}

void EventManager::Clear()
{
	for (EventQueue::Iterator eventIterator = m_eventQueue.Begin() ; !eventIterator.IsEnd();)
	{
		TriggerableEvent* evt = *eventIterator;
		m_heap.Deallocate(evt);
		eventIterator = m_eventQueue.Erase(eventIterator);
	}
	m_eventQueue.Clear();

	m_heap.FreeGroup(0);
	m_messenger = nullptr;
}

void EventManager::SetMessenger(Messenger* messenger)
{
	ASSERT(m_messenger == nullptr);
	m_messenger = messenger;
}

void EventManager::TriggerEventsForEntity(SystemCoordinator& systemCoordinator, Entity e, Entity rootEntity)
{
	ASSERT(e != nullptr);
	const float64 now = GetTimeNow();
	for (EventQueue::Iterator eventIterator = m_eventQueue.Begin(); !eventIterator.IsEnd();)
	{
		TriggerableEvent* evt = *eventIterator;
		if (evt->time <= now && e == evt->GetEntity())
		{
			evt->Trigger(systemCoordinator, rootEntity);
			evt->~TriggerableEvent();
			m_heap.Deallocate(evt);

			eventIterator = m_eventQueue.Erase(eventIterator);
		}
		else
		{
			++eventIterator;
		}
	}
}

void EventManager::TriggerEvents(SystemCoordinator& systemCoordinator, Entity rootEntity, uint32 uFrame)
{
	const float64 now = GetTimeNow();
	for( EventQueue::Iterator eventIterator = m_eventQueue.Begin()
	   ; !eventIterator.IsEnd()
	   ; )
	{
		TriggerableEvent* evt = *eventIterator;
		if(evt->time <= now)
		{
			if (!evt->GetEntity() || evt->GetEntity()->GetSpawnFrame() != 0)
			{
				evt->Trigger(systemCoordinator, rootEntity);
				evt->~TriggerableEvent();
				m_heap.Deallocate(evt);

				eventIterator = m_eventQueue.Erase(eventIterator);
			}
			else
			{
				++eventIterator;
			}
		}
		else
		{
			++eventIterator;
		}
	}
}

void EventManager::RegisterEntitiesRemoved(Entity* pEntities, uint32 uCount)
{
	double now = GetTimeNow();
	for (EventQueue::Iterator eventIterator = m_eventQueue.Begin(); !eventIterator.IsEnd(); )
	{
		TriggerableEvent* evt = *eventIterator;	
		Entity evtEntity = evt->GetEntity();
		if (!evtEntity)
		{
			++eventIterator;
			continue;
		}

		bool bFound = false;
		for (uint32 i = 0; i < uCount; i++)
		{
			if (evtEntity == pEntities[i])
			{
				eventIterator = m_eventQueue.Erase(eventIterator);
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			++eventIterator;
		}

	}
}


double EventManager::GetTimeNow()
{
	return NetTime::GetServerTimePrecise() - NET_TIME_INPUT_DELAY;
}
