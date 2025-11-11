/**
 * @file filesystem.h
 * @brief Cross-platform filesystem utilities
 *
 * This module provides platform-independent filesystem operations. Currently
 * it provides a path separator utility for constructing cross-platform file paths.
 *
 * @section path_separators Path Separators
 *
 * Different operating systems use different characters to separate path components:
 * - **Windows**: Backslash '\\' (e.g., C:\\Users\\name\\file.txt)
 * - **Unix/Linux/macOS**: Forward slash '/' (e.g., /home/name/file.txt)
 *
 * This module provides a function to get the correct separator for the current
 * platform at runtime, enabling code portability.
 *
 * @section usage Usage Examples
 *
 * **Building cross-platform file paths:**
 * @code
 * #include "teleios/filesystem.h"
 *
 * char path[256];
 * i8 sep = tl_filesystem_path_separator();
 *
 * // Construct path: data/models/character.mesh
 * snprintf(path, sizeof(path), "data%cmodels%ccharacter.mesh", sep, sep);
 *
 * // On Windows: data\\models\\character.mesh
 * // On Unix: data/models/character.mesh
 * @endcode
 *
 * **Dynamic resource paths:**
 * @code
 * void load_scene(const char* scene_name) {
 *     i8 sep = tl_filesystem_path_separator();
 *     char path[512];
 *
 *     snprintf(path, sizeof(path), "scenes%c%s.scene", sep, scene_name);
 *     // Results in: scenes\\scene01.scene (Windows) or scenes/scene01.scene (Unix)
 *
 *     FILE* file = fopen(path, "rb");
 *     // ... load scene ...
 * }
 * @endcode
 *
 * **Directory enumeration:**
 * @code
 * void list_textures(const char* texture_dir) {
 *     i8 sep = tl_filesystem_path_separator();
 *     char pattern[256];
 *
 *     snprintf(pattern, sizeof(pattern), "%s%c*.png", texture_dir, sep);
 *     // Results in: textures\\*.png (Windows) or textures/*.png (Unix)
 *
 *     // ... use with directory listing functions ...
 * }
 * @endcode
 *
 * @note This module is minimal and may be expanded in future versions to include:
 *       - Directory creation and listing
 *       - File existence checking
 *       - Path normalization
 *       - Absolute/relative path conversion
 *
 * @see platform.h - Platform detection and initialization
 *
 * @author TELEIOS Team
 * @version 0.1.0
 */

#ifndef __TELEIOS_FILESYSTEM__
#define __TELEIOS_FILESYSTEM__

#include "teleios/defines.h"

/**
 * @brief Get the platform-specific path separator character
 *
 * Returns the character used to separate directory components in file paths
 * on the current platform.
 *
 * @return Path separator character:
 *         - '\\' (92) on Windows
 *         - '/' (47) on Unix/Linux/macOS
 *
 * @note Returns the character as i8 (signed char) but should be treated as ASCII
 * @note This is a simple utility; modern code may use alternative path APIs
 *
 * @see defines.h - Platform detection macros (TL_PLATFORM_WINDOWS, etc.)
 *
 * @code
 * // Get current platform's path separator
 * i8 sep = tl_filesystem_path_separator();
 *
 * // Use in path construction
 * char texture_path[256];
 * snprintf(texture_path, sizeof(texture_path), "assets%ctextures%cwall.png", sep, sep);
 *
 * // Results on Windows: assets\\textures\\wall.png
 * // Results on Linux:   assets/textures/wall.png
 * @endcode
 */
i8 tl_filesystem_path_separator(void);

#endif
