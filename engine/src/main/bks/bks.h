#ifndef __BK_STANDARDS__
#define __BK_STANDARDS__

#include "bks/defines.h"

#if !defined(BKS_STACK_STRING_SIZE)
#   define BKS_STACK_STRING_SIZE        100
#endif

#if !defined(BKS_STACK_ARGUMENTS_SIZE)
#   define BKS_STACK_ARGUMENTS_SIZE     1024
#endif

#if !defined(BKS_STACK_SIZE_MAXIMUM)
#   define BKS_STACK_SIZE_MAXIMUM       10
#endif

/* Include order matters */

#include "bks/time.h"
#include "bks/trace.h"
#include "bks/logger.h"

#endif
