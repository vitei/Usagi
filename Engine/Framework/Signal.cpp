#include "Engine/Common/Common.h"
#include "Engine/Framework/GameComponents.h"
#include "Signal.h"

using namespace usg;

namespace usg
{
	struct RunSignal::RunClosure : public SignalClosure
	{
		RunSignal* signal;
		void(*pRunFunction)(const GenericInputOutputs&, GenericInputOutputs&, float32);

		RunClosure(RunSignal* sig, void* userData);
		virtual void operator()(const Entity e, const void* in, void* out);
	};

	void RunSignal::Trigger(Entity e, void* signal, const uint32 uSystemId, uint32 targets, void* userData)
	{
		RunClosure closure((RunSignal*)signal, userData);
		TriggerSignalOnEntity(e, closure, uSystemId, targets);
	}

	void RunSignal::TriggerFromRoot(Entity e, void* signal, const uint32 uSystemId, void* userData)
	{
		RunClosure closure((RunSignal*)signal, userData);
		TriggerSignalFromRoot(ComponentSystemInputOutputsSharedBase::GetRootSystem(uSystemId), closure);
	}

	RunSignal::RunClosure::RunClosure(RunSignal* sig, void* userData) : signal(sig), pRunFunction(reinterpret_cast<decltype(pRunFunction)>(userData))
	{

	}

	struct LateUpdateSignal::LateUpdateClosure : public SignalClosure
	{
		LateUpdateSignal* signal;
		void(*pRunFunction)(const GenericInputOutputs&, GenericInputOutputs&, float32);

		LateUpdateClosure(LateUpdateSignal* sig, void* userData);
		virtual void operator()(const Entity e, const void* in, void* out);
	};

	void LateUpdateSignal::LateUpdateClosure::operator()(const Entity e, const void* in, void* out)
	{
		pRunFunction(*(const GenericInputOutputs*)in, *(GenericInputOutputs*)out, e->GetCatchupTime() + signal->dt);
	}

	void LateUpdateSignal::Trigger(Entity e, void* signal, const uint32 uSystemId, uint32 targets, void* userData)
	{
		LateUpdateClosure closure((LateUpdateSignal*)signal, userData);
		TriggerSignalOnEntity(e, closure, uSystemId, targets);
	}

	void LateUpdateSignal::TriggerFromRoot(Entity e, void* signal, const uint32 uSystemId, void* userData)
	{
		LateUpdateClosure closure((LateUpdateSignal*)signal, userData);
		TriggerSignalFromRoot(ComponentSystemInputOutputsSharedBase::GetRootSystem(uSystemId), closure);
	}

	LateUpdateSignal::LateUpdateClosure::LateUpdateClosure(LateUpdateSignal* sig, void* userData) : signal(sig), pRunFunction(reinterpret_cast<decltype(pRunFunction)>(userData))
	{

	}

	void RunSignal::RunClosure::operator()(const Entity e, const void* in, void* out)
	{
		pRunFunction(*(const GenericInputOutputs*)in, *(GenericInputOutputs*)out, e->GetCatchupTime() + signal->dt);
	}

	GenericInputOutputs* Signal::GetRootSystem(const uint32 uSystemId)
	{
		return ComponentSystemInputOutputsSharedBase::GetRootSystem(uSystemId);
	}
}

void Signal::TriggerSignalFromRoot(GenericInputOutputs* root, SignalClosure& Signal)
{
	GenericInputOutputs* child = root->GetChildEntity();
	while(child)
	{
		TriggerSystemOnEntityAndChildren(child, Signal);
		child = child->GetNextSibling();
	}
}

void Signal::TriggerSignalOnEntity(Entity e, SignalClosure& Signal, uint32 systemID, uint32 targets)
{
	ASSERT(e != NULL);
	// Search for the first valid io pointer in either direction and jump from there
	GenericInputOutputs* pEntityIO = e->GetSystem(systemID);
	if(pEntityIO != NULL)
	{
		// If the current entity has what we need, just use it
		TriggerSystemOnTargets(pEntityIO, Signal, targets);
	}
	else
	{
		// Otherwise, search up and down the hierarchy until we find one
		if(targets & ON_PARENTS)
		{
			Entity parent = e->GetParentEntity();
			while(parent != NULL)
			{
				if(parent->GetSystem(systemID) != NULL)
				{
					GenericInputOutputs* io = parent->GetSystem(systemID);
					TriggerSystemOnParents(io, Signal);
					break;
				}
				parent = parent->GetParentEntity();
			}
		}

		if(targets & ON_CHILDREN)
		{
			Entity child = e->GetChildEntity();
			while(child)
			{
				TriggerSignalOnChildEntities(child, Signal, systemID);
				child = child->GetNextSibling();
			}
		}
	}
}

void Signal::TriggerSignalOnChildEntities(Entity e, SignalClosure& Signal, uint32 systemID)
{
	// Otherwise search for the first valid io pointer in either direction and jump from there
	if(e->GetSystem(systemID) != NULL)
	{
		// If the current entity has what we need, just use it
		GenericInputOutputs* io = e->GetSystem(systemID);
		TriggerSystemOnEntityAndChildren(io, Signal);
	}
	else
	{
		Entity child = e->GetChildEntity();
		while(child)
		{
			TriggerSignalOnChildEntities(child, Signal, systemID);
			child = child->GetNextSibling();
		}
	}
}

void Signal::TriggerSystemOnTargets(GenericInputOutputs* pSystem, SignalClosure& Signal, uint32 targets)
{
	if(targets & ON_PARENTS)
	{
		GenericInputOutputs* parent = pSystem->GetParentEntity();
		if(parent != NULL && parent->entity != NULL)
		{
			TriggerSystemOnParents(parent, Signal);
		}
	}

	if(targets & ON_ENTITY)
		Signal( pSystem->entity, pSystem->genericInputs, pSystem->genericOutputs );

	if(targets & ON_CHILDREN)
	{
		GenericInputOutputs* child = pSystem->GetChildEntity();
		while(child)
		{
			TriggerSystemOnEntityAndChildren(child, Signal);
			child = child->GetNextSibling();
		}
	}
}

void Signal::TriggerSystemOnParents(GenericInputOutputs* pSystem, SignalClosure& Signal)
{
	GenericInputOutputs* parent = pSystem->GetParentEntity();
	if(parent != NULL)
	{
		TriggerSystemOnParents(parent, Signal);
	}

	Signal( pSystem->entity, pSystem->genericInputs, pSystem->genericOutputs );
}

void Signal::TriggerSystemOnEntityAndChildren(GenericInputOutputs* pSystem, SignalClosure& Signal)
{
	Signal( pSystem->entity, pSystem->genericInputs, pSystem->genericOutputs );

	GenericInputOutputs* child = pSystem->GetChildEntity();
	while(child)
	{
		TriggerSystemOnEntityAndChildren(child, Signal);
		child = child->GetNextSibling();
	}
}
