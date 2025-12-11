#include "teleios/teleios.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static lua_State* m_state;

void tl_script_register(const char* identifier, const luaL_Reg operations[]) {
    luaL_newlib(m_state, operations);
    lua_setglobal(m_state, identifier);
}

#include "teleios/script/input.inl"

b8 tl_script_initialize(void) {
    TLDEBUG("LUA_VERSION %s.%s.%s", LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    m_state = luaL_newstate();
    if (m_state == NULL) {
        TLERROR("Failed to initialize LUA")
        return false;
    }

    tl_script_input_register();

    luaL_openlibs(m_state);
    return true;
}

b8 tl_script_terminate(void) {
    if (!m_state) return true;

    lua_close(m_state);
    return true;
}

b8 tl_script_execute(TLString* path) {
    if (luaL_dofile(m_state, tl_string_cstr(path)) != LUA_OK) {
        TLERROR("Failed %s: %s", tl_string_cstr(path),  lua_tostring(m_state, -1))
        return false;
    }

    return true;
}