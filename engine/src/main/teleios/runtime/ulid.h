#ifndef __TELEIOS_RUNTIME_ULID__
#define __TELEIOS_RUNTIME_ULID__

#include "teleios/defines.h"

/* Generator configuration flags */

#define ULID_RELAXED   (1 << 0)
#define ULID_PARANOID  (1 << 1)
#define ULID_SECURE    (1 << 2)

/* Initialize a new ULID generator instance.
 *
 * The ULID_RELAXED flag allows ULIDs generated within the same
 * millisecond to be non-monotonic, e.g. the random section is generated
 * fresh each time.
 *
 * The ULID_PARANOID flag causes the generator to clear the highest bit
 * of the random field, which guarantees that overflow cannot occur.
 * Normally the chance of overflow is non-zero, but negligible. This
 * makes it zero. It doesn't make sense to use this flag in conjunction
 * with ULID_RELAX.
 *
 * The ULID_SECURE flag doesn't fall back on userspace initialization if
 * system entropy could not be gathered. You _must_ check the return
 * value if you use this flag, since it now indicates a hard error.
 *
 * Returns 0 if the generator was successfully initialized from secure
 * system entropy. Returns 1 if this failed and instead derived entropy
 * in userspace (or is uninitialized in the case of ULID_SECURE).
 */
TLUlidGenerator* tl_ulid_generator_init(KAllocator *allocator, i32 flags);

/* Generate a new ULID.
 * A zero terminating byte is written to the output buffer.
 */
TLUlid* tl_ulid_generate(KAllocator *allocator, TLUlidGenerator *);

/* Decode a text ULID to a 128-bit binary ULID.
 * Returns non-zero if input was invalid.
 */
// b8 tl_ulid_decode(u8 [16], const i8 *);

/* The ulid textual representation */
TLString* tl_ulid(KAllocator *allocator, TLUlid * ulid);

#endif // __TELEIOS_RUNTIME_ULID__
