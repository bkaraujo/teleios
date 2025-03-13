#ifndef __TELEIOS_EVENT__
#define __TELEIOS_EVENT__

#include "teleios/defines.h"
typedef b8 (*PFN_handler)(void* data);
void tl_event_subscribe(u16 event, PFN_handler handler);
void tl_event_submmit(u16 event, void* data);

#endif // __TELEIOS_EVENT__