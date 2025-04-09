#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/runtime/libs.h"
#include "teleios/globals.h"

#define TLSCRIPTERR(s) K_FRAME_POP_WITH(luaL_error(state, s))

static i32 tl_script__application_exit(lua_State *state) {
    K_FRAME_PUSH_WITH("0x%p", state)
    // =========================================================================
    // Parameters validation
    // =========================================================================
    if (lua_gettop(state) != 0) TLSCRIPTERR("No parameter were expected.")
    // =========================================================================
    // Request execution
    // =========================================================================
    tl_event_submit(TL_EVENT_WINDOW_CLOSED, NULL);
    // =========================================================================
    // LUA stack push
    // =========================================================================
    K_FRAME_POP_WITH(0)
}

static i32 tl_script__is_key_pressed(lua_State *state) {
    K_FRAME_PUSH_WITH("0x%p", state)
    // =========================================================================
    // Parameters validation
    // =========================================================================
    if (lua_gettop(state) != 1) TLSCRIPTERR("Expected a single value: isKeyPressed(KEY)")
    if (!lua_isinteger(state, 1)) TLSCRIPTERR("parameter [KEY] must be a valid key")
    // =========================================================================
    // Request execution
    // =========================================================================
    const i32 key = lua_tonumber(state, 1);
    lua_pushboolean(state, tl_input_is_key_pressed(key));
    // =========================================================================
    // LUA stack push
    // =========================================================================
    K_FRAME_POP_WITH(1)
}

static i32 tl_script__is_key_released(lua_State *state) {
    K_FRAME_PUSH_WITH("0x%p", state)
    // =========================================================================
    // Parameters validation
    // =========================================================================
    if (lua_gettop(state) != 1) TLSCRIPTERR("Expected a single value: isKeyReleased(KEY)")
    if (!lua_isinteger(state, 1)) TLSCRIPTERR("parameter [KEY] must be a valid key")
    // =========================================================================
    // Request execution
    // =========================================================================
    const i32 key = lua_tonumber(state, 1);
    lua_pushboolean(state, tl_input_is_key_released(key));
    // =========================================================================
    // LUA stack push
    // =========================================================================
    K_FRAME_POP_WITH(1)
}

b8 tl_script_initialize(void) {
    K_FRAME_PUSH

    KTRACE("Initializing Script Engine");

    KDEBUG("LUA_VERSION %s.%s.%s", LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    global->platform.script.state = luaL_newstate();
    if (global->platform.script.state == NULL) {
        KERROR("Failed to initialize LUA")
    }

    luaL_openlibs(global->platform.script.state);

    const luaL_Reg teleios_input[] = {
        {"isKeyPressed", tl_script__is_key_pressed},
        {"isKeyReleased", tl_script__is_key_released},
        {NULL, NULL}
    };

    luaL_newlib(global->platform.script.state, teleios_input);
    lua_setglobal(global->platform.script.state, "teleios_input");

    const luaL_Reg teleios_application[] = {
        {"exit", tl_script__application_exit},
        {NULL, NULL}
    };

    luaL_newlib(global->platform.script.state, teleios_application);
    lua_setglobal(global->platform.script.state, "teleios_application");

    K_FRAME_POP_WITH(true)
}

b8 tl_script_terminate(void) {
    K_FRAME_PUSH
    KTRACE("Terminating Script Engine");
    lua_close(global->platform.script.state);
    global->platform.script.state = NULL;
    K_FRAME_POP_WITH(true)
}