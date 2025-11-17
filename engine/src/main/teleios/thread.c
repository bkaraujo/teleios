#include "teleios/teleios.h"
#include "teleios/thread.h"

// Include platform-specific implementations
#if defined(TL_PLATFORM_WINDOWS)
    #include "teleios/thread/windows.inl"
#elif defined(TL_PLATFORM_LINUX) || defined(TL_PLATFORM_APPLE) || defined(TL_PLATFORM_UNIX)
    #include "teleios/thread/unix.inl"
#else
    #error "Unsupported platform for threading"
#endif
