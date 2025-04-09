#include "teleios/globals.h"
#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/system.h"
#include "teleios/application.h"

b8 tl_application_load(void) {
    K_FRAME_PUSH
    // --------------------------------------------------------
    // Load the desired scene
    // --------------------------------------------------------
    if (!tl_scene_load("main")) {
        KERROR("Failed to load scene [main]");
        K_FRAME_POP_WITH(false)
    }
    // --------------------------------------------------------
    // Apply the scene's settings
    // --------------------------------------------------------
    glClearColor(
        global->application.scene.graphics.clear_color.x,
        global->application.scene.graphics.clear_color.y,
        global->application.scene.graphics.clear_color.z,
        global->application.scene.graphics.clear_color.w
    );

    if (global->application.scene.graphics.depth_enabled) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(global->application.scene.graphics.depth_function);
    }

    if (global->application.scene.graphics.blend_enabled) {
        glEnable(GL_BLEND);
        glBlendEquation(global->application.scene.graphics.blend_equation);
        glBlendFunc(
            global->application.scene.graphics.blend_function_src,
            global->application.scene.graphics.blend_function_tgt
        );
    }
    // u8 world = tl_layer_create("world");
    // u8 gui = tl_layer_create("gui");

    KDEBUG("Application initialized in %llu micros", K_RUNTIME_PROFILER_ELAPSED);
    K_FRAME_POP_WITH(true)
}

static TLEventStatus tl_process_window_minimized(const TLEvent *event) {
    K_FRAME_PUSH_WITH("0x%p", event)

    global->application.paused = true;

    global->application.frame.per_second = 0;
    global->application.simulation.per_second = 0;
    KINFO("Simulation paused")

    K_FRAME_POP_WITH(TL_EVENT_NOT_CONSUMED)
}

static TLEventStatus tl_process_window_closed(const TLEvent *event) {
    K_FRAME_PUSH_WITH("0x%p", event)

    global->application.running = false;

    K_FRAME_POP_WITH(TL_EVENT_NOT_CONSUMED)
}

static TLEventStatus tl_process_window_restored(const TLEvent *event) {
    K_FRAME_PUSH_WITH("0x%p", event)

    global->application.paused = false;
    KINFO("Simulation resumed")

    K_FRAME_POP_WITH(TL_EVENT_NOT_CONSUMED)
}

b8 tl_application_initialize(void) {
    K_FRAME_PUSH

    global->application.frame.number = 0;
    global->application.frame.overflow = 0;
    global->application.frame.per_second = 0;

    global->application.simulation.number = 0;
    global->application.simulation.overflow = 0;
    global->application.simulation.per_second = 0;

    global->application.arena = tl_memory_arena_create(K_MEBI_BYTES(10));

    tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, tl_process_window_closed);
    tl_event_subscribe(TL_EVENT_WINDOW_RESTORED, tl_process_window_restored);
    tl_event_subscribe(TL_EVENT_WINDOW_MINIMIZED, tl_process_window_minimized);


    KDEBUG("Engine initialized in %llu micros", K_RUNTIME_PROFILER_ELAPSED);
    K_FRAME_POP_WITH(true)
}

b8 tl_application_run(void) {
    K_FRAME_PUSH
    KClock t1, t2;
    k_time_clock(&t1);

    f64 accumulator = 0.0f;
    f64 lastTime = glfwGetTime();

    global->application.frame.arena = tl_memory_arena_create(K_MEBI_BYTES(10));

    glfwShowWindow(global->platform.window.handle);
    while (global->application.running) {
        const f64 deltaTime = glfwGetTime() - lastTime;
        lastTime += deltaTime;

        if (!global->application.paused) {
            global->application.frame.number++;
            if (global->application.frame.number == 0) {
                global->application.frame.overflow++;
                KWARN("global->application.frame.overflow = %u", global->application.frame.overflow)
            }

            // =========================================================
            // Simulation Pass
            // =========================================================
            accumulator += deltaTime;
            while (accumulator >= global->application.simulation.step) {
                global->application.simulation.number++;
                if (global->application.simulation.number == 0) {
                    global->application.simulation.overflow++;
                    KWARN("global->application.simulation.overflow = %u", global->application.simulation.overflow)
                }

                global->application.simulation.per_second++;

                accumulator -= global->application.simulation.step;
            }
            // =========================================================
            // Update Pass
            // =========================================================
            global->application.frame.per_second++;
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            {
                if (luaL_dofile(global->platform.script.state, "/mnt/nvme1/Cloud/Google/Trabalho/bkraujo/teleios/sandbox/assets/scripts/environment.lua") != LUA_OK) {
                    fprintf(stderr, "Erro ao executar arquivo: %s\n", lua_tostring(global->platform.script.state, -1));
                }
            }

            // =========================================================
            // Rendering Pass
            // =========================================================
        }

        // =========================================================
        // Presentation Pass
        // =========================================================
        glfwSwapBuffers(global->platform.window.handle);

        // =========================================================
        // FPS calculation
        // =========================================================
        k_time_clock(&t2);
        if (t1.second != t2.second) {
            k_memory_copy(&t1, &t2, sizeof(KClock));
            KDEBUG("FPS %llu, UPS %llu", global->application.frame.per_second, global->application.simulation.per_second);
            global->application.frame.per_second = global->application.simulation.per_second = 0;
        }
        // =========================================================
        // Cleanup Pass
        // =========================================================
        glfwPollEvents();
        tl_input_update();

        tl_memory_arena_reset(global->application.frame.arena);
    }

    global->application.frame.per_second = 0;
    global->application.simulation.per_second = 0;
    glfwHideWindow(global->platform.window.handle);

    K_FRAME_POP_WITH(true)
}

b8 tl_application_terminate(void) {
    K_FRAME_PUSH

    if (global->application.frame.arena != NULL) {
        tl_memory_arena_destroy(global->application.frame.arena);
        global->application.frame.arena = NULL;
    }

    if (global->application.scene.arena != NULL) {
        tl_memory_arena_destroy(global->application.scene.arena);
        global->application.scene.arena = NULL;
    }

    if (global->application.arena != NULL) {
        tl_memory_arena_destroy(global->application.arena);
        global->application.arena = NULL;
    }

    KDEBUG("Engine terminated in %llu micros", K_RUNTIME_PROFILER_ELAPSED);
    K_FRAME_POP_WITH(true)
}
