#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/runtime/libs.h"
#include "teleios/globals.h"

#define TLSCRIPTERR(s) BKS_TRACE_POPV(luaL_error(state, s))

static i32 tl_script__application_exit(lua_State *state) {
    BKS_TRACE_PUSHA("0x%p", state)
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
    BKS_TRACE_POPV(0)
}

static i32 tl_script__is_key_pressed(lua_State *state) {
    BKS_TRACE_PUSHA("0x%p", state)
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
    BKS_TRACE_POPV(1)
}

static i32 tl_script__is_key_released(lua_State *state) {
    BKS_TRACE_PUSHA("0x%p", state)
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
    BKS_TRACE_POPV(1)
}

b8 tl_script_initialize(void) {
    BKS_TRACE_PUSH

    BKSTRACE("Initializing Script Engine");

    BKSDEBUG("LUA_VERSION %s.%s.%s", LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    global->platform.script.state = luaL_newstate();
    if (global->platform.script.state == NULL) {
        BKSERROR("Failed to initialize LUA")
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

    BKS_TRACE_POPV(true)
}

b8 tl_script_terminate(void) {
    BKS_TRACE_PUSH
    BKSTRACE("Terminating Script Engine");
    lua_close(global->platform.script.state);
    global->platform.script.state = NULL;
    BKS_TRACE_POPV(true)
}