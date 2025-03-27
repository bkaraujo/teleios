#include "teleios/core.h"
#include "teleios/globals.h"

b8 tl_script_initialize(void) {
    TLSTACKPUSH

    TLDEBUG("LUA_VERSION %s.%s.%s", LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    global->platform.script.state = luaL_newstate();
    if (global->platform.script.state == NULL) {
        TLERROR("Failed to initialize LUA")
    }

    TLSTACKPOPV(TRUE)
}

b8 tl_script_terminate(void) {
    TLSTACKPUSH
    lua_close(global->platform.script.state);
    global->platform.script.state = NULL;
    TLSTACKPOPV(TRUE)
}