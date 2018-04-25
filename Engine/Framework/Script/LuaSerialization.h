/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _LUA_SERIALIZATION_H
#define _LUA_SERIALIZATION_H

#include "Engine/Framework/Script/LuaVM.h"
#include "Engine/Core/ProtocolBuffers/PBU8String.h"
#include "Engine/Core/String/String_Util.h"

namespace usg {

// Specialize this template to expose C types to Lua
template<typename T> struct LuaSerializer
{
	static const bool RECEIVE = false;
	static const bool SEND = false;
	static const bool GENERATE = false;

	static void Init(lua_State* L)
	{
		//static_assert(GENERATE || SEND || RECEIVE, "Invalid");
		ASSERT(false && "Attempting to initialise a type which we don't have a serializer for");
	}

	static int PushToLua(lua_State* L, const T& data)
	{
		//static_assert(GENERATE || SEND || RECEIVE, "Invalid");
		ASSERT(false && "Attempting to push a value to Lua which we don't have a serializer for");
		return 0;
	}

	static int GetFromLua(lua_State* L, int index, T* data)
	{
		//static_assert(GENERATE || SEND || RECEIVE, "Invalid");
		ASSERT(false && "Attempting to pop a value from Lua which we don't have a serializer for");
		return 0;
	}
};

template<> struct LuaSerializer<float>
{
	static void Init(lua_State* L) {}

	static int PushToLua(lua_State* L, const float& data)
	{
		lua_pushnumber(L, data);
		return 1;
	}

	static int GetFromLua(lua_State* L, int index, float* data)
	{
		*data = static_cast<float>(luaL_checknumber(L, index));
		return 0;
	}
};

template<> struct LuaSerializer<uint32>
{
	static void Init(lua_State* L) {}

	static int PushToLua(lua_State* L, const uint32& data)
	{
		lua_pushinteger(L, data);
		return 1;
	}

	static int GetFromLua(lua_State* L, int index, uint32* data)
	{
		*data = static_cast<uint32>(luaL_checkinteger(L, index));
		return 0;
	}
};

template<> struct LuaSerializer<uint64>
{
	static void Init(lua_State* L) {}

	static int PushToLua(lua_State* L, const uint64& data)
	{
		lua_pushinteger(L, data);
		return 1;
	}

	static int GetFromLua(lua_State* L, int index, uint64* data)
	{
		*data = static_cast<uint64>(luaL_checkinteger(L, index));
		return 0;
	}
};

template<> struct LuaSerializer<sint64>
{
	static void Init(lua_State* L) {}

	static int PushToLua(lua_State* L, const sint64& data)
	{
		lua_pushinteger(L, data);
		return 1;
	}

	static int GetFromLua(lua_State* L, int index, sint64* data)
	{
		*data = luaL_checkinteger(L, index);
		return 0;
	}
};

template<> struct LuaSerializer<sint32>
{
	static void Init(lua_State* L) {}

	static int PushToLua(lua_State* L, const sint32& data)
	{
		lua_pushinteger(L, data);
		return 1;
	}

	static int GetFromLua(lua_State* L, int index, sint32* data)
	{
		*data = (sint32)luaL_checkinteger(L, index);
		return 0;
	}
};

template<> struct LuaSerializer<bool>
{
	static void Init(lua_State* L) {}

	static int PushToLua(lua_State* L, const bool& data)
	{
		lua_pushboolean(L, data);
		return 1;
	}

	static int GetFromLua(lua_State* L, int index, bool* data)
	{
		*data = lua_toboolean(L, index) != 0;
		return 0;
	}
};

template<typename T, size_t ARRAY_LENGTH>
struct LuaSerializer<T[ARRAY_LENGTH]>
{
	static void Init(lua_State* L) {}

	static int PushToLua(lua_State* L, const T (&data)[ARRAY_LENGTH])
	{
		// Always push the length of a C array to the stack before calling this
		// push nil to use ARRAY_LENGTH
		const uint32 arrayLength = lua_isnil(L, -1) ? ARRAY_LENGTH : (uint32)lua_tointeger(L, -1);
		lua_pop(L, 1);
		ASSERT(arrayLength <= ARRAY_LENGTH);

		lua_newtable(L);
		for(uint32 i = 0; i < arrayLength; ++i)
		{
			LuaSerializer<T>::PushToLua(L, data[i]);
			lua_seti(L, -2, i + 1);
		}

		return 1;
	}

	static int GetFromLua(lua_State* L, int index, T (*data)[ARRAY_LENGTH])
	{
		lua_len(L, index);
		const pb_size_t arrayLength = (pb_size_t)lua_tointeger(L, -1);
		lua_pop(L, 1);
		ASSERT(arrayLength <= ARRAY_LENGTH);

		for(uint32 i = 0; i < arrayLength; ++i)
		{
			lua_pushinteger(L, i + 1);
			if(lua_gettable(L, -2) != LUA_TNIL)
			{
				LuaSerializer<T>::GetFromLua(L, -1, &(*data)[i]);
			}
			lua_pop(L, 1);
		}

		return arrayLength;
	}
};

template<size_t STRING_LENGTH>
struct LuaSerializer<char[STRING_LENGTH]>
{
	static void Init(lua_State* L) {}

	static int PushToLua(lua_State* L, const char (&data)[STRING_LENGTH])
	{
		lua_pushstring(L, data);
		return 1;
	}

	static int GetFromLua(lua_State* L, int index, char (*data)[STRING_LENGTH])
	{
		const char* luaString = lua_tostring(L, index);
		str::Copy(*data, luaString, STRING_LENGTH);
		return 0;
	}
};

template<int BUFFER_SIZE>
struct LuaSerializer< PBU8String<BUFFER_SIZE> >
{
	static void Init(lua_State* L) {}

	static int PushToLua(lua_State* L, const PBU8String<BUFFER_SIZE>& data)
	{
		lua_pushstring(L, data.Get().CStr());
		return 1;
	}

	static int GetFromLua(lua_State* L, int index, PBU8String<BUFFER_SIZE>* data)
	{
		const char* luaString = lua_tostring(L, index);
		data->m_data.m_string.ParseString(luaString);
		return 0;
	}
};

template<typename T>
void InitLuaType(lua_State* L)
{
	LuaSerializer<T>::Init(L);
}

template<typename T>
int PushToLua(lua_State* L, const T& data)
{
	return LuaSerializer<T>::PushToLua(L, data);
}

template<typename T>
int GetFromLua(lua_State* L, int index, T* data)
{
	return LuaSerializer<T>::GetFromLua(L, index, data);
}

}

#endif //_LUA_SERIALIZATION_H
