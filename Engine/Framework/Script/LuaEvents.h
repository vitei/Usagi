/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/

#ifndef USAGI_FRAMEWORK_SCRIPTS_LUA_EVENTS_H_
#define USAGI_FRAMEWORK_SCRIPTS_LUA_EVENTS_H_

#include "Engine/Framework/Script/LuaVM.h"
#include "Engine/Framework/Script/LuaSerialization.h"
#include "Engine/Framework/EntityRef.h"

namespace usg {

class EventManager;

::usg::EventManager& GetLuaEventManager(lua_State* L);
void RegisterEventManagerWithLua(EventManager* pEventManager, LuaVM* pLuaVM);
int SendLuaEvent(lua_State* L);
int SendLuaEventToEntity(lua_State* L);

template<typename T>
struct LuaEvents
{
	static int Send(lua_State* L);
	static int SendToEntity(lua_State* L);
	static int SendOverNetwork(lua_State* L);
	static int SendToNetworkEntity(lua_State* L);
};

template<typename T>
void RegisterLuaEventTransmitters(lua_State* L);

template<typename T>
int LuaEvents<T>::Send(lua_State* L)
{
	const T& message = ::usg::LuaSerializer<T>::CheckMessage(L, 1);
	::usg::EventManager& eventManager = GetLuaEventManager(L);
	eventManager.RegisterEvent(message);

	return 0;
}

template<typename T>
int LuaEvents<T>::SendToEntity(lua_State* L)
{
	::usg::EventManager& eventManager = GetLuaEventManager(L);

	const T& message = ::usg::LuaSerializer<T>::CheckMessage(L, 1);

	Entity entity = NULL;
	EntityRef* entityRef = (EntityRef*)luaL_testudata(L, 2, "usg.EntityRef");
	if(entityRef != NULL)
	{
		entity = entityRef->entity;
	}
	else
	{
		NetworkUID* nuid = (NetworkUID*)luaL_checkudata(L, 2, "usg.Components.NetworkUID");
		if(nuid != NULL)
		{
			for(GameComponents<NetworkUID>::Iterator it = GameComponents<NetworkUID>::GetIterator(); !it.IsEnd(); ++it)
			{
				if((*it)->GetData().gameUID == nuid->gameUID)
					entity = (*it)->GetEntity();
			}
		}
		else
		{
			return luaL_error(L, "Attempt to call SendToEntity with value of the wrong type!\n");
		}
	}

	if(lua_gettop(L) < 3)
	{
		eventManager.RegisterEventWithEntity(entity, message);
	}
	else
	{
		uint32 targets;
		::usg::GetFromLua(L, 3, &targets);
		eventManager.RegisterEventWithEntity(entity, message, targets);
	}

	return 0;
}

template<typename T>
int LuaEvents<T>::SendOverNetwork(lua_State* L)
{
	::usg::EventManager& eventManager = GetLuaEventManager(L);

	const T& message = ::usg::LuaSerializer<T>::CheckMessage(L, 1);

	if(lua_gettop(L) < 2)
	{
		eventManager.RegisterNetworkEvent(message);
	}
	else
	{
		bool includeSelf;
		::usg::GetFromLua(L, 2, &includeSelf);
		eventManager.RegisterNetworkEvent(message, includeSelf);
	}

	return 0;
}

template<typename T>
int LuaEvents<T>::SendToNetworkEntity(lua_State* L)
{
	::usg::EventManager& eventManager = GetLuaEventManager(L);

	const T& message = ::usg::LuaSerializer<T>::CheckMessage(L, 1);
	NetworkUID nuid;
	::usg::GetFromLua(L, 2, &nuid);

	if(lua_gettop(L) < 3)
	{
		eventManager.RegisterNetworkEventWithEntity(nuid.gameUID, message);
	}
	else if(lua_gettop(L) < 4)
	{
		uint32 targets;
		::usg::GetFromLua(L, 3, &targets);
		eventManager.RegisterNetworkEventWithEntity(nuid.gameUID, message, targets);
	}
	else
	{
		uint32 targets;
		::usg::GetFromLua(L, 3, &targets);
		bool includeSelf;
		::usg::GetFromLua(L, 3, &includeSelf);
		eventManager.RegisterNetworkEventWithEntity(nuid.gameUID, message, targets, includeSelf);
	}

	return 0;
}

}

#endif //USAGI_FRAMEWORK_SCRIPTS_LUA_EVENTS_H_
