#include "Engine/Common/Common.h"
#include "Engine/Framework/GameComponents.h"
#include "Script.h"

namespace usg
{
static void ReportError(lua_State* lua, const char* errString, const char* filename)
{
	const char* errMsg = luaL_checkstring(lua, -1);
	DEBUG_PRINT("%s %s: %s\n", errString, filename, errMsg);
	ASSERT(false);
}

template<>
void OnLoaded<Script>(Component<Script>& script, ComponentLoadHandles& handles, bool bWasPreviouslyCalled)
{
	if (bWasPreviouslyCalled)
	{
		return;
	}

	ASSERT(script->filename[0] != '\0');
	Required<LuaVMHandle, FromSelfOrParents> lua;
	GetComponent<LuaVMHandle>(script.GetEntity(), lua);

	LuaVM::Module* module = &script.GetRuntimeData().module;

	*module = lua->handle->Load(script->filename);
	ASSERT(script.GetRuntimeData().module.luaState != NULL);

	lua_rawgeti(module->luaState, LUA_REGISTRYINDEX, module->table);
	luaL_unref(module->luaState, LUA_REGISTRYINDEX, module->table);

	// Initialise OnEvent to an empty table
	lua_pushstring(module->luaState, "OnEvent");
	lua_newtable(module->luaState);
	lua_settable(module->luaState, -3);

	// Now call the function returned by the script
	lua_pushvalue(module->luaState, -2);
	int result = lua_pcall(module->luaState, 0, 0, 0);
	switch(result)
	{
		case LUA_OK:      break;
		case LUA_ERRRUN:  ReportError(module->luaState, "Runtime error loading", script->filename);         return;
		case LUA_ERRMEM:  ReportError(module->luaState, "Out of memory loading", script->filename);         return;
		case LUA_ERRERR:  ReportError(module->luaState, "Message handler error loading", script->filename); return;
		case LUA_ERRGCMM: ReportError(module->luaState, "GC error while loading", script->filename);        return;
		default:          ReportError(module->luaState, "Unknown error while loading", script->filename);   return;
	}

	// Generate a table mapping signal functions
	lua_newtable(module->luaState);

	static const uint32 RUN = utl::CRC32("Run");
	lua_pushinteger(module->luaState, RUN);
	lua_getfield(module->luaState, -3, "Run");
	lua_settable(module->luaState, -3);

	static const uint32 LATE_UPDATE = utl::CRC32("LateUpdate");
	lua_pushinteger(module->luaState, LATE_UPDATE);
	lua_getfield(module->luaState, -3, "LateUpdate");
	lua_settable(module->luaState, -3);

	static const uint32 INITALIZE = utl::CRC32("Initialize");
	lua_pushinteger(module->luaState, INITALIZE);
	lua_getfield(module->luaState, -3, "Initialize");
	lua_settable(module->luaState, -3);

	static const uint32 ON_EVENT = utl::CRC32("OnEvent");
	lua_pushinteger(module->luaState, ON_EVENT);
	lua_newtable(module->luaState);
	// Checksum all the keys in OnEvent
	const int stackTop = lua_gettop(module->luaState);
	lua_getfield(module->luaState, -4, "OnEvent");
	lua_pushnil(module->luaState);
	while(lua_next(module->luaState, -2) != 0)
	{
		lua_pushvalue(module->luaState, -2);
		const char* key = lua_tostring(module->luaState, -1);
		lua_pop(module->luaState, 1);

		lua_pushinteger(module->luaState, utl::CRC32(key));
		lua_pushvalue(module->luaState, -2);
		lua_settable(module->luaState, -6);

		lua_pop(module->luaState, 1);
	}
	lua_pop(module->luaState, 1);
	ASSERT(lua_gettop(module->luaState) == stackTop);
	lua_settable(module->luaState, -3);

	// Our new table is still at the top of the stack.
	// I'm assuming here that LUA_REGISTRYINDEX is shared between threads, as I load
	// a value into it from module->luaState's stack here, and then attempt below to copy
	// that value back out of it onto m_lua's stack.  If there are separate
	// registries for each thread, this won't work.
	module->table = luaL_ref(module->luaState, LUA_REGISTRYINDEX);

	lua->handle->Anchor(*module);

	// Pop the original version of the function we loaded from the stack
	lua_pop(module->luaState, 1);
}

template<>
void OnDeactivate<Script>(Component<Script>& script, ComponentLoadHandles& handles)
{
	Required<LuaVMHandle, FromSelfOrParents> lua;
	GetComponent<LuaVMHandle>(script.GetEntity(), lua);

	lua->handle->Unload(script.GetRuntimeData().module);
}
}
