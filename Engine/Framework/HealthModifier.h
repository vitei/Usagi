/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef Usagi_xcode_HealthComponent_h
#define Usagi_xcode_HealthComponent_h

#include "Engine/Maths/MathUtil.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/FrameworkEvents.pb.h"

namespace usg
{

class HealthModifier
{
protected:
	static void ChangeHealth(Entity entity, Components::HealthComponent& health,
	                         EventManager* pEventManager, float fNewValue)
	{
		ASSERT(pEventManager != NULL);

		HealthChangedEvent damageEvt;
		damageEvt.fPrev = health.fLife;
		damageEvt.fNew = fNewValue;
		health.fLife = damageEvt.fNew;
		health.fLife = Math::Clamp(health.fLife, 0.0f, 1.0f);

		if (!Math::IsEqual(damageEvt.fPrev, damageEvt.fNew))
		{
			usg::NetworkUID* pNUID = GameComponents<usg::NetworkUID>::GetComponentData(entity);

			if(pNUID != NULL)
			{
				pEventManager->RegisterNetworkEventWithEntity(*pNUID, damageEvt, ON_ENTITY | ON_CHILDREN);
			}
			else
			{
				pEventManager->RegisterEventWithEntity(entity, damageEvt, ON_ENTITY | ON_CHILDREN);
			}
		}
	}

	static void AddDamage(Entity entity, Components::HealthComponent& health,
	                      EventManager* pEventManager, float fDamage, uint32 uAttackerTeam = 0, sint64 iAttackerNUID = 0, bool bForce = false)
	{
		ASSERT(pEventManager != NULL);

		if (CanDamage(health) || bForce)
		{
			bool bAlive = (health.fLife > 0.0f);

			ChangeHealth(entity, health, pEventManager, health.fLife-fDamage);

			if(bAlive && (health.fLife == 0.0f))
			{
				health.uKillerTeam = uAttackerTeam;
				health.iKillerNUID = iAttackerNUID;
			}
		}
	}
	static void AddHealth(Entity entity, Components::HealthComponent& health,
	                      EventManager* pEventManager, float fHealth)
	{
		ASSERT(pEventManager != NULL);
		ChangeHealth(entity, health, pEventManager, health.fLife + fHealth);
	}

	static void SetHealth(Entity entity, Components::HealthComponent& health,
	                      EventManager* pEventManager, float fHealth)
	{
		ASSERT(pEventManager != NULL);
		ChangeHealth(entity, health, pEventManager, fHealth);
	}

	static bool CanDamage(const Components::HealthComponent& health)
	{
		return true;
	}
};

}


#endif
