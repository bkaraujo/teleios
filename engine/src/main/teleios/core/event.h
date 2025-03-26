#ifndef __TELEIOS_EVENT__
#define __TELEIOS_EVENT__

#include "teleios/defines.h"

typedef TLEventStatus (*PFN_handler)(const TLEvent*);

b8 tl_event_subscribe(u16 event, PFN_handler handler);
void tl_event_submit(u16 event, const TLEvent* data);

#endif // __TELEIOS_EVENT__