#include "teleios.h"
#include "thread.h"

// Include platform-specific implementations
#if defined(TL_PLATFORM_WINDOWS)
    #include "thread_windows.inl"
#elif defined(TL_PLATFORM_LINUX) || defined(TL_PLATFORM_APPLE) || defined(TL_PLATFORM_UNIX)
    #include "thread_unix.inl"
#else
    #error "Unsupported platform for threading"
#endif
