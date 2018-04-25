/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _LUA_VM_H
#define _LUA_VM_H

#include <lua.hpp>

#include "Engine/Memory/MemHeap.h"

namespace usg {

class LuaVM
{
public:
	LuaVM();
	~LuaVM();

	struct Module
	{
		lua_State* luaState;
		int        table;
	};

	Module Load(const char* szFilename);
	void   Anchor(Module& module);
	void   Unload(Module& script);
	void   Update();

	lua_State *Root() { return m_lua; }

private:
	void *m_pMemHeapBuffer;
	MemHeap m_memHeap;

	lua_State *m_lua;
	int m_threadsTable;

	static void* Alloc(void *ud, void *ptr, size_t osize, size_t nsize);
	static const char* Read(lua_State *L, void *data, size_t *size);
	static int Import(lua_State *L);
	static int Checksum(lua_State *L);
};

namespace Components {
struct LuaVMHandle {
	LuaVM *handle;
};
}

}

/* Check whether the table at the given index has an entry for the string k */
extern "C"
bool lua_hasfield(lua_State* L, int index, const char* k);

#endif //_LUA_VM_H
