#include <teleios/application/loader.h>

#include "teleios/globals.h"
#include "teleios/application.h"

TLGlobal *global;

int main (const int argc, const char *argv[]) {
    if (argc != 2) {
        TLERROR("argc != 2")
        TLSTACKPOPV(99)
    }

    TLGlobal local = { 0 };
    global = &local;
    TLSTACKPUSHA("%i, 0%xp", argc, argv)

    global->platform.arena = tl_memory_arena_create(TLMEBIBYTES(10));
    global->yaml = tl_string_clone(global->platform.arena, argv[1]);


    if (!tl_platform_initialize()) {
        TLERROR("Platform failed to initialize")
        if (!tl_platform_terminate()) TLFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_application_load()) {
        TLERROR("Application failed to initialize");
        if (!tl_platform_terminate()) TLFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_engine_initialize()) {
        TLERROR("Engine failed to initialize");
        if (!tl_engine_terminate  ()) TLERROR("Engine failed to terminate")
        if (!tl_platform_terminate()) TLFATAL("Platform failed to terminate")
        exit(99);
    }

    {
        // 1. Criar um novo estado Lua
        lua_State *L = luaL_newstate();
        if (L == NULL) {
            fprintf(stderr, "Erro ao criar o estado Lua\n");
            return 1;
        }

        // // 2. Carregar as bibliotecas padrão do Lua (opcional, mas geralmente útil)
        // luaL_openlibs(L);
        //
        // // 3. Executar um script Lua a partir de uma string
        // const char *script = "print('Olá do Lua!')";
        // if (luaL_dostring(L, script) != LUA_OK) {
        //     fprintf(stderr, "Erro ao executar script: %s\n", lua_tostring(L, -1));
        // }
        //
        // // 4. Executar um script Lua a partir de um arquivo
        // if (luaL_dofile(L, "meu_script.lua") != LUA_OK) {
        //     fprintf(stderr, "Erro ao executar arquivo: %s\n", lua_tostring(L, -1));
        // }

        // 5. Fechar o estado Lua
        lua_close(L);

    }

    if (!tl_engine_run      ()) TLERROR("Engine failed to execute")
    if (!tl_engine_terminate()) TLERROR("Engine failed to terminate")

    tl_memory_arena_destroy(global->application.frame.arena);
    tl_memory_arena_destroy(global->application.scene.arena);
    tl_memory_arena_destroy(global->application.arena);
    tl_memory_arena_destroy(global->platform.arena);

    if (!tl_platform_terminate()) TLFATAL("Platform failed to terminate")

#if ! defined(TELEIOS_BUILD_RELEASE)
    TLDEBUG("global->stack used: %u", global->stack_maximum);
    TLDEBUG("global->stack reserved: %u", TLARRSIZE(global->stack, TLStackFrame));
#endif

    TLSTACKPOPV(0)
}
