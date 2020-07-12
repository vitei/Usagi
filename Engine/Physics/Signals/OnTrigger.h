/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma  once

#include "Engine/Framework/Component.h"
#include "Engine/Framework/Signal.h"

namespace usg
{
	enum class TriggerEventType
	{
		Enter,
		Exit
	};

	struct OnTriggerSignal : public Signal
	{
		static constexpr uint32 ID = 0x7812aabb;

		Entity triggerer;
		TriggerEventType eventType;

		OnTriggerSignal(Entity triggerer, TriggerEventType eventType) : Signal(ID), triggerer(triggerer), eventType(eventType)
		{

		}

		SIGNAL_RESPONDER(OnTrigger)

		template<typename System>
		struct OnTriggerClosure : public SignalClosure
		{
			OnTriggerSignal* signal;
			OnTriggerClosure(OnTriggerSignal* sig) : signal(sig)
			{
				
			}

			virtual void operator()(const Entity e, const void* in, void* out)
			{
				ComponentGetter componentGetter(signal->triggerer);
				typename System::TriggererInputs triggererInputs;
				if (System::GetTriggererInputs(componentGetter, triggererInputs))
					System::OnTrigger(*(const typename System::Inputs*)in, *(typename System::Outputs*)out, triggererInputs, signal->eventType);
			}
		};
	};
}