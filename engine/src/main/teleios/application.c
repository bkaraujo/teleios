#include "teleios/globals.h"
#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/system.h"
#include "teleios/application.h"

b8 tl_application_load(void) {
    BKS_TRACE_PUSH
    // --------------------------------------------------------
    // Load the desired scene
    // --------------------------------------------------------
    if (!tl_scene_load("main")) {
        BKSERROR("Failed to load scene [main]");
        BKS_TRACE_POPV(false)
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

    BKSDEBUG("Application initialized in %llu micros", BKS_PROFILER_MICROS);
    BKS_TRACE_POPV(true)
}

static TLEventStatus tl_process_window_minimized(const TLEvent *event) {
    BKS_TRACE_PUSHA("0x%p", event)

    global->application.paused = true;

    global->application.frame.per_second = 0;
    global->application.simulation.per_second = 0;
    BKSINFO("Simulation paused")

    BKS_TRACE_POPV(TL_EVENT_NOT_CONSUMED)
}

static TLEventStatus tl_process_window_closed(const TLEvent *event) {
    BKS_TRACE_PUSHA("0x%p", event)

    global->application.running = false;

    BKS_TRACE_POPV(TL_EVENT_NOT_CONSUMED)
}

static TLEventStatus tl_process_window_restored(const TLEvent *event) {
    BKS_TRACE_PUSHA("0x%p", event)

    global->application.paused = false;
    BKSINFO("Simulation resumed")

    BKS_TRACE_POPV(TL_EVENT_NOT_CONSUMED)
}

b8 tl_application_initialize(void) {
    BKS_TRACE_PUSH

    global->application.frame.number = 0;
    global->application.frame.overflow = 0;
    global->application.frame.per_second = 0;

    global->application.simulation.number = 0;
    global->application.simulation.overflow = 0;
    global->application.simulation.per_second = 0;

    global->application.arena = tl_memory_arena_create(BKS_MEBI_BYTES(10));

    tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, tl_process_window_closed);
    tl_event_subscribe(TL_EVENT_WINDOW_RESTORED, tl_process_window_restored);
    tl_event_subscribe(TL_EVENT_WINDOW_MINIMIZED, tl_process_window_minimized);


    BKSDEBUG("Engine initialized in %llu micros", BKS_PROFILER_MICROS);
    BKS_TRACE_POPV(true)
}

b8 tl_application_run(void) {
    BKS_TRACE_PUSH
    BKSClock t1, t2;
    bks_time_clock(&t1);

    f64 accumulator = 0.0f;
    f64 lastTime = glfwGetTime();

    global->application.frame.arena = tl_memory_arena_create(BKS_MEBI_BYTES(10));

    glfwShowWindow(global->platform.window.handle);
    while (global->application.running) {
        const f64 deltaTime = glfwGetTime() - lastTime;
        lastTime += deltaTime;

        if (!global->application.paused) {
            global->application.frame.number++;
            if (global->application.frame.number == 0) {
                global->application.frame.overflow++;
                BKSWARN("global->application.frame.overflow = %u", global->application.frame.overflow)
            }

            // =========================================================
            // Simulation Pass
            // =========================================================
            accumulator += deltaTime;
            while (accumulator >= global->application.simulation.step) {
                global->application.simulation.number++;
                if (global->application.simulation.number == 0) {
                    global->application.simulation.overflow++;
                    BKSWARN("global->application.simulation.overflow = %u", global->application.simulation.overflow)
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
        bks_time_clock(&t2);
        if (t1.second != t2.second) {
            tl_platform_memory_copy(&t1, &t2, sizeof(BKSClock));
            BKSDEBUG("FPS %llu, UPS %llu", global->application.frame.per_second, global->application.simulation.per_second);
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

    if (global->application.frame.arena != NULL) {
        tl_memory_arena_destroy(global->application.frame.arena);
        global->application.frame.arena = NULL;
    }

    BKS_TRACE_POPV(true)
}

b8 tl_application_terminate(void) {
    BKS_TRACE_PUSH

    if (global->application.arena != NULL) {
        tl_memory_arena_destroy(global->application.arena);
        global->application.arena = NULL;
    }

    BKSDEBUG("Engine terminated in %llu micros", BKS_PROFILER_MICROS);
    BKS_TRACE_POPV(true)
}
