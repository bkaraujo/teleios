#ifndef __TELEIOS_FILESYSTEM__
#define __TELEIOS_FILESYSTEM__

#include "teleios/defines.h"

#include "teleios/strings.h"

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

const char* tl_filesystem_get_current_directory(void);

/**
 * @brief Read entire file contents into a TLString
 * 
 * @param path Path to the file to read
 * @return Pointer to TLString containing file contents, or NULL on error
 */
TLString* tl_filesystem_read(const TLString* path);

/**
 * @brief Check if a file exists
 * 
 * @param path Path to check
 * @return true if file exists and is readable, false otherwise
 */
b8 tl_filesystem_exists(const TLString* path);

/**
 * @brief Get file size
 * 
 * @param path Path to the file
 * @return File size in bytes, or 0 on error
 */
u64 tl_filesystem_size(const TLString* path);

#endif
