#ifndef __TELEIOS_SCRIPT_INPUT__
#define __TELEIOS_SCRIPT_INPUT__

#include "teleios/teleios.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define TL_LUA_ERROR(s) { return luaL_error(state, s); }

static i32 tl_script__application_exit(lua_State *state) {
    if (lua_gettop(state) != 0) TL_LUA_ERROR("No parameter were expected.")
    tl_event_submit(TL_EVENT_WINDOW_CLOSED, NULL);
    return 0;
}

static i32 tl_script__is_key_pressed(lua_State *state) {
    if (lua_gettop(state) != 1) TL_LUA_ERROR("Expected a single value: isKeyPressed(KEY)")
    if (!lua_isinteger(state, 1)) TL_LUA_ERROR("parameter [KEY] must be a valid key")

    const i32 key = lua_tonumber(state, 1);
    lua_pushboolean(state, tl_input_is_key_pressed(key));
    return 1;
}

static i32 tl_script__is_key_released(lua_State *state) {
    if (lua_gettop(state) != 1) TL_LUA_ERROR("Expected a single value: isKeyReleased(KEY)")
    if (!lua_isinteger(state, 1)) TL_LUA_ERROR("parameter [KEY] must be a valid key")

    const i32 key = lua_tonumber(state, 1);
    lua_pushboolean(state, tl_input_is_key_released(key));
    return 1;
}

#endif