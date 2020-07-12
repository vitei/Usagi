/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
****************************************************************************/
#pragma once

#include "Engine/Framework/Component.h"
#include "Engine/Framework/Signal.h"
#include "Engine/Physics/Collision.pb.h"

namespace usg
{
#ifndef FINAL_BUILD
	namespace physics
	{
		namespace details
		{
			extern float g_fOnCollisionClosureCallTimes;
			extern float g_fFoundColliderInputsTimes;
		}
	}
#endif

	struct OnCollisionSignal : public Signal
	{
		static constexpr uint32 ID = 0x78125552;

		Entity collider;
		const Collision& col;
		const uint32 uColliderGroup;

		OnCollisionSignal(Entity _collider, const Collision& _col, uint32 uColliderGroup) : Signal(ID), collider(_collider), col(_col), uColliderGroup(uColliderGroup)
		{
		
		}

		SIGNAL_RESPONDER(OnCollision)

		template<typename System>
		struct OnCollisionClosure : public SignalClosure
		{
			OnCollisionSignal* signal;
			OnCollisionClosure(OnCollisionSignal* sig) : signal(sig) {}
			virtual void operator()(const Entity e, const void* in, void* out)
			{
				ComponentGetter componentGetter(signal->collider);
				typename System::ColliderInputs colliderInputs;
#ifndef FINAL_BUILD
				physics::details::g_fOnCollisionClosureCallTimes++;
#endif
				if (System::GetColliderInputs(componentGetter, signal->uColliderGroup, colliderInputs))
				{
					System::OnCollision(*(const typename System::Inputs*)in, *(typename System::Outputs*)out, colliderInputs, signal->col);
#ifndef FINAL_BUILD
					physics::details::g_fFoundColliderInputsTimes++;
#endif
				}
			}
		};
	};
}