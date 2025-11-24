#ifndef __TELEIOS_APPLICATION__
#define __TELEIOS_APPLICATION__

#include "teleios/defines.h"

/**
 * @brief Initialize application and game
 *
 * Performs application-level initialization. This function should be called after
 * tl_platform_initialize() to set up game-specific resources.
 *
 * The application expects this function to be implemented by the game code.
 * It typically initializes:
 * - Game state and data structures
 * - Scene and level loading
 * - Resource loading (textures, meshes, etc.)
 * - Event subscriptions
 * - Audio and input setup
 *
 * This function is NOT in this header (it's implemented by the game).
 * The engine calls it through external linkage.
 *
 * @return true if initialization successful, false on failure
 *
 * @note Implemented by game code, not the engine
 * @note Called by tl_application_run() before main loop
 * @note Should only be called once
 *
 * @see tl_application_run - Calls this function before loop
 * @see tl_application_terminate - Cleanup counterpart
 *
 * @code
 * // Game implementation (not in engine)
 * b8 on_app_init(void) {
 *     TLINFO("Game initialization starting");
 *
 *     // Load resources
 *     if (!load_scenes()) {
 *         TLERROR("Failed to load scenes");
 *         return false;
 *     }
 *
 *     // Subscribe to events
 *     tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, on_window_close);
 *     tl_event_subscribe(TL_EVENT_INPUT_KEY_PRESSED, on_key_press);
 *
 *     TLINFO("Game initialization complete");
 *     return true;
 * }
 * @endcode
 */
b8 tl_application_initialize(void);

/**
 * @brief Run main application loop
 *
 * Executes the main game loop with fixed timestep (60 Hz). Calls the game's
 * update and render callbacks. Continues until window close event or callbacks return false.
 *
 * Loop sequence each frame:
 * 1. Poll GLFW events (window, input)
 * 2. Accumulate elapsed time
 * 3. Call on_app_update() for each accumulated 16.667ms
 * 4. Calculate interpolation alpha
 * 5. Call on_app_render(alpha) once per frame
 * 6. Log performance metrics every second
 *
 * @return true if loop completed normally, false on error
 *
 * @note The actual game update/render functions (on_app_update, on_app_render)
 *       must be implemented by game code
 * @note Window close event (TL_EVENT_WINDOW_CLOSED) exits the loop
 * @note Callbacks returning false also exits the loop
 * @note Performance metrics (FPS/UPS) logged to INFO level
 *
 * @see tl_application_initialize - Must be called before this
 * @see tl_application_terminate - Should be called after this
 *
 * @code
 * // Game loop is managed by engine
 * if (!tl_application_run()) {
 *     TLERROR("Application loop encountered error");
 *     return 1;
 * }
 *
 * // Loop has exited (window closed or callback returned false)
 * TLINFO("Application loop completed normally");
 * @endcode
 */
b8 tl_application_run(void);

/**
 * @brief Terminate application and cleanup
 *
 * Performs application-level cleanup. This function should be called after
 * tl_application_run() to release game-specific resources.
 *
 * The application expects this function to be implemented by the game code.
 * It typically cleans up:
 * - Scene and level data
 * - Allocated resources
 * - Graphics resources via graphics submission
 * - Audio resources
 * - Network connections
 *
 * This function is NOT in this header (it's implemented by the game).
 * The engine calls it through external linkage.
 *
 * @return true if termination successful, false on failure
 *
 * @note Implemented by game code, not the engine
 * @note Called by main() after tl_application_run()
 * @note Should only be called once (after run completes)
 * @note Should NOT call any application functions after this
 *
 * @see tl_application_initialize - Setup counterpart
 * @see tl_application_run - Called before this
 *
 * @code
 * // Game implementation (not in engine)
 * b8 tl_application_terminate(void) {
 *     TLINFO("Game cleanup starting");
 *
 *     // Unload resources
 *     unload_scenes();
 *
 *     // Free allocated memory
 *     cleanup_memory();
 *
 *     TLINFO("Game cleanup complete");
 *     return true;
 * }
 * @endcode
 */
b8 tl_application_terminate(void);

#endif
