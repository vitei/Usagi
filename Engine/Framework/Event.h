/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// Events
// The OnEvent signal is a special kind of signal
// which is parameterized on the event type.

#ifndef USAGI_FRAMEWORK_EVENT_H_
#define USAGI_FRAMEWORK_EVENT_H_

#include "Signal.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFields.h"
#include "Engine/Framework/MessageDispatch.h"
#include "Engine/Framework/SystemId.h"

namespace usg
{
	class SystemCoordinator;

	struct OnEventSignalBase : public Signal
	{
		const void* pEventData;

		struct OnEventClosure;

		static void Trigger(Entity e, void* signal, const uint32 uSystemId, uint32 targets, void* userData);
		static void TriggerFromRoot(Entity e, void* signal, const uint32 uSystemId, void* userData);

		OnEventSignalBase(const uint32 ID, const void* pEventData) : Signal(ID), pEventData(pEventData)
		{

		}

	};

	template<typename EventType>
	struct OnEventSignal : public OnEventSignalBase
	{
		OnEventSignal(const EventType& eventData) : OnEventSignalBase(ID, &eventData) {}

		enum { ID = ProtocolBufferFields<EventType>::ID };

		template<typename System>
		static inline bool FillSignalRunner(SignalRunner& runner, uint32 systemID)
		{
			runner.systemID = systemID;
			runner.priority = (sint32)System::CATEGORY;
			runner.Trigger = Trigger;
			runner.TriggerFromRoot = TriggerFromRoot;
			runner.userData = (void*)static_cast<void(*)(const typename System::Inputs&, typename System::Outputs&, const EventType&)>(System::OnEvent);
			return true;
		}
	};

	struct TriggerableEvent
	{
		virtual ~TriggerableEvent() {}
		float64 time;
		TriggerableEvent(double _time) : time(_time) {}
		virtual void Trigger(SystemCoordinator&, Entity) = 0;
		virtual Entity GetEntity() { return NULL; }
	protected:
		static Entity GetEntityFromNetworkUID(sint64 uid);
	};

	struct EventBase : public TriggerableEvent
	{
		const void* pData;
		const uint32 uSignalId;

		void Trigger(SystemCoordinator& sc, Entity root) override;

		EventBase(const uint32 uSignalId, const double fTime, const void* pData) : TriggerableEvent(fTime), pData(pData), uSignalId(uSignalId) {}
	};

	template<typename EventType>
	struct Event : public EventBase
	{
		typedef OnEventSignal<EventType> Signal;
		typedef void* ExtraData;
		EventType evt;
		ExtraData extra;
		Event(const EventType& _evt, double _time, ExtraData _extra = nullptr) : EventBase(Signal::ID, _time, (void*)&evt), evt(_evt), extra(_extra) {}
	};

	struct EventOnEntityBase : public TriggerableEvent
	{
		const void* pData;
		const uint32 uSignalId;
		Entity e;
		uint32 uTargets;

		void Trigger(SystemCoordinator& sc, Entity root) override;
		Entity GetEntity() override { return e; }

		EventOnEntityBase(const uint32 uSignalId, const double fTime, const void* pData, Entity e, uint32 uTargets) : TriggerableEvent(fTime), pData(pData), uSignalId(uSignalId), e(e), uTargets(uTargets) {}
	};

	template<typename EventType>
	struct EventOnEntity : public EventOnEntityBase
	{
		typedef OnEventSignal<EventType> Signal;
		typedef void* ExtraData;
		EventType evt;
		EventOnEntity(Entity _e, const EventType& _evt, uint32 _targets, double _time, ExtraData) : EventOnEntityBase(Signal::ID, _time, (void*)&evt, _e, _targets), evt(_evt) {}
	};

	struct EventOnNetworkEntityBase : public TriggerableEvent
	{
		const void* pData;
		const uint32 uSignalId;
		sint64 iNUID;
		uint32 uTargets;
		EventOnNetworkEntityBase(const uint32 uSignalId, double fTime, const void* pData, sint64 iNUID, uint32 uTargets) : TriggerableEvent(fTime), pData(pData), uSignalId(uSignalId), iNUID(iNUID), uTargets(uTargets) {}
		void Trigger(SystemCoordinator& sc, Entity root) override;
	};

// Visual studio isn't very good at inferring template parameters,
// particularly for callback functions.  This little function will
// help give it a prod in the right direction.
// See this StackOverflow post for more information:
//   http://stackoverflow.com/questions/3405719/unable-to-pass-template-function-as-a-callback-parameter
template<typename T>
T id(T t)
{
	return t;
}

//DELEGATE_EVENT allows you to derive off a system utility module which provides event handlers, and
//delegate the handling of certain events to that module.  It would be nicer if we didn't need to have
//this; if the presence of those event handlers in a System's parent would mean that C++ could just find
//it automatically, but because both BEGIN_EVENTS and handlers' OnEvent functions have to be template
//functions, C++ will pick the empty BEGIN_EVENT instead of the handler we want.
#define DELEGATE_EVENT(EVENT, HANDLER) \
	static void OnEvent(const Inputs& inputs, Outputs& outputs, const EVENT& evt) \
	{ HANDLER::OnEvent(inputs, outputs, evt); }

}

#endif //USAGI_FRAMEWORK_EVENT_H_
