#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "Engine/Memory/MemUtil.h"
#include "Engine/Memory/ScratchRaw.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Core/File/BufferedFile.h"
#include "LuaVM.h"
#include <errno.h>


using namespace usg;

// Perform a full garbage collect if we have 50k to cleanup
static const int FULL_GC_THRESHOLD = 50 * 1024;

/*** Copied and pasted from linit.c to allow us to customise the set of included libraries ***/
// I've removed the "io" and "os" libraries as they are not supported.
static const luaL_Reg loadedlibs[] = {
  {"_G", luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {LUA_DBLIBNAME, luaopen_debug},
#if defined(LUA_COMPAT_BITLIB)
  {LUA_BITLIBNAME, luaopen_bit32},
#endif
  {NULL, NULL}
};


LUALIB_API void openlibs (lua_State *L) {
  const luaL_Reg *lib;
  /* "require" functions from 'loadedlibs' and set results to global table */
  for (lib = loadedlibs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }
}
/*** End copy and paste from linit.c ***/

/*** Based on the equivalent functions from loadlib.c to allow us to add a custom fileloader ***/
#if !defined (LUA_PATH_SEP)
#define LUA_PATH_SEP		";"
#endif

#if !defined (LUA_PATH_MARK)
#define LUA_PATH_MARK		"?"
#endif

#if !defined(LUA_LSUBSEP)
#define LUA_LSUBSEP		LUA_DIRSEP
#endif

static int readable (const char *filename) {
  return File::FileStatus(filename) == FILE_STATUS_VALID ? 1 : 0;
}

static const char *pushnexttemplate (lua_State *L, const char *path) {
  const char *l;
  while (*path == *LUA_PATH_SEP) path++;  /* skip separators */
  if (*path == '\0') return NULL;  /* no more templates */
  l = strchr(path, *LUA_PATH_SEP);  /* find next separator */
  if (l == NULL) l = path + strlen(path);
  lua_pushlstring(L, path, l - path);  /* template */
  return l;
}

static const char *searchpath (lua_State *L, const char *name,
                                             const char *path,
                                             const char *sep,
                                             const char *dirsep) {
  luaL_Buffer msg;  /* to build error message */
  luaL_buffinit(L, &msg);
  if (*sep != '\0')  /* non-empty separator? */
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  while ((path = pushnexttemplate(L, path)) != NULL) {
    const char *filename = luaL_gsub(L, lua_tostring(L, -1),
                                     LUA_PATH_MARK, name);
    lua_remove(L, -2);  /* remove path template */
    if (readable(filename))  /* does file exist and is readable? */
      return filename;  /* return that file name */
    lua_pushfstring(L, "\n\tno file '%s'", filename);
    lua_remove(L, -2);  /* remove file name */
    luaL_addvalue(&msg);  /* concatenate error msg. entry */
  }
  luaL_pushresult(&msg);  /* create error message */
  return NULL;  /* not found */
}

static const char *findfile (lua_State *L, const char *name,
                                           const char *pname,
                                           const char *dirsep) {
  const char *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  if (path == NULL)
    luaL_error(L, "'package.%s' must be a string", pname);
  return searchpath(L, name, path, ".", dirsep);
}

static int checkload (lua_State *L, int stat, const char *filename) {
  if (stat) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    return 2;  /* return open function and file name */
  }
  else
    return luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}

typedef struct LoadF {
  int n;  /* number of pre-read characters */
  BufferedFile *f;  /* file being read */
  char buff[BUFSIZ];  /* area for reading file */
} LoadF;

static int skipBOM (LoadF *lf) {
  const char *p = "\xEF\xBB\xBF";  /* UTF-8 BOM mark */
  int c;
  lf->n = 0;
  do {
    c = lf->f->ReadByte();
    if (c == EOF || c != *(const unsigned char *)p++) return c;
    lf->buff[lf->n++] = c;  /* to be read by the parser */
  } while (*p != '\0');
  lf->n = 0;  /* prefix matched; discard it */
  return lf->f->ReadByte();  /* return next character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
static int skipcomment (LoadF *lf, int *cp) {
  int c = *cp = skipBOM(lf);
  if (c == '#') {  /* first line is a comment (Unix exec. file)? */
    do {  /* skip first line */
      c = lf->f->ReadByte();
    } while (c != EOF && c != '\n') ;
    *cp = lf->f->ReadByte();  /* skip end-of-line, if present */
    return 1;  /* there was a comment */
  }
  else return 0;  /* no comment */
}

static const char *getF (lua_State *L, void *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  (void)L;  /* not used */
  if (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  else {  /* read a block from file */
	const size_t bytesremaining = lf->f->GetSize() - lf->f->GetPos();
	const size_t bytestoread = bytesremaining < sizeof(lf->buff) ? bytesremaining : sizeof(lf->buff);
    if (bytesremaining == 0) return NULL;
    *size = lf->f->Read(bytestoread, lf->buff);  /* read block */
  }
  return lf->buff;
}

static int errfile (lua_State *L, const char *what, int fnameindex) {
#ifdef WIN32
  static const size_t ERR_BUF_SIZE = 256;
  char serr[ERR_BUF_SIZE];
  strerror_s(serr, ERR_BUF_SIZE, errno);
#else
  const char *serr = strerror(errno);
#endif
  const char *filename = lua_tostring(L, fnameindex) + 1;
  lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
  lua_remove(L, fnameindex);
  return LUA_ERRFILE;
}

// Based on luaL_loadfilex()
LUALIB_API int usagi_loadfile (lua_State *L, const char *filename) {
  LoadF lf;
  int status;
  int c;
  int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  ASSERT(filename != NULL);

  lua_pushfstring(L, "@%s", filename);
  BufferedFile f(filename);
  if (!f.IsOpen()) return errfile(L, "open", fnameindex);

  lf.f = &f;

  if (skipcomment(&lf, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add line to correct line numbers */
  if (c != EOF)
    lf.buff[lf.n++] = c;  /* 'c' is the first character of the stream */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), NULL);
  lua_remove(L, fnameindex);
  return status;
}

static int searcher_Usagi (lua_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  if (filename == NULL) return 1;  /* module not found in this path */
  return checkload(L, (usagi_loadfile(L, filename) == LUA_OK), filename);
}

// Based on createsearcherstable()
static void replacesearcher (lua_State *L, int (*searcher)(lua_State *)) {
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "searchers");
  const lua_Integer searcher_offset = 2; /* required searcher is at table index 2. */

  lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
  lua_pushcclosure(L, searcher, 1);
  lua_rawseti(L, -2, searcher_offset);

  lua_pop(L, 2);
}

static void setsearchpath (lua_State *L, const char *path) {
  lua_getglobal(L, "package");
  lua_pushstring(L, path);
  lua_setfield(L, -2, "path");
  lua_pop(L, 1);
}
/*** End copy and paste from loadlib.c ***/

static void ReportError(lua_State* lua, const char* errString, const char* filename)
{
	const char* errMsg = luaL_checkstring(lua, -1);
	DEBUG_PRINT("%s %s: %s\n", errString, filename, errMsg);
	ASSERT(false);
}

static lua_CFunction luaB_tonumber = NULL;
static int lua_tonumber_with_metadata(lua_State *L)
{
	ASSERT(luaB_tonumber != NULL);
	return luaL_callmeta(L, 1, "__tonumber") ? 1 : luaB_tonumber(L);
}

LuaVM::LuaVM()
{
	static const size_t POOL_SIZE = 1024 * 512;
	m_pMemHeapBuffer = mem::Alloc(MEMTYPE_STANDARD, ALLOC_SCRIPT, POOL_SIZE, 4U);
	m_memHeap.Initialize(m_pMemHeapBuffer, POOL_SIZE);

	m_lua = lua_newstate(LuaVM::Alloc, this);
	openlibs(m_lua);
	replacesearcher(m_lua, searcher_Usagi);
	setsearchpath(m_lua, "./Scripts/?.lua");
	lua_pushcfunction(m_lua, LuaVM::Import);
	lua_setglobal(m_lua, "import");
	lua_pushcfunction(m_lua, LuaVM::Checksum);
	lua_setglobal(m_lua, "checksum");

	//Override tonumber() with our special version which calls __tonumber if defined
	lua_getglobal(m_lua, "tonumber");
	luaB_tonumber = lua_tocfunction(m_lua, -1);
	lua_pop(m_lua, 1);
	lua_pushcfunction(m_lua, lua_tonumber_with_metadata);
	lua_setglobal(m_lua, "tonumber");

	lua_newtable(m_lua);
	m_threadsTable = luaL_ref(m_lua, LUA_REGISTRYINDEX);
}

LuaVM::~LuaVM()
{
	if(m_lua != NULL)
	{
		lua_close(m_lua);
	}

	if(m_pMemHeapBuffer != NULL)
	{
		mem::Free(MEMTYPE_STANDARD, m_pMemHeapBuffer);
		m_pMemHeapBuffer = NULL;
	}
}

LuaVM::Module LuaVM::Load(const char* szFilename)
{
	Module module = { NULL, 0 };

	lua_State* newThread = lua_newthread(m_lua);
	if(newThread == NULL) { ASSERT(false); return module; }

	File luaFile(szFilename);
	int result = lua_load(newThread, LuaVM::Read, &luaFile, szFilename, NULL);
	switch(result)
	{
		case LUA_OK:        break;
		case LUA_ERRSYNTAX: ReportError(newThread, "Syntax error in", szFilename);             return module;
		case LUA_ERRMEM:    ReportError(newThread, "Out of memory loading", szFilename);       return module;
		case LUA_ERRGCMM:   ReportError(newThread, "GC error while loading", szFilename);      return module;
		default:            ReportError(newThread, "Unknown error while loading", szFilename); return module;
	}

	// Initialise the thread with a new, empty globals table
	// a = {}
	lua_newtable(newThread);

	// b = { __index = _G } -- Inherit from the actual globals table
	lua_newtable(newThread);
	lua_getglobal(newThread,"_G");
	lua_setfield(newThread, -2, "__index");

	// setmetatable(a, b)
	lua_setmetatable(newThread, -2);

	lua_pushvalue(newThread, -1);
	module.table = luaL_ref(newThread, LUA_REGISTRYINDEX);

	// _ENV = a
	lua_setupvalue(newThread, -2, 1);

	module.luaState = newThread;

	return module;
}

void LuaVM::Anchor(LuaVM::Module& module)
{
	// Anchor the module, to prevent it from being garbage collected
	// The following code is equivalent to: m_threadsTable[module->luaState] = module,
	// where module is the module table returned at the end of the script.
	lua_rawgeti(m_lua, LUA_REGISTRYINDEX, module.table);
	lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_threadsTable);
	const ptrdiff_t state = (ptrdiff_t)module.luaState; // the pointer value of the thread
	lua_pushnumber(m_lua, (lua_Number)state);
	lua_pushvalue(m_lua, -3); // The result of lua_pcall, i.e. the module table
	lua_settable(m_lua, -3);  // The result of rawgeti, i.e. the threads table
	lua_pop(m_lua, 1);
}

void LuaVM::Unload(LuaVM::Module& script)
{
	// Pushing nil to the value pointed to by the script will remove the entry from the table.
	lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_threadsTable);
	const ptrdiff_t state = (ptrdiff_t)script.luaState;
	lua_pushnumber(m_lua, (lua_Number)state);
	lua_pushnil(m_lua);
	lua_settable(m_lua, -3);
}

void LuaVM::Update()
{
	// Perform a full garbage collection cycle.
	// If this starts causing spikes, look at the documentation for lua_gc for alternatives
	// which would allow us more fine-grained control over how we handle garbage collection.
	uint32 uBytes = lua_gc(m_lua, LUA_GCCOUNT, 0);
	if(uBytes > FULL_GC_THRESHOLD)
	{
		DEBUG_PRINT("Performing full garbage collect");
		lua_gc(m_lua, LUA_GCCOLLECT, 0);	
	}
	else
	{
		lua_gc(m_lua, LUA_GCSTEP, 0);	
	}
	
}

void* LuaVM::Alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	LuaVM *self = (LuaVM*)ud;

	if(nsize == 0)
	{
		if(ptr != NULL)
		{
			self->m_memHeap.Deallocate(ptr);
		}
		return NULL;
	}
	else
	{
		void* allocatedMemory = NULL;

		if(ptr != NULL)
		{
			const size_t smallerSize = osize < nsize ? osize : nsize;

			void* scratchMemory = NULL;
			ScratchRaw::Init(&scratchMemory, smallerSize, 4);
			MemCpy(scratchMemory, ptr, smallerSize);
			self->m_memHeap.Deallocate(ptr);
			allocatedMemory = self->m_memHeap.Allocate(nsize, 4, 0, ALLOC_SCRIPT);
			MemCpy(allocatedMemory, scratchMemory, smallerSize);

			ScratchRaw::Free(&scratchMemory);
		}
		else
		{
			allocatedMemory = self->m_memHeap.Allocate(nsize, 4, 0, ALLOC_SCRIPT);
		}

		return allocatedMemory;
	}
}

const char* LuaVM::Read(lua_State *L, void *data, size_t *size)
{
	static const size_t BUFFER_SIZE = 1024;
	static char readBuffer[BUFFER_SIZE];

	BufferedFile* luaFile = (BufferedFile*)data;
	ASSERT(size != NULL);

	const size_t fileSize       = luaFile->GetSize();
	const size_t filePos        = luaFile->GetPos();
	const size_t bytesRemaining = fileSize - filePos;
	const size_t bytesToRead    = bytesRemaining < BUFFER_SIZE ? bytesRemaining : BUFFER_SIZE;
	const size_t bytesRead      = luaFile->Read(bytesToRead, readBuffer);

	*size = bytesRead;
	return readBuffer;
}

int LuaVM::Import(lua_State *L)
{
	U8String filename;
	filename.ParseString("Scripts/%s.lua", luaL_checkstring(L, -1));
	const char* szFilename = filename.CStr();

	File luaFile(szFilename);
	int result = lua_load(L, LuaVM::Read, &luaFile, szFilename, NULL);

	switch(result)
	{
		case LUA_OK:        break;
		case LUA_ERRSYNTAX: ReportError(L, "Syntax error in", szFilename);             return 0;
		case LUA_ERRMEM:    ReportError(L, "Out of memory loading", szFilename);       return 0;
		case LUA_ERRGCMM:   ReportError(L, "GC error while loading", szFilename);      return 0;
		default:            ReportError(L, "Unknown error while loading", szFilename); return 0;
	}

	// Use the current script's environment as the upvalue
	// Based on set_env() from Lua's loadlib.c
	lua_Debug ar;
	if (lua_getstack(L, 1, &ar) == 0 ||
	    lua_getinfo(L, "f", &ar) == 0 ||  /* get calling function */
	    lua_iscfunction(L, -1))
		luaL_error(L, "'import' not called from a Lua function");
	lua_getupvalue(L, -1, 1);
	lua_setupvalue(L, -3, 1);
	lua_pop(L, 1);

	result = lua_pcall(L, 0, LUA_MULTRET, 0);
	switch(result)
	{
		case LUA_OK:      break;
		case LUA_ERRRUN:  ReportError(L, "Runtime error loading", szFilename);         return 0;
		case LUA_ERRMEM:  ReportError(L, "Out of memory loading", szFilename);         return 0;
		case LUA_ERRERR:  ReportError(L, "Message handler error loading", szFilename); return 0;
		case LUA_ERRGCMM: ReportError(L, "GC error while loading", szFilename);        return 0;
		default:          ReportError(L, "Unknown error while loading", szFilename);   return 0;
	}

	return 1;
}

int LuaVM::Checksum(lua_State *L)
{
	const char* szString = luaL_checkstring(L, -1);
	lua_pushinteger(L, utl::CRC32(szString));

	return 1;
}

extern "C"
bool lua_hasfield(lua_State* L, int index, const char* k)
{
	bool hasField = true;
	int result = lua_getfield(L, index, k);
	if(result == LUA_TNIL) { hasField = false; }
	lua_pop(L, 1);
	return hasField;
}
