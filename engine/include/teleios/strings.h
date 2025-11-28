#ifndef __TELEIOS_STRINGS__
#define __TELEIOS_STRINGS__

#include "teleios/defines.h"

// ============================================================================
// String Creation and Destruction
// ============================================================================

/**
 * @brief Create a new string from a C string literal
 * @param allocator Memory allocator to use
 * @param cstr C string to wrap (will be copied)
 * @return Newly allocated TLString (caller must free with tl_string_destroy)
 */
TLString* tl_string_create(TLAllocator* allocator, const char* cstr);

/**
 * @brief Create an empty string
 * @param allocator Memory allocator to use
 * @return Newly allocated empty TLString (caller must free with tl_string_destroy)
 */
TLString* tl_string_create_empty(TLAllocator* allocator);

/**
 * @brief Create a string with a specific capacity (pre-allocated buffer)
 * @param allocator Memory allocator to use
 * @param capacity Initial capacity in characters
 * @return Newly allocated TLString with pre-allocated buffer
 */
TLString* tl_string_reserve(TLAllocator* allocator, u32 capacity);

/**
 * @brief Destroy a string and free its memory
 * @param str The string to destroy
 */
void tl_string_destroy(TLString* str);

// ============================================================================
// String Properties
// ============================================================================

/**
 * @brief Get the length of the string
 * @param str The string
 * @return Length of the string (excluding null terminator)
 */
u32 tl_string_length(const TLString* str);

/**
 * @brief Get character at specified index
 * @param str The string
 * @param index The index (0-based)
 * @return Character at index, or '\0' if index out of bounds
 */
char tl_string_char_at(const TLString* str, u32 index);

/**
 * @brief Get the underlying C string
 * @param str The string
 * @return Pointer to null-terminated C string (do not modify or free)
 */
const char* tl_string_cstr(const TLString* str);

/**
 * @brief Check if string is empty (length 0)
 * @param str The string to check
 * @return TL_TRUE if string is NULL or empty, TL_FALSE otherwise
 */
b8 tl_string_is_empty(const TLString* str);

// ============================================================================
// String Comparison
// ============================================================================

/**
 * @brief Compare two strings for equality
 * @param str1 First string
 * @param str2 Second string
 * @return TL_TRUE if strings are equal, TL_FALSE otherwise
 */
b8 tl_string_equals(const TLString* str1, const TLString* str2);

/**
 * @brief Compare two strings for equality (case-insensitive)
 * @param str1 First string
 * @param str2 Second string
 * @return TL_TRUE if strings are equal ignoring case, TL_FALSE otherwise
 */
b8 tl_string_equals_ignore_case(const TLString* str1, const TLString* str2);

/**
 * @brief Compare string with C string for equality
 * @param str TLString instance
 * @param cstr C string to compare
 * @return TL_TRUE if strings are equal, TL_FALSE otherwise
 */
b8 tl_string_equals_cstr(const TLString* str, const char* cstr);

// ============================================================================
// String Search
// ============================================================================

/**
 * @brief Find first occurrence of character in string
 * @param str The string to search
 * @param ch The character to find
 * @return Index of first occurrence, or -1 if not found
 */
i32 tl_string_index_of_char(const TLString* str, char ch);

/**
 * @brief Find last occurrence of character in string
 * @param str The string to search
 * @param ch The character to find
 * @return Index of last occurrence, or -1 if not found
 */
i32 tl_string_last_index_of_char(const TLString* str, char ch);

/**
 * @brief Find first occurrence of substring in string
 * @param str The string to search
 * @param substr The substring to find
 * @return Index of first occurrence, or -1 if not found
 */
i32 tl_string_index_of(const TLString* str, const TLString* substr);

/**
 * @brief Find first occurrence of C string in string
 * @param str The string to search
 * @param cstr The C string to find
 * @return Index of first occurrence, or -1 if not found
 */
i32 tl_string_index_of_cstr(const TLString* str, const char* cstr);

/**
 * @brief Find last occurrence of substring in string
 * @param str The string to search
 * @param substr The substring to find
 * @return Index of last occurrence, or -1 if not found
 */
i32 tl_string_last_index_of(const TLString* str, const TLString* substr);

/**
 * @brief Check if string contains substring
 * @param str The string to search
 * @param substr The substring to find
 * @return TL_TRUE if substring found, TL_FALSE otherwise
 */
b8 tl_string_contains(const TLString* str, const TLString* substr);

/**
 * @brief Check if string contains C string
 * @param str The string to search
 * @param cstr The C string to find
 * @return TL_TRUE if substring found, TL_FALSE otherwise
 */
b8 tl_string_contains_cstr(const TLString* str, const char* cstr);

/**
 * @brief Check if string starts with prefix
 * @param str The string to check
 * @param prefix The prefix to test
 * @return TL_TRUE if string starts with prefix, TL_FALSE otherwise
 */
b8 tl_string_starts_with(const TLString* str, const TLString* prefix);

/**
 * @brief Check if string starts with C string prefix
 * @param str The string to check
 * @param cstr The C string prefix to test
 * @return TL_TRUE if string starts with prefix, TL_FALSE otherwise
 */
b8 tl_string_starts_with_cstr(const TLString* str, const char* cstr);

/**
 * @brief Check if string ends with suffix
 * @param str The string to check
 * @param suffix The suffix to test
 * @return TL_TRUE if string ends with suffix, TL_FALSE otherwise
 */
b8 tl_string_ends_with(const TLString* str, const TLString* suffix);

/**
 * @brief Check if string ends with C string suffix
 * @param str The string to check
 * @param cstr The C string suffix to test
 * @return TL_TRUE if string ends with suffix, TL_FALSE otherwise
 */
b8 tl_string_ends_with_cstr(const TLString* str, const char* cstr);

b8 tl_string_cstr_ends_with(const char* cstr, const TLString* str);

/**
 * @brief Count occurrences of C string in string
 * @param str The string to search
 * @param cstr The C string to count
 * @return Number of occurrences found
 */
u32 tl_string_count_of_cstr(const TLString* str, const char* cstr);

// ============================================================================
// String Transformation (returns new TLString)
// ============================================================================

/**
 * @brief Create a copy of a string
 * @param str The string to copy
 * @return Newly allocated copy (caller must free with tl_string_destroy)
 */
TLString* tl_string_copy(const TLString* str);

// ============================================================================
// C String Utilities
// ============================================================================

/**
 * @brief Extract substring from string
 * @param str The source string
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @return Newly allocated substring (caller must free with tl_string_destroy)
 */
TLString* tl_string_substring(const TLString* str, u32 start, u32 end);

/**
 * @brief Convert string to lowercase
 * @param str The string to convert
 * @return Newly allocated lowercase string (caller must free with tl_string_destroy)
 */
TLString* tl_string_to_lower(const TLString* str);

/**
 * @brief Convert string to uppercase
 * @param str The string to convert
 * @return Newly allocated uppercase string (caller must free with tl_string_destroy)
 */
TLString* tl_string_to_upper(const TLString* str);

/**
 * @brief Remove leading and trailing whitespace
 * @param str The string to trim
 * @return Newly allocated trimmed string (caller must free with tl_string_destroy)
 */
TLString* tl_string_trim(const TLString* str);

/**
 * @brief Replace all occurrences of a character with another
 * @param str The source string
 * @param old_char Character to replace
 * @param new_char Replacement character
 * @return Newly allocated string with replacements (caller must free with tl_string_destroy)
 */
TLString* tl_string_replace_char(const TLString* str, char old_char, char new_char);

/**
 * @brief Replace first occurrence of substring with another
 * @param str The source string
 * @param old_str Substring to replace
 * @param new_str Replacement substring
 * @return Newly allocated string with replacement (caller must free with tl_string_destroy)
 */
TLString* tl_string_replace_first(const TLString* str, const TLString* old_str, const TLString* new_str);

/**
 * @brief Replace all occurrences of substring with another
 * @param str The source string
 * @param old_str Substring to replace
 * @param new_str Replacement substring
 * @return Newly allocated string with replacements (caller must free with tl_string_destroy)
 */
TLString* tl_string_replace_all(const TLString* str, const TLString* old_str, const TLString* new_str);

/**
 * @brief Concatenate two strings
 * @param str1 First string
 * @param str2 Second string
 * @return Newly allocated concatenated string (caller must free with tl_string_destroy)
 */
TLString* tl_string_concat(const TLString* str1, const TLString* str2);

/**
 * @brief Concatenate string with C string
 * @param str TLString instance
 * @param cstr C string to append
 * @return Newly allocated concatenated string (caller must free with tl_string_destroy)
 */
TLString* tl_string_concat_cstr(const TLString* str, const char* cstr);

/**
 * @brief Append C string to existing string (modifies in-place)
 * @param str TLString instance to modify
 * @param cstr C string to append
 *
 * @note This function modifies the string in-place, reallocating if necessary
 * @note Use this for efficient appending when you don't need immutability
 * @note For immutable operations, use tl_string_concat_cstr instead
 */
void tl_string_append(TLString* str, const char* cstr);

/**
 * @brief Concatenate multiple strings (NULL-terminated array)
 * @param allocator Memory allocator to use
 * @param strings NULL-terminated array of TLString pointers
 * @return Newly allocated concatenated string (caller must free with tl_string_destroy)
 */
TLString* tl_string_concat_multiple(TLAllocator* allocator, const TLString** strings);

// ============================================================================
// String Splitting
// ============================================================================

/**
 * @brief Split string by delimiter character
 * @param str The string to split
 * @param delimiter Delimiter character
 * @param out_count Pointer to store number of resulting strings
 * @return Newly allocated array of TLString pointers (caller must free with tl_string_split_destroy)
 */
TLString** tl_string_split(const TLString* str, char delimiter, u32* out_count);

/**
 * @brief Free array of strings returned by tl_string_split
 * @param strings Array of TLString pointers to free
 * @param count Number of strings in array
 */
void tl_string_split_destroy(TLString** strings, u32 count);

// ============================================================================
// String Building (mutable operations)
// ============================================================================

/**
 * @brief String builder for efficient concatenation
 *
 * Use for building strings incrementally with multiple append operations.
 * More efficient than repeated concatenation.
 */
typedef struct TLStringBuilder TLStringBuilder;

/**
 * @brief Create a new string builder
 * @param allocator Memory allocator to use
 * @param initial_capacity Initial buffer capacity
 * @return Newly allocated string builder (caller must free with tl_string_builder_destroy)
 */
TLStringBuilder* tl_string_builder_create(TLAllocator* allocator, u32 capacity);

/**
 * @brief Append a TLString to the builder
 * @param builder The string builder
 * @param str String to append
 */
void tl_string_builder_append(TLStringBuilder* builder, const TLString* str);

/**
 * @brief Append a C string to the builder
 * @param builder The string builder
 * @param cstr C string to append
 */
void tl_string_builder_append_cstr(TLStringBuilder* builder, const char* cstr);

/**
 * @brief Append a character to the builder
 * @param builder The string builder
 * @param ch Character to append
 */
void tl_string_builder_append_char(TLStringBuilder* builder, char ch);

/**
 * @brief Build the final string and destroy the builder
 * @param builder The string builder to finalize
 * @return Newly allocated TLString (caller must free with tl_string_destroy)
 */
TLString* tl_string_builder_build(TLStringBuilder* builder);

/**
 * @brief Clear the builder without destroying it
 * @param builder The string builder to clear
 */
void tl_string_builder_clear(TLStringBuilder* builder);

/**
 * @brief Destroy the builder without building
 * @param builder The string builder to destroy
 */
void tl_string_builder_destroy(TLStringBuilder* builder);


#endif
