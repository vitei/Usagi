#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Network/Network.pb.h"
#include "Event.h"
#include "Engine/Framework/SystemCoordinator.h"

namespace usg
{

	struct OnEventSignalBase::OnEventClosure : public SignalClosure
	{
		const void* pEventData;
		void(*pEventFunction)(const GenericInputOutputs&, GenericInputOutputs&, const uint8& evt);
		OnEventClosure(const void* pEventData, void* userData) : pEventData(pEventData), pEventFunction(reinterpret_cast<decltype(pEventFunction)>(userData)) {}
		virtual void operator()(const Entity e, const void* in, void* out)
		{
			pEventFunction(*(const GenericInputOutputs*)in, *(GenericInputOutputs*)out, *(const uint8*)pEventData);
		}
	};

	void OnEventSignalBase::Trigger(Entity e, void* signal, const uint32 uSystemId, uint32 targets, void* userData)
	{
		OnEventSignalBase* pSignalBase = (OnEventSignalBase*)signal;
		OnEventClosure closure(pSignalBase->pEventData, userData);
		TriggerSignalOnEntity(e, closure, uSystemId, targets);
	}

	void OnEventSignalBase::TriggerFromRoot(Entity e, void* signal, const uint32 uSystemId, void* userData)
	{
		OnEventSignalBase* pSignalBase = (OnEventSignalBase*)signal;
		OnEventClosure closure(pSignalBase->pEventData, userData);
		TriggerSignalFromRoot(ComponentSystemInputOutputsSharedBase::GetRootSystem(uSystemId), closure);
	}

	Entity TriggerableEvent::GetEntityFromNetworkUID(sint64 uid)
	{
		for (GameComponents<NetworkUID>::Iterator it = GameComponents<NetworkUID>::GetIterator(); !it.IsEnd(); ++it)
		{
			if ((*it)->GetData().gameUID == uid)
				return (*it)->GetEntity();
		}

		return nullptr;
	}

	void EventOnNetworkEntityBase::Trigger(SystemCoordinator& sc, Entity root)
	{
		Entity e = GetEntityFromNetworkUID(iNUID);
		if (e != nullptr)
		{
			OnEventSignalBase sig(uSignalId, pData);
			sc.Trigger(e, sig, uTargets);
		}
		else
		{
			DEBUG_PRINT("WARNING: Network event for 0x%016llx dropped -- NULL entity\n", iNUID);
		}
	}

	void EventBase::Trigger(SystemCoordinator& sc, Entity root)
	{
		OnEventSignalBase sig(uSignalId, pData);
		sc.TriggerFromRoot(root, sig);
	}

	void EventOnEntityBase::Trigger(SystemCoordinator& sc, Entity root)
	{
		OnEventSignalBase sig(uSignalId, pData);
		sc.Trigger( e, sig, uTargets);
	}


}