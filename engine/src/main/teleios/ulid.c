#include "teleios/core.h"
#include "teleios/runtime.h"

#ifdef BKS_PLATFORM_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   pragma comment(lib, "advapi32.lib")
#elif BKS_PLATFORM_LINUX
// #   define _GNU_SOURCE
#   include <sys/time.h>
#   include <sys/syscall.h>
#elif BKS_PLATFORM_POSIX
#   define _POSIX_C_SOURCE 200112L
#   include <sys/time.h>
#else
#   error "!!!"
#endif

#include <time.h>

static int platform_entropy(void *buf, const i32 len) {
    BKS_STACK_PUSHA("0x%p, %d", buf, len)
#if _WIN32
    BOOLEAN NTAPI SystemFunction036(PVOID, ULONG);
    BKS_STACK_POPV(!SystemFunction036(buf, len))
#elif __linux__
    i32 result = syscall(SYS_getrandom, buf, len, 0);
    BKS_STACK_POPV(result != len)
#else
    int r = 0;
    FILE *f = fopen("/dev/urandom", "rb");
    if (f) {
        r = fread(buf, len, 1, f);
        fclose(f);
    }
    BKS_STACK_POPV(!r)
#endif
}

TLUlidGenerator* tl_ulid_generator_init(TLMemoryArena *arena, const i32 flags) {
    BKS_STACK_PUSHA("0x%p, %d", arena, flags)
    TLUlidGenerator* generator = tl_memory_alloc(arena, sizeof(TLUlidGenerator), TL_MEMORY_ULID);

    generator->timestamp = 0;
    generator->flags = flags;
    generator->i = generator->j = 0;
    for (i32 i = 0; i < 256; i++)
        generator->s[i] = i;

    /* RC4 is used to fill the random segment of ULIDs. It's tiny,
     * simple, perfectly sufficient for the task (assuming it's seeded
     * properly), and doesn't require fixed-width integers. It's not the
     * fastest option, but it's plenty fast for the task.
     *
     * Besides, when we're in a serious hurry in normal operation (not
     * in "relaxed" mode), we're incrementing the random field much more
     * often than generating fresh random bytes.
     */

    u8 key[256] = {0};
    if (!platform_entropy(key, 256)) {
        /* Mix entropy into the RC4 state. */
        for (i32 i = 0, j = 0; i < 256; ++i) {
            j = (j + generator->s[i] + key[i]) & 0xff;

            const i32 tmp = generator->s[i];
            generator->s[i] = generator->s[j];
            generator->s[j] = tmp;
        }

        BKS_STACK_POPV(generator)
    }

    if (!(flags & ULID_SECURE)) {
        /* Failed to read entropy from OS, so generate some. */
        u64 now, n = 0;
        const u64 start = bks_time_epoch_millis();

        do {
            struct {
                clock_t clk;
                u64 ts;
                i64 n;
                void *stackgap;
            } noise;

            noise.ts = now = bks_time_epoch_millis();
            noise.clk = clock();
            noise.stackgap = &noise;
            noise.n = (i64) n;

            const u8 *k = (u8 *)&noise;
            for (i32 i = 0, j = 0; i < 256; ++i) {
                j = (j + generator->s[i] + k[i % sizeof(noise)]) & 0xff;
                const i32 tmp = generator->s[i];
                generator->s[i] = generator->s[j];
                generator->s[j] = tmp;
            }
        } while (n++ < 1UL << 16 || now - start < 500000ULL);
        BKS_STACK_POPV(generator)
    }

    BKS_STACK_POPV(NULL)
}

void tl_ulid_encode(TLUlid* ulid, const u8 binary[16]) {
    static const i8 set[256] = {
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a
    };

    ulid->text[ 0] = set[ binary[ 0] >> 5];
    ulid->text[ 1] = set[ binary[ 0] >> 0];
    ulid->text[ 2] = set[ binary[ 1] >> 3];
    ulid->text[ 3] = set[(binary[ 1] << 2 | binary[ 2] >> 6) & 0x1f];
    ulid->text[ 4] = set[ binary[ 2] >> 1];
    ulid->text[ 5] = set[(binary[ 2] << 4 | binary[ 3] >> 4) & 0x1f];
    ulid->text[ 6] = set[(binary[ 3] << 1 | binary[ 4] >> 7) & 0x1f];
    ulid->text[ 7] = set[ binary[ 4] >> 2];
    ulid->text[ 8] = set[(binary[ 4] << 3 | binary[ 5] >> 5) & 0x1f];
    ulid->text[ 9] = set[ binary[ 5] >> 0];
    ulid->text[10] = set[ binary[ 6] >> 3];
    ulid->text[11] = set[(binary[ 6] << 2 | binary[ 7] >> 6) & 0x1f];
    ulid->text[12] = set[ binary[ 7] >> 1];
    ulid->text[13] = set[(binary[ 7] << 4 | binary[ 8] >> 4) & 0x1f];
    ulid->text[14] = set[(binary[ 8] << 1 | binary[ 9] >> 7) & 0x1f];
    ulid->text[15] = set[ binary[ 9] >> 2];
    ulid->text[16] = set[(binary[ 9] << 3 | binary[10] >> 5) & 0x1f];
    ulid->text[17] = set[ binary[10] >> 0];
    ulid->text[18] = set[ binary[11] >> 3];
    ulid->text[19] = set[(binary[11] << 2 | binary[12] >> 6) & 0x1f];
    ulid->text[20] = set[ binary[12] >> 1];
    ulid->text[21] = set[(binary[12] << 4 | binary[13] >> 4) & 0x1f];
    ulid->text[22] = set[(binary[13] << 1 | binary[14] >> 7) & 0x1f];
    ulid->text[23] = set[ binary[14] >> 2];
    ulid->text[24] = set[(binary[14] << 3 | binary[15] >> 5) & 0x1f];
    ulid->text[25] = set[ binary[15] >> 0];
    ulid->text[26] = 0;
}

// static b8 ulid_decode(u8 ulid[16], const i8 *s) {
//     BKS_STACK_PUSHA("0x%p, 0x%p", ulid, s)
//     static const i8 v[] = {
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//         0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
//         0x08, 0x09,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
//         0x11, 0x01, 0x12, 0x13, 0x01, 0x14, 0x15, 0x00,
//         0x16, 0x17, 0x18, 0x19, 0x1a,   -1, 0x1b, 0x1c,
//         0x1d, 0x1e, 0x1f,   -1,   -1,   -1,   -1,   -1,
//           -1, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
//         0x11, 0x01, 0x12, 0x13, 0x01, 0x14, 0x15, 0x00,
//         0x16, 0x17, 0x18, 0x19, 0x1a,   -1, 0x1b, 0x1c,
//         0x1d, 0x1e, 0x1f,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1
//     };
//     if (v[(i32)s[0]] > 7)
//         BKS_STACK_POPV(true)
//
//     for (i32 i = 0; i < 26; i++)
//         if (v[(i32)s[i]] == -1)
//             BKS_STACK_POPV(true)
//
//     ulid[ 0] = v[(i32)s[ 0]] << 5 | v[(i32)s[ 1]] >> 0;
//     ulid[ 1] = v[(i32)s[ 2]] << 3 | v[(i32)s[ 3]] >> 2;
//     ulid[ 2] = v[(i32)s[ 3]] << 6 | v[(i32)s[ 4]] << 1 | v[(i32)s[ 5]] >> 4;
//     ulid[ 3] = v[(i32)s[ 5]] << 4 | v[(i32)s[ 6]] >> 1;
//     ulid[ 4] = v[(i32)s[ 6]] << 7 | v[(i32)s[ 7]] << 2 | v[(i32)s[ 8]] >> 3;
//     ulid[ 5] = v[(i32)s[ 8]] << 5 | v[(i32)s[ 9]] >> 0;
//     ulid[ 6] = v[(i32)s[10]] << 3 | v[(i32)s[11]] >> 2;
//     ulid[ 7] = v[(i32)s[11]] << 6 | v[(i32)s[12]] << 1 | v[(i32)s[13]] >> 4;
//     ulid[ 8] = v[(i32)s[13]] << 4 | v[(i32)s[14]] >> 1;
//     ulid[ 9] = v[(i32)s[14]] << 7 | v[(i32)s[15]] << 2 | v[(i32)s[16]] >> 3;
//     ulid[10] = v[(i32)s[16]] << 5 | v[(i32)s[17]] >> 0;
//     ulid[11] = v[(i32)s[18]] << 3 | v[(i32)s[19]] >> 2;
//     ulid[12] = v[(i32)s[19]] << 6 | v[(i32)s[20]] << 1 | v[(i32)s[21]] >> 4;
//     ulid[13] = v[(i32)s[21]] << 4 | v[(i32)s[22]] >> 1;
//     ulid[14] = v[(i32)s[22]] << 7 | v[(i32)s[23]] << 2 | v[(i32)s[24]] >> 3;
//     ulid[15] = v[(i32)s[24]] << 5 | v[(i32)s[25]] >> 0;
//
//     BKS_STACK_POPV(false)
// }

TLUlid* tl_ulid_generate(TLMemoryArena *arena, TLUlidGenerator *generator) {
    BKS_STACK_PUSHA("0x%p, 0x%p", arena, generator)
    const u64 timestamp = bks_time_epoch_micros() / 1000;
    TLUlid* ulid = tl_memory_alloc(arena, sizeof(TLUlid), TL_MEMORY_ULID);

    if (!(generator->flags & ULID_RELAXED) && generator->timestamp == timestamp) {
        /* Chance of 80-bit overflow is so small that it's not considered. */
        for (i32 i = 15; i > 5; i--)
            if (++generator->last[i])
                break;

        tl_ulid_encode(ulid, generator->last);
        BKS_STACK_POPV(ulid)
    }

    /* Fill out timestamp */
    generator->timestamp = timestamp;
    generator->last[0] = timestamp >> 40;
    generator->last[1] = timestamp >> 32;
    generator->last[2] = timestamp >> 24;
    generator->last[3] = timestamp >> 16;
    generator->last[4] = timestamp >>  8;
    generator->last[5] = timestamp >>  0;

    /* Fill out random section */
    for (i32 k = 0; k < 10; k++) {
        generator->i = (generator->i + 1) & 0xff;
        generator->j = (generator->j + generator->s[generator->i]) & 0xff;

        const i32 tmp = generator->s[generator->i];
        generator->s[generator->i] = generator->s[generator->j];
        generator->s[generator->j] = tmp;
        generator->last[6 + k] = generator->s[(generator->s[generator->i] + generator->s[generator->j]) & 0xff];
    }

    if (generator->flags & ULID_PARANOID)
        generator->last[6] &= 0x7f;

    tl_ulid_encode(ulid, generator->last);
    BKS_STACK_POPV(ulid)
}

TLString* tl_ulid(TLMemoryArena *arena, TLUlid * ulid) {
    BKS_STACK_PUSHA("0x%p, 0x%p", arena, ulid)
    TLString* string = tl_string_clone(arena, ulid->text);
    BKS_STACK_POPV(string)
}