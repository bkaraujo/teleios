#include "teleios/core.h"
#include "teleios/globals.h"

#define ERROR(s) TLSTACKPOPV(luaL_error(state, s))

i32 tl_script_api__is_key_pressed(lua_State *state) {
    TLSTACKPUSHA("0x%p", state)

    if (lua_gettop(state) != 1) ERROR("Expected a single value: isKeyPressed(KEY)")
    if (!lua_isnumber(state, 1)) ERROR("parameter [KEY] must be a valid key")

    const i32 key = lua_tonumber(state, 1);
    lua_pushnumber(state, global->platform.input.keyboard.key[key]);

    TLSTACKPOPV(1)
}

i32 tl_script_api__is_key_released(lua_State *state) {
    TLSTACKPUSHA("0x%p", state)

    TLSTACKPOPV(0)
}

b8 tl_script_initialize(void) {
    TLSTACKPUSH

    TLDEBUG("LUA_VERSION %s.%s.%s", LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    global->platform.script.state = luaL_newstate();
    if (global->platform.script.state == NULL) {
        TLERROR("Failed to initialize LUA")
    }

    luaL_openlibs(global->platform.script.state);

    // Tabela para registrar as funções
    static const luaL_Reg functions[] = {
        {"isKeyPressed", tl_script_api__is_key_pressed},
        {"isKeyReleased", tl_script_api__is_key_released},
        {NULL, NULL}
    };

    luaL_newlib(global->platform.script.state, functions);
    lua_setglobal(global->platform.script.state, "teleios_input");

    TLSTACKPOPV(TRUE)
}

b8 tl_script_terminate(void) {
    TLSTACKPUSH
    lua_close(global->platform.script.state);
    global->platform.script.state = NULL;
    TLSTACKPOPV(TRUE)
}