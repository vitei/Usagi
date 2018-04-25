#include "Engine/Common/Common.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/EntityRef.pb.h"
#include "LuaEvents.h"
#include "Engine/Framework/Script/LuaEvents.pb.h"

namespace usg {

EventManager& GetLuaEventManager(lua_State* L)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	int result = lua_getfield(L, -1, "EventManager");
	EventManager** ppEventManager = (EventManager**)luaL_checkudata(L, -1, "EventManager");
	ASSERT(result != LUA_TNIL);
	lua_pop(L, 2);
	return **ppEventManager;
}

void RegisterEventManagerWithLua(EventManager* pEventManager, LuaVM* pLuaVM)
{
	lua_State* L = pLuaVM->Root();

	bool alreadyRegistered = luaL_newmetatable(L, "EventManager") == 0;
	ASSERT(!alreadyRegistered);
	lua_pop(L, 1);

	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	ASSERT( !lua_hasfield(L, -1, "EventManager") );

	EventManager** ppEventManager = (EventManager**)lua_newuserdata(L, sizeof(EventManager*));
	*ppEventManager = pEventManager;
	luaL_setmetatable(L, "EventManager");
	lua_setfield(L, -2, "EventManager");

	lua_newtable(L);
	lua_setfield(L, -2, "EventsQueue");

	lua_pushcfunction(L, SendLuaEvent);
	lua_setfield(L, -2, "SendLuaEvent");
	lua_pushcfunction(L, SendLuaEventToEntity);
	lua_setfield(L, -2, "SendLuaEventToEntity");

	lua_pop(L, 1);
}

static uint32 PushLuaEventToQueue(lua_State* L, int idx)
{
	int result = lua_getglobal(L, "EventsQueue");
	ASSERT(result != LUA_TNIL);
	ASSERT(lua_istable(L, -1));

	lua_pushvalue(L, idx);
	int uEventIndex = luaL_ref(L, -2);
	ASSERT(uEventIndex != LUA_REFNIL);

	lua_pop(L, 1);

	return uEventIndex;
}

static void InitLuaEvent(lua_State* L, LuaEvent& message, int idx)
{
	bool bHasEventType = luaL_getmetafield(L, 1, "__eventtype") != LUA_TNIL;
	ASSERT(bHasEventType);
	const uint32 uEventType = static_cast<uint32>(lua_tointeger(L, -1));
	lua_pop(L, 1);

	uint32 uEventIndex = PushLuaEventToQueue(L, 1);

	message.eventType = uEventType;
	message.eventIndex = uEventIndex;
}

int SendLuaEvent(lua_State* L)
{
	EventManager& eventManager = GetLuaEventManager(L);

	LuaEvent message;
	InitLuaEvent(L, message, 1);
	eventManager.RegisterEvent(message, L);

	return 0;
}

int SendLuaEventToEntity(lua_State* L)
{
	EventManager& eventManager = GetLuaEventManager(L);

	LuaEvent message;
	InitLuaEvent(L, message, 1);

	EntityRef entity;
	GetFromLua(L, 2, &entity);

	if(lua_gettop(L) < 3)
	{
		eventManager.RegisterEventWithEntity(entity.entity, message, ON_ENTITY, L);
	}
	else
	{
		uint32 targets;
		GetFromLua(L, 3, &targets);
		eventManager.RegisterEventWithEntity(entity.entity, message, targets, L);
	}

	return 0;
}

}
