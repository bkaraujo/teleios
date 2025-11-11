/**
 * @file application.h
 * @brief Application lifecycle and main game loop management
 *
 * This module provides the application control flow, including initialization,
 * the main game loop with fixed timestep, and shutdown.
 *
 * @section game_loop Game Loop Architecture
 *
 * The application implements a fixed-timestep accumulator-based game loop:
 *
 * ```
 * Frame 1: elapsed = 33.33ms
 *   accumulator = 0 + 33.33 = 33.33
 *   accumulator >= timestep? YES
 *     update() [timestep = 16.67ms]
 *     accumulator -= 16.67 = 16.66
 *   accumulator >= timestep? YES
 *     update() [timestep = 16.67ms]
 *     accumulator -= 16.67 = 0
 *   alpha = 0 / 16.67 = 0.0 (start of next frame)
 *   render() [no interpolation needed]
 *
 * Frame 2: elapsed = 16.67ms
 *   accumulator = 0 + 16.67 = 16.67
 *   accumulator >= timestep? YES
 *     update() [timestep = 16.67ms]
 *     accumulator -= 16.67 = 0
 *   alpha = 0 / 16.67 = 0.0
 *   render()
 * ```
 *
 * **Key characteristics:**
 * - **Fixed update frequency**: 60 Hz (16.667ms per update)
 * - **Variable render frequency**: Decoupled from update
 * - **Smooth interpolation**: Alpha parameter for rendering between states
 * - **Spiral of death protection**: Max frame time capped at 250ms
 *
 * @section timestep Fixed Timestep Details
 *
 * - **Target frequency**: 60 Hz (60 updates per second)
 * - **Target timestep**: 1/60 = 16.667 milliseconds
 * - **Accumulator pattern**: Handles variable frame times
 * - **Multiple updates**: Several updates may occur per rendered frame
 * - **Skipped updates**: If lag exceeds spiral of death threshold
 *
 * @section performance_metrics Performance Tracking
 *
 * The loop tracks and logs performance metrics every second:
 * - **FPS**: Frames per second (rendered frames)
 * - **UPS**: Updates per second (fixed timesteps)
 * - **Average frame time**: In milliseconds
 *
 * Metrics are logged at INFO level every second.
 *
 * @section initialization Initialization Order
 *
 * The application must be initialized in this specific order:
 *
 * 1. tl_platform_initialize() - OS abstraction, GLFW, window, graphics thread
 * 2. tl_application_initialize() - Game-specific setup
 * 3. tl_application_run() - Main loop
 * 4. tl_application_terminate() - Game cleanup
 * 5. tl_platform_terminate() - OS cleanup
 *
 * @section shutdown Graceful Shutdown
 *
 * The application stops when:
 * - Window close event is received (user clicks close button)
 * - Game code calls tl_event_submit(TL_EVENT_WINDOW_CLOSED, NULL)
 *
 * Shutdown sequence:
 * 1. Main loop exits
 * 2. tl_application_terminate() called
 * 3. Graphics system joins worker thread
 * 4. All events processed
 * 5. Resources cleaned up
 *
 * @section usage Usage Examples
 *
 * **Basic application structure:**
 * @code
 * #include "teleios/teleios.h"
 *
 * b8 on_app_init(void) {
 *     TLINFO("Game initialization");
 *     // Initialize game resources, load scenes, etc.
 *     return true;
 * }
 *
 * b8 on_app_update(f32 delta_time) {
 *     TLTRACE("Update frame, delta=%.6f", delta_time);
 *     // Update game logic
 *     // Physics, animations, input processing
 *     return true;  // Return false to exit
 * }
 *
 * b8 on_app_render(f32 alpha) {
 *     TLTRACE("Render frame, alpha=%.6f", alpha);
 *     // Render graphics
 *     // alpha = interpolation between frames (0.0 to 1.0)
 *     return true;
 * }
 *
 * void on_app_terminate(void) {
 *     TLINFO("Game cleanup");
 *     // Clean up resources
 * }
 *
 * int main(void) {
 *     tl_logger_loglevel(TL_LOG_LEVEL_DEBUG);
 *
 *     if (!tl_platform_initialize()) {
 *         TLFATAL("Platform initialization failed");
 *     }
 *
 *     if (!tl_application_initialize()) {
 *         tl_platform_terminate();
 *         TLFATAL("Application initialization failed");
 *     }
 *
 *     if (!tl_application_run()) {
 *         TLERROR("Application run failed");
 *     }
 *
 *     tl_application_terminate();
 *     tl_platform_terminate();
 *
 *     return 0;
 * }
 * @endcode
 *
 * **Subscribing to events:**
 * @code
 * static TLEventStatus on_window_closed(const TLEvent* event) {
 *     TLINFO("Window closed - exiting");
 *     return TL_EVENT_CONSUMED;
 * }
 *
 * b8 on_app_init(void) {
 *     // Subscribe to window close
 *     tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, on_window_closed);
 *     return true;
 * }
 * @endcode
 *
 * **Handling variable timesteps:**
 * @code
 * static f32 velocity = 0.0f;
 * static f32 position = 0.0f;
 *
 * b8 on_app_update(f32 delta_time) {
 *     // delta_time is always 1/60 second (fixed)
 *     // Move 100 units per second
 *     position += 100.0f * delta_time;
 *
 *     if (position > screen_width) {
 *         position = 0.0f;  // Wrap around
 *     }
 *
 *     return true;
 * }
 *
 * b8 on_app_render(f32 alpha) {
 *     // alpha is 0.0 at start of frame, approaches 1.0 by end
 *     // Use for smooth interpolation between update states
 *     f32 display_position = position + (velocity * alpha);
 *     render_at(display_position);
 *     return true;
 * }
 * @endcode
 *
 * @note The application assumes the presence of four callback functions:
 *       on_app_init, on_app_update, on_app_render, on_app_terminate
 *
 * @note If on_app_update or on_app_render returns false, the main loop exits
 *
 * @note Performance metrics are logged every second at INFO level
 *
 * @see platform.h - Platform initialization
 * @see event.h - Event system for window/input events
 * @see graphics.h - Graphics submission
 *
 * @author TELEIOS Team
 * @version 0.1.0
 */

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
