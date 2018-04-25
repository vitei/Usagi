#include "Engine/Common/Common.h"
#include "ExecuteScript.h"

#include "Engine/Framework/Script/LuaEvents.pb.h"

using namespace usg;

void ExecuteScript::CallInitialize(const LuaVM::Module& module)
{
	const int stackTop = lua_gettop(module.luaState);
	lua_rawgeti(module.luaState, LUA_REGISTRYINDEX, module.table);

	RunUnaryFunction<bool>(module, utl::CRC32("Initialize"), true);

	lua_pop(module.luaState, 1);

	// Make sure the stack frame has been correctly unrolled back to its starting point
	ASSERT(lua_gettop(module.luaState) == stackTop);
}

void ExecuteScript::SignalDT(const LuaVM::Module& module, uint32 functionID, float dt)
{
	const int stackTop = lua_gettop(module.luaState);
	lua_rawgeti(module.luaState, LUA_REGISTRYINDEX, module.table);

	RunUnaryFunction(module, functionID, dt);

	lua_pop(module.luaState, 1);

	// Make sure the stack frame has been correctly unrolled back to its starting point
	ASSERT(lua_gettop(module.luaState) == stackTop);
}

void ExecuteScript::Run(const Inputs& in, Outputs& out, float dt)
{
	if (!in.simactive->bActive)
		return;
	static const uint32 RUN = utl::CRC32("Run");
	SignalDT(in.script.GetRuntimeData().module, RUN, dt);
}

void ExecuteScript::LateUpdate(const Inputs& in, Outputs& out, float dt)
{
	if (!in.simactive->bActive)
		return;
	static const uint32 LATE_UPDATE = utl::CRC32("LateUpdate");
	SignalDT(in.script.GetRuntimeData().module, LATE_UPDATE, dt);
}

template<>
void ExecuteScript::OnEvent< ::usg::Events::LuaEvent >(const Inputs& in, Outputs& out, const ::usg::Events::LuaEvent& evt)
{
	static const uint32 ON_EVENT = utl::CRC32("OnEvent");

	const usg::LuaVM::Module& module = in.script.GetRuntimeData().module;
	const int stackTop = lua_gettop(module.luaState);

	lua_rawgeti(module.luaState, LUA_REGISTRYINDEX, module.table);
	lua_pushnumber(module.luaState, ON_EVENT);
	lua_gettable(module.luaState, -2);
	if(lua_istable(module.luaState, -1))
	{
		lua_pushnumber(module.luaState, evt.eventType);
		lua_gettable(module.luaState, -2);
		if(lua_isfunction(module.luaState, -1))
		{
			int result = lua_getglobal(module.luaState, "EventsQueue");
			ASSERT(result != LUA_TNIL);
			ASSERT(lua_istable(module.luaState, -1));
			lua_pushvalue(module.luaState, -2);
			lua_pushinteger(module.luaState, evt.eventIndex);
			result = lua_gettable(module.luaState, -3);
			ASSERT(result != LUA_TNIL);

			result = lua_pcall(module.luaState, 1, 0, 0);
			if(result != LUA_OK)
			{
				const char* errMsg = luaL_checkstring(module.luaState, -1);
				LOG_MSG(DEBUG_MSG_LOG|DEBUG_MSG_RAW, "%s\n", errMsg);
				lua_pop(module.luaState, 1);
			}

			lua_pop(module.luaState, 2);
		} else { lua_pop(module.luaState, 1); }
	}

	lua_pop(module.luaState, 2);

	// Make sure the stack frame has been correctly unrolled back to its starting point
	ASSERT(lua_gettop(module.luaState) == stackTop);
}
