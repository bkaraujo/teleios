#ifndef __TELEIOS_CORE_EVENT_CONSUMER__
#define __TELEIOS_CORE_EVENT_CONSUMER__

#include "teleios/core/event/types.h"

b8 tl_event_subscribe(u16 event, PFN_handler handler);

#endif //__TELEIOS_CORE_EVENT_CONSUMER__
