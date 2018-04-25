/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/

#ifndef _EXECUTE_SCRIPT_H_
#define _EXECUTE_SCRIPT_H_

#include "Engine/Core/Utility.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/FrameworkComponents.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/Signal.h"
#include "Engine/Framework/SystemCategories.h"
#include "Engine/Framework/Script/Script.h"
#include "Engine/Framework/Script/LuaSerialization.h"

namespace usg {
namespace Events { typedef struct _LuaEvent LuaEvent; }

namespace Systems {

class ExecuteScript : public usg::System
{
public:
	struct Inputs
	{
		Required<usg::Script> script;

		// this should be safe because simulationactive is always present
		// in multiplayer.
		Required<usg::Components::SimulationActive, FromParents> simactive;
	};

	struct Outputs
	{
		Required<usg::Script> script;
	};

	DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)

	static void Run(const Inputs& in, Outputs& out, float dt);
	static void LateUpdate(const Inputs& in, Outputs& out, float dt);

	template<typename EventType>
	static void OnEvent(const Inputs& in, Outputs& out, const EventType& evt)
	{
		static const uint32 ON_EVENT = utl::CRC32("OnEvent");
		static const uint32 eventNameCRC = usg::ProtocolBufferFields<EventType>::ID;

		const usg::LuaVM::Module& module = in.script.GetRuntimeData().module;
		const int stackTop = lua_gettop(module.luaState);

		lua_rawgeti(module.luaState, LUA_REGISTRYINDEX, module.table);
		lua_pushnumber(module.luaState, ON_EVENT);
		lua_gettable(module.luaState, -2);
		if(lua_istable(module.luaState, -1))
		{
			RunUnaryFunction(in.script.GetRuntimeData().module, eventNameCRC, evt);
		}

		lua_pop(module.luaState, 2);

		// Make sure the stack frame has been correctly unrolled back to its starting point
		ASSERT(lua_gettop(module.luaState) == stackTop);
	}

	static void CallInitialize(const LuaVM::Module& module);

private:
	static void SignalDT(const usg::LuaVM::Module& module, uint32 functionID, float dt);

	// Assumes a table mapping function IDs to closures has already been pushed onto the stack
	template<typename ParamType>
	static void RunUnaryFunction(const usg::LuaVM::Module& module, uint32 functionID, ParamType p)
	{
		lua_pushnumber(module.luaState, functionID);
		lua_gettable(module.luaState, -2);
		if(lua_isfunction(module.luaState, -1))
		{
			PushToLua(module.luaState, p);

			int result = lua_pcall(module.luaState, 1, 0, 0);
			if(result != LUA_OK)
			{
				const char* errMsg = luaL_checkstring(module.luaState, -1);
				LOG_MSG(DEBUG_MSG_LOG|DEBUG_MSG_RAW, "%s\n", errMsg);
				lua_pop(module.luaState, 1);
			}
		} else { lua_pop(module.luaState, 1); }
	}
};

template<>
void ExecuteScript::OnEvent< ::usg::Events::LuaEvent >(const Inputs& in, Outputs& out, const ::usg::Events::LuaEvent& evt);


}
}

#endif //_EXECUTE_SCRIPT_H_
