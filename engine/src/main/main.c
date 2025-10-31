#include "teleios/defines.h"
#include "teleios/logger.h"

int main(const int argc, const char* argv[]) {
    TLERROR("TLERROR")
    TLWARN("TLWARN: %s", "fa fi fo")
    TLINFO("TLINFO")
    TLDEBUG("TLDEBUG")
    TLTRACE("TLTRACE")
    TLVERBOSE("TLVERBOSE")
    TLFATAL("TLFATAL")
}