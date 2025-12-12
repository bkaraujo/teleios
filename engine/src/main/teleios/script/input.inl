#ifndef __TELEIOS_SCRIPT_INPUT__
#define __TELEIOS_SCRIPT_INPUT__

#include "teleios/teleios.h"
#include <lua.h>
#include <lauxlib.h>

#define TL_LUA_ERROR(s) { return luaL_error(state, s); }

static i32 tl_script_application_exit(lua_State *state) {
    if (lua_gettop(state) != 0) TL_LUA_ERROR("No parameter were expected: exit()")
    tl_event_submit(TL_EVENT_WINDOW_CLOSED, NULL);
    return 0;
}

static i32 tl_script_is_key_active(lua_State *state) {
    if (lua_gettop(state) != 1) TL_LUA_ERROR("Expected a single value: isKeyActive(KEY)")
    if (!lua_isinteger(state, 1)) TL_LUA_ERROR("parameter [KEY] must be a valid key")

    const i32 key = (i32) lua_tointeger(state, 1);
    lua_pushboolean(state, tl_input_is_key_active(key));

    return 1;
}

static i32 tl_script_is_key_pressed(lua_State *state) {
    if (lua_gettop(state) != 1) TL_LUA_ERROR("Expected a single value: isKeyPressed(KEY)")
    if (!lua_isinteger(state, 1)) TL_LUA_ERROR("parameter [KEY] must be a valid key")

    const i32 key = (i32) lua_tointeger(state, 1);
    lua_pushboolean(state, tl_input_is_key_pressed(key));

    return 1;
}

static i32 tl_script_is_key_released(lua_State *state) {
    if (lua_gettop(state) != 1) TL_LUA_ERROR("Expected a single value: isKeyReleased(KEY)")
    if (!lua_isinteger(state, 1)) TL_LUA_ERROR("parameter [KEY] must be a valid key")

    const i32 key = (i32) lua_tointeger(state, 1);
    lua_pushboolean(state, tl_input_is_key_released(key));

    return 1;
}

static i32 tl_script_is_cursor_active(lua_State *state) {
    if (lua_gettop(state) != 1) TL_LUA_ERROR("Expected a single value: isCursorActive(BUTTON)")
    if (!lua_isinteger(state, 1)) TL_LUA_ERROR("parameter [BUTTON] must be a valid button")

    const i32 button = (i32) lua_tointeger(state, 1);
    lua_pushboolean(state, tl_input_is_cursor_button_active(button));

    return 1;
}

static i32 tl_script_is_cursor_pressed(lua_State *state) {
    if (lua_gettop(state) != 1) TL_LUA_ERROR("Expected a single value: isCursorPressed(BUTTON)")
    if (!lua_isinteger(state, 1)) TL_LUA_ERROR("parameter [BUTTON] must be a valid button")

    const i32 button = (i32) lua_tointeger(state, 1);
    lua_pushboolean(state, tl_input_is_cursor_button_pressed(button));

    return 1;
}

static i32 tl_script_is_cursor_released(lua_State *state) {
    if (lua_gettop(state) != 1) TL_LUA_ERROR("Expected a single value: isCursorReleased(BUTTON)")
    if (!lua_isinteger(state, 1)) TL_LUA_ERROR("parameter [BUTTON] must be a valid button")

    const i32 button = (i32) lua_tointeger(state, 1);
    lua_pushboolean(state, tl_input_is_cursor_button_released(button));

    return 1;
}

static i32 tl_script_is_cursor_hovering(lua_State *state) {
    if (lua_gettop(state) != 0) TL_LUA_ERROR("No parameter were expected: isCursorHovering()")

    lua_pushboolean(state, tl_input_is_cursor_hovering());

    return 0;
}

static i32 tl_script_get_cursor_scroll(lua_State *state) {
    if (lua_gettop(state) != 0) TL_LUA_ERROR("No parameter were expected: getCursorScroll()")

    const ivec2s scroll = tl_input_get_cursor_scroll();
    lua_pushinteger(state, scroll.x);
    lua_pushinteger(state, scroll.y);

    return 2;  // Retorna 2 valores (x, y)
}

static i32 tl_script_get_cursor_position(lua_State *state) {
    if (lua_gettop(state) != 0) TL_LUA_ERROR("No parameter were expected: getCursorPosition()")

    const vec2s position = tl_input_get_cursor_position();
    lua_pushnumber(state, position.x);
    lua_pushnumber(state, position.y);

    return 2;  // Retorna 2 valores (x, y)
}

static void tl_script_input_register(lua_State* state) {
    const luaL_Reg operations[] = {
        {"isKeyActive", tl_script_is_key_active},
        {"isKeyPressed", tl_script_is_key_pressed},
        {"isKeyReleased", tl_script_is_key_released},

        {"isCursorActive", tl_script_is_cursor_active},
        {"isCursorPressed", tl_script_is_cursor_pressed},
        {"isCursorReleased", tl_script_is_cursor_released},
        {"isCursorHovering", tl_script_is_cursor_hovering},
        {"getCursorScroll", tl_script_get_cursor_scroll},
        {"getCursorPosition", tl_script_get_cursor_position},

        {"exit", tl_script_application_exit},
        {NULL, NULL}
    };

    luaL_newlib(state, operations);
    lua_setglobal(state, "tl");
}

#endif