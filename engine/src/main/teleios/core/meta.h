#ifndef __TELEIOS_META__
#define __TELEIOS_META__

#include "teleios/defines.h"

void tl_meta_frame_push(const char* filename, u64 lineno, const char* function, const char* arguments, ...);
void tl_meta_frame_pop();

#define TLSTACKPUSHA(args, ...) tl_meta_frame_push(__FILE__, __LINE__, __func__, args, ##__VA_ARGS__);
#define TLSTACKPUSH tl_meta_frame_push(__FILE__, __LINE__, __func__, NULL);
#define TLSTACKPOP tl_meta_frame_pop();

#endif //__TELEIOS_META__
